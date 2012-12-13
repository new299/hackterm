#include <stdbool.h>
#include <SDL/SDL.h>       
#include <SDL/SDL_thread.h>
#include <time.h>

extern SDL_Surface *inline_data_layer;
extern SDL_mutex   *inline_data_mutex;

void inline_data_init   (int width ,int height);
int  inline_data_receive(char *data,int length);

