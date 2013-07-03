#ifndef NGUI_FLOWBOX
#define NGUI_FLOWBOX

#include <SDL.h>

void ngui_move_flowbox(char *name,int nx,int ny);

int ngui_add_flowbox(int x,int y,char *text,void *callback);

void ngui_delete_flowbox(int id);

void ngui_receiveall_flowbox(SDL_Event *event);

void ngui_renderall_flowbox();

void ngui_flowbox_run();

extern SDL_Renderer *ngui_renderer;

#endif
