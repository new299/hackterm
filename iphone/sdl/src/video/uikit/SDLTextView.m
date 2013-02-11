#import "SDLTextView.h"
#import <CoreText/CoreText.h>
#include "../../events/SDL_keyboard_c.h"
#include "keyinfotable.h"

@interface StubPosition : UITextPosition {
}

+ (StubPosition *)positionWithIndex:(NSUInteger)index;

@end

@interface StubRange : UITextRange {
}

@end

@implementation SDLTextView

@synthesize inputDelegate = _inputDelegate;

-(void) keyPressed: (NSNotification*) notification
{}

- (id)init {
    
    if ((self = [super init])) {
        
        _tokenizer = [[UITextInputStringTokenizer alloc] initWithTextInput:self];

        self.userInteractionEnabled = YES;
        self.autoresizesSubviews = YES;
    }
    
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(keyPressed:) name: nil object: nil];
    
    return self;
}

- (void)dealloc {
    [_tokenizer release];
    
    [super dealloc];
}


- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (BOOL)isAccessibilityElement
{

    return YES;
}

- (BOOL)resignFirstResponder
{
	return [super resignFirstResponder];
}

- (void)tap:(UITapGestureRecognizer *)tap
{
}

- (NSString *)textInRange:(UITextRange *)range
{
    return @"";
}

- (void)replaceRange:(UITextRange *)range withText:(NSString *)text
{
}

- (UITextRange *)selectedTextRange
{
    return [StubRange alloc];
}

- (void)setSelectedTextRange:(UITextRange *)range
{
}

bool skip_char=false;
- (UITextRange *)markedTextRange
{
    return [StubRange alloc];
}

- (void)setMarkedText:(NSString *)markedText selectedRange:(NSRange)selectedRange
{
    skip_char=true;
}

- (void)unmarkText
{
    SDL_SendKeyboardText([skipped_text cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (UITextPosition *)beginningOfDocument
{
    return [StubPosition positionWithIndex:0];
}

- (UITextPosition *)endOfDocument
{
    return [StubPosition positionWithIndex:1];
}

- (UITextRange *)textRangeFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition
{
    return [StubRange alloc];
}


- (UITextPosition *)positionFromPosition:(UITextPosition *)position offset:(NSInteger)offset
{
    return [StubPosition positionWithIndex:1];
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction offset:(NSInteger)offset
{
    return [StubPosition positionWithIndex:1];
}

- (NSComparisonResult)comparePosition:(UITextPosition *)position toPosition:(UITextPosition *)other
{
    return NSOrderedAscending;
}

- (NSInteger)offsetFromPosition:(UITextPosition *)from toPosition:(UITextPosition *)toPosition
{
    return 1;
}

- (id <UITextInputTokenizer>)tokenizer
{
    return _tokenizer;
}

- (UITextPosition *)positionWithinRange:(UITextRange *)range farthestInDirection:(UITextLayoutDirection)direction
{
    return [StubPosition positionWithIndex:1];
}

- (UITextRange *)characterRangeByExtendingPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction
{
    return [StubRange alloc];
}

- (UITextWritingDirection)baseWritingDirectionForPosition:(UITextPosition *)position inDirection:(UITextStorageDirection)direction
{
    return UITextWritingDirectionLeftToRight;
}

- (void)setBaseWritingDirection:(UITextWritingDirection)writingDirection forRange:(UITextRange *)range
{
}

- (CGRect)firstRectForRange:(UITextRange *)range
{
    CGRect rect = CGRectMake(1,2,1,2);
    return rect;
}

- (CGRect)caretRectForPosition:(UITextPosition *)position
{
    StubPosition *pos = (StubPosition *)position;
    CGRect rect = CGRectMake(1,1,1,1);
    return rect;
}


- (UITextPosition *)closestPositionToPoint:(CGPoint)point
{
    return nil;
}


- (UITextPosition *)closestPositionToPoint:(CGPoint)point withinRange:(UITextRange *)range
{
    return nil;
}

- (UITextRange *)characterRangeAtPoint:(CGPoint)point
{
    return nil;
}

- (NSDictionary *)textStylingAtPosition:(UITextPosition *)position inDirection:(UITextStorageDirection)direction
{
    return [NSDictionary alloc];
}

- (BOOL)hasText
{
    return YES;
}

- (BOOL)shouldChangeTextInRange:(UITextRange *)range replacementText:(NSString *)text
{
    if([text length] == 0) {
        return YES; // a delete
    }
    [self processText:text];
    return YES;
}


- (void)insertText:(NSString *)text
{
    bool nosend=false;
    for(int i=0;i<[text length];i++) {
        int code;
        int mod;
        unichar c = [text characterAtIndex: i];
        if(c == 127) {
            code = unicharToUIKeyInfoTable[c].code;
            mod  = unicharToUIKeyInfoTable[c].mod;
//            SDL_SendKeyboardKey(SDL_PRESSED, code);
//            SDL_SendKeyboardKey(SDL_RELEASED, code);
        } else {
            nosend=true;
        }
        
//        if(!nosend) SDL_SendKeyboardText([text cStringUsingEncoding:NSUTF8StringEncoding]);
        //      SDL_SendKeyboardText([text cStringUsingEncoding:NSUTF16LittleEndianStringEncoding]);
    }
}

NSString *skipped_text;
bool last_skipped=false;
- (void)processText:(NSString *)text
{
    if(skip_char==true) {
        if(!last_skipped) {
            [self deleteBackward];
        }
        
        skipped_text = text;
        skip_char=false;
        last_skipped=true;
        return;
    }
    last_skipped=false;
    for(int i=0;i<[text length];i++) {
      int code;
      int mod;
      unichar c = [text characterAtIndex: i];
      printf("insert c: %d",(int)c);
      if(c <= 127) {
        code = unicharToUIKeyInfoTable[c].code;
        mod  = unicharToUIKeyInfoTable[c].mod;
        SDL_SendKeyboardKey(SDL_PRESSED, code);
        SDL_SendKeyboardKey(SDL_RELEASED, code);
      }

      SDL_SendKeyboardText([text cStringUsingEncoding:NSUTF8StringEncoding]);
    }
}


- (void)deleteBackward
{
  printf("Key pressing in UIKeyInput - delete\n");
  SDL_SendKeyboardKey(SDL_PRESSED , SDL_SCANCODE_DELETE);
  SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_DELETE);
}

@end

@implementation StubPosition 

+ (StubPosition *)positionWithIndex:(NSUInteger)index
{
    StubPosition *pos = [[StubPosition alloc] init];
    return [pos autorelease];
}

@end

@implementation StubRange 

- (UITextPosition *)start
{
    return [StubPosition positionWithIndex:1];
}

- (UITextPosition *)end
{
	return [StubPosition positionWithIndex:1];
}

-(BOOL)isEmpty
{
    return YES;
}

@end

