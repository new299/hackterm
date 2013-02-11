#import <UIKit/UIKit.h>

@class SDLTextView;

@interface SDLTextView : UIView <UITextInput> {
    UITextInputStringTokenizer        *_tokenizer;
    UITextAutocorrectionType autocorrectionType;
}

@property(nonatomic) UITextAutocorrectionType autocorrectionType;
@property(nonatomic) UITextAutocapitalizationType autocapitalizationType;
@property(nonatomic) UITextSpellCheckingType spellCheckingType;




@end
