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
#include "recentrw.h"

UIViewController *viewcon;
ServerSelectUIView *view;
SDL_Window *win;
RecentItemsDataSource *source;

int last_selection = -1;
void display_serverselect_run() {

  [[NSRunLoop currentRunLoop] runMode:UITrackingRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];

  if(source.selection != last_selection) {
  
    char *hostnames[RECENTCONNECTIONS];
    char *usernames[RECENTCONNECTIONS];
    char *passwords[RECENTCONNECTIONS];

    readall_connections(hostnames,usernames,passwords);

    int i = source.selection;
    [[view hostname] setText:[NSString stringWithCString:hostnames[i] encoding:NSASCIIStringEncoding]];
    [[view username] setText:[NSString stringWithCString:usernames[i] encoding:NSASCIIStringEncoding]];
    [[view password] setText:[NSString stringWithCString:passwords[i] encoding:NSASCIIStringEncoding]];

    last_selection = source.selection;
  }
  
  [view setNeedsDisplay];
  [[view hostname] setNeedsDisplay];
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
  const char *h = [[[view hostname] text] cStringUsingEncoding:NSASCIIStringEncoding];
  const char *u = [[[view username] text] cStringUsingEncoding:NSASCIIStringEncoding];
  const char *p = [[[view password] text] cStringUsingEncoding:NSASCIIStringEncoding];
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