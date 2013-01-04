#ifndef NGUI
#define NGUI

#include <SDL/SDL.h>
#include "ngui_info_prompt.h"

void ngui_receive_event(SDL_Event *event);
void ngui_render(SDL_Surface *screen);

#endif
