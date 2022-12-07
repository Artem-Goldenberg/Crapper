#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "stego.h"

#define BADARGS do { printUsage(); return FAILED; } while(0);

/// This program can run in 3 different mods depending on the command line arguments
/// `FAILED` mode is used for error handling
typedef enum { TRANSFORM, INSERT, EXTRACT, FAILED } Mode;

typedef struct {
    const char* input;
    const char* output;
    const char* key;
    const char* message;
} IOFiles;

/// Number of arguments in each mode
static const int transformModeArgsCount = 8;
static const int insertModeArgsCount = 6;
static const int extractModeArgsCount = 5;

void printUsage(void) {
    puts("Usage: bin/hw_01 crop-rotate ‹in-bmp› ‹out-bmp› ‹x› ‹y› ‹w› ‹h›");
    puts("Or     bin/hw_01 insert ‹in-bmp› ‹out-bmp› ‹key-txt› ‹msg-txt›");
    puts("Or     bin/hw_01 extract ‹in-bmp› ‹key-txt› ‹msg-txt›");
}

/**
 Extracts arguments and determines the mode in which programm will be running.
 
 `rect` and `files` will have valid information only if function returns 0, otherwise the contents of this structs is undefined.
 Valied content in these structs depends on the mode returned. `rect` struct will be filled only in `TRANSFORM` mode. `IOFiles` struct
 will be containing different valid strings depending on the mode as well.
 
 - Parameter argc: Number of the command line arguments
 - Parameter argv: Array of string arguments
 - Parameter rect: Pointer to a `Rect` struct, which will be initialized by this function
 - Parameter files: Pointer to a `IOFiles` struct, initializes by this function as well
 
 - Returns: `TRANSFORM`, `INSERT` or `EXTRACT` mode in case arguments were properly parsed. `FAILED` mode if error occured.
 */
Mode extractArgs(int argc, const char * argv[], Rect* rect, IOFiles* files) {
    if (argc < 1) BADARGS;
    
    Mode mode;
    
    if (strcmp(argv[1], "crop-rotate") == 0) {
        mode = TRANSFORM;
        
        if (argc != transformModeArgsCount) BADARGS;
        
        // just points to the same location, no need for free in the end
        files->input = argv[2];
        files->output = argv[3];
        
        rect->x = atoi(argv[4]);
        rect->y = atoi(argv[5]);
        rect->w = atoi(argv[6]);
        rect->h = atoi(argv[7]);
    }
    else if (strcmp(argv[1], "insert") == 0) {
        mode = INSERT;
        
        if (argc != insertModeArgsCount) BADARGS;
        
        files->input = argv[2];
        files->output = argv[3];
        files->key = argv[4];
        files->message = argv[5];
    }
    else if (strcmp(argv[1], "extract") == 0) {
        mode = EXTRACT;
        
        if (argc != extractModeArgsCount) BADARGS;
        
        files->input = argv[2];
        files->key = argv[3];
        files->message = argv[4];
    }
    else BADARGS;
    
    return mode;
}

// Functions to run for each different mode
// They all return 0 if no error occured, 1 otherwise

/// Crops `bmp` file, rotates it and saves, handles errors as well
int transformModeHandler(Image *image, Rect *rect, IOFiles *filesnames);

/// Encodes specified message into the `image` and saves resulting `bmp` file
int insertModeHandler(Image *image, IOFiles *filesnames);

/// Decodes message from the `image` according to the provided key file.
int extractModeHandler(Image *image, IOFiles *filesnames);


/// Entry point of the programm
/// - Parameters:
///   - argc: Number of arguments being passed from the command line
///   - argv: Array of null terminated char buffers representing arguments
int main(int argc, const char * argv[]) {
    Rect rect;
    IOFiles filenames;
    Mode mode = extractArgs(argc, argv, &rect, &filenames);
    if (mode == FAILED) return 1;
    
    Image image;
    int error = loadBmp(&image, filenames.input);
    if (error != 0) {
        fprintf(stderr, "%s: error while loading the file: %s\n", filenames.input, strerror(error));
        return 1;
    }
    
    switch (mode) {
        case TRANSFORM:
            error = transformModeHandler(&image, &rect, &filenames);
            break;
        case INSERT:
            error = insertModeHandler(&image, &filenames);
            break;
        case EXTRACT:
            error = extractModeHandler(&image, &filenames);
            break;
        case FAILED:
            fputs("Unknows error", stderr);
            destoryImage(&image);
            return 1;
    }
    
    destoryImage(&image);
    return error;
}


int transformModeHandler(Image *image, Rect *rect, IOFiles *filenames) {
    if (rect->x < 0 || rect->y < 0 || rect->w < 0 || rect->h < 0) {
        fputs("Rectangle dimensions should be non-negative integers", stderr);
        return 1;
    }
    
    if (rect->x + rect->w > image->width || rect->y + rect->h > image->height) {
        fprintf(stderr,
                "Specified rectangle with origin: (%d, %d) and size: (%d, %d) "
                "is out of bounds for the image of size: (%d, %d)\n",
                rect->x, rect->y,
                rect->w, rect->h,
                image->width, image->height);
        return 1;
    }
    
    crop(image, rect);
    
    int error = rotate(image);
    if (rotate(image) != 0) {
        fprintf(stderr, "%s: error while processing the image: %s\n", filenames->input, strerror(error));
        return 1;
    }
    
    error = saveBmp(image, filenames->output);
    if (error != 0) {
        fprintf(stderr, "%s: error while saving the file: %s\n", filenames->output, strerror(error));
        return 1;
    }
    
    return 0;
}


int insertModeHandler(Image *image, IOFiles *filenames) {
    int error = encode(image, filenames->key, filenames->message);
    if (error != 0) {
        fprintf(stderr, "%s, %s, %s: error while processing files: %s\n",
                filenames->input, filenames->key, filenames->message, strerror(error));
        return 1;
    }
    
    error = saveBmp(image, filenames->output);
    if (error != 0) {
        fprintf(stderr, "%s: error while saving the file: %s\n", filenames->output, strerror(error));
        return 1;
    }
    
    return 0;
}


int extractModeHandler(Image *image, IOFiles *filenames) {
    int error = decode(image, filenames->key, filenames->message);
    if (error != 0) {
        fprintf(stderr, "%s, %s, %s: error while processing files: %s\n",
                filenames->input, filenames->key, filenames->message, strerror(error));
        return 1;
    }
    
    return 0;
}
