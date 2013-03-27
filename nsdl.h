#ifndef NSDL_H
#define NSDL_H
#include <SDL/SDL.h>

void nsdl_point(void *screen,int x,int y,uint32_t value);
void nsdl_rectangle_hashed(void *screen,int sx,int sy,int ex,int ey,uint32_t value);
void nsdl_rectangle_shade(void *screen,int sx,int sy,int ex,int ey,uint32_t value_start,uint32_t value_end);
void nsdl_rectangle_wire(void *screen,int sx,int sy,int ex,int ey,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void nsdl_rectangle_softalph(void *screen,int sx,int sy,int ex,int ey,uint32_t value);
void nsdl_line(void *screen,int start_x,int start_y,int end_x,int end_y,int color);

#endif
