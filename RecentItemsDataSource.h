//
//  RecentItemsDataSource.h
//  hackterm
//
//  Created by new on 24/02/2013.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "recentrw.h"

@interface RecentItemsDataSource : UITableViewController <UITableViewDataSource,UITableViewDelegate>

  @property int selection;
  @property bool initialised;
@end
