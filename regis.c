#define _POSIX_C_SOURCE 199309L
#define _BSD_SOURCE

#include <stdbool.h>
#include "regis.h"
#include <SDL/SDL.h>       
#include <SDL/SDL_thread.h>
#include <time.h>

int pen_x = 0;
int pen_y = 0;

SDL_Surface *regis_layer=0;

SDL_mutex *regis_mutex=0;

void regis_clear() {
  SDL_mutexP(regis_mutex);
  SDL_FillRect(regis_layer,NULL, 0x000000); 
  SDL_mutexV(regis_mutex);
}

char *regis_process_cmd_screen(char *cmd) {
  printf("processing screen\n");

  char *buffer;
  char *code = strtok_r(cmd+2,")",&buffer);
  if(code == 0) return (cmd+1);

  return code+strlen(code)+1;
}

char *regis_process_cmd_text(char *cmd) {
  char *buffer=0;
  char *data=0;
  if(*(cmd+1) == '\'') {
    data = strtok_r(cmd+2,"\'",&buffer);
    if(data == 0) return (cmd+1);

    //regis_text_push(pen_x,pen_y,data);
    uint16_t wdata[1000];
    for(int n=0;(n<1000) && (data[n] != 0);n++) {
      wdata[n] = data[n];
      wdata[n+1] = 0;
    }
    SDL_mutexP(regis_mutex);
    draw_unitext(regis_layer,pen_x,pen_y,wdata,0x0,0xFFFFFFFF);
    SDL_mutexV(regis_mutex);
  } else 
  if(*(cmd+1) == '(') {
    data = strtok_r(cmd+2,")",&buffer);
    if(data == 0) return (cmd+1);
  }
  if(data == 0) return (cmd+1);
  return data+strlen(data)+1;
}

char *regis_process_cmd_w(char *cmd) {
  printf("processing w\n");
  char *buffer;
  char *code = strtok_r(cmd+2,")",&buffer);
  if(code == 0) return (cmd+1);
  
  return code+strlen(code)+1;
}

char *regis_process_cmd_position(char *cmd) {
  printf("processing position\n");


  char *buffer;
  char *xstr = strtok_r(cmd+2,",",&buffer);
  if(xstr == 0) return (cmd+1);
  char *ystr = strtok_r( NULL,"]",&buffer);
  if(ystr == 0) return (cmd+1);

  int new_x = atoi(xstr);
  int new_y = atoi(ystr);
  printf("processing position: %d %d\n",new_x,new_y);

  pen_x = new_x;
  pen_y = new_y;

  return ystr+strlen(ystr)+1;
}



void regis_init(int width,int height) {
  printf("regis init: %d %d\n",width,height);
  regis_layer = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
  printf("regis layer: %u\n",regis_layer);
}

char *regis_process_cmd_vector(char *cmd) {

  // vector commands look like this: v[] or v[100,200]
  // where 100 and 200 are the x and y positions respectively.
  // a line is drawn between the current pen position and the x,y position

  printf("processing vector\n",cmd);

  if(strncmp(cmd,"v[]",3) == 0) {
    printf("empry vector (dot) return\n");
    return cmd+3;
  }
  char *buffer;
  char *xstr = strtok_r(cmd+2,",",&buffer);
  if(xstr == 0) return cmd+2;
  char *ystr = strtok_r( NULL,"]",&buffer);
  if(ystr == 0) return cmd+2;

  int new_x = atoi(xstr);
  int new_y = atoi(ystr);
  printf("processed vector: %d %d\n",new_x,new_y);

  //regis_lines_push(pen_x,pen_y,new_x,new_y,0xFFFFFFFF);
  SDL_mutexP(regis_mutex);
  nsdl_line(regis_layer,pen_x,pen_y,new_x,new_y,0xFFFFFFFF);
  SDL_mutexV(regis_mutex);
  pen_x = new_x;
  pen_y = new_y;

  return ystr+strlen(ystr)+1;
}


char *regis_process_command(char *cmd) {
  if(cmd[0] == 'S') return regis_process_cmd_screen(cmd);    else
  if(cmd[0] == 'T') return regis_process_cmd_text(cmd);      else
  if(cmd[0] == 'W') return regis_process_cmd_w(cmd);         else
  if(cmd[0] == 'P') return regis_process_cmd_position(cmd);  else
  if(cmd[0] == 'v') return regis_process_cmd_vector(cmd);    else
  {
    printf("bad regis, incrementing %d\n",cmd);
    
    return cmd+1;
  }
}

struct timespec regis_last_render;

void regis_processor(const char *cmd,int cmdlen) {
 
  char *command = cmd;

  for(;;) {
    command = regis_process_command(command);
    clock_gettime(CLOCK_MONOTONIC,&regis_last_render);
    int clen = cmdlen-(command-cmd);
    if(clen<2) return;
    if(command == 0) return;
    if(command[0] == 0) return;
  }

}
 

bool regis_recent() {
 
  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC,&current_time);

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
