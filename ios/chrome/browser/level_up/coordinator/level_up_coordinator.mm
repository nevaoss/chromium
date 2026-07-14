// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/level_up/coordinator/level_up_coordinator.h"

#import "components/prefs/pref_service.h"
#import "ios/chrome/browser/level_up/coordinator/level_up_mediator.h"
#import "ios/chrome/browser/level_up/model/level_up_service.h"
#import "ios/chrome/browser/level_up/model/level_up_service_factory.h"
#import "ios/chrome/browser/level_up/model/task_info.h"
#import "ios/chrome/browser/level_up/ui/level_up_all_tasks_view_controller.h"
#import "ios/chrome/browser/level_up/ui/level_up_view_controller.h"
#import "ios/chrome/browser/shared/model/browser/browser.h"
#import "ios/chrome/browser/shared/model/profile/profile_ios.h"
#import "ios/chrome/browser/shared/public/commands/command_dispatcher.h"
#import "ios/chrome/browser/shared/public/commands/level_up_commands.h"
#import "ios/chrome/browser/signin/model/authentication_service_factory.h"
#import "ios/chrome/browser/signin/model/identity_manager_factory.h"

@interface LevelUpCoordinator () <LevelUpAllTasksViewControllerDelegate,
                                  LevelUpMediatorDelegate,
                                  LevelUpViewControllerDelegate>

@property(nonatomic, strong) LevelUpMediator* mediator;
@property(nonatomic, strong) LevelUpViewController* viewController;
@property(nonatomic, strong) UINavigationController* navigationController;

@end

@implementation LevelUpCoordinator {
  TaskInfo::NavigationAction _pendingNavigationAction;
}

- (void)start {
  [super start];

  self.viewController = [[LevelUpViewController alloc] init];
  self.viewController.handler =
      HandlerForProtocol(self.browser->GetCommandDispatcher(), LevelUpCommands);
  [self.viewController setDelegate:self];

  AuthenticationService* authService =
      AuthenticationServiceFactory::GetForProfile(self.browser->GetProfile());
  signin::IdentityManager* identityManager =
      IdentityManagerFactory::GetForProfile(self.browser->GetProfile());
  LevelUpService* levelUpService =
      LevelUpServiceFactory::GetForProfile(self.browser->GetProfile());
  PrefService* prefService = self.browser->GetProfile()->GetPrefs();
  self.mediator =
      [[LevelUpMediator alloc] initWithAuthenticationService:authService
                                             identityManager:identityManager
                                              levelUpService:levelUpService
                                                 prefService:prefService];

  self.mediator.delegate = self;
  self.mediator.profileConsumer = self.viewController;
  self.mediator.consumer = self.viewController;

  self.navigationController = [[UINavigationController alloc]
      initWithRootViewController:self.viewController];
  [self.navigationController
      setModalPresentationStyle:UIModalPresentationPageSheet];

  UISheetPresentationController* sheetPresentationController =
      self.navigationController.sheetPresentationController;
  sheetPresentationController.detents =
      @[ [UISheetPresentationControllerDetent largeDetent] ];

  [self.baseViewController presentViewController:self.navigationController
                                        animated:YES
                                      completion:nil];
}

- (void)stop {
  TaskInfo::NavigationAction pendingAction = _pendingNavigationAction;
  CommandDispatcher* dispatcher = self.browser->GetCommandDispatcher();

  [self.navigationController.presentingViewController
      dismissViewControllerAnimated:YES
                         completion:^{
                           // Execute the previously saved task if there is one.
                           if (!pendingAction.is_null()) {
                             pendingAction.Run(dispatcher);
                           }
                         }];
  self.viewController = nil;
  [self.mediator disconnect];
  self.mediator.delegate = nil;
  self.mediator.profileConsumer = nil;
  self.mediator.consumer = nil;
  self.mediator = nil;
  self.navigationController = nil;

  [super stop];
}

#pragma mark - LevelUpViewControllerDelegate

- (void)didTapSeeAllTasks:(LevelUpViewController*)controller {
  LevelUpAllTasksViewController* allTasksVC =
      [[LevelUpAllTasksViewController alloc] init];
  allTasksVC.delegate = self;
  [self.navigationController pushViewController:allTasksVC animated:YES];
  [self.mediator configureAllTasksConsumer:allTasksVC];
}

- (void)didTapToggleProgressUpdates:(LevelUpViewController*)controller {
  [self.mediator toggleProgressUpdates];
}

- (void)levelUpViewController:(LevelUpViewController*)controller
                   didTapTask:(LevelUpTask*)task {
  [self didTapTask:task];
}

#pragma mark - LevelUpMediatorDelegate

- (void)levelUpMediatorWantsToBeDismissed:(LevelUpMediator*)mediator {
  [HandlerForProtocol(self.browser->GetCommandDispatcher(), LevelUpCommands)
      dismissLevelUp];
}

#pragma mark - LevelUpAllTasksViewControllerDelegate

- (void)levelUpAllTasksViewController:(LevelUpAllTasksViewController*)controller
                           didTapTask:(LevelUpTask*)task {
  [self didTapTask:task];
}

#pragma mark - Private

// Handles a task tap by closing the level up screen and preparing to navigate
// to the beginning of the tapped task.
- (void)didTapTask:(LevelUpTask*)task {
  // Save the task so it can be executed once the level up view is closed.
  _pendingNavigationAction = task.taskInfo->GetNavigationAction();

  CommandDispatcher* dispatcher = self.browser->GetCommandDispatcher();
  id<LevelUpCommands> levelUpHandler =
      HandlerForProtocol(dispatcher, LevelUpCommands);
  [levelUpHandler dismissLevelUp];
}

@end
