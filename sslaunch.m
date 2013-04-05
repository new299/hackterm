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
#include "DisconnectAlertDelegate.h"

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
    char *fingerprintstrs[RECENTCONNECTIONS];

    readall_connections(hostnames,usernames,passwords,fingerprintstrs);

    int i = source.selection;
    printf("i is: %d\n",i);
    if(i < 0) return;
    if(i > 20) return;
    [[view hostname] setText:[NSString stringWithCString:hostnames[i] encoding:NSASCIIStringEncoding]];
    [[view username] setText:[NSString stringWithCString:usernames[i] encoding:NSASCIIStringEncoding]];
    [[view password] setText:[NSString stringWithCString:"" encoding:NSASCIIStringEncoding]];

    last_selection = source.selection;
  }
  
  [view setNeedsDisplay];
  [[view hostname] setNeedsDisplay];
}

void display_alert(char *str1,char *str2) {
  DisconnectAlertDelegate *discon = [[DisconnectAlertDelegate alloc] init];

  NSString *s1 = [NSString stringWithUTF8String:str1];
  NSString *s2 = [NSString stringWithUTF8String:str2];

  UIAlertView *alert = [[UIAlertView alloc] initWithTitle:s1
                                                message:s2
                                               delegate:discon
                                      cancelButtonTitle:@"OK"
                                      otherButtonTitles:nil];
  [alert show];
  
  for(;;) {
    if([discon ok_pressed] != false) {
      return;
    }
    
    [[NSRunLoop currentRunLoop] runMode:UITrackingRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.005]];
  }
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
    source.initialised=false;
    source.selection=-1;
    last_selection = -1;
    
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

void display_serverselect_keyxfer_ok() {
  display_alert("Key transfer","Key transfer complete.");
}

void display_serverselect_keyxfer_fail() {
  display_alert("Key transfer","Key transfer failed.");
}


void display_serverselect_keyfailure() {
  display_alert("Fingerprint failure","Fingerprints did not match, connection will not be opened.");
}

void display_serverselect_firstkey(char *fingerprintstr) {
  char fstr[1000];
  sprintf(fstr,"The server fingerprint is: %s",fingerprintstr);
  display_alert("Fingerprint",fstr);
}

int display_serverselect_get(char *ohostname,char *ousername,char *opassword,char *ofingerprintstr,char *pubkeypath,char *privkeypath) {
    
  if(view == nil) return -2;
  
  bool connectComplete = [view connectComplete];
  bool keyComplete     = [view keyComplete];
  if(connectComplete || keyComplete) {
  
    printf("here0\n");
    const char *h = [[[view hostname] text] cStringUsingEncoding:NSASCIIStringEncoding];
    printf("here1\n");
    const char *u = [[[view username] text] cStringUsingEncoding:NSASCIIStringEncoding];
    printf("here2\n");
    const char *p = [[[view password] text] cStringUsingEncoding:NSASCIIStringEncoding];
    printf("here3\n");
    strcpy(ohostname,h);
    strcpy(ousername,u);
    strcpy(opassword,p);
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *fpath = [paths objectAtIndex:0];
    NSString *pubpath  = [fpath stringByAppendingString:@"/pubkey"];
    NSString *privpath = [fpath stringByAppendingString:@"/privkey"];
    const char *pubpathc  = [pubpath  cStringUsingEncoding:NSASCIIStringEncoding];
    const char *privpathc = [privpath cStringUsingEncoding:NSASCIIStringEncoding];
    strcpy(pubkeypath,pubpathc);
    strcpy(privkeypath,privpathc);
    
    if((!keyComplete) && (access(pubkeypath, F_OK ) == -1)) {
      pubkeypath [0]=0;
      privkeypath[0]=0;
    }

    char *hostnames[RECENTCONNECTIONS];
    char *usernames[RECENTCONNECTIONS];
    char *passwords[RECENTCONNECTIONS];
    char *fingerprintstrs[RECENTCONNECTIONS];

    readall_connections(hostnames,usernames,passwords,fingerprintstrs);
    strcpy(ofingerprintstr,"");
    for(int n=0;n<RECENTCONNECTIONS;n++) {
      if(strcmp(hostnames[n],h)==0) {
        strcpy(ofingerprintstr,fingerprintstrs[n]);
      }
    }

    SDL_WindowData *data = (SDL_WindowData *) win->driverdata;
    UIWindow *uiwindow = data->uiwindow;
    view = nil;
    viewcon = nil;
    uiwindow.rootViewController = nil;
     
    if(keyComplete    ) return 1;
    if(connectComplete) return 2;
  }
  return -1;
}

void display_serverselect_complete() {
  [view removeFromSuperview];
}


bool connection_active=false;
void display_server_select_closedlg() {

  display_alert("Connection Closed","Disconnected");

}

void begin_background_task() {

  [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:^{
  
    if(connection_active) {
      [[UIApplication sharedApplication] cancelAllLocalNotifications];
      UILocalNotification *localNotification = [[UILocalNotification alloc] init];
      NSDate *now = [NSDate date];
      NSDate *dateToFire = [now dateByAddingTimeInterval:0.01];
      localNotification.alertBody = @"Connections terminating";
      localNotification.soundName = UILocalNotificationDefaultSoundName;
      localNotification.applicationIconBadgeNumber=1;
      [[UIApplication sharedApplication] scheduleLocalNotification:localNotification];
    }
  }];

}

void display_server_select_setactive(bool active) {
  connection_active=active;
}