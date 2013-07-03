#include <string.h>
#include <SDL.h>
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
  int shine;
  char **stringlist;
  void (*callback)(const char *);
} ngui_stringselect_data;

int ngui_stringselects_size = 0;
ngui_stringselect_data ngui_stringselects[50];

void ngui_receive_event_stringselect(SDL_Event *event, ngui_stringselect_data *d) {

  if(event->type == SDL_MOUSEBUTTONDOWN) {

    int x = event->button.x;
    int y = event->button.y;
    
    for(size_t n=0;d->stringlist[n] != 0;n++) {

      int cyoff = n*(32+d->y_padding);

      if((x > (d->x-d->x_padding)) && (x < ((d->x)+(strlen(d->stringlist[n])*8)+d->x_padding)) &&
         (y > (d->y+cyoff-d->y_padding)) && (y < ((d->y+cyoff)+16+d->y_padding))) {
        d->callback(d->stringlist[n]);
        d->shine=20;
        ngui_redraw_required();
      }
    }
  }
}


void ngui_render_stringselect(ngui_stringselect_data *d) {

  
  for(size_t n=0;d->stringlist[n] != 0;n++) {

    int cyoff = n*(32+d->y_padding);
    //bounding box
    SDL_Rect rect;
  
    rect.x = d->x-d->x_padding;
    rect.y = d->y-d->y_padding+cyoff;
    rect.w = (strlen(d->stringlist[n]))*8+(d->x_padding*2);
    rect.h = 16+d->y_padding;
    SDL_SetRenderDrawColor(ngui_renderer,0xff,0xff,0xff,0xff);
    SDL_RenderDrawRect(ngui_renderer,&rect);

    char *text = d->stringlist[n];

    uint16_t utext[1000];
    for(int i=0;text[i]!=0;i++) {
      utext[i] = text[i];
      utext[i+1] = 0;
    }

    draw_unitext_renderer(ngui_renderer,
                d->x,
                d->y+cyoff,
                utext,
                0,
                65535,0,0,0,0);
  }
}

int ngui_add_stringselect(int x,int y,char **stringlist,void (*callback)(const char *) ) {

  ngui_stringselects[ngui_stringselects_size].valid=true;
  ngui_stringselects[ngui_stringselects_size].x = x;
  ngui_stringselects[ngui_stringselects_size].y = y;
  ngui_stringselects[ngui_stringselects_size].x_padding = 20;
  ngui_stringselects[ngui_stringselects_size].y_padding = 20;
  ngui_stringselects[ngui_stringselects_size].shine = 0;
  ngui_stringselects[ngui_stringselects_size].stringlist = stringlist;
  ngui_stringselects[ngui_stringselects_size].callback = (void(*)(const char *)) callback;

  ngui_stringselects_size++;

  return ngui_stringselects_size-1;
}

void ngui_delete_stringselect(int id) {

  ngui_stringselects[id].valid = false;

}

void ngui_receiveall_stringselect(SDL_Event *event) {
  for(int n=0;n<ngui_stringselects_size;n++) {
    ngui_stringselect_data *d = &ngui_stringselects[n];
    if(d->valid) {
      ngui_receive_event_stringselect(event,d);
    }
  }
}

void ngui_renderall_stringselect() {
  for(int n=0;n<ngui_stringselects_size;n++) {
    ngui_stringselect_data *d = &ngui_stringselects[n];
    if(d->valid) {
      ngui_render_stringselect(d);
    }
  }
}
