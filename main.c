#include <string.h>
#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "vterm.h"

#include "nunifont.h"

static VTerm *vt;
static VTermScreen *vts;

static int cols;
static int rows;

SDL_Surface *screen;

void dump_row(int row) {
  VTermRect rect = {
    .start_row = row,
    .start_col = 0,
    .end_row   = row+1,
    .end_col   = cols,
  };

  size_t len = vterm_screen_get_text(vts, NULL, 0, rect);
  char *text = malloc(len + 1);
  text[len] = 0;

  vterm_screen_get_text(vts, text, len, rect);

  uint16_t atext[100];
  for(int a=0;a<=len;a++) {atext[a] = text[a];}
  draw_unitext(screen,0,row*18,atext,0);

  free(text);
}


static int screen_prescroll(VTermRect rect, void *user)
{
  if(rect.start_row != 0 || rect.start_col != 0 || rect.end_col != cols)
    return 0;

  for(int row = 0; row < rect.end_row; row++)
    dump_row(row);

  return 1;
}

static int screen_resize(int new_rows, int new_cols, void *user)
{
  rows = new_rows;
  cols = new_cols;
  return 1;
}

static VTermScreenCallbacks cb_screen = {
  .prescroll = &screen_prescroll,
  .resize    = &screen_resize,
};

int main(int argc, char **argv) {

  if(SDL_Init(SDL_INIT_VIDEO)<0) {
    printf("Initialisation failed");
    return 1;
  }
 
  screen=SDL_SetVideoMode(1200,800,32,SDL_ANYFORMAT);
  if(screen==NULL) {
    printf("Failed SDL_SetVideoMode: %d",SDL_GetError());
    SDL_Quit();
    return 1;
  }
   
  // grab pts
  //int fd = open("/dev/ptmx",O_RDWR | O_NOCTTY | O_NONBLOCK);
  int fd;
  int pid = forkpty(&fd,NULL,NULL,NULL);
  int flag=fcntl(fd,F_GETFL,0);
  flag|=O_NONBLOCK;
  fcntl(fd,F_SETFL,flag);

  //fcntl(fd, F_SETFL, FNDELAY);

  printf("fd: %d",fd);
  if(pid == 0) {
    char args[3];
    args[0] = "/bin/bash";
    args[1] =""; 
    args[2] = 0;

    //execv("/bin/bash",args);
    execl("/bin/bash","bash",NULL);
    return 0;
  }


//  grantpt(fd);
//  unlockpt(fd);
  printf("fd: %d",fd);
 
  SDL_EnableUNICODE(1);

  rows = 25;
  cols = 80;

  vt = vterm_new(rows, cols);
  vts = vterm_obtain_screen(vt);
  vterm_screen_set_callbacks(vts, &cb_screen, NULL);

  vterm_screen_reset(vts, 1);


  int x=0;int y=0;
  for(;;) {
    for(int row = 0; row < rows; row++) {
      dump_row(row);
    }
    int len;
    char buffer[1024];
    while((len = read(fd, buffer, sizeof(buffer))) > 0) {
      vterm_push_bytes(vt, buffer, len);
    }

    SDL_Event event;
    if(SDL_PollEvent(&event))
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_LEFT) exit(0);
 
      char buf[2];
      buf[0] = event.key.keysym.unicode;
      buf[1]=0;
      printf("key: %u\n",buf[0]);
      write(fd,buf,1);
    }

    SDL_Flip(screen);
    SDL_LockSurface(screen);

    SDL_UnlockSurface(screen);
    SDL_FillRect(screen,NULL, 0x000000); 

  }
  SDL_Quit();
  close(fd);

  //vterm_free(vt);
 
 // return 0;

}
