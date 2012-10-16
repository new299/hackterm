#include "nunifont.h"
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <stdbool.h>
 
bool get_widthmap(uint16_t p);

typedef struct fontchar {
  uint8_t data[32];
} fontchar;

fontchar *fontmap;
uint8_t  *widthmap;
bool     initialised=false;

void load_fonts(char *filename,fontchar **fontmap,uint8_t **widthmap);

void nfont_init() {
  load_fonts("unifont.hex",&fontmap,&widthmap);
  initialised = true;
}

uint32_t get_pixel(uint16_t c,int c_x,int c_y) {
  
  int pos  = (c_y*16) + c_x;
  int byte = pos/8;
  int bit  = pos%8;

  if(fontmap[c].data[byte] & (1 << bit)) { return 65535; }
                                    else { return 0; }
}

void draw_point(SDL_Surface *screen,int x,int y,uint32_t value) {

  int bpp = screen->format->BytesPerPixel;
  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

  #ifdef __APPLE__
  p += 1;
  #endif

  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return;

  *(uint32_t *) p = value;
}

void draw_character(SDL_Surface *screen,int x,int y,uint16_t c,uint32_t background) {

  for(size_t c_y=0;c_y<16;c_y++) {
    for(size_t c_x=0;c_x<16;c_x++) {
      int32_t value = get_pixel(c,c_x,c_y) - background;
      if(value < 0) value=0;

      if(background == -1) value = background ^ get_pixel(c,c_x,c_y); 

      draw_point(screen,x+c_x,y+c_y,value);
    }
  }
}

void draw_unitext(SDL_Surface *screen,int x,int y,const uint16_t *text,int16_t background) {

  if(!initialised) nfont_init();

  int length=0;
  for(int n=0;n<10000;n++) {if(text[n] == 0) {length=n; break;}}
  if(length < 0    ) return;
  if(length > 10000) return;

  int spacing=1;

  int c_x = x;
  int c_y = y;
  for(size_t n=0;n<length;n++) {

    if(text[n] == ' ') {
      c_x += 8 + spacing; 
    } else {
      draw_character(screen,c_x,c_y,text[n],background);
      if(get_widthmap(text[n]) != true) c_x+=16+spacing;
                                   else c_x+=8 +spacing;
    }
  }
}

void set_widthmap(uint8_t *widthmap,int p,int v) {

  int byte = p/8;
  int bit  = p%8;

  if(v == 1) { widthmap[byte] = widthmap[byte] & ~(1 << bit); }
        else { widthmap[byte] = widthmap[byte] |  (1 << bit); }

}

bool get_widthmap(uint16_t p) {
  int byte = p/8;
  int bit  = p%8;
  if((widthmap[byte] & (1 << bit)) > 0) return true;
                                   else return false;
}

int hex2dec(char h) {

  if(h == '0') return 0;
  if(h == '1') return 1;
  if(h == '2') return 2;
  if(h == '3') return 3;
  if(h == '4') return 4;
  if(h == '5') return 5;
  if(h == '6') return 6;
  if(h == '7') return 7;
  if(h == '8') return 8;
  if(h == '9') return 9;
  if(h == 'A') return 10;
  if(h == 'B') return 11;
  if(h == 'C') return 12;
  if(h == 'D') return 13;
  if(h == 'E') return 14;
  if(h == 'F') return 15;
}

//0000:AAAA00018000000180004A51EA505A51C99E0001800000018000000180005555
int load_line(char *line,fontchar *f) {

  int width=0;

  int len = strlen(line);
  if(len > 60) width = 16; else width = 8;
  int pos=0;

  // skip up to first :
  for(;line[pos] != 0;pos++) {if(line[pos] == ':') break;  }
  pos++;

  int byte=0;
  int bit =0;
  for(;line[pos+1] != 0;pos++) {
    
    int v = hex2dec(line[pos]);
    for(int n=0;n<4;n++) {

      if((v & (1<<(3-n))) == 0) {f->data[byte] = f->data[byte] & ~(1 << bit); }
                           else {f->data[byte] = f->data[byte] |  (1 << bit); }

      bit++;
      if(bit==8) {
        bit=0;
        byte++;
        if(width==8) byte++;
      }
    }
  }

  if(width == 16) return 1; else return 0;
}

void load_fonts(char *filename,fontchar **fontmap,uint8_t **widthmap) {

  *fontmap  = (fontchar *) malloc(sizeof(fontchar)*65535);
  *widthmap = (uint8_t  *) malloc(sizeof(uint8_t)*(65535/8));
  
  FILE *mfile = fopen(filename,"r");
  
  for(int n=0;!feof(mfile);n++) {
    char *line = (char *) malloc(50);
    size_t size = 50;
    getline(&line,&size,mfile);

    int width = load_line(line,&(*fontmap)[n]);
    set_widthmap(*widthmap,n,width);
    free(line);
  }
}
