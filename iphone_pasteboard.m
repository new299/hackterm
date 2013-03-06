#import "UIPasteboard.h"

void iphone_copy(char *text) {

  UIPasteboard *pb = [UIPasteboard generalPasteboard];

  NSString *nstext = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];

  [pb setValue:nstext forPasteboardType:@"public.plain-text"];
}