#import <UIKit/UIKit.h>
@protocol MAScrollViewDelegate <NSObject>
- (BOOL)MAScrollViewDistance:(CGFloat)distance upOrDown:(BOOL)upOrDown;
@end
@interface MACommentListTableView : UITableView
@property (assign, nonatomic) MACommentType commentType;
@property (weak, nonatomic) id<MAScrollViewDelegate> scrollViewdelegate;
@property (nonatomic) UINavigationController *currentNavigationController;
- (void)loadAndShowData;
@end
