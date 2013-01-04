#include <string.h>
#include <SDL/SDL.h>
#include "nunifont.h"

SDL_Surface *ngui_screen;
  
void (*ngui_redraw_required_callback)();

void ngui_set_screen(SDL_Surface *s,void *redraw_callback) {
  ngui_screen = s;
  ngui_redraw_required_callback = redraw_callback;
}

void ngui_receive_event(SDL_Event *event) {
  ngui_receiveall_info_prompt(event);
  ngui_receiveall_textlabel  (event);
  ngui_receiveall_button     (event);
  ngui_receiveall_textbox    (event);
}

void ngui_render() {
  ngui_renderall_info_prompt();
  ngui_renderall_textlabel  ();
  ngui_renderall_button     ();
  ngui_renderall_textbox    ();
}

void ngui_redraw_required() {

  ngui_redraw_required_callback();

}
