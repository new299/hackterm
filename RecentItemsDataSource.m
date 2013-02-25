//
//  RecentItemsDataSource.m
//  hackterm
//
//  Created by new on 24/02/2013.
//
//

#import "RecentItemsDataSource.h"

@implementation RecentItemsDataSource 

@synthesize selection = _selection;

- (UITableViewCell *)tableView:view cellForRowAtIndexPath:idx {


  UITableViewCell *cell = [view dequeueReusableCellWithIdentifier:@"i" forIndexPath:idx];
    
  // Cell label
  NSInteger i = [idx indexAtPosition:1];
  if(i == 0) cell.textLabel.text = @"41j.com";          else
  if(i == 1) cell.textLabel.text = @"sgenomics.org";    else
  if(i == 2) cell.textLabel.text = @"sdf.lonestar.org"; else
  if(i == 3) cell.textLabel.text = @"freeshells.org";   else
             cell.textLabel.text = @"localhost";

  return cell;
}

- (NSInteger)tableView:view numberOfRowsInSection:idx {

  return 20;
}

- (void)tableView:(UITableView *)view didSelectRowAtIndexPath:(NSIndexPath *)idx {

  NSLog(@"Did Select");
  _selection=[idx indexAtPosition:1];

}

- (BOOL)tableView:(UITableView *)tableView shouldHighlightRowAtIndexPath:(NSIndexPath *)indexPath {
  return YES;
}



@end
