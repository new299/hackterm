#include <string.h>
#include <SDL/SDL.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>

using namespace std;
#include "nfont.h"

int main(int argc, char **argv) {


  nfont_init();

  SDL_Surface *screen;
 
  if(SDL_Init(SDL_INIT_VIDEO)<0) {
    cout << "Failed SDL_Init " << SDL_GetError() << endl;
    return 1;
  }
 
  screen=SDL_SetVideoMode(800,600,32,SDL_ANYFORMAT);
  if(screen==NULL) {
    cout << "Failed SDL_SetVideoMode: " << SDL_GetError() << endl;
    SDL_Quit();
    return 1;
  }
 
  int x=0;int y=0; int s=1;
  for(;;) {
    SDL_Flip(screen);
    SDL_LockSurface(screen);

    uint16_t text[50];

    for(int n=0;n<16;n++) text[n] = s+n;
    text[16]=0;
    draw_text(screen,x,y,text,0);

    x += 16*16 + 2;
    if(x > 500) { x = 0; y+=18;}
    SDL_UnlockSurface(screen);
    s+=16;
    if(y>500) {x=0;y=0;}
    for(int n=0;n<1000000;n++) { 
    for(int n=0;n<100;n++) { }
    }
  }
  SDL_Quit();
 
  return 0;

}
