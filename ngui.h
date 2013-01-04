#ifndef NGUI
#define NGUI

#include <SDL/SDL.h>

extern SDL_Surface *ngui_screen;

#include "ngui_info_prompt.h"
#include "ngui_textlabel.h"

void ngui_set_screen(SDL_Surface *s);
void ngui_receive_event(SDL_Event *event);
void ngui_render();

#endif
