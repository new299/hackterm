#define _POSIX_C_SOURCE 2
#define _BSD_SOURCE

#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
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
#include <termios.h>
#include "vterm.h"
#include "utf8proc.h"
#include <locale.h>

#include "nunifont.h"
#include <pty.h>
#include <limits.h>

#include <pthread.h>
#include "nsdl.h"

void redraw_required();
    
int font_width  = 8;
int font_height = 16;
int font_space  = 1;

static VTerm *vt;
static VTermScreen *vts;

bool new_screen_size    = false;
int  new_screen_size_x;
int  new_screen_size_y;

static int cols;
static int rows;
int fd;

SDL_Surface *screen;

bool draw_selection = false;
int select_start_x=0;
int select_start_y=0;
int select_end_x  =0;
int select_end_y  =0;
int scroll_offset=0;

VTermScreenCell *grab_row(int row) {

  VTermScreenCell *rowdata = malloc(cols*sizeof(VTermScreenCell));

  VTermPos vp;
  for(int n=0;n<cols;n++) {
    vp.row = row;
    vp.col = n;
    vterm_screen_get_cell(vts,vp,&(rowdata[n]));
  }

  return rowdata;
}


void draw_row(VTermScreenCell *row,int ypos) {

  int xpos=0;

  for(int n=0;n<cols;n++) {
    uint16_t rtext[1000];

    rtext[0] = row[n].chars[0];
    if(rtext[0]==0) rtext[0]=' ';
    rtext[1]=0;

    //printf("%u,%u,%u,%u:%c ",c.chars[0],c.chars[1],c.chars[2],c.chars[3],rtext[n]);
    if(row[n].attrs.reverse == 1) { printf("b!"); }

    draw_unitext(screen,xpos,ypos,rtext,(row[n].bg.red << 16) + (row[n].bg.green << 8) + row[n].bg.blue,
                                        (row[n].fg.red << 16) + (row[n].fg.green << 8) + row[n].fg.blue);

    xpos+=(font_width+font_space);
    if(row[n].width == 2) {xpos +=(font_width+font_space);n++;}
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
}


size_t    scroll_buffer_size = 0;
VTermScreenCell **scroll_buffer = 0;

void scroll_buffer_push(VTermScreenCell *scroll_line,size_t len) {

  //printf("push line: %d\n",scroll_buffer_size);
  if(scroll_buffer == 0) { 
    scroll_buffer = malloc(sizeof(VTermScreenCell *)*1);  
  } else {
    scroll_buffer = realloc(scroll_buffer,sizeof(VTermScreenCell *)*(scroll_buffer_size+1));
  }
  scroll_buffer[scroll_buffer_size] = malloc(sizeof(VTermScreenCell)*len);

  for(size_t n=0;n<len;n++) {
    scroll_buffer[scroll_buffer_size][n] = scroll_line[n];
  }

  scroll_buffer_size++;
}

void scroll_buffer_get(size_t line_number,VTermScreenCell **line,int *len) {
  *line = scroll_buffer[scroll_buffer_size-line_number-1];
  *len  = 10;
}

void scroll_buffer_dump() {
}

static int screen_prescroll(VTermRect rect, void *user)
{
  if(rect.start_row != 0 || rect.start_col != 0 || rect.end_col != cols)
    return 0;

  
  for(int row=rect.start_row;row<rect.end_row;row++) {
    //uint16_t scrolloff[1000];
    VTermScreenCell scrolloff[1000];
    //printf("cols: %d\n",cols);

    size_t len=0;
    for(int n=0;n<cols;n++) {
      VTermPos vp;
      vp.row=row;
      vp.col=n;
      VTermScreenCell c;
      int i = vterm_screen_get_cell(vts,vp,&c);
      scrolloff[n] = c;
      //if(scrolloff[n] == 0) scrolloff[n] = ' ';
      //scrolloff[n+1] =0;
      len++;
    }
    //printf("strlen: %d",strlen(scrolloff));
    //printf("scroll off: %s\n",scrolloff);
    scroll_buffer_push(scrolloff,len);

    //scroll_buffer_dump();
  }
  // for(int row = 0; row < rect.end_row; row++)
  //   dump_row(row);
  redraw_required();
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

static int screen_bell(void* d) {

}

VTermScreenCallbacks cb_screen = {
  .prescroll = &screen_prescroll,
  .resize    = &screen_resize,
  .bell      = &screen_bell
};

int pen_x = 0;
int pen_y = 0;

char *regis_process_cmd_screen(char *cmd) {
  printf("processing screen\n");

  char *buffer;
  char *code = strtok_r(cmd+2,")",&buffer);
  printf("screen code: %s\n",code);
  
  return code+strlen(code)+1;
}

char *regis_process_cmd_text(char *cmd) {
  printf("processing text: %s\n",cmd);
  char *buffer=0;
  char *data=0;
  printf("cmd+1: %c\n",*(cmd+1));
  if(*(cmd+1) == '\'') {
    printf("type 1 text\n");
    data = strtok_r(cmd+2,"\'",&buffer);
    regis_text_push(pen_x,pen_y,data);
  } else 
  if(*(cmd+1) == '(') {
    printf("type 2 text\n");
    data = strtok_r(cmd+2,")",&buffer);
  }
  if(data != 0) printf("text data: %s\n",data);
           else printf("no text data\n");
  
  return data+strlen(data)+1;
}

char *regis_process_cmd_w(char *cmd) {
  printf("processing w\n");
  char *buffer;
  char *code = strtok_r(cmd+2,")",&buffer);
  printf("screen code: %s\n",code);
  
  return code+strlen(code)+1;
}

char *regis_process_cmd_position(char *cmd) {
  printf("processing position\n");


  char *buffer;
  char *xstr = strtok_r(cmd+2,",",&buffer);
  char *ystr = strtok_r( NULL,"]",&buffer);

  int new_x = atoi(xstr);
  int new_y = atoi(ystr);
  printf("processing position: %d %d\n",new_x,new_y);

  pen_x = new_x;
  pen_y = new_y;

  return ystr+strlen(ystr)+1;
}

struct regis_line {
  int start_x;
  int start_y;
  int end_x;
  int end_y;
  int color;
};

struct regis_text {
  int x;
  int y;
  char *text;
};

struct regis_line regis_lines[100];
int    regis_lines_size=0;

struct regis_text regis_texts[100];
int    regis_texts_size=0;

void regis_lines_push(int sx,int sy,int ex,int ey,int color) {
  
  struct regis_line r = { sx,sy,ex,ey,color };


  regis_lines[regis_lines_size] = r;
  regis_lines_size++;
}

void regis_text_push(int x,int y,char *text) {

  struct regis_text t;
  t.x = x;
  t.y = y;
  
  t.text = malloc(strlen(text)+1);
  strcpy(t.text,text);

  regis_texts[regis_texts_size] = t;
  regis_texts_size++;  
}

void regis_render_lines() {
//  if(regis_lines_size != 0) printf("regis lines size: %d\n",regis_lines_size);
  for(size_t n=0;n<regis_lines_size;n++) {
//    printf("regis line: %d %d %d %d %d\n",regis_lines[n].start_x,regis_lines[n].start_y,regis_lines[n].end_x,regis_lines[n].end_y,regis_lines[n].color);
    nsdl_line(screen,regis_lines[n].start_x,regis_lines[n].start_y,regis_lines[n].end_x,regis_lines[n].end_y,0xFFFFFFF);
  }
}

void regis_render_text() {
  for(size_t n=0;n<regis_texts_size;n++) {
    uint16_t utext[1000];
    for(int i=0;i<strlen(regis_texts[n].text);i++) {
      utext[i] = regis_texts[n].text[i];
      utext[i+1]=0;
    }
    draw_unitext(screen,regis_texts[n].x,regis_texts[n].y,utext,0x0,0xFFFFFFFF);
  }
}

void regis_render() {

  regis_render_lines();
  regis_render_text();
}

char *regis_process_cmd_vector(char *cmd) {

  // vector commands look like this: v[] or v[100,200]
  // where 100 and 200 are the x and y positions respectively.
  // a line is drawn between the current pen position and the x,y position

  printf("processing vector: %s\n",cmd);

  if(strncmp(cmd,"v[]",3) == 0) {
    printf("empry vector (dot) return\n");
    return cmd+3;
  }
  char *buffer;
  char *xstr = strtok_r(cmd+2,",",&buffer);
  char *ystr = strtok_r( NULL,"]",&buffer);

  int new_x = atoi(xstr);
  int new_y = atoi(ystr);
  printf("processed vector: %d %d\n",new_x,new_y);

  regis_lines_push(pen_x,pen_y,new_x,new_y,0xFFFFFFFF);
  pen_x = new_x;
  pen_y = new_y;

  return ystr+strlen(ystr)+1;
}


char *regis_process_command(char *cmd) {
  if(cmd[0] == 'S') return regis_process_cmd_screen(cmd);
  if(cmd[0] == 'T') return regis_process_cmd_text(cmd);
  if(cmd[0] == 'W') return regis_process_cmd_w(cmd);
  if(cmd[0] == 'P') return regis_process_cmd_position(cmd);
  if(cmd[0] == 'v') return regis_process_cmd_vector(cmd);
}

void regis_processor(const char *cmd) {
 
  char *command = cmd;

  for(;;) {
    command = regis_process_command(command);
    if(command[0] == 0) return;
  }

}

int dcs_handler(const char *command,size_t cmdlen,void *user) {
  printf("command is: ");
  for(int n=0;n<cmdlen;n++) {
    printf("%c",command[n]);
  }
  regis_processor(command+2);
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

SDL_mutex *screen_mutex;
SDL_mutex *vterm_mutex;
SDL_sem   *redraw_sem;

void terminal_resize(SDL_Surface *screen,VTerm *vt,int *cols,int *rows) {

  *rows = screen->h/(font_height+font_space);
  *cols = screen->w/(font_width+font_space);

  printf("resized: %d %d\n",*cols,*rows);

  struct winsize size = { *rows, *cols, 0, 0 };
  ioctl(fd, TIOCSWINSZ, &size);

  SDL_mutexP(vterm_mutex);
  if(vt != 0) vterm_set_size(vt,*rows,*cols);
  SDL_mutexV(vterm_mutex);
}

void cursor_position(int *cursorx,int *cursory) {
  VTermPos cursorpos;
  VTermState *vs = vterm_obtain_state(vt);
  vterm_state_get_cursorpos(vs,&cursorpos);

  *cursorx = cursorpos.col;
  *cursory = cursorpos.row;
}

void redraw_screen() {
  SDL_mutexP(screen_mutex);

  if(new_screen_size) {
    screen = SDL_SetVideoMode(new_screen_size_x, new_screen_size_y, 32, SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF);
    terminal_resize(screen,vt,&cols,&rows);
    new_screen_size = false;
  }

  SDL_LockSurface(screen);
  SDL_FillRect(screen,NULL, 0x000000); 

  for(int row = 0; row < rows; row++) {

    int trow = row-scroll_offset;

    VTermScreenCell *rowdata=0;
    if(trow >= 0) {
      rowdata = grab_row(trow);
      if(rowdata != 0) draw_row(rowdata,row*(font_height+font_space));
      if(rowdata != 0) free(rowdata);
    } else {
      //printf("trow: %d\n",trow);
      int len;
      if((0-trow) > scroll_buffer_size) { rowdata = 0; }
      else {
        scroll_buffer_get(0-trow,&rowdata,&len);
      }
      if(rowdata != 0) draw_row(rowdata,row*(font_height+font_space));
    }

    int cursorx,cursory;
    cursor_position(&cursorx,&cursory);
    if(cursory == trow) {
      int width=font_width+font_space;
      if(rowdata[cursorx].width == 2) width+=(font_width+font_space);
      nsdl_rectangle_softalph(screen,cursorx*(font_width+font_space),row*(font_height+font_space),(cursorx*(font_width+font_space))+width,(row*(font_height+font_space))+(font_height+font_space),0xFF);
      nsdl_rectangle_wire    (screen,cursorx*(font_width+font_space),row*(font_height+font_space),(cursorx*(font_width+font_space))+width,(row*(font_height+font_space))+(font_height+font_space),UINT_MAX);
    }

  }

  if(draw_selection) {
    //printf("selection %d %d %d %d\n",select_start_x,select_end_x,select_start_y,select_end_y);
    //int pselect_start_x = select_start_x;
    //int pselect_end_x   = select_end_x;
    //int pselect_start_y = select_start_y;
    //int pselect_end_y   = select_end_y;

    //if(pselect_start_x > pselect_end_x) {int c = pselect_start_x; pselect_start_x = pselect_end_x; pselect_end_x = c; }
    //if(pselect_start_y > pselect_end_y) {int c = pselect_start_y; pselect_start_y = pselect_end_y; pselect_end_y = c; }

    int text_start_x;
    int text_start_y;
    int text_end_x;
    int text_end_y;
    mouse_to_select_box(select_start_x,select_start_y,select_end_x,select_end_y,
                         &text_start_x, &text_start_y, &text_end_x, &text_end_y);


    //nsdl_rectangle_hashed(screen,pselect_start_x,pselect_start_y,pselect_end_x,pselect_end_y,0xFFFFFF);
    //nsdl_rectangle_wire(screen,pselect_start_x,pselect_start_y,pselect_end_x,pselect_end_y,0xFFFFFF);
    nsdl_rectangle_wire(screen,text_start_x*(font_width+font_space),text_start_y*(font_height+font_space),
                                 text_end_x*(font_width+font_space),text_end_y*(font_height+font_space),0xFFFFFF);
  }
  
  regis_render();

  SDL_UnlockSurface(screen);
  SDL_Flip(screen);

  SDL_mutexV(screen_mutex);
}

void sdl_render_thread() {
  for(;;) {
    SDL_SemWait(redraw_sem);
    regis_render();
    redraw_screen();
  }
}

void redraw_required() {
  SDL_SemPost(redraw_sem);
}

void console_read_thread() {
  for(;;) {
    // sending bytes from pts to vterm
    int len;
    char buffer[10241];
    len = read(fd, buffer, sizeof(buffer)-1);
    if(len == -1) {
      if(errno == EIO) break;
    }
    //if(len>0)printf("buffer: ");
    //for(int n=0;n<len;n++) printf("%c",buffer[n]); 
    //if(len>0)printf("\n");
    if(len > 0) {
      SDL_mutexP(vterm_mutex);
      vterm_push_bytes(vt, buffer, len);
      SDL_mutexV(vterm_mutex);
    }
    redraw_required();
  }
}

void copy_text(uint16_t *itext,int len) {
  
  
  printf("copy len: %d\n",len);
  char text[20000];
  for(int i=0;i<len;i++) {
    text[i] = itext[i];
    text[i+1] = 0;
  }
  printf("copy text: %s\n",text);

  FILE *w1 = popen("xclip -selection c","w");
  if(w1!=NULL) {
    fprintf(w1,"%s",text);
    pclose(w1);
  }

  FILE *w2 = popen("xclip -i","w");
  if(w2==NULL) return;
  fprintf(w2,"%s",text);
  pclose(w2);
  
  // execute these two commands on Linux/XWindows by default
  //echo "test" | xclip -selection c
  //echo "test" | xclip -i 
}

void mouse_to_select_box(int   sx,int   sy,int   ex,int   ey,
                         int *stx,int *sty,int *etx,int *ety) {
  
  if(ex<sx) {int c=ex; ex=sx;sx=c;}
  if(ey<sy) {int c=ey; ey=sy;sy=c;}

  *stx=floor(((float)sx/(font_width +font_space)));
  *etx=ceil( ((float)ex/(font_width +font_space)));
  *sty=floor(((float)sy/(font_height+font_space)));
  *ety=ceil( ((float)ey/(font_height+font_space)));

  if(*etx > cols) *etx = cols;
  if(*ety > rows) *ety = rows;
}

void get_text_region(int text_start_x,int text_start_y,int text_end_x,int text_end_y,uint16_t **itext,int *ilen) {

  int len=0;
  uint16_t *text = malloc(10240);
  for(int y=text_start_y;y<text_end_y;y++) {
    for(int x=text_start_x;x<text_end_x;x++) {
      VTermScreenCell c;
      VTermPos vp;
      vp.row=y;
      vp.col=x;
      int i = vterm_screen_get_cell(vts,vp,&c);
      text[len] = c.chars[0];
      if(text[len]==0) text[len]=' ';
      len++;
    }
    text[len] = '\n';
    len++;
  }
  text[len]=0;

  *itext = text;
  *ilen  = len;
}

void process_mouse_event(SDL_Event *event) {

  int mouse_x = event->motion.x;
  int mouse_y = event->motion.y;
  
  
  if(event->button.button == SDL_BUTTON_WHEELUP) {
    printf("wheel up\n");
    scroll_offset++;
    redraw_required();
    printf("scroll offset %d\n",scroll_offset);
  } else
  if(event->button.button == SDL_BUTTON_WHEELDOWN) {
    printf("wheel down\n");
    scroll_offset--;
    if(scroll_offset < 0) scroll_offset = 0;
    redraw_required();
    printf("scroll offset %d\n",scroll_offset);
  } else
  if(event->type == SDL_MOUSEMOTION    ) {


    if(draw_selection == true) {
      select_end_x = event->button.x;
      select_end_y = event->button.y;
      redraw_required();
    }
  } else
  if(event->type == SDL_MOUSEBUTTONUP  ) {
    draw_selection = false;

    int text_start_x;
    int text_start_y;
    int text_end_x;
    int text_end_y;
    mouse_to_select_box(select_start_x,select_start_y,select_end_x,select_end_y,
                         &text_start_x, &text_start_y, &text_end_x, &text_end_y);

    uint16_t *text=0;
    int      len=0;
    get_text_region(text_start_x,text_start_y,text_end_x,text_end_y,&text,&len);
    printf("copy: %d %d %d %d\n",text_start_x,text_start_y,text_end_x,text_end_y);

    if(len != 0) copy_text(text,len);
    if(text != 0) free(text);
    redraw_required();
  } else
  if(event->type == SDL_MOUSEBUTTONDOWN) {
    select_start_x = event->button.x;
    select_start_y = event->button.y;
    select_end_x = event->button.x;
    select_end_y = event->button.y;
    draw_selection = true;
  }
}

void sdl_read_thread() {
  for(;;) {
    // sending bytes from SDL to pts
    SDL_Event event;
    SDL_WaitEvent(&event);
    process_mouse_event(&event);

    if(event.type == SDL_KEYDOWN) {
      scroll_offset = 0;
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
        if(buf[0] != 0) {
          write(fd,buf,1);
        }
      }
    }

    if(event.type == SDL_VIDEORESIZE) {
      new_screen_size_x = event.resize.w;
      new_screen_size_y = event.resize.h;
      new_screen_size   = true;

      redraw_required();
    }
  }
}

int main(int argc, char **argv) {

  screen_mutex = SDL_CreateMutex();
  vterm_mutex  = SDL_CreateMutex();
  redraw_sem   = SDL_CreateSemaphore(1);

  if(SDL_Init(SDL_INIT_VIDEO)<0) {
    printf("Initialisation failed");
    return 1;
  }

  const SDL_VideoInfo *vid = SDL_GetVideoInfo();
  int maxwidth  = vid->current_w;
  int maxheight = vid->current_h-(font_height+font_space);
 
  screen=SDL_SetVideoMode(maxwidth,maxheight,32,SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF);//double buf?
  if(screen==NULL) {
    printf("Failed SDL_SetVideoMode: %d",SDL_GetError());
    SDL_Quit();
    return 1;
  }
   
  // grab pts
  //int fd = open("/dev/ptmx",O_RDWR | O_NOCTTY | O_NONBLOCK);
  /* None of the docs about termios explain how to construct a new one of
   * these, so this is largely a guess */
  struct termios termios = {
    .c_iflag = ICRNL|IXON|IUTF8,
    .c_oflag = OPOST|ONLCR|NL0|CR0|TAB0|BS0|VT0|FF0,
    .c_cflag = CS8|CREAD,
    .c_lflag = ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK,
    /* c_cc later */
  };

  int pid = forkpty(&fd,NULL,NULL,NULL);
  int flag=fcntl(fd,F_GETFL,0);

  char *termset = "TERM=xterm";
  putenv(termset);
  //flag|=O_NONBLOCK;
  //fcntl(fd,F_SETFL,flag);

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
  terminal_resize(screen,vt,&cols,&rows);

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

  SDL_Thread *thread1 = SDL_CreateThread(sdl_read_thread    ,0);
  SDL_Thread *thread2 = SDL_CreateThread(sdl_render_thread  ,0);
  SDL_Thread *thread3 = SDL_CreateThread(console_read_thread,0);

  SDL_WaitThread(thread3,NULL); 

  SDL_Quit();
  close(fd);

  vterm_free(vt);
 
  return 0;
}
