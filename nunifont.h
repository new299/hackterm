#ifndef NUNIFONT
#define NUNIFONT

#include <SDL/SDL.h>

void draw_unitext(SDL_Surface *screen,int x,int y,const uint16_t *text,uint32_t bg,uint32_t fg);
void draw_unitext_fancy(SDL_Surface *screen,int x,int y,const uint16_t *text, uint32_t bg,uint32_t fg, unsigned int bold, unsigned int underline, unsigned int italic, unsigned int blink, unsigned int reverse, unsigned int strike, unsigned int font);
void nunifont_blinktimer();

#endif
