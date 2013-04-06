
#import <UIKit/UIKit.h>

@interface SDLUIApplication : UIApplication {}

- (void)sendEvent:(UIEvent *)event;
- (BOOL)sendAction:(SEL)action to:(id)target from:(id)sender forEvent:(UIEvent *)event;
@end
