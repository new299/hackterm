//
//  ServerSelectUIView.m
//  hackterm
//
//  Created by new on 23/02/2013.
//
//

#import "ServerSelectUIView.h"
#include "iphone/sdl/src/video/SDL_sysvideo.h"
#include "iphone/sdl/include/SDL_video.h"
#include "iphone/sdl/src/video/uikit/SDL_uikitopenglview.h"
#include "iphone/sdl/src/video/uikit/SDL_uikitwindow.h"
#include "iphone/sdl/src/video/uikit/SDL_uikitmodes.h"


void display_serverselect_run() {
      [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
}

UIView *view;
void display_serverselect(SDL_Window * window)
{



    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
    SDL_DisplayData *displaydata = display->driverdata;
    SDL_DisplayModeData *displaymodedata = display->current_mode.driverdata;
    UIWindow *uiwindow = data->uiwindow;

    view = [[[NSBundle mainBundle] loadNibNamed:@"ServerSelect" owner:nil options:nil] objectAtIndex:0];

    [uiwindow addSubview: view];
}

BOOL display_serverselect_get(char *ohostname,char *ousername,char *opassword) {

  const char *h = [[[view hostname] text] cStringUsingEncoding:NSUTF8StringEncoding];
  const char *u = [[[view username] text] cStringUsingEncoding:NSUTF8StringEncoding];
  const char *p = [[[view password] text] cStringUsingEncoding:NSUTF8StringEncoding];
  strcpy(ohostname,h);
  strcpy(ousername,u);
  strcpy(opassword,p);
  
  if([view connectComplete]) return YES;
  return NO;
}

void display_serverselect_complete() {
  [view removeFromSuperview];
}