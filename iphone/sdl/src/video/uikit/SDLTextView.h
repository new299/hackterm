#import <UIKit/UIKit.h>

@class SDLTextView;

@interface SDLTextView : UIView <UITextInput> {
    UITextInputStringTokenizer        *_tokenizer;
}

@end
