#ifndef bmp_h
#define bmp_h

#include <stdint.h>

typedef struct {
    int x, y, w, h;
} Rect;

/**
 Struct to represent a pixel read from a bmp file;
 The oreder of the components is reversed because in bmp file components are storrd like 0xBBGGRR
 B - blue, G - green, R - red
 */
typedef struct {
    unsigned char b, g, r;
} Pixel;

/**
 Internal representation of bmp image contents
 
 `pixels` is a 2D array (1D with `size = height * width`) with `height` rows and `width` columns
 Each pixel is 3 bytes wide.
 
 - Warning: Do not modify contents of this struct directly, only use within functions in this header
 
 Should only be initialized via `loadBmp` function
 After no longer needed, should be destroyed by `destoryImage` function
 */
typedef struct {
    uint32_t width, height;
    Pixel *pixels;
} Image;

/**
 Load given bmp file. Initializes an `Image` struct with its contents
 
 In case of an error `image` argument will still be uninitialized, you should not pass it to the `destoryImage` function
 
 - Parameter image: pointer to an uninitialized `Image` struct
 - Parameter filename: name of the file to read from
 
 - Returns: 0 if file was successfully loaded, 1 if the error occured
 */
int loadBmp(Image *image, const char *filename);

void crop(Image *image, Rect *rect);

void rotate(Image *image);

int saveBmp(Image *image, const char *filename);

void destoryImage(Image *image);

#endif /* bmp_h */
