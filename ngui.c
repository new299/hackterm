#include <string.h>
#include <SDL/SDL.h>
#include "nunifont.h"

SDL_Surface *ngui_screen;

void ngui_set_screen(SDL_Surface *s) {

  ngui_screen = s;

}

void ngui_render_scrollbar(int widget_id) {
}


int ngui_add_scrollbar(int x,int y,int width,int height,int docsize,int data_start,int data_end,void *callback) {
}

void ngui_receive_event(SDL_Event *event) {
  ngui_receiveall_info_prompt(event);
  ngui_receiveall_textlabel  (event);
}

void ngui_render() {
  ngui_renderall_info_prompt(ngui_screen);
  ngui_renderall_textlabel  (ngui_screen);
}
