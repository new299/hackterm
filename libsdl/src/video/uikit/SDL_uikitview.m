 /*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

#if SDL_VIDEO_DRIVER_UIKIT

#include "SDL_uikitview.h"

#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_touch_c.h"

#if SDL_IPHONE_KEYBOARD
#include "keyinfotable.h"
#include "SDL_uikitappdelegate.h"
#include "SDL_uikitmodes.h"
#include "SDL_uikitwindow.h"
#import "SDLTextView.h"
#endif

@implementation SDL_uikitview

- (void)dealloc
{
    [super dealloc];
}


//NW - THIS GETS SCREWED
- (id)initWithFrame:(CGRect)frame
{
    SDL_TouchQuit(); // reinit of rotation/recreation.

    self = [super initWithFrame: frame];


    self.multipleTouchEnabled = YES;

    SDL_Touch touch;
    touch.id = 0; //TODO: Should be -1?

    //touch.driverdata = SDL_malloc(sizeof(EventTouchData));
    //EventTouchData* data = (EventTouchData*)(touch.driverdata);

    touch.x_min = 0;
    touch.x_max = 1;
    touch.native_xres = touch.x_max - touch.x_min;
    touch.y_min = 0;
    touch.y_max = 1;
    touch.native_yres = touch.y_max - touch.y_min;
    touch.pressure_min = 0;
    touch.pressure_max = 1;
    touch.native_pressureres = touch.pressure_max - touch.pressure_min;

    touchId = SDL_AddTouch(&touch, "IPHONE SCREEN");

#if SDL_IPHONE_KEYBOARD
    [self initializeKeyboard];
#endif

    return self;

}

- (void) touch_reinit {
    SDL_Touch touch;
    touch.id = 0; //TODO: Should be -1?

    //touch.driverdata = SDL_malloc(sizeof(EventTouchData));
    //EventTouchData* data = (EventTouchData*)(touch.driverdata);

    touch.x_min = 0;
    touch.x_max = 1;
    touch.native_xres = touch.x_max - touch.x_min;
    touch.y_min = 0;
    touch.y_max = 1;
    touch.native_yres = touch.y_max - touch.y_min;
    touch.pressure_min = 0;
    touch.pressure_max = 1;
    touch.native_pressureres = touch.pressure_max - touch.pressure_min;

    touchId = SDL_AddTouch(&touch, "IPHONE SCREEN");
}

- (CGPoint)touchLocation:(UITouch *)touch shouldNormalize:(BOOL)normalize
{
    CGPoint point = [touch locationInView: self];

    // Get the display scale and apply that to the input coordinates
    SDL_Window *window = self->viewcontroller.window;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
    SDL_DisplayModeData *displaymodedata = (SDL_DisplayModeData *) display->current_mode.driverdata;
    
    if (normalize) {
        CGRect bounds = [self bounds];
        point.x /= bounds.size.width;
        point.y /= bounds.size.height;
    } else {
        point.x *= displaymodedata->scale;
        point.y *= displaymodedata->scale;
    }
    return point;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSEnumerator *enumerator = [touches objectEnumerator];
    UITouch *touch = (UITouch*)[enumerator nextObject];

    while (touch) {
        if (!leftFingerDown) {
            CGPoint locationInView = [self touchLocation:touch shouldNormalize:NO];

            /* send moved event */
            SDL_SendMouseMotion(NULL, 0, locationInView.x, locationInView.y);

            /* send mouse down event */
            SDL_SendMouseButton(NULL, SDL_PRESSED, SDL_BUTTON_LEFT);

            leftFingerDown = (SDL_FingerID)touch;
        }

        CGPoint locationInView = [self touchLocation:touch shouldNormalize:YES];
#ifdef IPHONE_TOUCH_EFFICIENT_DANGEROUS
        // FIXME: TODO: Using touch as the fingerId is potentially dangerous
        // It is also much more efficient than storing the UITouch pointer
        // and comparing it to the incoming event.
        SDL_SendFingerDown(touchId, (SDL_FingerID)touch,
                           SDL_TRUE, locationInView.x, locationInView.y,
                           1);
#else
        int i;
        for(i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++) {
            if (finger[i] == NULL) {
                finger[i] = touch;
                SDL_SendFingerDown(touchId, i,
                                   SDL_TRUE, locationInView.x, locationInView.y,
                                   1);
                break;
            }
        }
#endif
        touch = (UITouch*)[enumerator nextObject];
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSEnumerator *enumerator = [touches objectEnumerator];
    UITouch *touch = (UITouch*)[enumerator nextObject];

    while(touch) {
        if ((SDL_FingerID)touch == leftFingerDown) {
            /* send mouse up */
            SDL_SendMouseButton(NULL, SDL_RELEASED, SDL_BUTTON_LEFT);
            leftFingerDown = 0;
        }

        CGPoint locationInView = [self touchLocation:touch shouldNormalize:YES];
#ifdef IPHONE_TOUCH_EFFICIENT_DANGEROUS
        SDL_SendFingerDown(touchId, (long)touch,
                           SDL_FALSE, locationInView.x, locationInView.y,
                           1);
#else
        int i;
        for (i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++) {
            if (finger[i] == touch) {
                SDL_SendFingerDown(touchId, i,
                                   SDL_FALSE, locationInView.x, locationInView.y,
                                   1);
                finger[i] = NULL;
                break;
            }
        }
#endif
        touch = (UITouch*)[enumerator nextObject];
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    /*
        this can happen if the user puts more than 5 touches on the screen
        at once, or perhaps in other circumstances.  Usually (it seems)
        all active touches are canceled.
    */
    [self touchesEnded: touches withEvent: event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSEnumerator *enumerator = [touches objectEnumerator];
    UITouch *touch = (UITouch*)[enumerator nextObject];

    while (touch) {
        if ((SDL_FingerID)touch == leftFingerDown) {
            CGPoint locationInView = [self touchLocation:touch shouldNormalize:NO];

            /* send moved event */
            SDL_SendMouseMotion(NULL, 0, locationInView.x, locationInView.y);
        }

        CGPoint locationInView = [self touchLocation:touch shouldNormalize:YES];
#ifdef IPHONE_TOUCH_EFFICIENT_DANGEROUS
        SDL_SendTouchMotion(touchId, (long)touch,
                            SDL_FALSE, locationInView.x, locationInView.y,
                            1);
#else
        int i;
        for (i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++) {
            if (finger[i] == touch) {
                SDL_SendTouchMotion(touchId, i,
                                    SDL_FALSE, locationInView.x, locationInView.y,
                                    1);
                break;
            }
        }
#endif
        touch = (UITouch*)[enumerator nextObject];
    }
}

/*
    ---- Keyboard related functionality below this line ----
*/
#if SDL_IPHONE_KEYBOARD

/* Is the iPhone virtual keyboard visible onscreen? */
- (BOOL)keyboardVisible
{
    return keyboardVisible;
}

-(void) keyPressed: (NSNotification*) notification
{
}

-(void)keyboardDidShow:(NSNotification*)notification {
  NSDictionary* keyboardInfo = [notification userInfo];
  NSValue* keyboardFrameEnd = [keyboardInfo valueForKey:UIKeyboardFrameEndUserInfoKey];
  CGRect keyboardFrameEndRect = [keyboardFrameEnd CGRectValue];
  
  SDL_Window *window = self->viewcontroller.window;
  SDL_WindowData *data = window->driverdata;
  SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
  if(display == 0) return; // For some reason display is /sometimes/ not set on second call, no idea why.
  SDL_DisplayModeData *displaymodedata = (SDL_DisplayModeData *) display->current_mode.driverdata;
    
    
  const CGSize size = data->view.bounds.size;
  
  int kb_h = keyboardFrameEndRect.size.height;
  int kb_w = keyboardFrameEndRect.size.width;
  int kb_x = keyboardFrameEndRect.origin.x;
  int kb_y = keyboardFrameEndRect.origin.y;

  CGFloat deviceHeight = [UIScreen mainScreen].bounds.size.height;
  CGFloat deviceWidth  = [UIScreen mainScreen].bounds.size.width;

  CGFloat newKeyboardHeight;

  UIDeviceOrientation *interfaceOrientation = [[UIDevice currentDevice] orientation];
  
  keyboardVisible = YES;

  // iOS is sometimes sending supurious DidShow notifications when a bluetooth keyboard is connected.
  // this is my (vain) attempt to detect some of them, but they often look /exactly/ like real events.
  if (interfaceOrientation == UIInterfaceOrientationPortrait) {
      //orientation: portrait
  }
  else if (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown) {
      //orientation: upsidedown
  }
  else if (interfaceOrientation == UIInterfaceOrientationLandscapeLeft) {
      //orientation: landscapeleft
      if(kb_w > kb_h) keyboardVisible=NO;
      if(kb_x == 0  ) keyboardVisible=NO;
  }
  else if (interfaceOrientation == UIInterfaceOrientationLandscapeRight){
      //orientation: landscaperight
      if(kb_w > kb_h) keyboardVisible=NO;
  }
  else {
      //other orientation?
  }

  if(keyboardVisible == NO) {
    kb_w=0;
    kb_h=0;
  }
  

  int nkb_h;
  if(kb_h > kb_w) {
    nkb_h = kb_w;
  } else {
    nkb_h = kb_h;
  }

  
  int w, h;
  w = (int)(size.width  * displaymodedata->scale);
  h = (int)(size.height * displaymodedata->scale)-(nkb_h*displaymodedata->scale);

  SDL_SendWindowEvent(window, SDL_WINDOWEVENT_ROTATE, w, h);//TODO: NW ADD NEW EVENT TYPE
}

-(void)keyboardWillShow:(NSNotification*)notification {
/*
  NSDictionary* keyboardInfo = [notification userInfo];
  NSValue* keyboardFrameBegin = [keyboardInfo valueForKey:UIKeyboardFrameBeginUserInfoKey];
  CGRect keyboardFrameBeginRect = [keyboardFrameBegin CGRectValue];
  
  SDL_Window *window = self->viewcontroller.window;
  SDL_WindowData *data = window->driverdata;
  SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
  if(display == 0) return; // For some reason display is /sometimes/ not set on second call, no idea why.
  SDL_DisplayModeData *displaymodedata = (SDL_DisplayModeData *) display->current_mode.driverdata;
    
    
  const CGSize size = data->view.bounds.size;
  
  int kb_h = keyboardFrameBeginRect.size.height;
  int kb_w = keyboardFrameBeginRect.size.width;
  int kb_x = keyboardFrameBeginRect.origin.x;
  int kb_y = keyboardFrameBeginRect.origin.y;

  if(kb_h > kb_w) {
    int t = kb_h;
    kb_h = kb_w;
    kb_w = t;
  }


  CGFloat deviceHeight = [UIScreen mainScreen].bounds.size.height;
  CGFloat deviceWidth  = [UIScreen mainScreen].bounds.size.width;

  CGFloat newKeyboardHeight;

  UIDeviceOrientation *interfaceOrientation = [[UIDevice currentDevice] orientation];
  
  if (interfaceOrientation == UIInterfaceOrientationPortrait) {
      //orientation: portrait
      newKeyboardHeight = deviceHeight - keyboardFrameBeginRect.origin.y;
  }
  else if (interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown) {
      //orientation: upsidedown
      newKeyboardHeight = keyboardFrameBeginRect.size.height + keyboardFrameBeginRect.origin.y;
  }
  else if (interfaceOrientation == UIInterfaceOrientationLandscapeLeft) {
      //orientation: landscapeleft
      newKeyboardHeight = deviceHeight - keyboardFrameBeginRect.origin.x;
  }
  else if (interfaceOrientation == UIInterfaceOrientationLandscapeRight){
      //orientation: landscaperight
      newKeyboardHeight = keyboardFrameBeginRect.size.width - keyboardFrameBeginRect.origin.x;
  }
  else {
      //other orientation
  }

  //if(newKeyboardHeight != 0) keyboardVisible=YES;
  //else keyboardVisible=NO;

  if(keyboardVisible == NO) {
    kb_w=0;
    kb_h=0;
  }
  int w, h;
  w = (int)(size.width  * displaymodedata->scale);
  h = (int)(size.height * displaymodedata->scale)-(kb_h*displaymodedata->scale);

  SDL_SendWindowEvent(window, SDL_WINDOWEVENT_ROTATE, w, h);//TODO: NW ADD NEW EVENT TYPE
*/
}


- (void)fullsize_window
{
  SDL_Window *window = self->viewcontroller.window;
  SDL_WindowData *data = window->driverdata;

  SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
  if(display == 0) return; // For some reason display is /sometimes/ not set on second call, no idea why.
  SDL_DisplayModeData *displaymodedata = (SDL_DisplayModeData *) display->current_mode.driverdata;
  
  const CGSize size = data->view.bounds.size;

  int w, h;
  w = (int)(size.width  * displaymodedata->scale);
  h = (int)(size.height * displaymodedata->scale);
  SDL_SendWindowEvent(window, SDL_WINDOWEVENT_ROTATE, w, h);//TODO: NW ADD NEW EVENT TYPE
}

-(void)keyboardWillHide:(NSNotification*)notification {
  // Animate the current view back to its original position
  [self fullsize_window];
  keyboardVisible = NO;
}


-(void)keyboardDidHide:(NSNotification*)notification {
  // Animate the current view back to its original position
  [self fullsize_window];
  keyboardVisible = NO;
}


/* Set ourselves up as a UITextFieldDelegate */
- (void)initializeKeyboard
{
  textInput = [[SDLTextView alloc] init];

  [textInput reloadInputViews];
  [self addSubview: textInput];
  [textInput becomeFirstResponder];
  [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(keyPressed:) name: nil object: nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(keyboardWillShow:)
                                               name:UIKeyboardWillShowNotification
                                             object:nil];
    
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardDidShow:) name:UIKeyboardDidShowNotification object:nil];


  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(keyboardWillHide:)
                                               name:UIKeyboardWillHideNotification
                                             object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(keyboardDidHide:)
                                               name:UIKeyboardDidHideNotification
                                             object:nil];

}


/* reveal onscreen virtual keyboard */
- (void)showKeyboard
{
    //keyboardVisible = YES;
    [textInput becomeFirstResponder];
}

/* hide onscreen virtual keyboard */
- (void)hideKeyboard
{
    keyboardVisible = NO;
}


/* UITextFieldDelegate method.  Invoked when user types something. 
- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    if ([string length] == 0) {
        /* it wants to replace text with nothing, ie a delete 
        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_DELETE);
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_DELETE);
    }
    else {
        /* go through all the characters in the string we've been sent
           and convert them to key presses 
        int i;
        for (i = 0; i < [string length]; i++) {

            unichar c = [string characterAtIndex: i];

            Uint16 mod = 0;
            SDL_Scancode code;

            if (c < 127) {
                /* figure out the SDL_Scancode and SDL_keymod for this unichar 
                code = unicharToUIKeyInfoTable[c].code;
                mod  = unicharToUIKeyInfoTable[c].mod;
            }
            else {
                /* we only deal with ASCII right now 
                code = SDL_SCANCODE_UNKNOWN;
                mod = 0;
            }

            if (mod & KMOD_SHIFT) {
                /* If character uses shift, press shift down 
                SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_LSHIFT);
            }
            /* send a keydown and keyup even for the character 
            SDL_SendKeyboardKey(SDL_PRESSED, code);
            SDL_SendKeyboardKey(SDL_RELEASED, code);
            if (mod & KMOD_SHIFT) {
                /* If character uses shift, press shift back up 
                SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LSHIFT);
            }
        }
        SDL_SendKeyboardText([string UTF8String]);
    }
    return NO; /* don't allow the edit! (keep placeholder text there) 
}

/* Terminates the editing session 
- (BOOL)textFieldShouldReturn:(UITextField*)_textField
{
    SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_RETURN);
    SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RETURN);
    SDL_StopTextInput();
    SDL_StartTextInput();
    return YES;
}
*/

#endif

@end

/* iPhone keyboard addition functions */
#if SDL_IPHONE_KEYBOARD

SDL_uikitview * getWindowView(SDL_Window * window)
{
    if (window == NULL) {
        SDL_SetError("Window does not exist");
        return nil;
    }

    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;
    SDL_uikitview *view = data != NULL ? data->view : nil;

    if (view == nil) {
        SDL_SetError("Window has no view");
    }

    return view;
}

SDL_bool UIKit_HasScreenKeyboardSupport(_THIS)
{
    return SDL_TRUE;
}

void UIKit_ShowScreenKeyboard(_THIS, SDL_Window *window)
{
    SDL_uikitview *view = getWindowView(window);
    if (view != nil) {
        [view showKeyboard];
    }
}

void UIKit_HideScreenKeyboard(_THIS, SDL_Window *window)
{
    SDL_uikitview *view = getWindowView(window);
    if (view != nil) {
        [view hideKeyboard];
    }
}

SDL_bool UIKit_IsScreenKeyboardShown(_THIS, SDL_Window *window)
{
    SDL_uikitview *view = getWindowView(window);
    if (view == nil) {
        return 0;
    }

    return view.keyboardVisible;
}

#endif /* SDL_IPHONE_KEYBOARD */

#endif /* SDL_VIDEO_DRIVER_UIKIT */

/* vi: set ts=4 sw=4 expandtab: */
