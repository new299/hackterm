#ifndef NGUI_BUTTON
#define NGUI_BUTTON

#include <SDL/SDL.h>

void ngui_add_button(int x,int y,char *text,void *callback);

void ngui_receiveall_button(SDL_Event *event);

void ngui_renderall_button();

extern SDL_Surface *ngui_screen;

#endif
