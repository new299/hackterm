#ifndef NGUI_STRINGSELECT_H
#define NGUI_STRINGSELECT_H

#include <SDL.h>

int ngui_add_stringselect(int x,int y,char **stringlist,void (*callback)(const char *) );

void ngui_delete_stringselect(int id);

void ngui_receiveall_stringselect(SDL_Event *event);

void ngui_renderall_stringselect();

extern SDL_Renderer *ngui_renderer;

#endif
