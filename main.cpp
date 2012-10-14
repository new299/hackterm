#include <string.h>
#include <SDL/SDL.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
#include "nunifont.h"

int main(int argc, char **argv) {

  SDL_Surface *screen;
 
  if(SDL_Init(SDL_INIT_VIDEO)<0) {
    cout << "Failed SDL_Init " << SDL_GetError() << endl;
    return 1;
  }
 
  screen=SDL_SetVideoMode(1200,800,32,SDL_ANYFORMAT);
  if(screen==NULL) {
    cout << "Failed SDL_SetVideoMode: " << SDL_GetError() << endl;
    SDL_Quit();
    return 1;
  }
   
  // grab pts
  int fd = open("/dev/ptmx",O_RDWR | O_NOCTTY | O_NONBLOCK);

 // int fd = getpt();
  printf("fd: %d",fd);

  grantpt(fd);
  unlockpt(fd);
 
  SDL_EnableUNICODE(1);

  int x=0;int y=0;
  for(;;) {

    char buf[10];
    int i = read(fd,buf,1);
    if(i != -1) {
      if(buf[0] >= 0) {
				uint16_t text[50];
				text[0] = buf[0];
				text[1]=0;
				draw_unitext(screen,x,y,text,0);
				x += 16 + 2;
				if(x > 900) { x = 0; y+=18;}
				if(y>800) {
          y=0;
          SDL_FillRect(screen,NULL, 0x000000); 
        }

				printf("r: %d\n",buf[0]);
        if(buf[0] == 10) {
          x =0;
          y +=18; 
        }
      }
    }


    SDL_Event event;
    if(SDL_PollEvent(&event))
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_LEFT) exit(0);
 
      buf[0] = event.key.keysym.unicode;
      printf("key: %u\n",buf[0]);
      write(fd,buf,1);
    }



    SDL_Flip(screen);
    SDL_LockSurface(screen);


    SDL_UnlockSurface(screen);
  }
  SDL_Quit();
 
  return 0;

}


