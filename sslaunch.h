//
//  ServerSelectUIView.h
//  hackterm
//
//  Created by new on 23/02/2013.
//
//

#import <UIKit/UIKit.h>
#include "iphone/sdl/src/video/SDL_sysvideo.h"
#include "iphone/sdl/include/SDL_video.h"
#include "iphone/sdl/src/video/uikit/SDL_uikitopenglview.h"
#include "iphone/sdl/src/video/uikit/SDL_uikitwindow.h"
#include "iphone/sdl/src/video/uikit/SDL_uikitmodes.h"

void display_serverselect_run();
void display_serverselect(SDL_Window * window);

BOOL display_serverselect_get(char *hostname,char *username,char *password,char *fingerprintstr);
void display_server_select_closedlg();
void begin_background_task();
void display_serverselect_keyfailure();
void display_serverselect_firstkey(char *fingerprintstr);