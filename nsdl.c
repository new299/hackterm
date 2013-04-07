#include "nsdl.h"
#include <SDL/SDL.h>
#include <stdint.h>

// require initialisation function
void nsdl_init() {}

void nsdl_pointS(SDL_Surface *screen,int x,int y,uint32_t value) {

  int bpp = screen->format->BytesPerPixel;

  #ifdef OSX_BUILD
  // not sure why, but OSX isn't setting bytes per pixel correctly.
  bpp=4;
  #endif

  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return;

  *(uint32_t *) p = value;
}

uint32_t nsdl_getpointS(SDL_Surface *screen,int x,int y) {

  int bpp = screen->format->BytesPerPixel;
  uint8_t *p = (uint8_t *) screen->pixels + (y * screen->pitch) + (x * bpp);

  if((x<0)||(y<0)|| (x>=screen->w)||(y>=screen->h)) return 0;

  return (*(uint32_t *)p);
}

void nsdl_rectangle_wire(void *screen,int sx,int sy,int ex,int ey,uint8_t r,uint8_t g,uint8_t b,uint8_t a) {

  SDL_SetRenderDrawColor(screen, r, g, b,a);
  SDL_RenderDrawLine(screen,sx,sy,sx,ey);
  SDL_RenderDrawLine(screen,sx,sy,ex,sy);
  SDL_RenderDrawLine(screen,ex,ey,sx,ey);
  SDL_RenderDrawLine(screen,ex,ey,ex,sy);
}

void nsdl_lineS(void *screen,int start_x,int start_y,int end_x,int end_y,int color) {

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
    nsdl_pointS(screen,cx,cy,color);
    if((cx==end_x) && (cy==end_y)) return;
    int e2 = 2*err;
    if(e2 > (0-dy)) { err = err - dy; cx = cx + sx; }
    if(e2 < dx    ) { err = err + dx; cy = cy + sy; }
  }

}
