#include "stego.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

static const char table[] = {
    [0b00000001] = 'A',
    [0b00000010] = 'B',
    [0b00000011] = 'C',
    [0b00000100] = 'D',
    [0b00000101] = 'E',
    [0b00000110] = 'F',
    [0b00000111] = 'G',
    [0b00001000] = 'H',
    [0b00001001] = 'I',
    [0b00001010] = 'J',
    [0b00001011] = 'K',
    [0b00001100] = 'L',
    [0b00001101] = 'M',
    [0b00001110] = 'N',
    [0b00001111] = 'O',
    [0b00010000] = 'P',
    [0b00010001] = 'Q',
    [0b00010010] = 'R',
    [0b00010011] = 'S',
    [0b00010100] = 'T',
    [0b00010101] = 'U',
    [0b00010110] = 'V',
    [0b00010111] = 'W',
    [0b00011000] = 'X',
    [0b00011001] = 'Y',
    [0b00011010] = 'Z',
    [0b00011011] = ' ',
    [0b00011100] = ',',
    [0b00011110] = '.'
};


/// inserts ether 1 or 0 into the `image` using `key`
static int insertBit(Image *image, char **instruction, unsigned bit) {
    int x = atoi(strsep(instruction, " "));
    int y = atoi(strsep(instruction, " "));
    
    Pixel *p = image->pixels + y * image->width + x;
    unsigned char *component;
    
    switch (*instruction[0]) {
        case 'R': component = &p->r; break;
        case 'G': component = &p->g; break;
        case 'B': component = &p->b; break;
        default: return EFTYPE;
    }
    
    if (bit)
        // encode 1
        // apply | 0000 0001 = 0x01
        *component |= 0x01;
    else
        // encode 0
        // apply & 1111 1110 = 0xFE
        *component &= 0xFE;
    
    return 0;
}


int encode(Image *image, const char *keyFile, const char *messageFile) {
    FILE *key = fopen(keyFile, "r");
    if (!key) return errno;
    
    FILE *message = fopen(messageFile, "r");
    if (!message) return errno;
    
    int error = 0;
    
    char letter;
    int length = 0;
    uint8_t codes[STEGO_MESSAGE_MAX_LENGTH];
    
    // convert characters to codes, predefined codes for `,` `.` ` `
    // and code for letter is its last 5 bits
    while ((letter = fgetc(message)) != EOF && length++ < STEGO_MESSAGE_MAX_LENGTH)
        switch (letter) {
            case ' ': codes[length - 1] = 0b00011011; break;
            case ',': codes[length - 1] = 0b00011100; break;
            case '.': codes[length - 1] = 0b00011110; break;
            default: codes[length - 1] = letter & 0b00011111;
        }
    
    size_t size;
    char *instruction = NULL;
    
    // insert all first 5 bits of each byte in codes into image
    for (int i = length - 1; i >= 0; --i) {
        // shift 1 in mask from 0th to 5th bit to extract bits
        for (uint8_t mask = 1; mask < 32; mask <<= 1) {
            getline(&instruction, &size, key);
            if (ferror(key)) { error = errno; break; }
            assert(instruction);
            
            // instruction -> valid string
            // temp -> valid string
            // insertBit -> temp -> invalid string
            char *temp = instruction;
            
            // code = 0000 1100
            // mask = 0000 0100 (extract third bit)
            // code & mask = 0000 0100 = mask => third bit is one
            // code = 0000 1000
            // mask = 0000 0100
            // code & mask = 0000 0000 != mask => third bit is zero
            error = insertBit(image, &temp, (codes[i] & mask) == mask);
            
            if (error != 0) break;
        }
        if (error != 0) break;
    }
    
    free(instruction);
    fclose(key);
    fclose(message);
    return error;
}


/// extract bit from `image` according to the `instruction`
/// returns bit or -1 and sets `errno` on error
static int extractBit(const Image *image, char **instruction) {
    int x = atoi(strsep(instruction, " "));
    int y = atoi(strsep(instruction, " "));
    
    Pixel p = image->pixels[y * image->width + x];
    unsigned char component;
    
    switch (*instruction[0]) {
        case 'R': component = p.r; break;
        case 'G': component = p.g; break;
        case 'B': component = p.b; break;
        default: errno = EFTYPE; return -1;
    }
    
    // extract lowest bit
    return component & 0x01;
}


int decode(const Image *image, const char *keyFile, const char* filename) {
    FILE *key = fopen(keyFile, "r");
    if (!key) return errno;
    
    FILE *message = fopen(filename, "w");
    if (!message) return errno;
    
    int error = 0;
    long good = 0;
    
    int length = 0;
    uint8_t codes[STEGO_MESSAGE_MAX_LENGTH];
    
    size_t size;
    char *instruction = NULL;
    
    while (!feof(key) && length++ < STEGO_MESSAGE_MAX_LENGTH) {
        uint8_t code = 0;
        // 'fill' the `code` starting from the lowest bit
        for (unsigned shift = 0; shift < 5; ++shift) {
            good = getline(&instruction, &size, key);
            if (good < 0) break;
            if (ferror(key)) { error = errno; break; }
            assert(instruction);
            
            char *temp = instruction;
            int bit = extractBit(image, &temp);
            if (bit < 0) { error = errno; break; }
            
            code |= bit << shift;
        }
        if (good < 0) continue;;
        if (error != 0) break;
        
        codes[length - 1] = code;
    }
    
    for (int i = length - 1; i >= 0; --i) {
        fputc(table[codes[i]], message);
        if (ferror(message)) { error = errno; break; }
    }
        
    free(instruction);
    fclose(key);
    fclose(message);
    return error;
}
