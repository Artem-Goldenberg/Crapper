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
    
    // bmp file format: https://en.wikipedia.org/wiki/BMP_file_format
    uint32_t realWidth = ceil(bitCount * image->width / 32.0) * 4;
    
    // read the pixels
    fseek(file, pixelsPosition, SEEK_SET);
    if (ferror(file)) return errno;
    
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
    
    // sizeof(Pixel) == 3
    image->pixels = calloc(image->width * image->height, sizeof(Pixel));
    if (!image->pixels) return errno;
    
    for (uint32_t i = 0; i < image->height; ++i) {
        fseek(file, lastRowPosition - i * realWidth, SEEK_SET);
        if (ferror(file)) return errno;
        fread(image->pixels + i * image->width, sizeof(Pixel), image->width, file);
        if (ferror(file)) return errno;
    }
    
    return 0;
}

int loadBmp(Image *image, const char *filename) {
    FILE *file = fopen(filename, "rb");
    
    int error = load(image, file);
    
    if (file) fclose(file);
    return error;
}

void destoryImage(Image *image) {
    if (image->pixels) free(image->pixels);
    image->pixels = NULL;
}
