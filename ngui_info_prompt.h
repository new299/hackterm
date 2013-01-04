#ifndef NGUI
#define NGUI

#include <SDL/SDL.h>

void ngui_add_info_prompt(int x,int y,
                          const char *p1    ,const char *p2    ,const char *p3,
                          int         p1_opt,int         p2_opt,int         p3_opt,
                          void       *callback);

void ngui_receiveall_info_prompt(SDL_Event *event);

void ngui_renderall_info_prompt(SDL_Surface *screen);

#endif
