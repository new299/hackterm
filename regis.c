#define _POSIX_C_SOURCE 199309L
#define _BSD_SOURCE

#include <string.h>
#include <stdbool.h>
#include "regis.h"
#include <SDL/SDL.h>       
#include <SDL/SDL_thread.h>
#include <time.h>
#include "nunifont.h"

int pen_x = 0;
int pen_y = 0;

SDL_Surface *regis_layer=0;

SDL_mutex *regis_mutex=0;

bool regis_is_clear=true;

bool regis_cleared() {

  if(regis_is_clear) return true; else return false;

}

void regis_clear() {
  SDL_mutexP(regis_mutex);
  SDL_FillRect(regis_layer,NULL, 0x000000);
  regis_is_clear=true;
  SDL_mutexV(regis_mutex);
}

char *regis_process_cmd_screen(char *cmd) {
  char *code = cmd+2;
  strsep(&code,")",")");
  if(code == 0) return (cmd+1);

  return code; 
}

char *regis_process_cmd_text(char *cmd) {

  char *data=0;
  if(*(cmd+1) == '\'') {
    data = cmd+2;
    strsep(&data,"\'");
    if(data == 0) return (cmd+1);

    uint16_t wdata[1000];
    char *odata = cmd+2;
    for(int n=0;(n<1000) && (odata[n] != 0);n++) {
      wdata[n] = odata[n];
      wdata[n+1] = 0;
    }
    draw_unitext_surface(regis_layer,pen_x,pen_y,wdata,0x0,0xFFFFFFFF,0,0,0,0);
  } else 
  if(*(cmd+1) == '(') {
    data=cmd+2;
    strsep(&data,")");
    if(data == 0) return (cmd+1);
  }
  if(data == 0) return (cmd+1);
  return data;
}

char *regis_process_cmd_w(char *cmd) {
  char *buffer;
  char *code = cmd+2;
  strsep(&code,")");
  if(code == 0) return (cmd+1);
  
  return code;
}

char *regis_process_cmd_position(char *cmd) {

  char *buffer;
  char *xstr = cmd+2;
  strsep(&xstr,",");
  if(xstr == 0) return (cmd+1);
  char *ystr = xstr+1;
  strsep(&ystr,"]");
  if(ystr == 0) return (cmd+1);

  int new_x = atoi(cmd+2);
  int new_y = atoi(xstr);

  pen_x = new_x;
  pen_y = new_y;

  return ystr;
}


void regis_init(int width,int height) {
  regis_layer = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
}

// This may not appear to work, because actually the screen gets cleared on rotation anyway, because the
// screen size changes, and the terminal sends a signal. For this reason, resize currently doesn't try to
// copy data.
void regis_resize(int width,int height) {

  SDL_Surface *old_regis_layer=regis_layer;
  regis_layer = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
  SDL_FreeSurface(old_regis_layer);
}

char *regis_process_cmd_vector(char *cmd) {

  // vector commands look like this: v[] or v[100,200]
  // where 100 and 200 are the x and y positions respectively.
  // a line is drawn between the current pen position and the x,y position


  if(strncmp(cmd,"v[]",3) == 0) {
    return cmd+3;
  }

  char *xstr = cmd+2;
  strsep(&xstr,",");
  if(xstr == 0) return cmd+2;

  char *ystr = xstr;
  strsep(&ystr,"]");
  if(ystr == 0) return cmd+2;
  int new_x = atoi(cmd+2);
  int new_y = atoi(xstr);

  nsdl_lineS(regis_layer,pen_x,pen_y,new_x,new_y,0xFFFFFFFF);
  pen_x = new_x;
  pen_y = new_y;

  return ystr;
}


char *regis_process_command(char *cmd) {
  regis_is_clear=false;
  if(cmd[0] == 'S') return regis_process_cmd_screen(cmd);    else
  if(cmd[0] == 'T') return regis_process_cmd_text(cmd);      else
  if(cmd[0] == 'W') return regis_process_cmd_w(cmd);         else
  if(cmd[0] == 'P') return regis_process_cmd_position(cmd);  else
  if(cmd[0] == 'v') return regis_process_cmd_vector(cmd);    else
  {
    return cmd+1;
  }
}

struct timespec regis_last_render;

void regis_processor(const char *cmd,int cmdlen) {
 
  char *command = cmd;

  for(;;) {
    command = regis_process_command(command);

    #ifdef LINUX_BUILD
    clock_gettime(CLOCK_MONOTONIC,&regis_last_render);
    #endif
    
    #if defined(IPHONE_BUILD) || defined(OSX_BUILD)
    #if _POSIX_TIMERS > 0
    clock_gettime(CLOCK_REALTIME, &tp);
    #else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    regis_last_render.tv_sec = tv.tv_sec;
    regis_last_render.tv_nsec = tv.tv_usec*1000;
    #endif
    #endif

    int clen = cmdlen-(command-cmd);
    if(clen<2) return;
    if(command == 0) return;
    if(command[0] == 0) return;
  }
}

bool regis_recent() {
 
  struct timespec current_time;
  
  #ifdef LINUX_BUILD
  clock_gettime(CLOCK_MONOTONIC,&current_time);
  #endif
  
  #if defined(IPHONE_BUILD) || defined(OSX_BUILD)
  #if _POSIX_TIMERS > 0
  clock_gettime(CLOCK_REALTIME, &tp);
  #else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  current_time.tv_sec = tv.tv_sec;
  current_time.tv_nsec = tv.tv_usec*1000;
  #endif
  #endif

  struct timespec delta_time;
  delta_time.tv_sec  = current_time.tv_sec  - regis_last_render.tv_sec;
  delta_time.tv_nsec = current_time.tv_nsec - regis_last_render.tv_nsec;

  if(delta_time.tv_nsec < 0) {
     delta_time.tv_sec--;
     delta_time.tv_nsec += 1000000000;
  }

  if(delta_time.tv_sec  > 0        ) return false;
  if(delta_time.tv_nsec > 200000000) return false;

  return true;
}
