//
//  RecentItemsDataSource.m
//  hackterm
//
//  Created by new on 24/02/2013.
//
//

#import "RecentItemsDataSource.h"
#include "recentrw.h"

@implementation RecentItemsDataSource 

@synthesize selection = _selection;


//TODO: make part of class
char *hostnames[RECENTCONNECTIONS];
char *usernames[RECENTCONNECTIONS];
char *passwords[RECENTCONNECTIONS];
bool init=false;

- (UITableViewCell *)tableView:view cellForRowAtIndexPath:idx {

  if(init==false) {
    readall_connections(hostnames,usernames,passwords);
    init=true;
  }

  UITableViewCell *cell = [view dequeueReusableCellWithIdentifier:@"i" forIndexPath:idx];
    
  // Cell label
  NSInteger i = [idx indexAtPosition:1];
  
  if(hostnames[i][0] != 0) {
    cell.textLabel.text = [NSString stringWithCString:hostnames[i] encoding:NSASCIIStringEncoding];
  }  else {
    cell.textLabel.text = @"NONE";
  }

  return cell;
}

- (NSInteger)tableView:view numberOfRowsInSection:idx {

  if(init==false) {
    readall_connections(hostnames,usernames,passwords);
    init=true;
  }

  for(int n=0;n<RECENTCONNECTIONS;n++) {
    if(hostnames[n][0]==0) return n;
  }

  return RECENTCONNECTIONS;
}

- (void)tableView:(UITableView *)view didSelectRowAtIndexPath:(NSIndexPath *)idx {

  NSLog(@"Did Select");
  _selection=[idx indexAtPosition:1];

}

- (BOOL)tableView:(UITableView *)tableView shouldHighlightRowAtIndexPath:(NSIndexPath *)indexPath {
  return YES;
}



@end
