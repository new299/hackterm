#import "SDLTextView.h"
#import <CoreText/CoreText.h>
#include "../../events/SDL_keyboard_c.h"
#include "keyinfotable.h"

@interface StubPosition : UITextPosition {
@public
    NSInteger index;
    
}

- (NSInteger)index;
+ (StubPosition *)positionWithIndex:(NSUInteger)index;

@end

@interface StubRange : UITextRange {
    
@public
    StubPosition *_start;
    StubPosition *_end;
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
        [self setAutocorrectionType:UITextAutocorrectionTypeNo];
        [self setAutocapitalizationType:UITextAutocapitalizationTypeNone];
        [self setSpellCheckingType:UITextSpellCheckingTypeNo];
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

    return NO;
}

- (BOOL)resignFirstResponder
{
	return [super resignFirstResponder];
}

//- (void)tap:(UITapGestureRecognizer *)tap
//{
//}

- (NSString *)textInRange:(UITextRange *)range
{
    return @"                    ";
}

- (void)replaceRange:(UITextRange *)range withText:(NSString *)text
{
}

int c=10;
- (UITextRange *)selectedTextRange
{
    StubRange *i = [StubRange alloc];
    int j=c;
    i->_start = [StubPosition positionWithIndex:j];
    i->_end   = [StubPosition positionWithIndex:j];
    
    return i;
}


//TODO: The code here is currently disabled, because this method isn't /always/ called
//      see: http://stackoverflow.com/questions/14921030/ios-understanding-positionfrompositionindirectionoffset
- (void)setSelectedTextRange:(UITextRange *)range
{
    
    NSUInteger s = [[range start] index];
    NSUInteger e = [[range end]  index];

    int cur  = s;
    
    
    // and this is why you shouldn't code at 4am.
    int j=c-1;
    int next = ((j-6)%9)+6;
    //if(s==next) { SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_LEFT); SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_LEFT);}

    j=c+1;
    next = ((j-6)%9)+6;
    //if(s==next) { SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_RIGHT); SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_RIGHT);}
    
    j=c+3;
    next = ((j-6)%9)+6;
    //if(s==next) { SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_DOWN); SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_DOWN);}
    
    j=c-3;
    if(j<6) next=15-(6-j); else next=j;
    //if(s==next) { SDL_SendKeyboardKey(SDL_PRESSED,SDL_SCANCODE_UP);   SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_UP);}
    
    c=s;
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
    return [StubPosition positionWithIndex:6];
}

- (UITextPosition *)endOfDocument
{
    return [StubPosition positionWithIndex:14];
}

- (UITextRange *)textRangeFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition
{
    StubRange *s = [StubRange alloc];
    s->_start = fromPosition;
    s->_end   = toPosition;
    return s;
}


- (UITextPosition *)positionFromPosition:(UITextPosition *)position offset:(NSInteger)offset
{
    return [StubPosition positionWithIndex:[position index]+offset];
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction offset:(NSInteger)offset
{
    int i = [position index];
    
    if(direction == UITextLayoutDirectionUp   ) {
      int j=i-(3*offset);
      if(j<6) {
      
        j=j-6;
        j=0-j;
        j=j%9;
        j=15-j;
      }
      
      return [StubPosition positionWithIndex:j];
    }
    
    if(direction == UITextLayoutDirectionDown ) {
      int j=i+(3*offset);
      if(j>14) {
      
        j=j-6;
        j=j%9;
        j=6+j;
      }
      
      return [StubPosition positionWithIndex:j];
    }
    
    if(direction == UITextLayoutDirectionLeft ) {
      int j=i-(1*offset);
//      if(j==5) j=8; else
//      if(j==8) j=11; else
      if(j==5)j=14;
      return [StubPosition positionWithIndex:j];
    }
    
    if(direction == UITextLayoutDirectionRight) {
      int j=i+(1*offset);
     // if(j==9) j=6; else
     // if(j==12) j=9; else
      if(j==15) j=6;
      return [StubPosition positionWithIndex:j];
    }
}

- (NSComparisonResult)comparePosition:(UITextPosition *)position toPosition:(UITextPosition *)other
{
    if([position index] > [other index])return NSOrderedAscending;
    if([position index] < [other index])return NSOrderedDescending;
    return NSOrderedSame;
}

- (NSInteger)offsetFromPosition:(UITextPosition *)from toPosition:(UITextPosition *)to
{
    return [to index] - [from index];
}

- (id <UITextInputTokenizer>)tokenizer
{
    return _tokenizer;
}

- (UITextPosition *)positionWithinRange:(UITextRange *)range farthestInDirection:(UITextLayoutDirection)direction
{
    if(direction == UITextLayoutDirectionLeft ) return [StubPosition positionWithIndex:9];
    if(direction == UITextLayoutDirectionRight) return [StubPosition positionWithIndex:11];
    if(direction == UITextLayoutDirectionUp   ) return [StubPosition positionWithIndex:0];
    if(direction == UITextLayoutDirectionDown ) return [StubPosition positionWithIndex:20];
    return [StubPosition positionWithIndex:0];
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
    CGRect rect = CGRectMake([[range start] index],[[range start] index]+1,[[range end] index],[[range end] index]+1);
    return rect;
}

- (CGRect)caretRectForPosition:(UITextPosition *)position
{
    StubPosition *pos = (StubPosition *)position;
    CGRect rect = CGRectMake([position index],[position index]+1,[position index]+1,[position index]+1);
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
  SDL_SendKeyboardKey(SDL_PRESSED , SDL_SCANCODE_DELETE);
  SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_DELETE);
}

@end

@implementation StubPosition 

+ (StubPosition *)positionWithIndex:(NSUInteger)index
{
    StubPosition *pos = [[StubPosition alloc] init];
    pos->index=index;
    return [pos autorelease];
}

- (NSInteger)index
{
    return index;
}

@end

@implementation StubRange 

- (id)init {
    if(self = [super init]) {
        _start = [StubPosition positionWithIndex:0];
        _end   = [StubPosition positionWithIndex:0];
    }
    return self;
}

- (UITextPosition *)start
{
    return _start;
}

- (UITextPosition *)end
{
    return _end;
}

-(BOOL)isEmpty
{
    return YES;
}

@end

