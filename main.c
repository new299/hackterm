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
#include "utf8proc.h"

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

//  printf("vterm text: %s\n",text);

//  size_t len = vterm_screen_get_chars(vts,NULL,0,rect);
//  size_t len = vterm_screen_get_text(vts, NULL, 0, rect);
//  if(len <= 0) return;
  uint8_t text1[1000];// = malloc(len + 1);
  uint32_t text[1000];// = malloc(len + 1);
  uint16_t rtext[1000];// = malloc(len + 1);
  text[0] = 0;


  VTermPos vp;
  for(int n=0;n<cols;n++) {
    vp.row=row;
    vp.col=n;
    VTermScreenCell c;
    int i = vterm_screen_get_cell(vts,vp,&c);
    rtext[n] = c.chars[0];
    if(rtext[n]==0) rtext[n]=' ';
    rtext[n+1]=0;
    //printf("%u,%u,%u,%u:%c ",c.chars[0],c.chars[1],c.chars[2],c.chars[3],rtext[n]);
  }
  //printf("\n");
  draw_unitext(screen,0,row*18,rtext,0);
/*
  size_t len=1000;
        vterm_screen_get_text (vts, text1, len, rect);
  len = vterm_screen_get_chars(vts, text , len, rect);
  printf("len: %d\n",len);

//  printf("vterm text: %s\n",text);

  uint16_t atext[1000];
  atext[0]=0;
  size_t pos=0;
  for(size_t n=0;n<len;n++) { atext[n] = text[n]; atext[n+1]=0; }

  for(size_t n=0;;) {
    if(pos >= len) break;
    int32_t uc;
    ssize_t p = utf8proc_iterate(text+pos,-1,&uc);
    if(p==-1) break;
    atext[n] = uc;
    atext[n+1]=0;
    n++;
    pos+=p;
  }

  printf("vterm atext: ");
  for(int n=0;n<len+5;n++) {
    printf("rt %u       ",rtext[n]);
    printf("t1 %u ",text1[n]);
    printf("tt %u ",text[n]);
    printf("at %u       ",atext[n]);
    printf("cc %c       ",text[n]);
  }
  printf("\n");
*/

//  free(text);
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

  const SDL_VideoInfo *vid = SDL_GetVideoInfo();
  int maxwidth  = vid->current_w;
  int maxheight = vid->current_h;
 
  screen=SDL_SetVideoMode(maxwidth,maxheight,32,SDL_ANYFORMAT);//double buf?
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

  rows = maxheight/17;
  cols = maxwidth /17;

  vt = vterm_new(rows, cols);
  vterm_parser_set_utf8(vt,1);

  vterm_state_set_bold_highbright(vterm_obtain_state(vt),1);
  vts = vterm_obtain_screen(vt);

  vterm_screen_enable_altscreen(vts,1);

  vterm_screen_set_callbacks(vts, &cb_screen, NULL);
  vterm_parser_set_utf8(vts,1);

  vterm_screen_reset(vts, 1);
  vterm_parser_set_utf8(vts,1);

  VTermColor fg;
  fg.red   =  257;
  fg.green =  257;
  fg.blue  =  257;

  VTermColor bg;
  bg.red   = 0;
  bg.green = 0;
  bg.blue  = 0;

  vterm_state_set_default_colors(vterm_obtain_state(vt), &fg, &bg);


  int x=0;int y=0;
  for(;;) {

    // redraw complete screen from vterm
    for(int row = 0; row < rows; row++) {
      dump_row(row);
    }

    // sending bytes from pts to vterm
    int len;
    char buffer[1024];
    len = read(fd, buffer, sizeof(buffer));
    if(len > 0) {
      vterm_push_bytes(vt, buffer, len);
      //printf("read: "); for(int n=0;n<len;n++) printf("%u,",(uint8_t) buffer[n]); printf("\n");
      //printf("nead: "); for(int n=0;n<len;n++) printf("%c",buffer[n]); printf("\n");
    }

    // sending bytes from SDL to pts
    SDL_Event event;
    if(SDL_PollEvent(&event))
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_LEFT) exit(0);
 
      char buf[2];
      buf[0] = event.key.keysym.unicode;
      buf[1]=0;
    //  printf("key: %u\n",buf[0]);
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
