//
//  SCEnterpriseServiceKindsCell.m
//  TownForRemain
//
//  Created by zhy on 08/05/2017.
//  Copyright © 2017 UAMA. All rights reserved.
//

#import "SCEnterpriseServiceKindsCell.h"
#import "SCEnterpriseServiceCollectionViewCell.h"
#import "SCApplyListViewController.h"
#import "SCParkServiceViewController.h"
#import "SCCompanyCatalogViewController.h"
#import "SCCompanyPolicyContainerViewController.h"
#import "SCAgencyServiceViewController.h"
#import "SCCompanyMentorViewController.h"
#import "SCEnterpriseCooperationViewController.h"
#import "SCProgramFinancingViewController.h"
#import "SCBankLoanViewController.h"
#import "SCTownHotContainerViewController.h"
#import "SCResumeLibraryViewController.h"
#import "SCEnterpriseHunterContainerViewController.h"
#import "SCEnterpriseServiceAdministrationViewController.h"
/**
 这是一个测试
 */
@interface SCEnterpriseServiceKindsCell () <UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout>
@property (weak, nonatomic) IBOutlet UICollectionView *collectionView;
@property (weak, nonatomic) IBOutlet UICollectionViewFlowLayout *flowLayout;

@property (nonatomic) NSArray *dataSourceArray;
@end

@implementation SCEnterpriseServiceKindsCell

- (void)awakeFromNib {
    [super awakeFromNib];
    // Initialization code
    self.selectionStyle = UITableViewCellSelectionStyleNone;
    
    self.collectionView.delegate = self;
    self.collectionView.dataSource = self;
    self.collectionView.backgroundColor = [UIColor whiteColor];
    [self.collectionView registerNib:[UINib nibWithNibName:[SCEnterpriseServiceCollectionViewCell sc_className] bundle:nil] forCellWithReuseIdentifier:[SCEnterpriseServiceCollectionViewCell sc_className]];
    self.flowLayout.itemSize = CGSizeMake(SCREEN_WIDTH/3, 92);
    self.collectionView.scrollEnabled = NO;
}

#pragma mark - UICollectionViewDataSource
- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return self.dataSourceArray.count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    SCEnterpriseServiceCollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:[SCEnterpriseServiceCollectionViewCell sc_className] forIndexPath:indexPath];
    [cell loadData:self.dataSourceArray[indexPath.row]];
    [cell hideRightSeparatorLine:(indexPath.row % 3 == 2)];
    return cell;
}

#pragma mark - UICollectionViewDelegate
- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumLineSpacingForSectionAtIndex:(NSInteger)section
{
    return 0;
}
- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout minimumInteritemSpacingForSectionAtIndex:(NSInteger)section
{
    return 0;
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    SCLog(@"%@", self.dataSourceArray[indexPath.row][@"title"]);
    if (indexPath.row == 0) { //小镇活动
        SCApplyListViewController *applyVC = [[SCApplyListViewController alloc] init];
        applyVC.title = @"小镇活动";
        [self.currentNavigationController pushViewController:applyVC animated:YES];
    } else if (indexPath.row == 1) { //企业政策
        SCCompanyPolicyContainerViewController *companyPolicyVc = [[SCCompanyPolicyContainerViewController alloc]init];
        [self.currentNavigationController pushViewController:companyPolicyVc animated:YES];
    } else if (indexPath.row == 2) { //小镇服务
        UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
        layout.itemSize = CGSizeMake((SCREEN_WIDTH - 1.5)/4, 100);
        
        SCParkServiceViewController *parkServiceVC = [[SCParkServiceViewController alloc] initWithCollectionViewLayout:layout];
        parkServiceVC.title = @"小镇服务";
        parkServiceVC.enterpriseServiceType = SCEnterpriseServiceTypeTownService;
        [self.currentNavigationController pushViewController:parkServiceVC animated:YES];
    } else if (indexPath.row == 3) { //行政后勤
        UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
        layout.itemSize = CGSizeMake((SCREEN_WIDTH - 1.5)/4, 100);
        
        SCEnterpriseServiceAdministrationViewController *parkServiceVC = [[SCEnterpriseServiceAdministrationViewController alloc] initWithCollectionViewLayout:layout];
        parkServiceVC.title = @"行政后勤";
        [self.currentNavigationController pushViewController:parkServiceVC animated:YES];
    } else if (indexPath.row == 4) { //代办服务
        UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
        layout.itemSize = CGSizeMake((SCREEN_WIDTH - 1.5)/4, 100);
        
        SCParkServiceViewController *parkServiceVC = [[SCParkServiceViewController alloc] initWithCollectionViewLayout:layout];
        parkServiceVC.title = @"代办服务";
        parkServiceVC.enterpriseServiceType = SCEnterpriseServiceTypeProxyService;
        [self.currentNavigationController pushViewController:parkServiceVC animated:YES];
    } else if (indexPath.row == 5) { //政务服务
        UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
        layout.itemSize = CGSizeMake((SCREEN_WIDTH - 1.5)/4, 100);
        
        SCParkServiceViewController *parkServiceVC = [[SCParkServiceViewController alloc] initWithCollectionViewLayout:layout];
        parkServiceVC.title = @"政务服务";
        parkServiceVC.enterpriseServiceType = SCEnterpriseServiceTypeGovernmentAffaris;
        [self.currentNavigationController pushViewController:parkServiceVC animated:YES];
    } else if (indexPath.row == 6) { //企业名录
        SCCompanyCatalogViewController *companyCatalogVc = [[SCCompanyCatalogViewController alloc]init];
        [self.currentNavigationController pushViewController:companyCatalogVc animated:YES];
    } else if (indexPath.row == 7) { //企业合作
        if (![self checkEnterpriseAuthorizedStatus]) {
            return;
        }
        SCEnterpriseCooperationViewController *enterpriseCooperationVC = [[SCEnterpriseCooperationViewController alloc] init];
        enterpriseCooperationVC.postCategory = SCPostCategoryCooperation;
        [self.currentNavigationController pushViewController:enterpriseCooperationVC animated:YES];
    } else if (indexPath.row == 8) { //企业招聘
        if (![self checkEnterpriseAuthorizedStatus]) {
            return;
        }
        SCEnterpriseHunterContainerViewController *vc = [[SCEnterpriseHunterContainerViewController alloc] init];
        [self.currentNavigationController pushViewController:vc animated:YES];
    } else if (indexPath.row == 9) { //小镇热点
        SCTownHotContainerViewController *townHotVc = [[SCTownHotContainerViewController alloc]init];
        [self.currentNavigationController pushViewController:townHotVc animated:true];
    } else if (indexPath.row == 10) { //企业导师
        SCCompanyMentorViewController *companyMentorVc = [[SCCompanyMentorViewController alloc]init];
        [self.currentNavigationController pushViewController:companyMentorVc animated:YES];
    } else if (indexPath.row == 11) { //项目融资
        if (![self checkEnterpriseAuthorizedStatus]) {
            return;
        }
        SCProgramFinancingViewController *financingVc = [[SCProgramFinancingViewController alloc]init];
        [self.currentNavigationController pushViewController:financingVc animated:true];
    } else if (indexPath.row == 12) { //银行贷款
        SCBankLoanViewController *bankLoanVc = [[SCBankLoanViewController alloc]init];
        bankLoanVc.isCompanyLoan  = YES;
        [self.currentNavigationController pushViewController:bankLoanVc animated:true];
    } else if (indexPath.row == 13) { 
    //企业理财
        SCBankLoanViewController *bankLoanVc = [[SCBankLoanViewController alloc]init];
        [self.currentNavigationController pushViewController:bankLoanVc animated:true];
    }
}

- (void)loadData:(id)data
{
    if ([data isKindOfClass:[NSArray class]]) {
        self.dataSourceArray = data;
        [self.collectionView reloadData];
    }
}

- (BOOL)checkEnterpriseAuthorizedStatus
{
    if ([SCUser currentLoggedInUser].enterpriseId.length > 0) {
        return YES;
    }
    RIButtonItem *cancelButtonItem = [RIButtonItem itemWithLabel:@"知道了" action:^{
    }];
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:nil
                                                        message:@"您的账号未得到企业授权，无法进入此页面，请联系企业授权"
                                               cancelButtonItem:cancelButtonItem
                                               otherButtonItems:nil, nil];
    
    [alertView show];
    return NO;
}
@end
