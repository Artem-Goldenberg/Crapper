#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bmp.h"

#define NARGS 8
#define BADARGS do { printUsage(); return 1; } while(0)

void printUsage(void) {
    puts("Usage: bin/hw_01 crop-rotate ‹in-bmp› ‹out-bmp› ‹x› ‹y› ‹w› ‹h›");
}

/**
 Assuming number of args is correct, validates them and in case of no errors fill the `Rect` struct with valid origin and size values.
 
 `rect` will have valid information only if function returns 0, otherwise the contents of this struct is undefined
 
 - Parameter argv: Array of string arguments, its size is assumed to be `NARGS`
 
 - Parameter rect: Pointer to a `Rect` struct, which will be initialized by this function
 
 - Returns: 0 if there was no error parsing args, 1 otherwise
 */
int validateArgs(const char * argv[], Rect* rect) {
    if (strcmp(argv[1], "crop-rotate") != 0) BADARGS;
    
    const char* inputFilename = argv[2];
    const char* outputFilename = argv[3];
    
    rect->x = atoi(argv[4]);
    rect->y = atoi(argv[5]);
    rect->w = atoi(argv[6]);
    rect->h = atoi(argv[7]);
    
    return 0;
}

int main(int argc, const char * argv[]) {
    if (argc != NARGS) BADARGS;
    
    Rect rect;
    if (validateArgs(argv, &rect) != 0) return 1;
    
    return 0;
}
