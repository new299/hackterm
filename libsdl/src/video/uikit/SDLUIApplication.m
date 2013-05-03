
#import "SDLUIApplication.h"
#import <objc/runtime.h>
#include "../../events/SDL_keyboard_c.h"

#define GSEVENT_TYPE 2
#define GSEVENT_FLAGS 12
#define GSEVENTKEY_KEYCODE    15
#define GSEVENT_TYPE_KEYDOWN  10
#define GSEVENT_TYPE_KEYUP    11
#define GSEVENT_TYPE_MODKEY   12

@implementation SDLUIApplication

int modset_ralt  = 0;
int modset_lctrl = 0;
int modset_lalt  = 0;
int modset_lcmd  = 0;
int modset_rcmd  = 0;

#define MODCODE1_RALT  64
#define MODCODE1_LCTRL 16
#define MODCODE1_LALT  8
#define MODCODE1_LCMD  1
#define MODCODE2_RCMD  1

#ifndef ITUNES_BUILD
bool any_modkey() {
    if(modset_ralt  == 1) return true;
    if(modset_lctrl == 1) return true;
    if(modset_lalt  == 1) return true;
    if(modset_lcmd  == 1) return true;
    if(modset_rcmd  == 1) return true;
    
    return false;
}

int lookup_scancode(int code) {
    return code;
}

void check_code(int keymod,int keymask,int *key) {
    if(keymod & keymask) {
        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_RALT);
        *key=1;
    } else {
        if(*key == 1) {
            SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RALT);
            *key=0;
        }
    }
}


- (void)sendEvent:(UIEvent *)event {

    [super sendEvent:event];

    if ([event respondsToSelector:@selector(_gsEvent)]) {
        
        unsigned char *eventMem;
        eventMem = (unsigned char *)[event performSelector:@selector(_gsEvent)];
        if (eventMem) {
            int eventType = eventMem[8];
            int keycode   = eventMem[60];
            int keymod1   = eventMem[50];
            int keymod2   = eventMem[51];

            check_code(keymod1,MODCODE1_RALT ,&modset_ralt);
            check_code(keymod1,MODCODE1_LCTRL,&modset_lctrl);
            check_code(keymod1,MODCODE1_LALT ,&modset_lalt);
            check_code(keymod1,MODCODE1_LCMD ,&modset_lcmd);
            check_code(keymod2,MODCODE2_RCMD ,&modset_rcmd);
            
            // if any modifier key is pressed, then we need to send normal keypressed, otherwise
            // they are handled by the UITextField which takes keyboard language into account,
            // and works with the virtual keyboard.
            if(modset_lctrl == 1) {
                int sdl_scancode = lookup_scancode(keycode);
                if(eventType == GSEVENT_TYPE_KEYDOWN) {
                    char text[5];
                    text[0] = keycode-4+1;
                    text[1] = 0;
                    SDL_SendKeyboardKey(SDL_PRESSED,sdl_scancode);
                    SDL_SendKeyboardText(text);
                }
                
                if(eventType == GSEVENT_TYPE_KEYUP) {
                    SDL_SendKeyboardKey(SDL_RELEASED,sdl_scancode);
                }
            }
            
            if ((eventType == GSEVENT_TYPE_KEYDOWN) && (keycode == 41)) {
                SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_ESCAPE);
            } else
            if ((eventType == GSEVENT_TYPE_KEYUP  ) && (keycode == 41)) {
                SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_ESCAPE);
                char text[5];
                text[0] = 27;
                text[1] = 0;
                SDL_SendKeyboardText(text);
            } else
            if ((eventType == GSEVENT_TYPE_KEYDOWN) && (keycode == 3 )) {
                SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_MODE);
            } else
            if ((eventType == GSEVENT_TYPE_KEYUP  ) && (keycode == 3 )) {
                SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_MODE);
            } else
            if ((eventType == GSEVENT_TYPE_KEYDOWN  ) && (keycode == 82 )) {
              SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_UP);
            } else
            if ((eventType == GSEVENT_TYPE_KEYUP  ) && (keycode == 82 )) {
              SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_UP);
            } else 
            if ((eventType == GSEVENT_TYPE_KEYDOWN  ) && (keycode == 81 )) {
              SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_DOWN);
            } else
            if ((eventType == GSEVENT_TYPE_KEYUP  ) && (keycode == 81 )) {
              SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_DOWN);
            } else 
            if ((eventType == GSEVENT_TYPE_KEYDOWN  ) && (keycode == 80 )) {
              SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_LEFT);
            } else
            if ((eventType == GSEVENT_TYPE_KEYUP  ) && (keycode == 80 )) {
              SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_LEFT);
            } else 
            if ((eventType == GSEVENT_TYPE_KEYDOWN  ) && (keycode == 79 )) {
              SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_RIGHT);
            } else
            if ((eventType == GSEVENT_TYPE_KEYUP  ) && (keycode == 79 )) {
              SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_RIGHT);
            } else {
            }
        }
    }
}
#endif

- (BOOL)sendAction:(SEL)action to:(id)target from:(id)sender forEvent:(UIEvent *)event {
    return [super sendAction:action to:target from:sender forEvent:event];
}

@end
