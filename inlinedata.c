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
SDL_mutex   *inline_data_mutex = 0;

char *inline_magic = "HTERMFILEXFER";

void inline_data_init(int width,int height) {

  base64_init();
  inline_data_layer = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
}


int width, height;
int pixel_depth;

png_structp png_ptr=0;
png_infop   info_ptr=0;
png_bytep  *row_pointers=0;
bool processing_png=false;
int file_end=0;

/* This function is called (as set by png_set_progressive_read_fn() above) when enough data has been supplied so all of the header has been read.  */
void info_callback(png_structp png_ptr, png_infop info) {
/* Do any setup here, including setting any of the transformations mentioned in the Reading PNG files section. For now, you _must_ call either png_start_read_image() or png_read_update_info() after all the transformations are set (even if you don’t set any). You may start getting rows before png_process_data() returns, so this is your last chance to prepare for that.  */

  printf("image height %u\n",info->height);
  printf("image width  %u\n",info->width );
  printf("pixel depth  %u\n",info->pixel_depth);

  width  = info->width;
  height = info->height;
  pixel_depth = info->pixel_depth;

  row_pointers = malloc(sizeof(png_bytep *) * info->height);
  for(size_t n=0;n<info->height;n++) {
    row_pointers[n] = malloc(info->rowbytes);
  }

  png_start_read_image(png_ptr);
}

int inlineget_pixel(char *row,int pixel_depth,int idx) {

  int pos  = pixel_depth*idx;

  int byte = pos/8;
  int bit  = pos-((pos/8)*8);

  int value = 0;
  for(int n=0;n<pixel_depth;n++) {
    value = value << 1;
    if(row[byte] & (1 << (8-bit))) value |= (value + 1);
    bit++;
    if(bit > 8) {bit=0; byte++;}
  }
  return value;
}

/* This function is called when each row of image data is complete */
void row_callback(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass) {
  /* If the image is interlaced, and you turned on the interlace handler, this function will be called for every row in every pass. Some of these rows will not be changed from the previous pass. When the row is not changed, the new_row variable will be NULL. The rows and passes are called in order, so you don’t really need the row_num and pass, but I’m supplying them because it may make your life easier.  For the non-NULL rows of interlaced images, you must call png_progressive_combine_row() passing in the row and the old row. You can call this function for NULL rows (it will just return) and for non-interlaced images (it just does the memcpy for you) if it will make the code easier. Thus, you can just do this for all cases: */

  //printf("read line: %u: ",row_num);
  png_progressive_combine_row(png_ptr, row_pointers[row_num], new_row);
  for(int n=0;n<width;n++) {
    int pixel = inlineget_pixel(row_pointers[row_num],pixel_depth,n);

    if(pixel==1) pixel = 0xFFFFFFFF;
    nsdl_point(inline_data_layer,n,row_num,pixel);

    //if(pixel == 0) printf("0"); else printf("1");
  }
  //printf("\n");
  /* where old_row is what was displayed for previously for the row. Note that the first pass (pass == 0, really) will completely cover the old row, so the rows do not have to be initialized. After the first pass (and only for interlaced images), you will have to pass the current row, and the function will combine the old row and the new row.  */
}

void end_callback(png_structp png_ptr, png_infop info) {
/* This function is called after the whole image has been read, including any chunks after the image (up to and including the IEND). You will usually have the same info chunk as you had in the header, although some data may have been added to the comments and time fields.  Most people won’t do much here, perhaps setting a flag that marks the image as finished.  */
  printf("************************************************************ png processing complete\n");
  file_end=1;
}

/* An example code fragment of how you would initialize the progressive reader in your application. */
int initialize_png_reader() {
  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
  if(!png_ptr) return 1;
  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return 1;
  }
  if(setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return 1;
  }
  /* This one’s new. You can provide functions to be called when the header info is valid, when each row is completed, and when the image is finished. If you aren’t using all functions, you can specify NULL parameters. Even when all three functions are NULL, you need to call png_set_progressive_read_fn(). You can use any struct as the user_ptr (cast to a void pointer for the function call), and retrieve the pointer from inside the callbacks using the function png_get_progressive_ptr(png_ptr); which will return a void pointer, which you have to cast appropriately.  */
  png_set_progressive_read_fn(png_ptr, (void *)NULL, info_callback, row_callback, end_callback);
  return 0;
}

/* A code fragment that you call as you receive blocks of data */
int inlinepng_process_data(png_bytep buffer, png_uint_32 length) {
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return 1;
  }

  //printf("png receiving data: ");
  //for(int n=0;n<length;n++) {
  //  printf("%x,",(unsigned char) buffer[n]);
  //}
  //printf("\n");

  /* This one’s new also. Simply give it a chunk of data from the file stream (in order, of course). On machines with segmented memory models machines, don’t give it any more than 28 64K. The library seems to run fine with sizes of 4K. Although you can give it much less if necessary (I assume you can give it chunks of 1 byte, I haven’t tried less then 256 bytes yet). When this function returns, you may want to display any rows that were generated in the row callback if you don’t already do so there.  */
  png_process_data(png_ptr, info_ptr, buffer, length);
  return 0;
}

#define BUFFERSIZE 2048

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
    if(strncmp(v,buffer+n,strlen(v)) == 0) return n;
  }

  return -1;
}

void buffer_dump() {

  printf("current buffer: ");
  for(int n=0;n<buffer_size;n++) {
    int v = buffer[n];
    printf("%x,",v);
  }
  printf("\n");

}

int inline_data_receive(char *data,int length) {

  printf("buf received: ");
  for(int n=0;n<length;n++) {
    printf("%c,",data[n]);
  }
  printf("\n");

  //printf("inline data received data\n");

  if(processing_png == true) {
    printf("currently processing png\n");
    if(file_end==1) {processing_png=false; return 1;}
    char decoded_buffer[4096]; // should be malloc'd based on length.
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

  printf("buffer located magic: %d\n",pos);
  if(pos < 0) return 0;

  processing_png=true;

  initialize_png_reader();
  char decoded_buffer[4096]; // should be mallco'd based on length.
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
  SDL_mutexP(inline_data_mutex);
  SDL_FillRect(inline_data_layer,NULL, 0x000000); 
  SDL_mutexV(inline_data_mutex);
}
