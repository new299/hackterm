#ifndef NUNIFONT
#define NUNIFONT

#include <SDL.h>

// These funtions are used for loading and saving static font data, rather than using the unifont hex files.
void nunifont_init(); // usually init is automatically called if required, this require unifont.hex in the same directory.
void nunifont_initcache();
void nunifont_load_staticmap(void *fontmap_static,void *widthmap_static,int fontmap_static_size,int widthmap_static_size);
void nunifont_save_staticmap(char *fontmap_filename,char *widthmap_filename);

void set_system_bg(uint32_t b);
void draw_unitext_surface(void *screen,int x,int y,const uint16_t *text,uint32_t bg,uint32_t fg,int bold,int underline,int italic,int strike);
void draw_unitext_renderer(void *screen,int x,int y,const uint16_t *text,uint32_t bg,uint32_t fg,int bold,int underline,int italic,int strike);
void draw_unitext_fancy_surface(void *screen,int x,int y,const uint16_t *text, uint32_t bg,uint32_t fg, unsigned int bold, unsigned int underline, unsigned int italic, unsigned int blink, unsigned int reverse, unsigned int strike, unsigned int font);
void draw_unitext_fancy_renderer(void *screen,int x,int y,const uint16_t *text, uint32_t bg,uint32_t fg, unsigned int bold, unsigned int underline, unsigned int italic, unsigned int blink, unsigned int reverse, unsigned int strike, unsigned int font);
void nunifont_blinktimer();

#endif
