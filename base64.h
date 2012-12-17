#ifndef BASE64_H
#define BASE64_H

#include <stdbool.h>
int base64_init();
unsigned char base64_bits2byte();
int base64_decode(char *input_string,int input_length,char *output_buffer,bool *failflag);

#endif
