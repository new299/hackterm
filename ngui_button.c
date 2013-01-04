#include <string.h>
#include <SDL/SDL.h>
#include "nsdl.h"
#include "nunifont.h"
#include "ngui.h"

typedef struct {
  int x;
  int y;
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
    if((x > (d->x-10)) && (x < ((d->x)+(strlen(d->text)*8)+10)) &&
       (y > (d->y-10)) && (y < ((d->y)+16+10))) {
      d->callback("press");
    }
  }
}

void ngui_render_button(ngui_button_data *d) {

  uint16_t text[100];
  for(int n=0;n<100;n++) text[n] = d->text[n];

  nsdl_rectangle_shade(ngui_screen,d->x-10,d->y-10,d->x+(strlen(d->text))*8+10,d->y+16+10,1000,10000);

  draw_unitext(ngui_screen,
              d->x,
              d->y,
              text,
              1,
              65535,0,0,0,0);
}

void ngui_add_button(int x,int y,char *text,void *callback) {

  ngui_buttons[ngui_buttons_size].x = x;
  ngui_buttons[ngui_buttons_size].y = y;
  strcpy(ngui_buttons[ngui_buttons_size].text,text);
  ngui_buttons[ngui_buttons_size].callback = callback;

  ngui_buttons_size++;
}

void ngui_receiveall_button(SDL_Event *event) {
  for(int n=0;n<ngui_buttons_size;n++) {
    ngui_button_data *d = &ngui_buttons[n];
    ngui_receive_event_button(event,d);
  }
}

void ngui_renderall_button() {
  for(int n=0;n<ngui_buttons_size;n++) {
    ngui_button_data *d = &ngui_buttons[n];
    ngui_render_button(d);
  }
}
