#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

/// specified bmp file format constants
static const size_t pixelsPositionOffset = 0x0A;
static const size_t imageSizeOffset = 0x12;
static const size_t bitsPerPixelOffset = 0x1C;

/**
 Internal function, accepts an open file descriptor and process it
 
 - Returns: 0 on success, -1 value on error
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
    
    for (uint32_t i = 0; i < image->height; ++i) {
        fseek(file, lastRowPosition - i * realWidth, SEEK_SET);
        if (ferror(file)) return errno;
        fread(image->pixels + i * image->width, sizeof(Pixel), image->width, file);
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
    
    int rowSize = rect->w * sizeof(Pixel); // new row size in bytes
    for (int y = rect->y; y < rect->h; ++y) {
        int rowOffset = y * image->width + rect->x; // start of the row in old coordinates
        int verticalOffset = (y - rect->y) * rect->w; // start of the row in new coordinates
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
        // for 90˚ rotate
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


void destoryImage(Image *image) {
    if (image->pixels) free(image->pixels);
    if (image->rawHeader) free(image->rawHeader);
    image->pixels = NULL;
    image->rawHeader = NULL;
}
