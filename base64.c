#include "base64.h"
#include <stdio.h>

// from RFC4648
char base64alphabet[65] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='};

int  base64lookup[256];

int base64_bits[8];
int base64_bit_pos = 0;

int base64_init() {

  for(int n=0;n<256;n++) {
    base64lookup[n] = -1;
  }

  for(int n=0;n<65;n++) {
    base64lookup[base64alphabet[n]]=n;
  }

  base64lookup[base64alphabet[64]]=0;
  base64_bit_pos=0;
}

unsigned char base64_bits2byte() {

  //printf("bits %u %u %u %u %u %u %u %u\n", base64_bits[0], base64_bits[1], base64_bits[2], base64_bits[3], base64_bits[4], base64_bits[5], base64_bits[6], base64_bits[7]);
  unsigned char byte=0;
  for(int n=0;n<8;n++) {
    if(base64_bits[n] == 1) {
      byte |= (1 << (7-n));
    }
  }
  //printf("bits2byte decode: %x\n",(unsigned char) byte);

  return byte;
}

int base64_decode(char *input_string,int input_length,char *output_buffer,bool *failflag) {

  printf("performing decode: ");
  for(int n=0;n<input_length;n++) {
    printf("%c",input_string[n]);
  }
  printf("\n");

  int output_buffer_pos = 0;

  *failflag=false;
  for(int n=0;n<input_length;n++) {
    // skip whitespace and linefeeds
    if(input_string[n] == '\n') continue;
    if(input_string[n] == '\r') continue;
    if(input_string[n] == ' ' ) continue;
    if(input_string[n] == '=' ) {
      printf("received =, base64_bit_pos is %d\n",base64_bit_pos);
      base64_bit_pos=0;
      break;
    }

    int current = base64lookup[input_string[n]];
    
    if(current == -1) {
      *failflag=true;
      base64_bit_pos=0;
      break;
    }

    for(int i=5;i>=0;i--) {
      int bit=0;
      if((current & (1 << i)) > 0) bit = 1; else bit = 0;

      //printf("current: %d input_string: %c\n",current,input_string[n]);

      base64_bits[base64_bit_pos] = bit;
      //printf("a bit: %d %d\n",i,bit);
      base64_bit_pos++;
      if(base64_bit_pos==8) {
        output_buffer[output_buffer_pos] = base64_bits2byte();
        base64_bit_pos = 0;
        output_buffer_pos++;
      }
    }
  }

  printf("decoded %d: ",output_buffer_pos);
  for(int n=0;n<output_buffer_pos;n++) {
    printf("%x,",(unsigned char) output_buffer[n]);
  }
  printf("\n");

  return output_buffer_pos;
}
