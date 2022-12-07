#ifndef stego_h
#define stego_h

#include "bmp.h"
#include <stdio.h>

/// Messages longer than this aren't allowed
#define STEGO_MESSAGE_MAX_LENGTH 1000

/// encodes message into image
int encode(Image *image, const char *keyFile, const char *messageFile);

/// decodes message from image and writes it to the file
int decode(const Image *image, const char *keyFile, const char *filename);

#endif /* stego_h */
