#include "iphone_pasteboard.h"
#include <UIKit/UIPasteboard.h>

void iphone_copy(char *text) {

  UIPasteboard *pb = [UIPasteboard generalPasteboard];

  NSString *nstext = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];

  if(nstext != nil) {
    [pb setValue:nstext forPasteboardType:@"public.plain-text"];
  }
}

const char *iphone_paste() {

  UIPasteboard *pb = [UIPasteboard generalPasteboard];
  
  NSString *nstext = [pb valueForPasteboardType:@"public.utf8-plain-text"];

  return [nstext cStringUsingEncoding:NSUTF8StringEncoding];

}