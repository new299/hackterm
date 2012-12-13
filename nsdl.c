#include "nsdl.h"
#include <SDL/SDL.h>
#include <stdint.h>

void nsdl_point(SDL_Surface *screen,int x,int y,uint32_t value) {

  int bpp = screen->format->BytesPerPixel;
  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

  #ifdef __APPLE__
  p += 1;
  #endif

  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return;

  *(uint32_t *) p = value;
}

uint32_t nsdl_getpoint(SDL_Surface *screen,int x,int y) {

  int bpp = screen->format->BytesPerPixel;
  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

  #ifdef __APPLE__
  p += 1;
  #endif

  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return 0;

  return (*(uint32_t *)p);
}

void nsdl_rectangle_hashed(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value) {

  int n=0;

  for(int x=sx;x<=ex;x++) {
    for(int y=sy;y<=ey;y++) {
     
      if(n%2 == 0) {
        nsdl_point(screen,x,y,value);
      }
      n++;
    }
    n=x%2;
  }

}

void nsdl_rectangle_shade(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value_start,uint32_t value_end) {
  
  int shade=value_start;
  int shade_inc = ((value_end-value_start)/(ex-sx))/2;

  for(int x=sx;x<=ex;x++) {
    for(int y=sy;y<=ey;y++) {
      uint32_t v = nsdl_getpoint(screen,x,y);
      printf("%u %u %u\n",x,y,v);
      nsdl_point(screen,x,y,v^shade);
    }
    if(x==(sx+((ex-sx)/2))) { shade_inc = 0 - shade_inc; }
    shade += shade_inc;
  }
}

void nsdl_rectangle_softalph(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value) {
  int shade=value;

  for(int x=sx;x<ex;x++) {
    for(int y=sy;y<ey;y++) {
      uint32_t v = nsdl_getpoint(screen,x,y);
      nsdl_point(screen,x,y,v^value);
    }
  }
}

void nsdl_rectangle_wire(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value) {

  for(int x=sx;x<ex;x++) {
    nsdl_point(screen,x,sy,value);
    nsdl_point(screen,x,ey-1,value);
  }

  for(int y=sy;y<ey;y++) {
    nsdl_point(screen,sx,y,value);
    nsdl_point(screen,ex-1,y,value);
  }
}

void nsdl_line(SDL_Surface *screen,int start_x,int start_y,int end_x,int end_y,int color) {

  // Bresenham's
  int cx = start_x;
  int cy = start_y;

  int dx = end_x - cx;
  int dy = end_y - cy;
  if(dx<0) dx = 0-dx;
  if(dy<0) dy = 0-dy;

  int sx=0; int sy=0;
  if(cx < end_x) sx = 1; else sx = -1;
  if(cy < end_y) sy = 1; else sy = -1;
  int err = dx-dy;

  for(int n=0;n<1000;n++) {
    nsdl_point(screen,cx,cy,color);
    if((cx==end_x) && (cy==end_y)) return;
    int e2 = 2*err;
    if(e2 > (0-dy)) { err = err - dy; cx = cx + sx; }
    if(e2 < dx    ) { err = err + dx; cy = cy + sy; }
  }
}
