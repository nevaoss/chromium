// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/cobrowse/debugger/aim_srp_debugger_url_view_controller.h"

#import <UIKit/UIKit.h>

#import "base/strings/sys_string_conversions.h"
#import "ios/chrome/browser/cobrowse/ui/assistant_aim_ui_constants.h"
#import "ios/chrome/browser/shared/ui/symbols/symbols.h"
#import "ios/chrome/common/ui/colors/semantic_color_names.h"
#import "ios/chrome/common/ui/util/constraints_ui_util.h"
#import "url/gurl.h"

namespace {

// The padding between the text view and the safe area layout guide.
const CGFloat kPadding = 16.0;

}  // namespace

@implementation AIMSRPDebuggerURLViewController {
  GURL _url;
  UITextView* _textView;
  UIBarButtonItem* _copyButton;
}

- (instancetype)initWithURL:(const GURL&)url {
  self = [super initWithNibName:nil bundle:nil];
  if (self) {
    _url = url;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  self.title = @"Loaded AIM URL";
  self.view.backgroundColor = [UIColor colorNamed:kPrimaryBackgroundColor];
  self.view.accessibilityIdentifier =
      kAIMSRPDebuggerURLViewControllerAccessibilityIdentifier;

  // Set up close button.
  UIBarButtonItem* closeButton =
      [[UIBarButtonItem alloc] initWithTitle:@"Close"
                                       style:UIBarButtonItemStylePlain
                                      target:self
                                      action:@selector(dismissModal)];
  closeButton.accessibilityIdentifier =
      kAIMSRPDebuggerURLViewControllerCloseButtonAccessibilityIdentifier;
  self.navigationItem.leftBarButtonItem = closeButton;

  // Set up copy button in navigation bar.
  UIImage* copyImage = DefaultSymbolWithPointSize(kCopyActionSymbol, 18);
  _copyButton = [[UIBarButtonItem alloc] initWithImage:copyImage
                                                 style:UIBarButtonItemStylePlain
                                                target:self
                                                action:@selector(copyURL)];
  _copyButton.accessibilityIdentifier =
      kAIMSRPDebuggerURLViewControllerCopyButtonAccessibilityIdentifier;
  self.navigationItem.rightBarButtonItem = _copyButton;

  // Set up text view to display the URL.
  _textView = [[UITextView alloc] init];
  _textView.translatesAutoresizingMaskIntoConstraints = NO;
  _textView.editable = NO;
  _textView.selectable = YES;
  _textView.font = [UIFont monospacedSystemFontOfSize:14
                                               weight:UIFontWeightRegular];
  _textView.textColor = [UIColor colorNamed:kTextPrimaryColor];
  _textView.backgroundColor = [UIColor colorNamed:kSecondaryBackgroundColor];
  _textView.layer.cornerRadius = 8;
  _textView.textContainerInset = UIEdgeInsetsMake(12, 12, 12, 12);
  _textView.text = base::SysUTF8ToNSString(_url.spec());
  _textView.accessibilityIdentifier =
      kAIMSRPDebuggerURLViewControllerTextViewAccessibilityIdentifier;

  [self.view addSubview:_textView];

  AddSameConstraintsWithInset(_textView, self.view.safeAreaLayoutGuide,
                              kPadding);
}

- (void)dismissModal {
  [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)copyURL {
  UIPasteboard.generalPasteboard.string = base::SysUTF8ToNSString(_url.spec());

  // Visual feedback for copying.
  UIImage* checkmarkImage = DefaultSymbolWithPointSize(kCheckmarkSymbol, 18);
  _copyButton.image = checkmarkImage;
  _copyButton.enabled = NO;

  __weak __typeof(self) weakSelf = self;
  dispatch_after(
      dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.5 * NSEC_PER_SEC)),
      dispatch_get_main_queue(), ^{
        [weakSelf resetCopyButton];
      });
}

- (void)resetCopyButton {
  UIImage* copyImage = DefaultSymbolWithPointSize(kCopyActionSymbol, 18);
  _copyButton.image = copyImage;
  _copyButton.enabled = YES;
}

@end
