#include <string.h>
#include <SDL/SDL.h>
#include "nunifont.h"

void ngui_render_scrollbar(int widget_id) {
}


int ngui_create_scrollbar(int x,int y,int width,int height,int docsize,int data_start,int data_end) {
}

void ngui_add_scrollbar_callback(int widget_id,void *callback) {
}

typedef struct {
  int x;
  int y;
  char p1[100];
  char p2[100];
  char p3[100];
  int   p1_opt;
  int   p2_opt;
  int   p3_opt;
  void (*callback)(char *,char *,char *);
} ngui_info_prompt_data;

int ngui_info_prompts_size = 0;

ngui_info_prompt_data ngui_info_prompts[50];

void ngui_receive_event(SDL_Event *event) {
  for(int n=0;n<ngui_info_prompts_size;n++) {
    ngui_receive_event_info_prompt(event,ngui_info_prompts[n]);
  }
}

void ngui_render(SDL_Surface *screen) {

  for(int n=0;n<ngui_info_prompts_size;n++) {
    ngui_render_info_prompt(screen,ngui_info_prompts[n]);
  }

}



void ngui_receive_event_info_prompt(SDL_Event *event, ngui_info_prompt_data *d) {
  d->callback("127.0.0.1","user","password");
}

void ngui_render_info_prompt(SDL_Surface *screen, ngui_info_prompt_data *d) {

  uint16_t p1u[100];
  uint16_t p2u[100];
  uint16_t p3u[100];

  for(int n=0;n<100;n++) p1u[n] = d->p1[n];
  for(int n=0;n<100;n++) p2u[n] = d->p2[n];
  for(int n=0;n<100;n++) p3u[n] = d->p3[n];

  draw_unitext(screen,
            (screen->w/2)-(strlen(d->p1)*8),
            (screen->h/2)-(strlen(d->p1)*8),
            p1u,
            0,
            65535,0,0,0,0);

  draw_unitext(screen,
            (screen->w/2)-(strlen(d->p2)*8),
            (screen->h/2)-(strlen(d->p2)*8)+16,
            p2u,
            0,
            65535,0,0,0,0);

  draw_unitext(screen,
            (screen->w/2)-(strlen(d->p3)*8),
            (screen->h/2)-(strlen(d->p3)*8)+32,
            p3u,
            0,
            65535,0,0,0,0);
}

void ngui_add_info_prompt(int x,int y,
                          const char *p1    ,const char *p2    ,const char *p3,
                          int         p1_opt,int         p2_opt,int         p3_opt,
                          void       *callback) {

  ngui_info_prompts[ngui_info_prompts_size].x = x;
  ngui_info_prompts[ngui_info_prompts_size].y = y;
  strcpy(ngui_info_prompts[ngui_info_prompts_size].p1,p1);
  strcpy(ngui_info_prompts[ngui_info_prompts_size].p2,p2);
  strcpy(ngui_info_prompts[ngui_info_prompts_size].p3,p3);
  ngui_info_prompts[ngui_info_prompts_size].p1_opt = p1_opt;
  ngui_info_prompts[ngui_info_prompts_size].p2_opt = p2_opt;
  ngui_info_prompts[ngui_info_prompts_size].p3_opt = p3_opt;
  ngui_info_prompts[ngui_info_prompts_size].callback = callback;

  ngui_info_prompts_size++;
}
