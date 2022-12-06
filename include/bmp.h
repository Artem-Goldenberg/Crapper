#ifndef bmp_h
#define bmp_h

#include <stdint.h>

/**
 Convinience struct to hold properties of a rectangle

 `x, y` - origin (top left corner) coordinates
 `w, h` - width and height of the rectangle
 */
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
 
 `rawHeader` pointer is needed to store initial file's header to use when saving new bmp file. To save it with the same configurations.
 Though some header properties like size, will have to be changed to represent new picture.
 
 - Warning: Do not modify contents of this struct directly, only use within functions in this header
 
 Should only be initialized via `loadBmp` function
 After no longer needed, should be destroyed by `destoryImage` function
 */
typedef struct {
    uint32_t width, height;
    Pixel *pixels;
    void *rawHeader;
} Image;


/**
 Load given bmp file. Initializes an `Image` struct with its contents
 
 In case of an error `image` argument will still be uninitialized, you should not pass it to the `destoryImage` function
 
 - Parameter image: pointer to an uninitialized `Image` struct
 - Parameter filename: name of the file to read from
 
 - Returns: 0 if file was successfully loaded, 1 if the error occured
 */
int loadBmp(Image *image, const char *filename);


/**
 Crops a rectangle in the picture
 
 Modifies the contents of the `Image` struct to match the cropped picture
 
 - Parameter image: pointer to an `Image` struct, where all imformation about the image is stored
 - Parameter rect: pointer to a `Rect` struct, holds dimensions of a rectangle to crop
 
 - Warning: Function assumes that:
 ```
 0 <= rect.x < rect.x + rect.w <= image.width
 0 <= rect.y < rect.y + rect.h <= image.height
 ```
 */
void crop(Image *image, Rect *rect);


/**
 Rotates image 90° clockwise.
 
 Modifies the contents of the `Image` struct to store rotated image
 
 - Parameter image: pointer to an `Image` struct to be processed
 
 - Returns: 0 if there was no errors, error code otherwise
 */
int rotate(Image *image);


/**
 Saves image in `.bmp` format at the given path.
 
 - Parameter image: pointer to an `Image` struct to be saved
 - Parameter filename: name of the file to save into
 
 - Returns: 0 on success, error code on failure
 */
int saveBmp(Image *image, const char *filename);


/**
 Frees the resourses of the `Image` struct
 
 All `Image` structs created by `loadBmp` function should be passed to this function.
 After the `Image` is freed, you should not pass it to any other functions except `loadBmp`,
 to initialize it again. Otherwise - bahaviour is undefined.
 
 - Parameter image: pointer to an `Image` struct to be freed
 */
void destoryImage(Image *image);

#endif /* bmp_h */
