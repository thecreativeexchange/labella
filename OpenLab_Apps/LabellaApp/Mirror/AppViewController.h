//
//  AppViewController.h
//
//  Created by Gavin Wood on 07/11/2012.

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <GameKit/GameKit.h>
#import <AVFoundation/AVFoundation.h>
#import <MessageUI/MessageUI.h>
#import <MessageUI/MFMailComposeViewController.h>

@interface AppViewController : GLKViewController
<
    AVCaptureVideoDataOutputSampleBufferDelegate,
    MFMailComposeViewControllerDelegate,
    UIImagePickerControllerDelegate
>
{
        AVCaptureSession                    *captureSession;
}

@property (nonatomic, strong) AVCaptureSession *captureSession;

@end
