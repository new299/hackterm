#include "osx_pasteboard.h"
#include <UIKit/UIPasteboard.h>

void iphone_copy(char *text) {

  NSPasteboard *pb = [NSPasteboard generalPasteboard];

  NSString *nstext = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
  [pasteBoard declareTypes:[[[NSArray]] arrayWithObjects:NSStringPboardType, nil] owner:nil];

  if(nstext != nil) {
    [pb setString:nstext forType:NSStringPboardType];
  }
}

const char *iphone_paste() {

  NSPasteboard *pb = [UIPasteboard generalPasteboard];
  
  NSString *nstext = [pb dataForType:NSStringPboardType];

  return [nstext cStringUsingEncoding:NSUTF8StringEncoding];

}
