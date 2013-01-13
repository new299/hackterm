#ifndef NGUI_TEXTLABEL
#define NGUI_TEXTLABEL

#include <SDL/SDL.h>

int ngui_add_textlabel(int x,int y,char *text);

void ngui_delete_textlabel(int id);

void ngui_receiveall_textlabel(SDL_Event *event);

void ngui_renderall_textlabel();

extern SDL_Surface *ngui_screen;

#endif
