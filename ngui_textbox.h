#ifndef NGUI_TEXTBOX
#define NGUI_TEXTBOX

#include <stdbool.h>
#include <SDL.h>

int ngui_add_textbox(int x,int y,char *text,bool passwordbox,void *callback);

void ngui_delete_textbox(int id);

void ngui_receiveall_textbox(SDL_Event *event);

void ngui_renderall_textbox();
                                
char *ngui_textbox_get_value(int tb);

extern SDL_Surface *ngui_screen;

#endif
