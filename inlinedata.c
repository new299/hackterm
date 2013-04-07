#include "inlinedata.h"
#include <stdbool.h>
#include <SDL/SDL.h>       
#include <SDL/SDL_thread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nsdl.h"
#include "base64.h"

#define PNG_DEBUG 3
#include <png.h>

SDL_Surface *inline_data_layer = 0;

char *inline_magic = "HTERMFILEXFER";

void inline_data_init(int swidth,int sheight) {

  base64_init();
  inline_data_layer = SDL_CreateRGBSurface(SDL_SWSURFACE,swidth,sheight,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
}

void inline_data_resize(int swidth,int sheight) {
  SDL_FreeSurface(inline_data_layer);
  inline_data_layer = SDL_CreateRGBSurface(SDL_SWSURFACE,swidth,sheight,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
}

int width, height;
int pixel_depth;

png_structp Gpng_ptr=0;
png_infop   Ginfo_ptr=0;
png_bytep  *row_pointers=0;
bool processing_png=false;
int file_end=0;
png_byte channels=0;
png_byte color_type=0;
png_colorp palette;
int num_palette;

/* This function is called (as set by png_set_progressive_read_fn() above) when enough data has been supplied so all of the header has been read.  */
void info_callback(png_structp png_ptr, png_infop info) {

  file_end=0;
  width = png_get_image_width(png_ptr,info);
  height = png_get_image_height(png_ptr,info);
  pixel_depth = png_get_bit_depth(png_ptr,info);
  
  channels = png_get_channels(png_ptr,info);
  
  color_type = png_get_color_type(png_ptr,info);
  if(color_type == PNG_COLOR_TYPE_GRAY)      {}
  if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA){}
  if(color_type == PNG_COLOR_TYPE_RGB)       {}
  if(color_type == PNG_COLOR_TYPE_RGB_ALPHA) {}
  if(color_type == PNG_COLOR_TYPE_PALETTE )  {

    int r = png_get_PLTE(png_ptr,info,&palette,&num_palette);
    if(r == 0) {
    }
    
    png_uint_16p histogram = NULL;

    png_get_hIST(png_ptr, info, &histogram);
    png_set_expand(png_ptr);
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_read_update_info(png_ptr, info);
    pixel_depth = 8;
  }

  int row_bytes = png_get_rowbytes(png_ptr,info);

  row_pointers = malloc(sizeof(png_bytep *) * height);
  for(size_t n=0;n<height;n++) {
    row_pointers[n] = malloc(row_bytes);
  }

  png_start_read_image(png_ptr);
}

int32_t inlineget_pixel(void *row,int pixel_depth,int idx) {

  if(pixel_depth == 1) {
    int pos  = pixel_depth*idx;

    int byte = pos/8;
    int bit  = pos-((pos/8)*8);

    int value = 0;
    for(int n=0;n<pixel_depth;n++) {
      value = value << 1;
      if(((uint8_t *)row)[byte] & (1 << (8-bit))) value |= (value + 1);
      bit++;
      if(bit > 8) {bit=0; byte++;}
    }
    return value;
  }
  
  if(pixel_depth == 8) {
    return ((uint32_t *)row)[idx];
  }
}

/* This function is called when each row of image data is complete */
void row_callback(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass) {

  if(row_num >= height) {
    // bad row number
  }

  png_progressive_combine_row(png_ptr, row_pointers[row_num], new_row);
  for(int n=0;n<width;n++) {
    uint32_t pixel = inlineget_pixel(new_row,pixel_depth,n);
    
    if(pixel_depth == 1) {
      if(pixel!=0) pixel = 0xFFFFFFFF;
    }

    nsdl_pointS(inline_data_layer,n,row_num,pixel);
  }
}

void png_cleanup() {

  png_destroy_read_struct(&Gpng_ptr, &Ginfo_ptr, (png_infopp)NULL);
  Gpng_ptr  = NULL;
  Ginfo_ptr = NULL;
  file_end=1;
}

void end_callback(png_structp png_ptr, png_infop info) {
  png_cleanup();

}

int initialize_png_reader() {
  Gpng_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
  if(!Gpng_ptr) return 1;
  Ginfo_ptr = png_create_info_struct(Gpng_ptr);
  if(!Ginfo_ptr) {
    png_destroy_read_struct(&Gpng_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return 1;
  }
  if(setjmp(png_jmpbuf(Gpng_ptr))) {
    png_destroy_read_struct(&Gpng_ptr, &Ginfo_ptr, (png_infopp)NULL);
    return 1;
  }

  png_set_progressive_read_fn(Gpng_ptr, (void *)NULL, info_callback, row_callback, end_callback);
  return 0;
}

/* A code fragment that you call as you receive blocks of data */
int inlinepng_process_data(png_bytep buffer, png_uint_32 length) {
  if(Gpng_ptr == 0) {
    png_cleanup();
    return 0;
  };
  
  if (setjmp(png_jmpbuf(Gpng_ptr))) {
    png_destroy_read_struct(&Gpng_ptr, &Ginfo_ptr, (png_infopp)NULL);
    return 1;
  }

  png_process_data(Gpng_ptr, Ginfo_ptr, buffer, length);
  
  return 0;
}

#define BUFFERSIZE 10241

unsigned char buffer[BUFFERSIZE];
int  buffer_size=0;

void buffer_shift() {

  for(int n=0;n<BUFFERSIZE-1;n++) {
    buffer[n] = buffer[n+1];
  }

}

void buffer_push(char *data,int length) {

  for(int n=0;n<length;n++) {
    if(buffer_size < BUFFERSIZE) {
      buffer[buffer_size] = data[n];
      buffer_size++;
    } else {
      buffer_shift();
      buffer[BUFFERSIZE-1] = data[n];
    }
  }

}

void buffer_clear() {
  buffer_size=0;
}

int buffer_search(char *v) {

  if((buffer_size-((int)strlen(v))) < 0) return -1;
  // shameful search
  for(int n=0;n<(buffer_size-strlen(v));n++) {
    if(strncmp(v,buffer+n,strlen(v)) == 0) {
      return n;
    }
  }

  return -1;
}

int inline_data_receive(char *data,int length) {

  if(processing_png == true) {
    if(file_end==1) {processing_png=false; return 1;}
    char decoded_buffer[10240]; // should be malloc'd based on length.
    bool failflag;
    int decoded_buffer_size = base64_decode(data,length,decoded_buffer,&failflag);
 
    if(decoded_buffer_size != 0) {
      inlinepng_process_data(decoded_buffer,decoded_buffer_size);
      if(file_end==1) { processing_png=false; return 1; }
    }

    if(failflag == true) {
      file_end =1;
      processing_png=false;
      return 2;
    }
    if(file_end==1) processing_png=false;
    return 1;
  }

  file_end=0;
  buffer_push(data,length);
  int pos = buffer_search(inline_magic);

  if(pos < 0) return 0;

  processing_png=true;

  base64_init();
  initialize_png_reader();
  char decoded_buffer[4096]; // should be malloc'd based on length.
  bool failflag;
  int decoded_buffer_size = base64_decode(buffer+pos+strlen(inline_magic),buffer_size-pos-strlen(inline_magic),decoded_buffer,&failflag);
  if(decoded_buffer_size != 0) {
    inlinepng_process_data(decoded_buffer,decoded_buffer_size);
  }
  buffer_clear();

  if(failflag) {
    processing_png=false;
    return 0;
  }
  return 2;
}

void inline_data_clear() {
  SDL_FillRect(inline_data_layer,NULL, 0x000000);
}
