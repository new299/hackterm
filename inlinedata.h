#include <stdbool.h>
#include <SDL.h>
#include <time.h>

extern SDL_Surface *inline_data_layer;

void inline_data_init   (int width,int height);
void inline_data_resize (int width,int height);
int  inline_data_receive(char *data,int length);
void inline_data_clear();

