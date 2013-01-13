#include <string.h>
#include <SDL/SDL.h>
#include "nsdl.h"
#include "nunifont.h"
#include "ngui.h"
#include <stdbool.h>

typedef struct {
  bool valid;
  int x;
  int y;
  int x_padding;
  int y_padding;
  char text[100];
  void (*callback)(char *);
} ngui_button_data;

int ngui_buttons_size = 0;
ngui_button_data ngui_buttons[50];

void ngui_receive_event_button(SDL_Event *event, ngui_button_data *d) {
  if(event->type == SDL_MOUSEBUTTONDOWN) {
    // event->button.x;
    // event->button.y;
    int x = event->button.x;
    int y = event->button.y;
    if((x > (d->x-d->x_padding)) && (x < ((d->x)+(strlen(d->text)*8)+d->x_padding)) &&
       (y > (d->y-d->y_padding)) && (y < ((d->y)+16+d->y_padding))) {
      d->callback("press");
    }
  }
}

void ngui_render_button(ngui_button_data *d) {

  uint16_t text[100];
  for(int n=0;n<100;n++) text[n] = d->text[n];

  nsdl_rectangle_shade(ngui_screen,d->x-d->x_padding,d->y-d->y_padding,d->x+(strlen(d->text))*8+d->x_padding,d->y+16+d->y_padding,1000,10000);

  draw_unitext(ngui_screen,
              d->x,
              d->y,
              text,
              0,
              65535,0,0,0,0);
}

int ngui_add_button(int x,int y,char *text,void *callback) {

  ngui_buttons[ngui_buttons_size].valid=true;
  ngui_buttons[ngui_buttons_size].x = x;
  ngui_buttons[ngui_buttons_size].y = y;
  ngui_buttons[ngui_buttons_size].x_padding = 10;
  ngui_buttons[ngui_buttons_size].y_padding = 10;
  strcpy(ngui_buttons[ngui_buttons_size].text,text);
  ngui_buttons[ngui_buttons_size].callback = callback;

  ngui_buttons_size++;

  return ngui_buttons_size-1;
}

void ngui_delete_button(int id) {

  ngui_buttons[id].valid = false;

}

void ngui_receiveall_button(SDL_Event *event) {
  for(int n=0;n<ngui_buttons_size;n++) {
    ngui_button_data *d = &ngui_buttons[n];
    if(d->valid) {
      ngui_receive_event_button(event,d);
    }
  }
}

void ngui_renderall_button() {
  for(int n=0;n<ngui_buttons_size;n++) {
    ngui_button_data *d = &ngui_buttons[n];
    if(d->valid) {
      ngui_render_button(d);
    }
  }
}
