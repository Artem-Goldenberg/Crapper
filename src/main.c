#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "stego.h"

#define NARGS 8
#define BADARGS do { printUsage(); return 1; } while(0)

typedef struct {
    const char* input;
    const char* output;
} IOFiles;

void printUsage(void) {
    puts("Usage: bin/hw_01 crop-rotate ‹in-bmp› ‹out-bmp› ‹x› ‹y› ‹w› ‹h›");
}

/**
 Extracts arguments for the dimensions of `Rect` to crop and filenames to read from and write to.
 
 `rect` and `files` will have valid information only if function returns 0, otherwise the contents of this struct is undefined
 
 - Parameter argv: Array of string arguments, its size is assumed to be `NARGS`
 - Parameter rect: Pointer to a `Rect` struct, which will be initialized by this function
 - Parameter files: Pointer to a `IOFiles` struct, initializes by this function as well
 
 - Returns: 0 if there was no error parsing args, 1 otherwise
 */
int extractArgs(const char * argv[], Rect* rect, IOFiles* files) {
    if (strcmp(argv[1], "crop-rotate") != 0) BADARGS;
    
    // just points to the same location, no need for free in the end
    files->input = argv[2];
    files->output = argv[3];
    
    rect->x = atoi(argv[4]);
    rect->y = atoi(argv[5]);
    rect->w = atoi(argv[6]);
    rect->h = atoi(argv[7]);
    
    return 0;
}

/// Entry point of the programm
/// - Parameters:
///   - argc: Number of arguments being passed from the command line
///   - argv: Array of null terminated char buffers representing arguments
int main(int argc, const char * argv[]) {
    if (argc != NARGS) BADARGS;
    
    Rect rect;
    IOFiles filenames;
    if (extractArgs(argv, &rect, &filenames) != 0) return 1;
    
    if (rect.x < 0 || rect.y < 0 || rect.w < 0 || rect.h < 0) {
        fputs("Rectangle dimensions should be non-negative integers", stderr);
        return 1;
    }
    
    Image image;
    int error = loadBmp(&image, filenames.input);
    if (error != 0) {
        fprintf(stderr, "%s: error while loading the file: %s\n", filenames.input, strerror(error));
        return 1;
    }
    
    if (rect.x + rect.w > image.width || rect.y + rect.h > image.height) {
        fprintf(stderr,
                "Specified rectangle with origin: (%d, %d) and size: (%d, %d) "
                "is out of bounds for the image of size: (%d, %d)\n",
                rect.x, rect.y,
                rect.w, rect.h,
                image.width, image.height);
        destoryImage(&image);
        return 1;
    }
    
    crop(&image, &rect);
    
    error = rotate(&image);
    if (error != 0) {
        fprintf(stderr, "%s: error while processing the image: %s\n", filenames.input, strerror(error));
        destoryImage(&image);
        return 1;
    }
    
    error = saveBmp(&image, filenames.output);
    if (error != 0) {
        fprintf(stderr, "%s: error while saving the file: %s\n", filenames.output, strerror(error));
        destoryImage(&image);
        return 1;
    }
    
    encode(&image, "samples/key.txt", "samples/message.txt");
    
    destoryImage(&image);
    return 0;
}
