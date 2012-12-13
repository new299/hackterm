#ifndef NSDL_H
#define NSDL_H
#include <SDL/SDL.h>

void nsdl_point(SDL_Surface *screen,int x,int y,uint32_t value);
void nsdl_rectangle_hashed(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value);
void nsdl_rectangle_shade(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value_start,uint32_t value_end);
void nsdl_rectangle_wire(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value);
void nsdl_rectangle_softalph(SDL_Surface *screen,int sx,int sy,int ex,int ey,uint32_t value);
void nsdl_line(SDL_Surface *screen,int start_x,int start_y,int end_x,int end_y,int color);

#endif
