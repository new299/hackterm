#include "ngui_info_prompt.h"
#include "ngui_textlabel.h"
#include <string.h>
#include <SDL/SDL.h>
#include "nunifont.h"

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

void ngui_receive_event_info_prompt(SDL_Event *event, ngui_info_prompt_data *d) {
//  d->callback("127.0.0.1","user","password");
}

void ngui_render_info_prompt(ngui_info_prompt_data *d) {

  // draw data entry boxes

  // draw "OK" button
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
  
  ngui_add_textlabel((ngui_screen->w/2)-(strlen(p1)*8),
                     (ngui_screen->h/2)-(strlen(p1)*8),
                     p1);
  ngui_add_textlabel((ngui_screen->w/2)-(strlen(p2)*8),
                     (ngui_screen->h/2)-(strlen(p2)*8)+16,
                     p2);
  ngui_add_textlabel((ngui_screen->w/2)-(strlen(p3)*8),
                     (ngui_screen->h/2)-(strlen(p3)*8)+32,
                     p3);

  ngui_info_prompts_size++;
}

void ngui_receiveall_info_prompt(SDL_Event *event) {
  for(int n=0;n<ngui_info_prompts_size;n++) {
    ngui_info_prompt_data *d = &ngui_info_prompts[n];
    ngui_receive_event_info_prompt(event,d);
  }
}

void ngui_renderall_info_prompt() {
  for(int n=0;n<ngui_info_prompts_size;n++) {
    ngui_info_prompt_data *d = &ngui_info_prompts[n];
    ngui_render_info_prompt(d);
  }
}
