#ifndef REGIS_H
#define REGIS_H

#define _POSIX_C_SOURCE 199309L
#define _BSD_SOURCE

#include <SDL/SDL.h>       
#include <SDL/SDL_thread.h>
#include <stdint.h>   
#include <stdio.h>    
#include <stdbool.h>


extern SDL_Surface *regis_layer;
extern SDL_mutex *regis_mutex;

void regis_clear();
char *regis_process_cmd_screen(char *cmd);
char *regis_process_cmd_text(char *cmd);
char *regis_process_cmd_w(char *cmd);
char *regis_process_cmd_position(char *cmd);
void regis_init(int width,int height);
char *regis_process_cmd_vector(char *cmd);
char *regis_process_command(char *cmd);
void regis_processor(const char *cmd,int cmdlen);
bool regis_recent();

#endif
