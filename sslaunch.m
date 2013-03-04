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
#import "RecentItemsDataSource.h"


UIViewController *viewcon;
ServerSelectUIView *view;
SDL_Window *win;
RecentItemsDataSource *source;

int last_selection = -1;
void display_serverselect_run() {

  [[NSRunLoop currentRunLoop] runMode:UITrackingRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];

  if(source.selection != last_selection) {
    if(source.selection == 0) {
      [[view hostname] setText:@"41j.com"];
      [[view username] setText:@"new"];
      [[view password] setText:@""];
    } else
    if(source.selection == 1) {
      [[view hostname] setText:@"sgenomics.org"];
      [[view username] setText:@"new"];
      [[view password] setText:@""];
    } else
    if(source.selection == 2) {
      [[view hostname] setText:@"sdf.lonestar.org"];
      [[view username] setText:@""];
      [[view password] setText:@""];
    } else
    if(source.selection == 3) {
      [[view hostname] setText:@"freeshells.org"];
      [[view username] setText:@""];
      [[view password] setText:@""];
    } else {
      [[view hostname] setText:@"localhost"];
      [[view username] setText:@"root"];
      [[view password] setText:@"tastycakes"];
    }
    last_selection = source.selection;
  }

  
  [view setNeedsDisplay];
  [[view hostname] setNeedsDisplay];


//  [[NSRunLoop currentRunLoop] runMode:UITrackingRunLoopMode beforeDate:[NSDate distantFuture]];


}

void display_serverselect(SDL_Window *window)
{
    win = window;
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
    SDL_DisplayData *displaydata = display->driverdata;
    SDL_DisplayModeData *displaymodedata = display->current_mode.driverdata;
    UIWindow *uiwindow = data->uiwindow;

    viewcon = [[[NSBundle mainBundle] loadNibNamed:@"ServerSelect" owner:nil options:nil] objectAtIndex:0];
    
    source = [[RecentItemsDataSource alloc] initWithStyle:UITableViewStylePlain];
    source.selection=-1;

    view = viewcon.view;
    
    [view.recentservers registerClass:[UITableViewCell class] forCellReuseIdentifier:@"i"];
    view.recentservers.allowsSelection=YES;
    view.recentservers.editing=NO;
    view.recentservers.userInteractionEnabled=YES;
    view.recentservers.scrollEnabled=YES;
    view.recentservers.allowsSelectionDuringEditing = YES;
    view.recentservers.autoresizingMask = (UIViewAutoresizingFlexibleHeight);
    
    
    [view.recentservers setDataSource:source];
    [view.recentservers setDelegate:source];

    uiwindow.rootViewController = viewcon;

    [view.recentservers reloadData];
}

BOOL display_serverselect_get(char *ohostname,char *ousername,char *opassword) {
    
  if(view == nil) return NO;
  const char *h = [[[view hostname] text] cStringUsingEncoding:NSUTF8StringEncoding];
  const char *u = [[[view username] text] cStringUsingEncoding:NSUTF8StringEncoding];
  const char *p = [[[view password] text] cStringUsingEncoding:NSUTF8StringEncoding];
  strcpy(ohostname,h);
  strcpy(ousername,u);
  strcpy(opassword,p);
  
  if([view connectComplete]) {
    SDL_WindowData *data = (SDL_WindowData *) win->driverdata;
    UIWindow *uiwindow = data->uiwindow;
    view = nil;
    viewcon = nil;
    uiwindow.rootViewController = nil;
     
    return YES;
  }
  return NO;
}

void display_serverselect_complete() {
  [view removeFromSuperview];
}