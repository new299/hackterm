//
//  ServerSelectUIView.h
//  hackterm
//
//  Created by new on 23/02/2013.
//
//

#import <UIKit/UIKit.h>

@interface ServerSelectUIView : UIView

@property (retain, nonatomic) IBOutlet UITextField *hostname;

@property (retain, nonatomic) IBOutlet UITextField *username;

@property (retain, nonatomic) IBOutlet UITextField *password;

@property BOOL connectComplete;

@end
