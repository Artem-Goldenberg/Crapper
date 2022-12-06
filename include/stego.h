#ifndef stego_h
#define stego_h

#include "bmp.h"
#include <stdio.h>

/// encodes message into image
int encode(Image *image, const char *keyFile, const char *messageFile);

/// decodes message from image
int decode(Image *image, const char *keyFile);

#endif /* stego_h */
