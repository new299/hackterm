#include "osx_pasteboard.h"
#include <AppKit/NSPasteboard.h>

void osx_copy(char *text) {

  NSPasteboard *pb = [NSPasteboard generalPasteboard];

  NSString *nstext = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
  [pb declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];

  if(nstext != nil) {
    [pb setString:nstext forType:NSStringPboardType];
  }
}

const char *osx_paste() {

  NSPasteboard *pb = [NSPasteboard generalPasteboard];
  
  NSString *nstext = [pb stringForType:NSStringPboardType];

  if(nstext == 0) return 0;

  return [nstext cStringUsingEncoding:NSUTF8StringEncoding];

}
