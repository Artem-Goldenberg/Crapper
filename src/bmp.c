#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

/// specified bmp file format constants
static const size_t pixelsPositionOffset = 0x0A;
static const size_t imageSizeOffset = 0x12;
static const size_t imageRawSizeOffset = 0x22;
static const size_t bitsPerPixelOffset = 0x1C;

/**
 Internal function to load image, accepts a file descriptor and process it
 
 - Returns: 0 on success, error code on error
 */
static int load(Image *image, FILE *file) {
    if (!file) return errno;

    // read the offset to pixels
    uint32_t pixelsPosition;
    fseek(file, pixelsPositionOffset, SEEK_SET);
    if (ferror(file)) return errno;
    fread(&pixelsPosition, 4, 1, file);
    if (ferror(file)) return errno;
    
    // read the size of an image
    fseek(file, imageSizeOffset, SEEK_SET);
    if (ferror(file)) return errno;
    fread(&image->width, 4, 1, file);
    if (ferror(file)) return errno;
    fread(&image->height, 4, 1, file);
    if (ferror(file)) return errno;
    if (image->height <= 0) return EFTYPE;

    // read bits per pixel
    uint16_t bitCount;
    fseek(file, bitsPerPixelOffset, SEEK_SET);
    if (ferror(file)) return errno;
    fread(&bitCount, 2, 1, file);
    if (ferror(file)) return errno;
    
    // read the pixels
    // sizeof(Pixel) == 3
    image->pixels = calloc(image->width * image->height, sizeof(Pixel));
    if (!image->pixels) return errno;
    
    // bmp file format: https://en.wikipedia.org/wiki/BMP_file_format
    uint32_t realWidth = ceil(bitCount * image->width / 32.0) * 4;
    
    // Pixels layout
    //
    // [0                          ][                           ][                           ]{}{}
    // [realWidth                  ][                           ][                           ]{}{}
    // [2*realWidth                ][                           ][                           ]{}{}
    // [                           ][                           ][                           ]{}{}
    // [                           ][                           ][                           ]{}{}
    // [                           ][                           ][                           ]{}{}
    // [                           ][                           ][                           ]{}{}
    // [                           ][                           ][                           ]{}{}
    // [                           ][                           ][                           ]{}{}
    // [(image->height-1)*realWidth][                           ][                           ]{}{}
    //
    // {} - padding, to be removed
    //
    // Because the image is flipped, start reading from the last row and advance to the first, reading only
    // useful pixels (without {}{})
    
    uint32_t lastRowPosition = pixelsPosition + (image->height - 1) * realWidth;
    
    for (uint32_t y = 0; y < image->height; ++y) {
        fseek(file, lastRowPosition - y * realWidth, SEEK_SET);
        if (ferror(file)) return errno;
        fread(image->pixels + y * image->width, sizeof(Pixel), image->width, file);
        if (ferror(file)) return errno;
    }
    
    // preserve the initial file's header to save bmp file with the same configurations
    image->rawHeader = malloc(pixelsPosition);
    if (!image->rawHeader) return errno;
    
    fseek(file, 0, SEEK_SET);
    if (ferror(file)) return errno;
    fread(image->rawHeader, pixelsPosition, 1, file);
    if (ferror(file)) return errno;
    
    return 0;
}


int loadBmp(Image *image, const char *filename) {
    FILE *file = fopen(filename, "rb");
    
    int error = load(image, file);
    
    if (file) fclose(file);
    return error;
}


void crop(Image *image, Rect *rect) {
    // Move pixels row by row by their offset to the (0, 0) position
    
    int rowSize = rect->w * sizeof(Pixel);
    for (int y = 0; y < rect->h; ++y) { // new row size in bytes
        int rowOffset = (y + rect->y) * image->width + rect->x; // start of the row in old coordinates
        int verticalOffset = y * rect->w; // start of the row in new coordinates
        memmove(image->pixels + verticalOffset, image->pixels + rowOffset, rowSize);
    }
    
    image->width = rect->w;
    image->height = rect->h;
}


int rotate(Image *image) {
    // Rotated image will take exactly the same space as the original one
    Pixel *buffer = calloc(image->width * image->height, sizeof(Pixel));
    if (!buffer) return errno;
    
    // iterate over all pixels, map indices to the new location and copy elements
    for (int i = 0; i < image->width * image->height; ++i) {
        // i = w * y + x
        int oldX = i % image->width;
        int oldY = i / image->width;
        // for 90?? rotate
        // newX = h - 1 - y
        // newY = x
        int newX = image->height - 1 - oldY;
        int newY = oldX;
        // rotata is similare to transposition, so
        // index in new array = h * newY + newX
        int j = image->height * newY + newX;
        
        buffer[j] = image->pixels[i];
    }
    
    free(image->pixels);
    image->pixels = buffer;

    uint32_t temp = image->width;
    image->width = image->height;
    image->height = temp;
    
    return 0;
}


/**
 Internal function to save image, accepts a file descriptor and process it
 
 - Returns: 0 on success, error code on error
 */
static int save(const Image *image, FILE *file) {
    if (!file) return errno;
    
    // write new image's width and height to the header
    memcpy(image->rawHeader + imageSizeOffset, image, 4 * 2);
    
    uint32_t rowPadding = (4 - image->width * sizeof(Pixel) % 4) % 4;
    uint32_t rawSize = image->height * (image->width * sizeof(Pixel) + rowPadding);
    
    
    // write new image's size including padding to the header
    memcpy(image->rawHeader + imageRawSizeOffset, &rawSize, 4);
    
    // read the offset to pixels storage from the header again
    // it also is the size of the header, pretty handy ha
    uint32_t pixelsPosition = *(uint32_t*)(image->rawHeader + pixelsPositionOffset);
    
    // write header to the file
    fwrite(image->rawHeader, pixelsPosition, 1, file);
    if (ferror(file)) return errno;
    
    // buffer to provide to `fwrite` function for padding,
    // padding length can't be greater than 3 but store 4 bytes just for safety
    char nullBytes[4] = {0};
    
    for (int y = image->height - 1; y >= 0; --y) {
        fwrite(image->pixels + y * image->width, sizeof(Pixel), image->width, file);
        if (ferror(file)) return errno;
        fwrite(nullBytes, 1, rowPadding, file);
        if (ferror(file)) return errno;
    }
    
    return 0;
}


int saveBmp(const Image *image, const char *filename) {
    FILE *file = fopen(filename, "wb");
    
    int error = save(image, file);
    
    if (file) fclose(file);
    return error;
}


void destoryImage(Image *image) {
    if (image->pixels) free(image->pixels);
    if (image->rawHeader) free(image->rawHeader);
    image->pixels = NULL;
    image->rawHeader = NULL;
}
