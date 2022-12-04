#ifndef bmp_h
#define bmp_h

#include <stdint.h>

typedef struct {
    int x, y, w, h;
} Rect;

/**
 Load given bmp file with specifyed width and height
 
 All pixel values will be written in this array in format 0xAABBGGRR
 
 where     
 
 AA - 8 bit space (currently unused),    
 
 BB - 8 bit space for blue component,   
 
 GG - 8 bit space for green component,   
 
 RR - 8 bit space for red component    
 
 - Parameter pixels: 2D array with predefined size where values of all read pixels will be stored
 
 - Returns: 0 if file was successfully loaded, 1 if the error occured
 */
int loadBmp(int width, int height, uint32_t pixels[height][width], const char *filename);

void crop(int width, int height, uint32_t pixels[height][width], Rect rect);

void rotate(int width, int height, uint32_t pixels[height][width]);

int saveBmp(int width, int height, uint32_t pixels[height][width], const char *filename);

#endif /* bmp_h */
