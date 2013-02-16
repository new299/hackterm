#include <string.h>
#include <SDL/SDL.h>
#include "nunifont.h"
#include "ngui.h"
#include <stdbool.h>

typedef struct {
  bool valid;
  int x;
  int y;
  char text[100];
} ngui_textlabel_data;

int ngui_textlabels_size = 0;
ngui_textlabel_data ngui_textlabels[50];

void ngui_receive_event_textlabel(SDL_Event *event, ngui_textlabel_data *d) {
//  d->callback("127.0.0.1","user","password");
}

void ngui_render_textlabel(ngui_textlabel_data *d) {

  uint16_t text[100];
  for(int n=0;n<100;n++) text[n] = d->text[n];

  draw_unitext_renderer(ngui_renderer,
              d->x,
              d->y,
              text,
              1,
              65535,0,0,0,0);
}

int ngui_add_textlabel(int x,int y,char *text) {

  ngui_textlabels[ngui_textlabels_size].valid = true;
  ngui_textlabels[ngui_textlabels_size].x = x;
  ngui_textlabels[ngui_textlabels_size].y = y;
  strcpy(ngui_textlabels[ngui_textlabels_size].text,text);

  ngui_textlabels_size++;

  return ngui_textlabels_size-1;
}

void ngui_delete_textlabel(int id) {

  ngui_textlabels[id].valid = false;

}

void ngui_receiveall_textlabel(SDL_Event *event) {
  for(int n=0;n<ngui_textlabels_size;n++) {
    ngui_textlabel_data *d = &ngui_textlabels[n];
    if(d->valid == true) {
      ngui_receive_event_textlabel(event,d);
    }
  }
}

void ngui_renderall_textlabel() {
  for(int n=0;n<ngui_textlabels_size;n++) {
    ngui_textlabel_data *d = &ngui_textlabels[n];
    if(d->valid == true) {
      ngui_render_textlabel(d);
    }
  }
}
