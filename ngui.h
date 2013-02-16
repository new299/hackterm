#ifndef NGUI
#define NGUI

#include <SDL/SDL.h>

extern SDL_Renderer *ngui_renderer;

#include "ngui_info_prompt.h"
#include "ngui_textlabel.h"

void ngui_set_renderer(SDL_Renderer *s,void *redraw_callback);
void ngui_receive_event(SDL_Event *event);
void ngui_render();

#endif
