#ifndef NGUI_BUTTON
#define NGUI_BUTTON

#include <SDL/SDL.h>

void ngui_add_textbox(int x,int y,char *text,void *callback);

void ngui_receiveall_textbox(SDL_Event *event);

void ngui_renderall_textbox();

extern SDL_Surface *ngui_screen;

#endif
