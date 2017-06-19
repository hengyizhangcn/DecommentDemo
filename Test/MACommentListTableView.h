//
//  MACommentListTableView.h
//  MerchantAide
//
//  Created by zhy on 23/03/2017.
//  Copyright © 2017 xujunhao. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol MAScrollViewDelegate <NSObject>
// 通知该对象当前scrollView滚动了多少距离
// upOrDown YES:up, NO:down
// return YES:还有移动的空间  NO:这个方向已经达到最大位置
- (BOOL)MAScrollViewDistance:(CGFloat)distance upOrDown:(BOOL)upOrDown;

@end

@interface MACommentListTableView : UITableView

@property (assign, nonatomic) MACommentType commentType;
@property (weak, nonatomic) id<MAScrollViewDelegate> scrollViewdelegate;

@property (nonatomic) UINavigationController *currentNavigationController;

- (void)loadAndShowData;
@end
