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
#include <pty.h>
#include <limits.h>

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

  VTermPos vp;
  int ncol=0;
  int xpos=0;

  VTermPos cursorpos;
  VTermState *vs = vterm_obtain_state(vt);
  vterm_state_get_cursorpos(vs,&cursorpos);
  for(int n=0;n<cols;n++) {
    uint16_t rtext[1000];

    vp.row=row;
    vp.col=ncol;
    VTermScreenCell c;
    int i = vterm_screen_get_cell(vts,vp,&c);
    if(c.width == 2) ncol++; // required?
    rtext[0]= c.chars[0];
    if(rtext[0]==0) rtext[0]=' ';
    rtext[1]=0;
    ncol++;

    //printf("%u,%u,%u,%u:%c ",c.chars[0],c.chars[1],c.chars[2],c.chars[3],rtext[n]);
    if(c.attrs.reverse == 1) { printf("b!"); }

    if((cursorpos.row == vp.row) && (cursorpos.col == vp.col)) {
      draw_unitext(screen,xpos,row*18,rtext,UINT_MAX,0);
    } else {
      draw_unitext(screen,xpos,row*18,rtext,0,UINT_MAX);
      draw_unitext(screen,xpos,row*18,rtext,(c.bg.red << 16) + (c.bg.green << 8) + c.bg.blue,
                                            (c.fg.red << 16) + (c.fg.green << 8) + c.fg.blue);
    }

    xpos+=9;
    if(c.width == 2) xpos +=9;
  }
/*
typedef struct {
#define VTERM_MAX_CHARS_PER_CELL 6
  uint32_t chars[VTERM_MAX_CHARS_PER_CELL];
  char     width;
  struct {
    unsigned int bold      : 1;
    unsigned int underline : 2;
    unsigned int italic    : 1;
    unsigned int blink     : 1;
    unsigned int reverse   : 1;
    unsigned int strike    : 1;
    unsigned int font      : 4; // 0 to 9 
  } attrs;
  VTermColor fg, bg;
} VTermScreenCell;
*/
  //printf("\n");
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

static int parser_resize(int new_rows, int new_cols, void *user)
{
//  rows = new_rows;
//  cols = new_cols;
  return 1;
}

VTermScreenCallbacks cb_screen = {
  .prescroll = &screen_prescroll,
  .resize    = &screen_resize,
};

int dcs_handler(const char *command,size_t cmdlen,void *user) {
  printf("command is: ");
  for(int n=0;n<cmdlen;n++) {
    printf("%c",command[n]);
  }
  printf("\n");
}

VTermParserCallbacks cb_parser = {
  .text    = 0,
  .control = 0,
  .escape  = 0,
  .csi     = 0,
  .osc     = 0,
  .dcs     = dcs_handler,
  .resize  = 0  //&parser_resize,
//  int (*text)(const char *bytes, size_t len, void *user);
//  int (*control)(unsigned char control, void *user);
//  int (*escape)(const char *bytes, size_t len, void *user);
//  int (*csi)(const char *leader, const long args[], int argcount, const char *intermed, char command, void *user);
//  int (*osc)(const char *command, size_t cmdlen, void *user);
//  int (*dcs)(const char *command, size_t cmdlen, void *user);
//  int (*resize)(int rows, int cols, void *user);
};

void terminal_resize(SDL_Surface *screen,int fd,VTerm *vt,int *cols,int *rows) {

  *rows = screen->h/18;
  *cols = screen->w/9;

  printf("resized: %d %d\n",*cols,*rows);

  struct winsize size = { *rows, *cols, 0, 0 };
  ioctl(fd, TIOCSWINSZ, &size);
  if(vt != 0) vterm_set_size(vt,*rows,*cols);
}


int main(int argc, char **argv) {

  if(SDL_Init(SDL_INIT_VIDEO)<0) {
    printf("Initialisation failed");
    return 1;
  }

  const SDL_VideoInfo *vid = SDL_GetVideoInfo();
  int maxwidth  = vid->current_w;
  int maxheight = vid->current_h-18;
 
  screen=SDL_SetVideoMode(maxwidth,maxheight,32,SDL_ANYFORMAT | SDL_RESIZABLE);//double buf?
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
  printf("fd: %d\n",fd);
 
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(500,50);

  printf("screen size %d %d\n",screen->w,screen->h);
  vt=0;
  terminal_resize(screen,fd,vt,&cols,&rows);

  printf("init rows: %d cols: %d\n",rows,cols);
  vt = vterm_new(rows, cols);

  vterm_state_set_bold_highbright(vterm_obtain_state(vt),1);
  vts = vterm_obtain_screen(vt);

  vterm_screen_enable_altscreen(vts,1);

  vterm_screen_set_callbacks(vts, &cb_screen, NULL);

  vterm_screen_set_damage_merge(vts, VTERM_DAMAGE_SCROLL);
  vterm_set_parser_backup_callbacks(vt , &cb_parser, NULL);

  vterm_screen_reset(vts, 1);
  vterm_parser_set_utf8(vt,1); // should be vts?
  
  // cope with initial resize
  //struct winsize size = { rows, cols, 0, 0 };
  //ioctl(fd, TIOCSWINSZ, &size);

  VTermColor fg;
  fg.red   =  257;
  fg.green =  257;
  fg.blue  =  257;

  VTermColor bg;
  bg.red   = 0;
  bg.green = 0;
  bg.blue  = 0;

//  vterm_state_set_default_colors(vterm_obtain_state(vt), &fg, &bg);

  int rowsc;
  int colsc;
 // vterm_get_size(vt,&rowsc,&colsc);
 // printf("read rows: %d cols: %d\n",rowsc,colsc);
 // vterm_get_size(vt,&rowsc,&colsc);
 // printf("read rows: %d cols: %d\n",rowsc,colsc);


  int x=0;int y=0;
  for(;;) {
    bool redraw=false;

    // redraw complete screen from vterm
    for(int row = 0; row < rows; row++) {
      dump_row(row);
    }

    // sending bytes from pts to vterm
    int len;
    char buffer[1024];
    len = read(fd, buffer, sizeof(buffer));
    if(len == -1) {
      if(errno == EIO) break;
    }
    //if(len>0)printf("buffer: ");
    //for(int n=0;n<len;n++) printf("%c",buffer[n]); 
    //if(len>0)printf("\n");
    if(len > 0) {
      vterm_push_bytes(vt, buffer, len);
      redraw=true;
    }

    // sending bytes from SDL to pts
    SDL_Event event;
    if(SDL_PollEvent(&event))
    if(event.type == SDL_KEYDOWN) {
      redraw=true;
      if(event.key.keysym.sym == SDLK_LSHIFT) continue;
      if(event.key.keysym.sym == SDLK_RSHIFT) continue;
      if(event.key.keysym.sym == SDLK_LEFT) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'D';
        buf[3] = 0;
        write(fd,buf,3);
      } else 
      if(event.key.keysym.sym == SDLK_RIGHT) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'C';
        buf[3] = 0;
        write(fd,buf,3);
      } else 
      if(event.key.keysym.sym == SDLK_UP) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'A';
        buf[3] = 0;
        write(fd,buf,3);
      } else 
      if(event.key.keysym.sym == SDLK_DOWN) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'B';
        buf[3] = 0;
        write(fd,buf,3);
      } else {
 
        // normal character
        char buf[2];
        buf[0] = event.key.keysym.unicode;
        buf[1]=0;
        write(fd,buf,1);
      }
    }

    if(event.type == SDL_VIDEORESIZE) {
      screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_ANYFORMAT | SDL_RESIZABLE);
      terminal_resize(screen,fd,vt,&cols,&rows);
      redraw=true;
    }

    if(redraw) {
      SDL_Flip(screen);
      SDL_LockSurface(screen);

      SDL_UnlockSurface(screen);
      SDL_FillRect(screen,NULL, 0x000000); 
    }
  }

  SDL_Quit();
  close(fd);

  vterm_free(vt);
 
  return 0;
}
