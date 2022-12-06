#include "stego.h"

#include <errno.h>

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
    [0b00000000] = ' ',
    [0b00011100] = ',',
    [0b00011110] = '.'
};


int encode(Image *image, const char *keyFile, const char *messageFile) {
    FILE *key = fopen(keyFile, "r");
    if (!key) return errno;
    FILE *message = fopen(messageFile, "r");
    if (!message) return errno;
    
    char letter;
    uint8_t code;
    
    while ((letter = fgetc(message)) != EOF) {
        if (letter == ',') code = 0b00011100;
        else if (letter == '.') code = 0b00011110;
        else code = letter & 0b00011111;
        
        printf("letter: %c, got the code: %d, for letter: %c\n", letter, code, table[code]);
    }
    
    return 0;
}
