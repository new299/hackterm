#ifndef NFONT
#define NFONT

#include <SDL/SDL.h>

void nfont_init();
void draw_text(SDL_Surface *screen,int x,int y,const uint16_t *text,int16_t background);

#endif
