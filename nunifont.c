#include "nunifont.h"
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <limits.h>
#include "uthash.h"

bool get_widthmap(uint16_t p);

typedef struct fontchar {
  uint8_t data[32];
} fontchar;

fontchar *fontmap  =0;
uint8_t  *widthmap =0;
int fontmap_size   =0;
int widthmap_size  =0;

bool     initialised=false;
bool     blink_value=false;

uint32_t system_bg;

void load_fonts(char *filename,fontchar **fontmap,uint8_t **widthmap);

typedef struct {
    UT_hash_handle hh; /* makes this structure hashable */
    //int len;
    uint32_t c;
    uint32_t bg;
    uint32_t fg;
    int32_t bold;
    int32_t underline;
    int32_t italic;
    int32_t strike;
    SDL_Texture *texture;
} char_render_t;

typedef struct {
    uint32_t c;
    uint32_t bg;
    uint32_t fg;
    int32_t bold;
    int32_t underline;
    int32_t italic;
    int32_t strike;
} lookup_key_t;

int char_render_t_keylen=-1;
//SDL_Texture *display_cache[65535];
void nunitfont_initcache() {
    //unsigned keylen;
    //char_render_t *msg, *tmp, *msgs = NULL;
    //lookup_key_t *lookup_key;
    char_render_t_keylen = offsetof(char_render_t, strike) + 4 - offsetof(char_render_t, c);
}

void nunifont_init() {
  load_fonts("unifont.hex",&fontmap,&widthmap);
  nunitfont_initcache();
  initialised = true;
}

void set_system_bg(uint32_t b) {

  system_bg = b;

}

uint32_t get_pixel(uint32_t c,int c_x,int c_y) {
  
  if(c_x < 0 ) return 0;
  if(c_y < 0 ) return 0;
  if(c_x > 15) return 0;
  if(c_y > 15) return 0;


  int pos  = (c_y*16) + c_x;
  int byte = pos/8;
  int bit  = pos%8;

  if(fontmap[c].data[byte] & (1 << bit)) { return 65535; }
                                    else { return 0; }
}

void draw_point(void *s,int x,int y,uint32_t value) {

 // SDL_Rect rect;
  //rect.w = 1;
  //rect.h = 1;
 // rect.x = x;
 // rect.y = y;
    SDL_Surface *screen = (SDL_Surface *) s;
 // SDL_SetRenderDrawColor(screen, value,value,value, 255);
  //SDL_RenderFillRect(screen, &rect);
 // SDL_RenderDrawPoint(screen,x,y);
  int bpp = screen->format->BytesPerPixel;
  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

 // #ifdef __APPLE__
 // p += 1;
//  #endif

  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return;

  *(uint32_t *) p = value;
}
/*
void draw_point(void *screen,int x,int y,uint32_t value) {

////  int bpp = screen->format->BytesPerPixel;
////  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

  #ifdef __APPLE__
////  p += 1;
  #endif

////  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return;

////  *(uint32_t *) p = value;
}
*/
char_render_t *display_cache = NULL;

void draw_character_surface(SDL_Surface *screen,int x,int y,int w,uint32_t cin,uint32_t bg,uint32_t fg,int bold,int underline,int italic,int strike) {
    for(size_t c_y=y;c_y<(y+16);c_y++) {
    for(size_t c_x=x;c_x<(x+w);c_x++) {

      if((c_y==15) && (underline == 1)) {
        draw_point(screen,c_x,c_y,fg);
      } else
      if((c_y==8 ) && (strike == 1)) {
        draw_point(screen,c_x,c_y,fg);
      } else {

        int32_t value  = get_pixel(cin,c_x-x,c_y-y);
        int32_t value1 = get_pixel(cin,c_x+1-x,c_y-y);
//        int32_t value2 = get_pixel(cin,c_x-1,c_y);
//        int32_t value3 = get_pixel(cin,c_x,c_y+1);
//        int32_t value4 = get_pixel(cin,c_x,c_y-1);

        int i=0;
        if(italic==1) i=1;

        if(value > 0) {
          draw_point(screen,c_x+i,c_y,fg);
        } else {
          draw_point(screen,c_x+i,c_y,bg);
          if(bold == 1) {
            if(value1 > 0) {
              draw_point(screen,c_x+i,c_y,fg);
            }
          }
        }
      }
    }
  }
}

void draw_character(void *screen,int x,int y,int w,uint32_t cin,uint32_t bg,uint32_t fg,int bold,int underline,int italic,int strike) {

    SDL_Texture *texture;
 
    lookup_key_t chr;
    chr.c = cin;
    chr.fg = fg;
    chr.bg = bg;
    chr.bold = bold;
    chr.underline = underline;
    chr.italic = italic;
    chr.strike = strike;
    
    char_render_t *mchr=0;
    HASH_FIND( hh, display_cache, &chr, char_render_t_keylen, mchr);
    
    if(!mchr) {
      uint32_t Rmask, Gmask, Bmask, Amask;      /* masks for desired format */
   
      Rmask = 0xff000000;
      Gmask = 0x00ff0000;
      Bmask = 0x0000ff00;
      Amask = 0x000000ff;
    
      int bpp=32;                /* bits per pixel for desired format */
    
      SDL_Surface *converted = SDL_CreateRGBSurface(SDL_HWSURFACE, w, 16, bpp, Rmask, Gmask, Bmask, Amask);
    
      if(converted == NULL) {
        printf("failed %s\n",SDL_GetError());
      }
      
      draw_character_surface(converted,0,0,w,cin,bg,fg,bold,underline,italic,strike);

      texture = SDL_CreateTextureFromSurface(screen, converted);
    
    uint32_t format;
    int access;
    int ww;
    int hh;
    int v = SDL_QueryTexture(texture,
                     &format,
                     &access,
                     &ww,
                     &hh);
    printf("rgba8 %u\n",SDL_PIXELFORMAT_ABGR8888);
    printf("rgb565 %u\n",SDL_PIXELFORMAT_BGRA8888);
    printf("bgra %u\n",SDL_PIXELFORMAT_BGR24);
    printf("rgb8 %u\n",SDL_PIXELFORMAT_RGB24);
    printf("argb8 %u\n",SDL_PIXELFORMAT_BGR565);
    printf("last8 %u\n",SDL_PIXELFORMAT_RGB565);
    
        

    
      SDL_FreeSurface(converted);

      mchr = malloc(sizeof(char_render_t));
      mchr->c = cin;
      mchr->fg = fg;
      mchr->bg = bg;
      mchr->bold = bold;
      mchr->underline = underline;
      mchr->italic = italic;
      mchr->strike = strike;
      mchr->texture = texture;
        
      HASH_ADD( hh, display_cache, c, char_render_t_keylen, mchr);
    }
    
    SDL_Rect dstRect = { x, y, w, 16 };
    SDL_RenderCopy(screen, mchr->texture, NULL, &dstRect);
}

void draw_space_surface(void *screen,int x,int y,int w,uint32_t bg,uint32_t fg) {
    SDL_Rect rect;
    rect.w = w;
    rect.h = 16;
    rect.x = x;
    rect.y = y;

  for(size_t c_y=0;c_y<16;c_y++) {
    for(size_t c_x=0;c_x<w;c_x++) {
      draw_point(screen,x+c_x,y+c_y,bg);
    }
  }
}

void draw_space_renderer(void *renderer,int x,int y,int w,uint32_t bg,uint32_t fg) {
    
    SDL_Rect rect;
    rect.w = w;
    rect.h = 16;
    rect.x = x;
    rect.y = y;
    
    uint32_t  Rmask = 0xff000000;
    uint32_t  Gmask = 0x00ff0000;
    uint32_t  Bmask = 0x0000ff00;
    uint32_t  Amask = 0x000000ff;
    
//    uint32_t Rmask = 0xff000000;
//    uint32_t Gmask = 0x00ff0000;
//    uint32_t Bmask = 0x0000ff00;
//   uint32_t Amask = 0x000000ff;
    
    int r=(bg & Rmask)>>24;
    int g=(bg & Gmask)>>16;
    int b=(bg & Bmask)>>8;
    int a=(bg & Amask);
    
    SDL_SetRenderDrawColor(renderer,r,g,b,a);
    SDL_RenderFillRect(renderer, &rect);
}

//void draw_space_h(void *screen,int x,int y,int w,uint32_t bg,uint32_t fg) {
 //   SDL_Rect rect;
 //   rect.w = 8;
 //   rect.h = 16;
 //   rect.x = x;
 //   rect.y = y;
 //
 //   SDL_SetRenderDrawColor(screen, bg,bg,bg, 255);
 //   SDL_RenderFillRect(screen, &rect);
    
    //  for(size_t c_y=0;c_y<w;c_y++) {
//    for(size_t c_x=0;c_x<9;c_x++) {
//      draw_point(screen,x+c_x,y+c_y,bg);
//    }
//  }
//}

void draw_unitext_fancy_surface(void *screen,int x,int y,const uint16_t *text,
                                             uint32_t bg,uint32_t fg,
                                             unsigned int bold,
                                             unsigned int underline,
                                             unsigned int italic,
                                             unsigned int blink,
                                             unsigned int reverse,
                                             unsigned int strike,
                                             unsigned int font) {
}

void draw_unitext_fancy_renderer(void *renderer,int x,int y,const uint16_t *text,
                                             uint32_t bg,uint32_t fg,
                                             unsigned int bold,
                                             unsigned int underline,
                                             unsigned int italic,
                                             unsigned int blink,
                                             unsigned int reverse,
                                             unsigned int strike,
                                             unsigned int font) {

  if(blink == 1) {
    if(blink_value) {reverse=1;}
  }

  if(reverse == 1) {
    uint32_t c = bg;
    bg = fg;
    fg = c;
  }

  draw_unitext_renderer(renderer,x,y,text,bg,fg,bold,underline,italic,strike);
}

void draw_unitext_surface(void *screen,int x,int y,const uint16_t *text,uint32_t bg,uint32_t fg,int bold,int underline,int italic,int strike) {

  if(!initialised) nunifont_init();

  int length=0;
  for(int n=0;n<10000;n++) {if(text[n] == 0) {length=n; break;}}
  if(length < 0    ) return;
  if(length > 10000) return;

  int spacing=1;

  int c_x = x;
  int c_y = y;
  for(size_t n=0;n<length;n++) {

    if(text[n] == ' ') {
      draw_space_surface(screen,c_x,c_y,8+spacing,bg,fg);
//      draw_space_h(screen,c_x,c_y+16,spacing,bg,fg);
      c_x += 8 + spacing; 
    } else {
      int w=8;
      if(get_widthmap(text[n]) != true) w=16; else w=8;
      draw_character_surface(screen,c_x,c_y,w,text[n],bg,fg,bold,underline,italic,strike);

      //draw spacing
      draw_space_surface(screen,c_x+w,c_y,spacing,bg,fg);
//      draw_space_h(screen,c_x,c_y+16,spacing,bg,fg);
//      if(w==16) draw_space_h(screen,c_x+8,c_y+16,spacing,bg,fg);
      if(w==16) draw_space_surface(screen,c_x+w+1,c_y,spacing,bg,fg);
      if(w==8 ) c_x+=w+spacing;
      if(w==16) c_x+=w+spacing+spacing;
    }
  }
}


void draw_unitext_renderer(void *renderer,int x,int y,const uint16_t *text,uint32_t bg,uint32_t fg,int bold,int underline,int italic,int strike) {

  if(!initialised) nunifont_init();

  int length=0;
  for(int n=0;n<10000;n++) {if(text[n] == 0) {length=n; break;}}
  if(length < 0    ) return;
  if(length > 10000) return;

  int spacing=1;

  int c_x = x;
  int c_y = y;
  for(size_t n=0;n<length;n++) {

    if(text[n] == ' ') {
      int w=8;
      //draw_character(renderer,c_x,c_y,w,text[n],bg,fg,bold,underline,italic,strike);
      if(bg!=system_bg) draw_space_renderer(renderer,c_x,c_y,8+spacing,bg,fg);
 //     draw_space_h(renderer,c_x,c_y+16,spacing,bg,fg);
      c_x += 8 + spacing; 
    } else {
      int w=8;
      if(get_widthmap(text[n]) != true) w=16; else w=8;
      draw_character(renderer,c_x,c_y,w,text[n],bg,fg,bold,underline,italic,strike);

      //draw spacing - currently no spacing.
 //     draw_space_renderer(renderer,c_x+w,c_y,spacing,bg,fg);
 //     draw_space_h(renderer,c_x,c_y+16,spacing,bg,fg);
//      if(w==16) draw_space_h(renderer,c_x+8,c_y+16,spacing,bg,fg);
//      if(w==16) draw_space_renderer(renderer,c_x+w+1,c_y,spacing,bg,fg);
      if(w==8 ) c_x+=w+spacing;
      if(w==16) c_x+=w+spacing+spacing;
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

  *fontmap  = (fontchar *) malloc(sizeof(fontchar)*65536);
  *widthmap = (uint8_t  *) malloc(sizeof(uint8_t)*(65536/8));

  fontmap_size  = sizeof(fontchar)*65536;
  widthmap_size = sizeof(uint8_t)*(65536/8);
  
  for(size_t n=0;n<65536;n++) for(int i=0;i<32;i++) (*fontmap)[n].data[i] = 0;
  for(size_t n=0;n<(sizeof(uint8_t)*(65536/8));n++) (*widthmap)[n] = 0;

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

void nunifont_save_staticmap(char *fontmap_filename,char *widthmap_filename) {

  FILE *fontmap_file = fopen(fontmap_filename,"w");

  uint8_t *f = (uint8_t *) fontmap;
  for(int n=0;n<fontmap_size;n++) {
    putc(f[n],fontmap_file);
  }
  fclose(fontmap_file);

  FILE *widthmap_file = fopen(widthmap_filename,"w");

  uint8_t *w = (uint8_t *) widthmap;
  for(int n=0;n<widthmap_size;n++) {
    putc(w[n],widthmap_file);
  }
  fclose(widthmap_file);

}

void nunifont_load_staticmap(void *fontmap_static,void *widthmap_static,int fontmap_static_size,int widthmap_static_size) {

  fontmap       = (fontchar *) fontmap_static;
  widthmap      = (uint8_t  *) widthmap_static;
  fontmap_size  = fontmap_static_size;
  widthmap_size = widthmap_static_size;
  nunitfont_initcache();
  initialised = true;
}


void nunifont_blinktimer() {

  blink_value = !blink_value;

}
