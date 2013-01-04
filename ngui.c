#include <string.h>
#include <SDL/SDL.h>
#include "nunifont.h"

void ngui_render_scrollbar(int widget_id) {
}


int ngui_add_scrollbar(int x,int y,int width,int height,int docsize,int data_start,int data_end,void *callback) {
}

void ngui_receive_event(SDL_Event *event) {

  ngui_receiveall_info_prompt(event);

}

void ngui_render(SDL_Surface *screen) {

  ngui_renderall_info_prompt(screen);

}
