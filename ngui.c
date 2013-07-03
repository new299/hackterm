#include <string.h>
#include <SDL.h>
#include "nunifont.h"

#include "ngui_flowbox.h"
#include "ngui_info_prompt.h"
#include "ngui_textbox.h"
#include "ngui_textlabel.h"
#include "ngui_button.h"
#include "ngui_stringselect.h"

SDL_Renderer *ngui_renderer;
  
void (*ngui_redraw_required_callback)();

void ngui_set_renderer(struct SDL_Renderer *s,void (*redraw_callback)()) {
  ngui_renderer = s;
  ngui_redraw_required_callback = (void (*)()) redraw_callback;
}

void ngui_receive_event(SDL_Event *event) {
  ngui_receiveall_info_prompt (event);
  ngui_receiveall_textlabel   (event);
  ngui_receiveall_button      (event);
  ngui_receiveall_textbox     (event);
  ngui_receiveall_stringselect(event);
}

void ngui_render() {
  ngui_renderall_info_prompt ();
  ngui_renderall_textlabel   ();
  ngui_renderall_button      ();
  ngui_renderall_textbox     ();
  ngui_renderall_stringselect();
}

void ngui_redraw_required() {

  ngui_redraw_required_callback();

}
