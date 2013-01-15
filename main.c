#define _POSIX_C_SOURCE 199309L
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
#include <locale.h>

#include "nunifont.h"
#include <limits.h>

#include "nsdl.h"
#include <time.h>
#include "regis.h"
#include <stdbool.h>
#include "ssh.h"
#include "local.h"
#include "inlinedata.h"
#include "ngui.h"

#define CONNECTION_LOCAL 1
#define CONNECTION_SSH   2

#define IPHONE_BUILD

void redraw_required();

bool sdl_init_complete=false;
 
int font_width  = 8;
int font_height = 16;
int font_space  = 0;

static VTerm *vt;
static VTermScreen *vts;

bool any_blinking       = false;
bool new_screen_size    = false;
int  new_screen_size_x;
int  new_screen_size_y;

static int cols;
static int rows;

SDL_Surface *screen=0;

bool draw_selection = false;
int select_start_x=0;
int select_start_y=0;
int select_end_x  =0;
int select_end_y  =0;
int select_start_scroll_offset;
int select_end_scroll_offset;

bool hterm_quit = false;

int       scroll_offset=0;
size_t    scroll_buffer_initial_size = 10000;

size_t    scroll_buffer_size = 0;
size_t    scroll_buffer_start =0;
size_t    scroll_buffer_end   =0;
VTermScreenCell **scroll_buffer = 0;

SDL_cond   *cond_quit;
SDL_mutex  *screen_mutex;
SDL_mutex  *vterm_mutex;
SDL_sem    *redraw_sem;
SDL_mutex  *quit_mutex;
VTermState *vs;

// Funtions used to communicate with host
int (*c_open)(char *hostname,char *username, char *password) = 0;
int (*c_close)() = 0;
int (*c_write)(char *bytes,int len) = 0;       
int (*c_read)(char *bytes,int len) = 0;    
int (*c_resize)(int rows,int cols) = 0;

void scroll_buffer_get(size_t line_number,VTermScreenCell **line);

void regis_render() {                                        
  SDL_mutexP(regis_mutex);                                   
  int res = SDL_BlitSurface(regis_layer,NULL,screen,NULL);   
  SDL_mutexV(regis_mutex);                                   
} 

void inline_data_render() {                                        
  SDL_mutexP(inline_data_mutex);                                   
  int res = SDL_BlitSurface(inline_data_layer,NULL,screen,NULL);   
  SDL_mutexV(inline_data_mutex);                                   
}

void mouse_to_select_box(int   sx,int   sy,int so,
                         int   ex,int   ey,int eo,
                         int *stx,int *sty,int *etx,int *ety) {
    
    *stx=floor(((float)sx/(font_width +font_space)));
    *etx=ceil( ((float)ex/(font_width +font_space)));
    *sty=floor(((float)sy/(font_height+font_space)))-so;
    *ety=ceil( ((float)ey/(font_height+font_space)))-eo;
    
}

VTermScreenCell *grab_row(int trow,bool *dont_free) {

  VTermScreenCell *rowdata = 0;

  if(trow >= 0) {
    // a screen row
    rowdata = malloc(cols*sizeof(VTermScreenCell));
    VTermPos vp;
    for(int n=0;n<cols;n++) {
      vp.row = trow;
      vp.col = n;
      vterm_screen_get_cell(vts,vp,&(rowdata[n]));
    }
    *dont_free =false;
  } else {
    // a scrollback row
    if((0-trow) > scroll_buffer_size) { rowdata = 0; }
    else {
      scroll_buffer_get(0-trow,&rowdata);
      *dont_free=true;
    }
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

    VTermColor fg = row[n].fg;
    VTermColor bg = row[n].bg;

    if(row[n].attrs.blink == 1) any_blinking = true;
    draw_unitext_fancy(screen,xpos,ypos,rtext,(bg.red << 16) + (bg.green << 8) + bg.blue,
                                              (fg.red << 16) + (fg.green << 8) + fg.blue,
                                              row[n].attrs.bold,
                                              row[n].attrs.underline,
                                              row[n].attrs.italic,
                                              row[n].attrs.blink,
                                              row[n].attrs.reverse,
                                              row[n].attrs.strike,
                                              row[n].attrs.font);

    xpos+=(font_width+font_space);
    if(row[n].width == 2) {xpos +=(font_width+font_space);n++;}
  }

}



void scroll_buffer_init() {
  scroll_buffer = malloc(sizeof(VTermScreenCell *)*scroll_buffer_initial_size);
  for(int n=0;n<scroll_buffer_initial_size;n++) scroll_buffer[n] = 0;
  scroll_buffer_size = scroll_buffer_initial_size;
  scroll_buffer_start=0;
  scroll_buffer_end  =0;
}

void scroll_buffer_push(VTermScreenCell *scroll_line,size_t len) {

   if(scroll_buffer == 0) scroll_buffer_init();

   if(scroll_buffer_end >= scroll_buffer_size) scroll_buffer_end = 0;

   if(scroll_buffer[scroll_buffer_end] != 0) {
     // if infini buffer, do resize
     // scroll_buffer_resize(scroll_buffer_size+10000);
     // else
     free(scroll_buffer[scroll_buffer_end]);
     scroll_buffer[scroll_buffer_end]=0;
   }

  scroll_buffer[scroll_buffer_end] = malloc(sizeof(VTermScreenCell)*len);

  for(size_t n=0;n<len;n++) {
    scroll_buffer[scroll_buffer_end][n] = scroll_line[n];
  }

  scroll_buffer_end++;
}

void scroll_buffer_get(size_t line_number,VTermScreenCell **line) {
  int idx = scroll_buffer_end-line_number-1;

  if(idx < 0) idx = scroll_buffer_size+idx;
  if(idx < 0) *line = 0;

  *line = scroll_buffer[idx];
}

void scroll_buffer_dump() {
}

static int screen_prescroll(VTermRect rect, void *user)
{
  if(rect.start_row != 0 || rect.start_col != 0 || rect.end_col != cols)
    return 0;

  
  for(int row=rect.start_row;row<rect.end_row;row++) {
    VTermScreenCell scrolloff[1000];

    size_t len=0;
    for(int n=0;n<cols;n++) {
      VTermPos vp;
      vp.row=row;
      vp.col=n;
      VTermScreenCell c;
      int i = vterm_screen_get_cell(vts,vp,&c);
      scrolloff[n] = c;
      len++;
    }
    scroll_buffer_push(scrolloff,len);

  }
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
  return 1;
}

static int screen_bell(void* d) {

}

int state_erase(VTermRect r,void *user) {
  printf("********************************************** clear received\n");
  return 0;
}

VTermScreenCallbacks cb_screen = {
  .prescroll = &screen_prescroll,
  .resize    = &screen_resize,
  .bell      = &screen_bell
};

VTermStateCallbacks cb_state = {
  .putglyph     = 0,
  .movecursor   = 0,
  .scrollrect   = 0,
  .moverect     = 0,
  .erase        = &state_erase,
  .initpen      = 0,
  .setpenattr   = 0,
  .settermprop  = 0,
  .setmousefunc = 0,
  .bell         = 0,
  .resize       = 0
};


int csi_handler(const char *leader, const long args[], int argcount, const char *intermed, char command, void *user) {
  if(command == 'J') {
    printf("************************* this clear\n");
    if(sdl_init_complete) {
      if(!regis_recent()) regis_clear();
      inline_data_clear();
    }
  }

  return 0;
}

int dcs_handler(const char *command,size_t cmdlen,void *user) {
  printf("command is: ");
  for(int n=0;n<cmdlen;n++) {
    printf("%u,",command[n]);
  }
  if(cmdlen < 3) return 0;

  regis_processor(command+2,cmdlen);
  printf("\n");
}

VTermParserCallbacks cb_parser = {
  .text    = 0,
  .control = 0,
  .escape  = 0,
  .csi     = csi_handler,
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


void terminal_resize(SDL_Surface *screen,VTerm *vt,int *cols,int *rows) {

  printf("terminal resize, size: %d %d\n",screen->w,screen->h);

  *rows = screen->h/(font_height+font_space);
  *cols = screen->w/(font_width+font_space);

  printf("resized: %d %d\n",*cols,*rows);

  if(c_resize != NULL) (*c_resize)(*rows,*cols);

  SDL_mutexP(vterm_mutex);
  if(vt != 0) vterm_set_size(vt,*rows,*cols);
  SDL_mutexV(vterm_mutex);
}

void cursor_position(int *cursorx,int *cursory) {
  VTermPos cursorpos;
  vterm_state_get_cursorpos(vs,&cursorpos);

  *cursorx = cursorpos.col;
  *cursory = cursorpos.row;
}

void redraw_screen() {
  SDL_mutexP(screen_mutex);

  if(new_screen_size) {
    printf("SCREEN RESIZE DETECTED\n");
    screen = SDL_SetVideoMode(new_screen_size_x, new_screen_size_y, 32, SDL_RESIZABLE | SDL_DOUBLEBUF);
    terminal_resize(screen,vt,&cols,&rows);
    new_screen_size = false;
  }

  SDL_LockSurface(screen);
  SDL_FillRect(screen,NULL, 0x000000); 

  any_blinking = false;
  for(int row = 0; row < rows; row++) {

    int trow = row-scroll_offset;
    bool dont_free=false;

    VTermScreenCell *rowdata=grab_row(trow,&dont_free);

    if(rowdata != 0) draw_row(rowdata,row*(font_height+font_space));

    int cursorx=0;
    int cursory=0;
    cursor_position(&cursorx,&cursory);
    if(cursory == trow) {
      int width=font_width+font_space;
      if((cursorx < cols) && (cursory < rows) && (rowdata != 0)) {
        if(rowdata[cursorx].width == 2) width+=(font_width+font_space);
        nsdl_rectangle_softalph(screen,cursorx*(font_width+font_space),row*(font_height+font_space),(cursorx*(font_width+font_space))+width,(row*(font_height+font_space))+(font_height+font_space),0xFF);
        nsdl_rectangle_wire    (screen,cursorx*(font_width+font_space),row*(font_height+font_space),(cursorx*(font_width+font_space))+width,(row*(font_height+font_space))+(font_height+font_space),UINT_MAX);
      }
    }

    if((rowdata != 0) && (dont_free==false)){free(rowdata); rowdata=0;}
  }

  if(draw_selection) {

    int text_start_x;
    int text_start_y;
    int text_end_x;
    int text_end_y;
    select_end_scroll_offset = scroll_offset;
    mouse_to_select_box(select_start_x,select_start_y,select_start_scroll_offset,
                          select_end_x,  select_end_y,  select_end_scroll_offset,
                         &text_start_x, &text_start_y, &text_end_x, &text_end_y);

    text_end_y   += scroll_offset;
    text_start_y += scroll_offset;

    if(text_end_x<text_start_x) {int c=text_end_x; text_end_x=text_start_x;text_start_x=c;}
    if(text_end_y<text_start_y) {int c=text_end_y; text_end_y=text_start_y;text_start_y=c;}


    nsdl_rectangle_wire(screen,text_start_x*(font_width+font_space),text_start_y*(font_height+font_space),
                                 text_end_x*(font_width+font_space),text_end_y*(font_height+font_space),0xFFFFFF);
  }
  
  SDL_UnlockSurface(screen);
  regis_render();
  inline_data_render();
  ngui_render(screen);

  SDL_Flip(screen);

  SDL_mutexV(screen_mutex);
}


void sdl_render_thread() {
  
  if(SDL_Init(SDL_INIT_VIDEO)<0) {
    printf("Initialisation failed");
    return;
  }

  // initialise SDL rendering
  //const SDL_VideoInfo *vid = SDL_GetVideoInfo();
  //int maxwidth  = vid->current_w;
  //int maxheight = vid->current_h-(font_height+font_space);

  screen=SDL_SetVideoMode(640,480,32,SDL_RESIZABLE | SDL_DOUBLEBUF);
  if(screen==NULL) {
    printf("Failed SDL_SetVideoMode: %d",SDL_GetError());
    SDL_Quit();
    return;
  }
  
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(500,50);
  
  terminal_resize(screen,vt,&cols,&rows);
  regis_init(screen->w,screen->h);
  inline_data_init(screen->w,screen->h);


  sdl_init_complete=true;
  for(;;) {
    SDL_SemWait(redraw_sem);
    redraw_screen();
  }
}

void redraw_required() {
  uint32_t v =  SDL_SemValue(redraw_sem);
  if(v > 5) return;

  SDL_SemPost(redraw_sem);
}

void console_read_thread() {
  for(;;) {
    // sending bytes from pts to vterm
    int len;
    char buffer[10241];
    len = c_read(buffer, sizeof(buffer)-1);
    
    if(len < 0) {
 //     if(errno == EIO) {
        hterm_quit = true;
        SDL_CondSignal(cond_quit);

        break;
 //     }
    }

    if(len > 0) {
      inline_data_receive(buffer,len);
      SDL_mutexP(vterm_mutex);
      if((buffer != 0) && (len != 0)) {
        vterm_push_bytes(vt, buffer, len);
      }
      SDL_mutexV(vterm_mutex);
    }
    redraw_required();
  }
}

uint8_t *paste_text() {

  uint8_t *paste_data = malloc(sizeof(uint8_t)*10240);

  FILE *r1 = popen("xclip -o","r");
  if(r1!=NULL) {

    for(size_t n=0;feof(r1) == false;n++) {
      int c = fgetc(r1);
      if(!feof(r1)) {
        paste_data[n] = c;
        paste_data[n+1] = 0;
      }
    }

    pclose(r1);
  }

  return paste_data;
}

void copy_text(uint16_t *itext,int len) {
  
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

void get_text_region(int text_start_x,int text_start_y,int text_end_x,int text_end_y,uint16_t **itext,int *ilen) {

  int len=0;
  uint16_t *text = malloc(10240);
  for(int y=text_start_y;y<text_end_y;y++) {
    bool dont_free=false;
    VTermScreenCell *row_data = grab_row(y,&dont_free);
    if(row_data == 0) { text[0]=0; }
    else {
      for(int x=text_start_x;x<text_end_x;x++) {

        text[len] = row_data[x].chars[0];
        if(text[len]==0) text[len]=' ';
        len++;
      }
    }
    if(!dont_free) free(row_data);

    text[len] = '\n';
    len++;
  }
  text[len]=0;

  *itext = text;
  *ilen  = len;
}

void process_mouse_event(SDL_Event *event) {
  
  if((event->type != SDL_MOUSEMOTION) && (event->type != SDL_MOUSEBUTTONUP) && (event->type != SDL_MOUSEBUTTONDOWN)) return;

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

      printf("motion: %d %d\n",event->button.x,event->button.y);
      //if(event->button.y <= 0            ) scroll_offset++;
      //if(event->button.y >= (screen->h-1)) {if(scroll_offset != 0) scroll_offset--;}

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
    select_end_scroll_offset = scroll_offset;
    mouse_to_select_box(select_start_x,select_start_y,select_start_scroll_offset,
                          select_end_x  ,select_end_y,select_end_scroll_offset,
                         &text_start_x, &text_start_y, &text_end_x, &text_end_y);

    uint16_t *text=0;
    int      len=0;

    if(text_end_x<text_start_x) {int c=text_end_x; text_end_x=text_start_x;text_start_x=c;}
    if(text_end_y<text_start_y) {int c=text_end_y; text_end_y=text_start_y;text_start_y=c;}

    printf("copy: %d %d %d %d\n",text_start_x,text_start_y,text_end_x,text_end_y);
    get_text_region(text_start_x,text_start_y,text_end_x,text_end_y,&text,&len);

    if(len != 0) copy_text(text,len);
    if(text != 0) free(text);
    redraw_required();
  } else
  if(event->type == SDL_MOUSEBUTTONDOWN) {
    select_start_scroll_offset = scroll_offset;
    select_start_x = event->button.x;
    select_start_y = event->button.y;
    select_end_x = event->button.x;
    select_end_y = event->button.y;
    draw_selection = true;
  }
}


void sdl_read_thread() {

  for(;sdl_init_complete==false;){}

  for(;;) {
    // sending bytes from SDL to pts
    SDL_Event event;
    SDL_WaitEvent(&event);
    process_mouse_event(&event);
    ngui_receive_event(&event);
    
    uint8_t *keystate;// = SDL_GetKeyState(NULL); // FIXFIXFIXFIXFIXFIX SDL1.3

    if(event.type == SDL_QUIT) {
      hterm_quit = true;
      SDL_CondSignal(cond_quit);
      return;
    }

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
        c_write(buf,3);


      } else 
      if(event.key.keysym.sym == SDLK_RIGHT) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'C';
        buf[3] = 0;
        c_write(buf,3);
      } else 
      if(event.key.keysym.sym == SDLK_UP) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'A';
        buf[3] = 0;
        c_write(buf,3);
      } else 
      if(event.key.keysym.sym == SDLK_DOWN) {
        char buf[4];
        buf[0] = 0x1b;
        buf[1] = 'O';
        buf[2] = 'B';
        buf[3] = 0;
        c_write(buf,3);
      } else
      if((event.key.keysym.sym == SDLK_p) && (keystate[SDLK_LCTRL])) {

        // perform text paste
        uint8_t *text = paste_text();
        if(text != 0) {
          c_write(text,strlen(text));
          free(text);
        }
      } else {
 
        // normal character
        char buf[2];
        buf[0] = event.key.keysym.unicode;
        buf[1]=0;
        if(buf[0] != 0) {
          c_write(buf,1);
        }
      }
    }


    if(event.type == SDL_VIDEORESIZE) {
      printf("resize detected A\n");
      new_screen_size_x = event.resize.w;
      new_screen_size_y = event.resize.h;
      new_screen_size   = true;

      redraw_required();
    }
  }
}

void timed_repeat() {

  for(;;) {
    SDL_Delay(100);
    if(draw_selection == true) {

      if(select_end_y <= 0            ) scroll_offset++;
      if(select_end_y >= (screen->h-1)) {if(scroll_offset != 0) scroll_offset--;}

      redraw_required();
    }

    nunifont_blinktimer();
    if(any_blinking) redraw_required();
  }

}

void vterm_initialisation() {
  vt=0;

  printf("init rows: %d cols: %d\n",rows,cols);
  vt = vterm_new(rows, cols);

  vts = vterm_obtain_screen(vt);
  vs  = vterm_obtain_state(vt);
  vterm_state_set_bold_highbright(vs,1);

  vterm_screen_enable_altscreen(vts,1);

  vterm_screen_set_callbacks(vts, &cb_screen, NULL);

  vterm_state_set_backup_callbacks(vs,&cb_state,0);

  vterm_screen_set_damage_merge(vts, VTERM_DAMAGE_SCROLL);
  vterm_set_parser_backup_callbacks(vt , &cb_parser, NULL);

  vterm_screen_reset(vts, 1);
  vterm_parser_set_utf8(vt,1); // should be vts?
}


bool ssh_received = false;
char ssh_hostname[100];
char ssh_username[100];
char ssh_password[100];

int prompt_id = 0;

void receive_ssh_info(char *o1,char *o2,char *o3) {

  strcpy(ssh_hostname,o1);
  strcpy(ssh_username,o2);
  strcpy(ssh_password,o3);

  ssh_received=true;

  ngui_delete_info_prompt(prompt_id);
}

int main(int argc, char **argv) {

  regis_mutex  = SDL_CreateMutex();
  screen_mutex = SDL_CreateMutex();
  vterm_mutex  = SDL_CreateMutex();
  quit_mutex   = SDL_CreateMutex();
  redraw_sem   = SDL_CreateSemaphore(1);
  inline_data_mutex = SDL_CreateMutex();
  cond_quit = SDL_CreateCond();

  int connection_type = CONNECTION_LOCAL; // replace with commandline lookup
  if(argc > 1) {
    if(strcmp(argv[1],"ssh") == 0) {
      connection_type = CONNECTION_SSH; // replace with commandline lookup
    }
  }
    
  #ifdef IPHONE_BUILD
    connection_type = CONNECTION_SSH;
  #endif

  //char *open_arg1 = "localhost";
  char open_arg1[100];// = "127.0.0.1        ";
  char open_arg2[100];// = "new              ";
  char open_arg3[100];// = "password         ";
  
  rows = 10;
  cols = 10;
  vterm_initialisation();
  SDL_Thread *thread2 = SDL_CreateThread(sdl_render_thread  ,0,0);
  SDL_Thread *thread1 = SDL_CreateThread(sdl_read_thread    ,0,0);

  for(;sdl_init_complete == false;);
  ngui_set_screen(screen, redraw_required);

  if(connection_type == CONNECTION_LOCAL) {
    c_open   = &local_open;
    c_close  = &local_close;
    c_write  = &local_write;
    c_read   = &local_read;
    c_resize = &local_resize;
  } else
  if(connection_type == CONNECTION_SSH) {
    c_open   = &ssh_open;
    c_close  = &ssh_close;
    c_write  = &ssh_write;
    c_read   = &ssh_read;
    c_resize = &ssh_resize;

    // we now need to read connection information.
    
    prompt_id = ngui_add_info_prompt(-1,-1,
                                     "hostname:","username:","password:",
                                     0,0,1,
                                     receive_ssh_info);

    for(;ssh_received == false;);

    strcpy(open_arg1,ssh_hostname);
    strcpy(open_arg2,ssh_username);
    strcpy(open_arg3,ssh_password);

    printf("arg1: %s\n",open_arg1);
    printf("arg2: %s\n",open_arg2);
    printf("arg3: %s\n",open_arg3);
  }

  int open_ret = c_open(open_arg1,open_arg2,open_arg3);

  
  SDL_Thread *thread3;
  SDL_Thread *thread5;
  if(open_ret >= 0) {
    SDL_Thread *thread3 = SDL_CreateThread(console_read_thread,0,0);
    SDL_Thread *thread5 = SDL_CreateThread(timed_repeat       ,0,0);

    SDL_mutexP(quit_mutex);

    SDL_CondWait(cond_quit,quit_mutex);
  } else {
    printf("Unable to connect\n");
  }

  SDL_Quit();

  c_close();
  vterm_free(vt);
 
  return 0;
}
