//
//  ServerSelectUIView.m
//  hackterm
//
//  Created by new on 23/02/2013.
//
//

#import "ServerSelectUIView.h"

@implementation ServerSelectUIView

@synthesize hostname;
@synthesize username;
@synthesize password;
@synthesize connectComplete = _connectComplete;

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait
            || interfaceOrientation == UIInterfaceOrientationLandscapeLeft
            || interfaceOrientation == UIInterfaceOrientationLandscapeRight);
}


- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        self.connectComplete=NO;
    }
    return self;
}

- (IBAction)connectPressed:(id)sender {

  const char *chostname = [[hostname text] cStringUsingEncoding:NSUTF8StringEncoding];
  const char *cusername = [[username text] cStringUsingEncoding:NSUTF8StringEncoding];
  const char *cpassword = [[password text] cStringUsingEncoding:NSUTF8StringEncoding];
  
  printf("hostname %s\n",chostname);
  printf("username %s\n",cusername);
  printf("password %s\n",cpassword);
  self.connectComplete=YES;
}

- (IBAction)keyXferPressed:(id)sender {
}

- (IBAction)bookmarkPressed:(id)sender {
}


- (void)dealloc {
    [super dealloc];
}
@end
