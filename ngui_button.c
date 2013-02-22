#include <string.h>
#include <SDL/SDL.h>
#include "nsdl.h"
#include "nunifont.h"
#include "ngui.h"
#include <stdbool.h>

typedef struct {
  bool valid;
  int x;
  int y;
  int x_padding;
  int y_padding;
  char text[100];
  void (*callback)(char *);
} ngui_button_data;

int ngui_buttons_size = 0;
ngui_button_data ngui_buttons[50];

void ngui_receive_event_button(SDL_Event *event, ngui_button_data *d) {

  printf("button received event\n");
  printf("event type: %x\n",event->type);
  if(event->type == SDL_MOUSEBUTTONDOWN) {
    printf("button received button down\n");
    printf("%d\n", event->button.x);
    printf("%d\n", event->button.y);

    int x = event->button.x;
    int y = event->button.y;
    
    if(d->text[0] == 'I') {
      if((x > (d->x)) && (x < ((d->x)+(16*6))) &&
         (y > (d->y)) && (y < ((d->y)+(16*6)))) {
        d->callback("press");
      }
    }
    
    if((x > (d->x-d->x_padding)) && (x < ((d->x)+(strlen(d->text)*8)+d->x_padding)) &&
       (y > (d->y-d->y_padding)) && (y < ((d->y)+16+d->y_padding))) {
      d->callback("press");
    }
  }
}

int ustrcmp(uint16_t *a,char *b) {

  for(size_t n=0;;n++) {
    if(a[n] != b[n]) return 1;
    if((a[n] == 0) && (b[n] == 0)) return 0;
  }

  return 1;
}

void ngui_move_button(char *name,int nx,int ny) {

  for(int n=0;n<ngui_buttons_size;n++) {
    if(strcmp(ngui_buttons[n].text,name)==0) {
      ngui_buttons[n].x = nx;
      ngui_buttons[n].y = ny;
    }
  }

}

void draw_esc_icon(int x,int y) {


  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);

  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  // E
  SDL_RenderDrawLine(ngui_renderer,x+16,y+(16*0),x   ,y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*0),x   ,y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*2),x+16,y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*1),x+16,y+(16*1));
  
  // S
  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*0),x+(16*2),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*0),x+(16*2),y+(16*1));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*1),x+(16*3),y+(16*1));
  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*1),x+(16*3),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*2),x+(16*2),y+(16*2));
  
  // C
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*0),x+(16*4),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*0),x+(16*4),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*2),x+(16*5),y+(16*2));

}

void draw_tab_icon  (int x,int y){


  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  // T
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*4),x   ,y+(16*4));  // Top line
  SDL_RenderDrawLine(ngui_renderer,x+16    ,y+(16*4),x+16,y+(16*6)-1);
  
  // A
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*4),x+(16*3),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*4),x+(16*3),y+(16*6)-1);
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*4),x+(16*4),y+(16*6)-1);
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*5),x+(16*3),y+(16*5));

  // B
  SDL_RenderDrawLine(ngui_renderer,x+(16*6)-2,y+(16*4),x+(16*5),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*4),x+(16*5),y+(16*6)-1);
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*6)-1,x+(16*6)-2,y+(16*6)-1);
  
  SDL_RenderDrawLine(ngui_renderer,x+(16*6)-1,y+(16*6)-1,x+(16*6)-1,y+(16*5)+2);
  SDL_RenderDrawLine(ngui_renderer,x+(16*6)-1,y+(16*5)-2,x+(16*6)-1,y+(16*4)+1);
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*5),x+(16*6)-2,y+(16*5));
}

void draw_alt_icon  (int x,int y){


  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;

  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  // A
  SDL_RenderDrawLine(ngui_renderer,x+16,y+(16*4),x   ,y+(16*4));  // Top line
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*4),x   ,y+(16*6)-1);  // Down line
//  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*6),x+16,y+(16*6)); //Bottom line
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*5),x+16,y+(16*5));  // Midline
  SDL_RenderDrawLine(ngui_renderer,x+16,y+(16*4),x+16,y+(16*6)-1);  // Right down line
  
  // L
//  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*4),x+(16*2),y+(16*4)); top
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*4),x+(16*2),y+(16*6)-1); // left
//  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*5),x+(16*3),y+(16*5));
//  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*5),x+(16*3),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*6)-1,x+(16*2),y+(16*6)-1); //bottom
  
  // T
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*4),x+(16*4),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*4),x+(16*5),y+(16*6)-1);
//  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*6),x+(16*5),y+(16*6));

}


void draw_ctrl_icon (int x,int y){


  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  // C
  SDL_RenderDrawLine(ngui_renderer,x+16,y+(16*0),x   ,y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*0),x   ,y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x   ,y+(16*2),x+16,y+(16*2));
  
  // T
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*0),x+(16*2)   ,y+(16*0));  // Top line
  SDL_RenderDrawLine(ngui_renderer,x+(16*3),y+(16*0),x+(16*3),y+(16*2));

  // L
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*0),x+(16*5),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*2),x+(16*6),y+(16*2));

}

void draw_up_icon   (int x,int y){

  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y       ,x+(16*4),y       );
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y       ,x+(16*4),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*2),x+(16*5),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*2),x+(16*5),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*4),x+(16*6),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*4),x+(16*6),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*6),x+(16*0),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*6),x+(16*0),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*4),x+(16*1),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*1),y+(16*4),x+(16*1),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*1),y+(16*2),x+(16*2),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*2),x+(16*2),y+(16*0));
}

void draw_down_icon (int x,int y){


  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*6)-1,x+(16*4),y+(16*6)-1);
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*6)-1,x+(16*4),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*4),x+(16*5),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*4),x+(16*5),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*5),y+(16*2),x+(16*6),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*2),x+(16*6),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*0),x+(16*0),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*0),x+(16*0),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*2),x+(16*1),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*1),y+(16*2),x+(16*1),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*1),y+(16*4),x+(16*2),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*4),x+(16*2),y+(16*6)-1);
}

void draw_right_icon (int x,int y){

  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  SDL_RenderDrawLine(ngui_renderer,x+(16*6)-1,y+(16*2),x+(16*6)-1,y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*4),x+(16*4),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*4),x+(16*4),y+(16*5));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*5),x+(16*2),y+(16*5));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*5),x+(16*2),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*6),x+(16*0),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*6),x+(16*0),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*0),x+(16*2),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*0),x+(16*2),y+(16*1));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*1),x+(16*4),y+(16*1));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*1),x+(16*4),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*2),x+(16*6),y+(16*2));

}

void draw_left_icon(int x,int y){
  

  SDL_Rect rect;
  
  rect.x = x;
  rect.y = y;
  rect.w = 6*16;
  rect.h = 6*16;
  
  SDL_SetRenderDrawColor(ngui_renderer,0x50,0x50,0x50,0xFF);
  SDL_RenderDrawRect(ngui_renderer,&rect);
  
  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*4),x+(16*0),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*0),y+(16*2),x+(16*2),y+(16*2));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*2),x+(16*2),y+(16*1));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*1),x+(16*4),y+(16*1));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*1),x+(16*4),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*0),x+(16*6),y+(16*0));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*0),x+(16*6),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*6),y+(16*6),x+(16*4),y+(16*6));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*6),x+(16*4),y+(16*5));
  SDL_RenderDrawLine(ngui_renderer,x+(16*4),y+(16*5),x+(16*2),y+(16*5));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*5),x+(16*2),y+(16*4));
  SDL_RenderDrawLine(ngui_renderer,x+(16*2),y+(16*4),x+(16*0),y+(16*4));
  
}

void draw_menu_icon (int x,int y){}

void ngui_render_button(ngui_button_data *d) {

  uint16_t text[100];
  for(int n=0;n<100;n++) text[n] = d->text[n];

  SDL_SetRenderDrawColor(ngui_renderer,0xA0,0xA0,0xA0,0xFF);
  
  bool notext=false;
  if(ustrcmp(text,"Iesc" )  == 0) { draw_esc_icon  (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Itab" )  == 0) { draw_tab_icon  (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Ialt" )  == 0) { draw_alt_icon  (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Ictrl")  == 0) { draw_ctrl_icon (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Iup"  )  == 0) { draw_up_icon   (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Idown")  == 0) { draw_down_icon (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Ileft")  == 0) { draw_left_icon (d->x,d->y); notext=true;}
  if(ustrcmp(text,"Iright") == 0) { draw_right_icon(d->x,d->y); notext=true;}
  if(ustrcmp(text,"Imenu")  == 0) { draw_menu_icon (d->x,d->y); notext=true;}
    

//  nsdl_rectangle_shade(ngui_screen,d->x-d->x_padding,d->y-d->y_padding,d->x+(strlen(d->text))*8+d->x_padding,d->y+16+d->y_padding,1000,10000);

  if(!notext) {
    SDL_Rect rect;
  
    rect.x = d->x-d->x_padding;
    rect.y = d->y-d->y_padding;
    rect.w = (strlen(d->text))*8+d->x_padding;
    rect.h = 16+d->y_padding;
  
    SDL_RenderDrawRect(ngui_renderer,&rect);
  
    draw_unitext_renderer(ngui_renderer,
                d->x,
                d->y,
                text,
                0,
                65535,0,0,0,0);
  }
}

int ngui_add_button(int x,int y,char *text,void *callback) {

  ngui_buttons[ngui_buttons_size].valid=true;
  ngui_buttons[ngui_buttons_size].x = x;
  ngui_buttons[ngui_buttons_size].y = y;
  ngui_buttons[ngui_buttons_size].x_padding = 10;
  ngui_buttons[ngui_buttons_size].y_padding = 10;
  strcpy(ngui_buttons[ngui_buttons_size].text,text);
  ngui_buttons[ngui_buttons_size].callback = callback;

  ngui_buttons_size++;

  return ngui_buttons_size-1;
}

void ngui_delete_button(int id) {

  ngui_buttons[id].valid = false;

}

void ngui_receiveall_button(SDL_Event *event) {
  for(int n=0;n<ngui_buttons_size;n++) {
    ngui_button_data *d = &ngui_buttons[n];
    if(d->valid) {
      ngui_receive_event_button(event,d);
    }
  }
}

void ngui_renderall_button() {
  for(int n=0;n<ngui_buttons_size;n++) {
    ngui_button_data *d = &ngui_buttons[n];
    if(d->valid) {
      ngui_render_button(d);
    }
  }
}
