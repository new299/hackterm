#include "virtual_buttons.h"
#include <SDL.h>
#include <stdbool.h>
#include "ngui_button.h"
#include "nunifont.h"

extern bool hterm_next_key_ctrl;
extern bool hterm_next_key_alt;
extern int  display_width;
extern int  display_height;
extern int  display_width_abs;
extern int  display_height_abs;

extern int (*c_open)(char *hostname,char *username, char *password,char *fingerprintstr,char *key1,char *key2);
extern int (*c_close)();
extern int (*c_write)(char *bytes,int len);
extern int (*c_read)(char *bytes,int len);
extern int (*c_resize)(int rows,int cols);

void virtual_kb_up(char *c) {
  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_UP);
  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_UP);
}

void virtual_kb_down(char *c) {
  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_DOWN);
  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_DOWN);
}

void virtual_kb_left(char *c) {
  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_LEFT);
  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_LEFT);
}

void virtual_kb_right(char *c) {
  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_RIGHT);
  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_RIGHT);
}

void virtual_kb_esc(char *c) {
  char text[5];
  text[0] = 27;
  text[1] = 0;
  SDL_SendKeyboardText(text);
  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_ESCAPE);
  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_ESCAPE);
}

void virtual_kb_ctrl(char *c) {
  hterm_next_key_ctrl=true;
//  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_CTRL);
}

void virtual_kb_alt(char *c) {
  hterm_next_key_alt=true;
//  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_RALT);
//  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_RALT);
}

void virtual_kb_tab(char *c) {
  char text[5];
  text[0] = '\t';
  text[1] = 0;
  SDL_SendKeyboardText(text);
  SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_TAB);
  SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_TAB);
}

void virtual_kb_paste(char *c) {
  // perform text paste
  uint8_t *text = paste_text();
  if(text != 0) {
    c_write(text,strlen(text));
  }
}

void virtual_kb_kbshow(char *c) {
  SDL_StartTextInput();
}

void virtual_kb_close(char *c) {
  c_close();
}

void virtual_buttons_add() {
  // set intial button positions, these should get overwritten almost right away anyway.
  int dwidth  = display_width -(display_width %16);
  int dheight = display_height-(display_height%16);
  ngui_add_button(dwidth-(16*6*3),dheight-(16*6*3),"Iesc"  ,virtual_kb_esc  );
  ngui_add_button(dwidth-(16*6*3),dheight-(16*6*1),"Ialt"  ,virtual_kb_alt  );
  ngui_add_button(dwidth-(16*6*1),dheight-(16*6*3),"Ictrl" ,virtual_kb_ctrl );
  ngui_add_button(dwidth-(16*6*1),dheight-(16*6*1),"Itab"  ,virtual_kb_tab  );

  ngui_add_button(dwidth-(16*6*2),dheight-(16*6*3),"Iup"   ,virtual_kb_up   );
  ngui_add_button(dwidth-(16*6*2),dheight-(16*6*1),"Idown" ,virtual_kb_down );
  ngui_add_button(dwidth-(16*6*3),dheight-(16*6*2),"Ileft" ,virtual_kb_left );
  ngui_add_button(dwidth-(16*6*1),dheight-(16*6*2),"Iright",virtual_kb_right);

  ngui_add_button(dwidth-(16*6*2),dheight-(16*6*2),"Ipaste",virtual_kb_paste);

  // check if close overlaps with escape
  if((display_height-(16*6*3)) > 80) {
    ngui_add_button(dwidth-(16*6*1),               0,"Iclose" ,virtual_kb_close);
  } else {
    ngui_add_button(dwidth-(16*6*4),dheight-(16*6*3),"Iclose" ,virtual_kb_close);
  }
  ngui_add_button(display_width_abs-(16*7),display_height_abs-(5*16),"Ikbshow",virtual_kb_kbshow);

  virtual_buttons_reposition();
}

void virtual_buttons_disable() {
  ngui_move_button("Iesc"   ,-1000,-1000);
  ngui_move_button("Ialt"   ,-1000,-1000);
  ngui_move_button("Ictrl"  ,-1000,-1000);
  ngui_move_button("Itab"   ,-1000,-1000);
      
  ngui_move_button("Iup"    ,-1000,-1000);
  ngui_move_button("Idown"  ,-1000,-1000);
  ngui_move_button("Ileft"  ,-1000,-1000);
  ngui_move_button("Iright" ,-1000,-1000);
      
  ngui_move_button("Ipaste" ,-1000,-1000);
  ngui_move_button("Ikbshow",-1000,-1000);
}

void virtual_buttons_reposition() {
  int doublewidth = nunifont_width*2;

  int dwidth  = display_width -(display_width %(nunifont_width*2));
  int dheight = display_height-(display_height%(nunifont_height));
  ngui_move_button("Iesc"  ,dwidth-(doublewidth*6*3),dheight-(nunifont_height*6*3));
  ngui_move_button("Ialt"  ,dwidth-(doublewidth*6*3),dheight-(nunifont_height*6*1));
  ngui_move_button("Ictrl" ,dwidth-(doublewidth*6*1),dheight-(nunifont_height*6*3));
  ngui_move_button("Itab"  ,dwidth-(doublewidth*6*1),dheight-(nunifont_height*6*1));
      
  ngui_move_button("Iup"   ,dwidth-(doublewidth*6*2),dheight-(nunifont_height*6*3));
  ngui_move_button("Idown" ,dwidth-(doublewidth*6*2),dheight-(nunifont_height*6*1));
  ngui_move_button("Ileft" ,dwidth-(doublewidth*6*3),dheight-(nunifont_height*6*2));
  ngui_move_button("Iright",dwidth-(doublewidth*6*1),dheight-(nunifont_height*6*2));
      
  ngui_move_button("Ipaste",dwidth-(doublewidth*6*2),dheight-(nunifont_height*6*2));
  
  // check if close overlaps with escape
  if((display_height-(nunifont_height*6*3)) > 80) {
    ngui_move_button("Iclose",dwidth-(doublewidth*6*1)     ,0);
  } else {
    ngui_move_button("Iclose",dwidth-(doublewidth*6*4),dheight-(nunifont_height*6*3));
  }
  ngui_move_button("Ikbshow",display_width_abs-(doublewidth*7),display_height_abs-(5*nunifont_height));
}
