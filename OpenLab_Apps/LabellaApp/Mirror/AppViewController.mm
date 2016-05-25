//
//  AppViewController.mm
//
//  Created by Gavin Wood on 07/06/2012.

#import "AppViewController.h"
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioServices.h>

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>

// Struffy entry point
#include "GaProject.h"
#include "ApConfig.h"
#include "RsImpl.h"
#include "ShTouch.h"
#include "BtTimeGLES.h"
#include "SdSoundWinGL.h"
#include "ShIMU.h"
#include "ShMail.h"
#include "ShCamera.h"
#include "ShVibration.h"
#include "ShRecorder.h"
#include <stdio.h>

#import "BtString.h"
#import "pgCameraInput.h"
#import "pgMain.h"
#import "BtTime.h"


// http://iphonedevwiki.net/index.php/AppleISL29003

// My globals
pgMain myProject;
CFMutableDictionaryRef touchToLabelMapping;
NSMutableArray *availableLabels;
float RenderScale = 1;                          // Render scale allows code to run between retina and normal devices
#define MotionInterval 0.01f                    // This is in seconds
const float RECORDTIME = 20.0f;
AVAudioPlayer *audioPlayer;
AVAudioRecorder *audioRecorder;

@interface AppViewController () {
}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation AppViewController
@synthesize context = _context;
@synthesize captureSession;
unsigned int buffer[640 * 480];

//---------------------------------------------------------------------------------
// helpers

void interruptionListenerCallback (void   *inUserData, UInt32    interruptionState )
{
    /*
    // you could do this with a cast below, but I will keep it here to make it clearer
    YourSoundControlObject *controller = (YourSoundControlObject *) inUserData;
    
    if (interruptionState == kAudioSessionBeginInterruption) {
    [controller _haltOpenALSession];
    } else if (interruptionState == kAudioSessionEndInterruption) {
    [controller _resumeOpenALSession];
    }
    */
}

enum {
    UIDeviceResolution_Unknown          = 0,
    UIDeviceResolution_iPhoneStandard   = 1,    // iPhone 1,3,3GS Standard Display  (320x480px)
    UIDeviceResolution_iPhoneRetina35   = 2,    // iPhone 4,4S Retina Display 3.5"  (640x960px)
    UIDeviceResolution_iPhoneRetina4    = 3,    // iPhone 5 Retina Display 4"       (640x1136px)
    UIDeviceResolution_iPadStandard     = 4,    // iPad 1,2 Standard Display        (1024x768px)
    UIDeviceResolution_iPadRetina       = 5     // iPad 3 Retina Display            (2048x1536px)
}; typedef NSUInteger UIDeviceResolution;

//stopLocationTracking
// http://stackoverflow.com/questions/12395200/how-to-develop-or-migrate-apps-for-iphone-5-screen-resolution

- (UIDeviceResolution)resolution
{
    UIDeviceResolution resolution = UIDeviceResolution_Unknown;
    UIScreen *mainScreen = [UIScreen mainScreen];
    CGFloat scale = ([mainScreen respondsToSelector:@selector(scale)] ? mainScreen.scale : 1.0f);
    CGFloat pixelHeight = (CGRectGetHeight(mainScreen.bounds) * scale);
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
    {
        if (scale == 2.0f)
        {
            if (pixelHeight == 960.0f)
                resolution = UIDeviceResolution_iPhoneRetina35;
            else if (pixelHeight == 1136.0f)
                resolution = UIDeviceResolution_iPhoneRetina4;
        }
        else if (scale == 1.0f && pixelHeight == 480.0f)
        {
            resolution = UIDeviceResolution_iPhoneStandard;
        }
    } else
    {
        if (scale == 2.0f && pixelHeight == 2048.0f)
        {
            resolution = UIDeviceResolution_iPadRetina;
        }
        else if (scale == 1.0f && pixelHeight == 1024.0f)
        {
            resolution = UIDeviceResolution_iPadStandard;
        }
    }
    return resolution;
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskPortrait;
    //return UIInterfaceOrientationMaskAllButUpsideDown;
}

//////////////////////////////////c//////////////////////////////////////////////
// setupAudioAsMixed

-(void)setupAudioAsMixed
{
    // Objective-C way
    NSError *nsError;
    //[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:&nsError];
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord error:&nsError];
    
    if( nsError != nil )  // handle the error
    {
        NSLog( @"Audio session error" );
    }
}

-(void)setupTouch
{
    // Create the mapping for touch
    touchToLabelMapping = CFDictionaryCreateMutable (kCFAllocatorDefault, 8, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    availableLabels = [[NSMutableArray alloc] initWithObjects:
                       [NSNumber numberWithInt:1],
                       [NSNumber numberWithInt:2],
                       [NSNumber numberWithInt:3],
                       [NSNumber numberWithInt:4],
                       [NSNumber numberWithInt:5],
                       [NSNumber numberWithInt:6],
                       [NSNumber numberWithInt:7],
                       [NSNumber numberWithInt:8],
                       nil];
}

- (void)handleTap:(UITapGestureRecognizer *)sender
{
    if (sender.state == UIGestureRecognizerStateEnded)
    {
        ShTouch::SetNumberOfTaps(3);
    }
}

-(void)setupTap
{
    UITapGestureRecognizer *singleFingerTap =
    [[UITapGestureRecognizer alloc] initWithTarget:self
                                            action:@selector(handleTap:)];
    singleFingerTap.numberOfTapsRequired = 3;
    [self.view addGestureRecognizer:singleFingerTap];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
   
    [self setupAudioAsMixed];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableMultisample = GLKViewDrawableMultisampleNone;
    //view.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
    view.multipleTouchEnabled = true;
    
    [self setupGL];
    
    //self.preferredFramesPerSecond = 60;
    BtTime::SetTick( 1.0f / 30.0f );

    NSString *resourceDirectory = [[NSBundle mainBundle] resourcePath];
    resourceDirectory = [resourceDirectory stringByAppendingString:@"/"];
    const BtChar *resources = [resourceDirectory cStringUsingEncoding:NSASCIIStringEncoding];
    ApConfig::SetResourcePath(resources);
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    const BtChar *documents = [documentsDirectory cStringUsingEncoding:NSASCIIStringEncoding];
    ApConfig::SetDocuments(documents);
        
    // Set the extension
    ApConfig::SetExtension( ".iPhonez" );
    
    // Set the platform
    ApConfig::SetPlatform( ApPlatform_GLES );
    
    MtVector2 v2Dimension( 0, 0 );
    RenderScale = 1;
    
    // Get the resolution
    UIDeviceResolution res = [self resolution];
    
    switch( res )
    {
        case UIDeviceResolution_Unknown:
            break;
        case UIDeviceResolution_iPhoneStandard:
            v2Dimension = MtVector2( 480.0f, 320.0f );
            ApConfig::SetDevice( ApDevice_iPhone );
            break;
        case UIDeviceResolution_iPhoneRetina35:
            v2Dimension = MtVector2( 960.0f, 640.0f );
            ApConfig::SetDevice( ApDevice_iPhone );
            RenderScale = 2;
            break;
        case UIDeviceResolution_iPhoneRetina4:
            v2Dimension = MtVector2( 1136.0f, 640.0f );
            ApConfig::SetDevice( ApDevice_iPhone );
            RenderScale = 2;
            break;
        case UIDeviceResolution_iPadStandard:
            v2Dimension = MtVector2( 1024.0f, 768.0f );
            ApConfig::SetDevice( ApDevice_iPad );
            break;
        case UIDeviceResolution_iPadRetina:
            v2Dimension = MtVector2( 2048.0f, 1536.0f );
            ApConfig::SetDevice( ApDevice_iPad );
            RenderScale = 2;
            break;
    }
    
    NSLog( @"Grabbed resolution %d %d", (int)v2Dimension.x, (int)v2Dimension.y );
    
    // Determine the device orientation
    UIInterfaceOrientation interfaceOrientation = self.interfaceOrientation;
    
    // If it is portrait make sure our engine knows this
    if( interfaceOrientation == UIInterfaceOrientationPortrait )
    {
        MtVector2 v2SwapDimension = v2Dimension;
        v2Dimension.x = v2SwapDimension.y;
        v2Dimension.y = v2SwapDimension.x;
    }
    
    // Set the dimension
    RsImpl::pInstance()->SetDimension( v2Dimension );
    
    // Create the renderer implementation
    RsImpl::pInstance()->Create();
    
    // Init the time
    BtTimeGLES::Init();
    
    // Create the project
    myProject.Create();
    
    // Initialise the project
    myProject.Init();
    
    // Become the first responder for all the delegates in this view controller
    [self becomeFirstResponder];

    // Setup touch
    [self setupTouch];
    
    // Setup the sensor fusion
    //[self setupSensorFusion];
    
    // Setup the tap response
    //[self setupTap];
    
    // Start tracking location
    //[self startLocationTracking];
    
    // Start ranging beacons
    //[self startRangingBeacons];
    
    // Setup the GAI tracker
  //  self.screenName = @"Wild Man";
    
    
    //[self showGameCenter];
    
    [[UIApplication sharedApplication] setStatusBarHidden:YES];
    
    {
    // http://stackoverflow.com/questions/1010343/how-do-i-record-audio-on-iphone-with-avaudiorecorder
    audioRecorder = nil;
    
    // Init audio with record capability
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    [audioSession setCategory:AVAudioSessionCategoryRecord error:nil];

    NSArray *dirPaths;
    NSString *docsDir;
    dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    docsDir = [dirPaths objectAtIndex:0];
    NSString *soundFilePath = [docsDir stringByAppendingPathComponent:@"record.caf"];
    NSURL *soundFileURL = [NSURL fileURLWithPath:soundFilePath];
    NSDictionary *recordSettings = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithInt: kAudioFormatMPEG4AAC], AVFormatIDKey,
                                    //[NSNumber numberWithInt: kAudioFormatMPEGLayer3], AVFormatIDKey,
                                    [NSNumber numberWithInt:AVAudioQualityMin], AVEncoderAudioQualityKey,
                                    [NSNumber numberWithInt:16], AVEncoderBitRateKey,
                                    [NSNumber numberWithInt: 1], AVNumberOfChannelsKey,
                                    [NSNumber numberWithFloat:16000.0], AVSampleRateKey,
                                    nil];
    NSError *error = nil;
    audioRecorder = [[AVAudioRecorder alloc] initWithURL:soundFileURL
                                                settings:recordSettings
                                                   error:&error];
    }
}

static void completionCallback (SystemSoundID  mySSID, void* myself)
{
}

-(void)vibrate
{
    AudioServicesAddSystemSoundCompletion (kSystemSoundID_Vibrate,NULL,NULL,
                                           completionCallback,(__bridge void*) self);
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

////////////////////////////////////////////////////////////////////////////////
// viewDidUnload

- (void)viewDidUnload
{
    // Release the renderer
    RsImpl::pInstance()->Destroy();
    
    // Unload the view
    [super viewDidUnload];
    
    // End OpenGL and its related contexts
    [self tearDownGL];
    
    // Complete tearing down the view conxt
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
	self.context = nil;
}

-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	for (UITouch *touch in touches)
	{
        //NSLog( @"Touches began %d %d\r\n", (int)touch.self, [availableLabels count] );
        
        // Cache the current position
		CGPoint currentPosition = [touch locationInView:self.view];
        
        // Get the index using our dictionary
        NSNumber *label = (NSNumber *)[availableLabels objectAtIndex:0];
        
        CFDictionaryAddValue (touchToLabelMapping, (__bridge const void*)touch, (__bridge_retained const void*)label );
        
        [availableLabels removeObjectAtIndex:0];
        
        int touchIndex = [label integerValue];
        
        ShTouch::BeginTouch( touchIndex, MtVector2( currentPosition.x * RenderScale, currentPosition.y * RenderScale ) );
	}
}

-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	for (UITouch *touch in touches)
	{
        CGPoint currentPosition = [touch locationInView:self.view];
        
        // Get the label for this touch
        NSNumber *label = (__bridge_transfer NSNumber *)CFDictionaryGetValue(touchToLabelMapping, (__bridge const void*)touch);
        
        //NSLog( @"Touches ended %d %d\r\n", (int)touch.self, [label integerValue] );
        
        int touchIndex = [label integerValue];
        
        ShTouch::EndTouch( touchIndex, MtVector2( currentPosition.x * RenderScale, currentPosition.y * RenderScale ) );
        
        // Remove this from the dictionary
        [availableLabels insertObject:label atIndex:0];
        
        CFDictionaryRemoveValue (touchToLabelMapping, (__bridge const void*)touch);
	}
}

-(void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	for (UITouch *touch in touches)
	{
        //NSLog( @"Touches moved %d\r\n", (int)touch.self );
        
		CGPoint currentPosition = [touch locationInView:self.view];
        
        // Get the label for this touch
        NSNumber *label = (__bridge NSNumber *)CFDictionaryGetValue(touchToLabelMapping, (__bridge const void*)touch);
        
        int touchIndex = [label integerValue];
        
        ShTouch::MoveTouch( touchIndex, MtVector2( currentPosition.x * RenderScale, currentPosition.y * RenderScale ) );
	}
}

-(void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
	for (UITouch *touch in touches)
	{
        CGPoint currentPosition = [touch locationInView:self.view];
        
        // Get the label for this touch
        NSNumber *label = (__bridge_transfer NSNumber *)CFDictionaryGetValue(touchToLabelMapping, (__bridge const void*)touch);
        
        //NSLog( @"Touches ended %d %d\r\n", (int)touch.self, [label integerValue] );
        
        int touchIndex = [label integerValue];
        
        ShTouch::EndTouch( touchIndex, MtVector2( currentPosition.x * RenderScale, currentPosition.y * RenderScale ) );
        
        // Remove this from the dictionary
        [availableLabels insertObject:label atIndex:0];
        
        CFDictionaryRemoveValue (touchToLabelMapping, (__bridge const void*)touch);
	}
}

static bool isShaken = false;

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event {
    
    if ( event.subtype == UIEventSubtypeMotionShake ) {
        NSLog( @"Device shaken" );
        isShaken = true;
    }
    
    if ([super respondsToSelector:@selector(motionEnded:withEvent:)]) {
        [super motionEnded:motion withEvent:event];
    }
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        printf( "setupGL. glError: 0x%04X", err);
}
-(void)captureBackCamera : (GLKView *)view
{
    NSError *error = nil;
    
    // create a low-quality capture session
    captureSession = [[AVCaptureSession alloc] init];
    
    // Setup the configuration
    [captureSession beginConfiguration];
    
    // We might need to consider this for older iPhones - AVCaptureSessionPreset352x288
    // Add inputs and outputs.
    if ([captureSession canSetSessionPreset:AVCaptureSessionPreset640x480])
    {
        captureSession.sessionPreset = AVCaptureSessionPreset640x480;
    }
    else {
        NSLog(@"Cannot set session preset to 640x480");
    }
    
    // Get the back facing camera
    AVCaptureDevice *captureDevice = nil;
    NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for( AVCaptureDevice *device in videoDevices )
    {
        if( device.position == AVCaptureDevicePositionBack )
        {
            captureDevice = device;
            break;
        }
    }
    //  couldn't find one on the front, so just get the default video device.
    if( captureDevice == nil )
    {
        captureDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
    
    // ...and a device input
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:&error];
    [captureSession addInput:input];
    
    // create a VideDataOutput to route output to us
    AVCaptureVideoDataOutput *output = [[AVCaptureVideoDataOutput alloc] init];
    [output setAlwaysDiscardsLateVideoFrames:YES];
    
    // set 32bpp BGRA pixel format, since I'll want to make sense of the frame
    [output setVideoSettings:
     [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]
                                 forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    
    //[output setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
    dispatch_queue_t queue = dispatch_queue_create("myQueue", NULL);
    [output setSampleBufferDelegate:self queue:queue];
    [captureSession addOutput:output];
    
    AVCaptureVideoPreviewLayer *previewLayer = [AVCaptureVideoPreviewLayer layerWithSession:captureSession];
    previewLayer.frame = view.bounds;
    previewLayer.hidden = true;
    [view.layer addSublayer:previewLayer];
    
    // Save this configuration
    [captureSession commitConfiguration];
    
    // Begin capturing the video
    [captureSession startRunning];
}

-(void)captureStop
{
    [captureSession stopRunning];
    self.captureSession = nil;
}

-(void)captureFrontCamera : (GLKView *)view
{
    NSError *error = nil;
    
    // create a low-quality capture session
    captureSession = [[AVCaptureSession alloc] init];
    
    // Setup the configuration
    [captureSession beginConfiguration];
    
    // We might need to consider this for older iPhones - AVCaptureSessionPreset352x288
    // Add inputs and outputs.
    if ([captureSession canSetSessionPreset:AVCaptureSessionPreset640x480])
    {
        captureSession.sessionPreset = AVCaptureSessionPreset640x480;
    }
    else {
        NSLog(@"Cannot set session preset to 640x480");
    }
    
    // Get the back facing camera
    AVCaptureDevice *captureDevice = nil;
    NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for( AVCaptureDevice *device in videoDevices )
    {
        if( device.position == AVCaptureDevicePositionFront )
        {
            captureDevice = device;
            break;
        }
    }
    //  couldn't find one on the front, so just get the default video device.
    if( captureDevice == nil )
    {
        captureDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }
    
    // ...and a device input
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:&error];
    [captureSession addInput:input];
    
    // create a VideDataOutput to route output to us
    AVCaptureVideoDataOutput *output = [[AVCaptureVideoDataOutput alloc] init];
    [output setAlwaysDiscardsLateVideoFrames:YES];
    
    // set 32bpp BGRA pixel format, since I'll want to make sense of the frame
    [output setVideoSettings:
     [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]
                                 forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    
    //[output setSampleBufferDelegate:self queue:dispatch_get_main_queue()];
    dispatch_queue_t queue = dispatch_queue_create("myQueue", NULL);
    [output setSampleBufferDelegate:self queue:queue];
    [captureSession addOutput:output];
    
    AVCaptureVideoPreviewLayer *previewLayer = [AVCaptureVideoPreviewLayer layerWithSession:captureSession];
    previewLayer.frame = view.bounds;
    previewLayer.hidden = true;
    [view.layer addSublayer:previewLayer];
    
    // Save this configuration
    [captureSession commitConfiguration];
    
    // Begin capturing the video
    [captureSession startRunning];
}

-(void) captureOutput:(AVCaptureOutput*)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection*)connection

{
    // Get the image buffer
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer( sampleBuffer );
    
    CVPixelBufferLockBaseAddress(imageBuffer,0);
    
    // Get the base address of the captured movie
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer);
    
    if( baseAddress != NULL )
    {
        //size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
        size_t width = CVPixelBufferGetWidth(imageBuffer);
        size_t height = CVPixelBufferGetHeight(imageBuffer);
        
        static int displayed = false;
        if( !displayed )
        {
           // NSLog( @"Bytes per row %d", (int)bytesPerRow );
           // NSLog( @"Width %d",  (int)width );
           // NSLog( @"Height %d", (int)height );
            displayed = true;
        }
        
        // Set the camera in our software. Eventually we will have a separate shared camera class to handle this
        pgCameraInput::SetData( (BtU8*)baseAddress, width, height );
        
        int a=0;
        a++;
    }
    
    // Release the image buffer
    CVPixelBufferUnlockBaseAddress(imageBuffer,0);
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
}

// http://forum.sparrow-framework.org/topic/create-uiimage-from-pixel-data-problems
-(UIImage *) imageFromTexturePixels:(unsigned char *)rawImageData
{
    UIImage *newImage = nil;
    
    int width  = 480;
    int height = 640;
    int nrOfColorComponents = 4; //RGBA
    int bitsPerColorComponent = 8;
    int rawImageDataLength = width * height * nrOfColorComponents;
    BOOL interpolateAndSmoothPixels = NO;
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    CGDataProviderRef dataProviderRef;
    CGColorSpaceRef colorSpaceRef;
    CGImageRef imageRef;
    
    BtU32 *source = (BtU32*)rawImageData;
    BtU32 size = width * height * nrOfColorComponents;
    BtMemory::Copy( buffer, rawImageData, size );
    
    BtU32 start = 0;
    for( BtU32 x=0; x<640; x++ )
    {
        for (BtU32 y = 0; y < 480; y++)
        {
            BtU32 offset = ( y * 640 ) + x;
            
            BtU32 word = *(source + offset);
            
            BtU8 result[4];
            result[3] = (word & 0xFF000000 ) >> 24;
            result[0] = (word & 0x00FF0000 ) >> 16;
            result[1] = (word & 0x0000FF00 ) >> 8;
            result[2] = (word & 0x000000FF );
            
            // Save the result
            *(buffer + start) = result[0] | (result[1] << 8 ) | (result[2] << 16) | result[3] << 24;
            ++start;
        }
    }
    
    @try
    {
        GLubyte *rawImageDataBuffer = (GLubyte *)buffer;
        dataProviderRef = CGDataProviderCreateWithData(NULL, rawImageDataBuffer, rawImageDataLength, nil);
        colorSpaceRef = CGColorSpaceCreateDeviceRGB();
        imageRef = CGImageCreate(width, height, bitsPerColorComponent, bitsPerColorComponent * nrOfColorComponents, width * nrOfColorComponents, colorSpaceRef, bitmapInfo, dataProviderRef, NULL, interpolateAndSmoothPixels, renderingIntent);
        newImage = [[UIImage alloc] initWithCGImage:imageRef];
    }
    @finally
    {
        CGDataProviderRelease(dataProviderRef);
        CGColorSpaceRelease(colorSpaceRef);
        CGImageRelease(imageRef);
    }
    return newImage;
}


// processEvents

-(void)processEvents
{
    if( ShCamera::GetNumItems() )
    {
        GLKView *view = (GLKView *)self.view;
        
        ShCameraAction action = ShCamera::PopAction();
        
        if( action.m_action == ShCameraActionType_CaptureBack )
        {
            [self captureBackCamera : view ];
        }
        else if( action.m_action == ShCameraActionType_CaptureFront )
        {
            [self captureFrontCamera : view ];
        }
        else if( action.m_action == ShCameraActionType_CaptureStop )
        {
            [self captureStop];
        }
        if( action.m_action == ShCameraActionType_SaveToAlbums )
        {
            // Make an image from the raw data
            UIImage *image = [self imageFromTexturePixels:action.m_pMemory];
            
            // Sae the image to the photo album
            UIImageWriteToSavedPhotosAlbum(image, nil, nil, nil);
           
            // write the image to our documents folder
            NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            NSString *filePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"portrait.png"];
            
            // Save image.
            [UIImagePNGRepresentation(image) writeToFile:filePath atomically:YES];
        }
    }
    if( ShRecorder::GetNumItems() )
    {
        BtU32 numItems =ShMail::GetNumItems();
        (void)numItems;
        
        // Peek at the mail packet
        ShRecorderAction action = ShRecorder::PopAction();
        
        if( action.m_action == ShRecorderActionType_Record )
        {
            NSLog(@"prepareToRecord");
            
            if ([audioRecorder prepareToRecord] == YES){
                 NSLog(@"Started recording");
                [audioRecorder recordForDuration:RECORDTIME];
            }
        }
        else if( action.m_action == ShRecorderActionType_Stop )
        {
            NSLog(@"Stopped");
            [audioPlayer stop];
        }
    }
    
    if( ShMail::GetNumItems() )
    {
        BtU32 numItems =ShMail::GetNumItems();
        (void)numItems;
        
        // Peek at the mail packet
        ShMailAction action = ShMail::PopAction();
        
        // http://stackoverflow.com/questions/7824647/send-an-iphone-attachment-through-email-programmatically
        // Shall we add a badge?
        if( action.m_action == ShMailActionType_Send )
        {
            if( [MFMailComposeViewController canSendMail] )
            {
                MFMailComposeViewController *mailer = [[MFMailComposeViewController alloc] init];
                
                mailer.mailComposeDelegate = self;
                
                NSString *emailRecipients = [NSString stringWithFormat:@"%s", action.m_email];
                [mailer setToRecipients:[NSArray arrayWithObjects:emailRecipients, nil]];
                
                NSString *emailSubject = [NSString stringWithFormat:@"%s", action.m_subject];
                [mailer setSubject:emailSubject];
                
                NSString *emailBody = [NSString stringWithFormat:@"%s", action.m_body];
                [mailer setMessageBody:emailBody isHTML:NO];
                
                // Make an image from the raw data
                NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                NSString *attachmentFilename = [NSString stringWithFormat:@"%s", action.m_filename];
                NSString *filePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:attachmentFilename];
                NSData *data = [[NSFileManager defaultManager] contentsAtPath:filePath];
                if( [data length] )
                {
                    NSString *mimeType           = [NSString stringWithFormat:@"%s", action.m_mimeType];
                    NSString *attachmentFilename = [NSString stringWithFormat:@"%s", action.m_filename];
                    [mailer addAttachmentData:data mimeType:mimeType fileName:attachmentFilename ];
                }
                [self presentViewController:mailer animated:YES completion:nil];
            }
            else
            {
                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Failure"
                                                                message:@"Your device doesn't support the composer sheet"
                                                               delegate:nil
                                                      cancelButtonTitle:@"OK"
                                                      otherButtonTitles:nil];
                [alert show];
            }
        }
    }
    /*
    if( ShBadge::GetNumItems() )
    {
        // Peek at the vibration action
        ShBadgeAction action = ShBadge::PopAction();
        
        // Shall we add a badge?
        if( action == ShBadgeAction_Add )
        {
            [[UIApplication sharedApplication] setApplicationIconBadgeNumber:1];
        }
        
        // Shall we start vibrating
        if( action == ShBadgeAction_Remove )
        {
            [[UIApplication sharedApplication] setApplicationIconBadgeNumber:0];
            [[UIApplication sharedApplication] cancelAllLocalNotifications];
        }
    }
    */
    if( ShVibration::GetNumItems() )
    {
        // Peek at the vibration action
        ShVibrationAction action = ShVibration::PeekAction();
        
        // Shall we start vibrating
        if( action.m_type == ShVibrationActionType_Start )
        {
            // Pop the action
            ShVibration::PopAction();
            
            // Vibrate the phone
            [self vibrate];
        }
    }
}

- (void)mailComposeController:(MFMailComposeViewController *)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError *)error
{
    switch (result) {
        case MFMailComposeResultSent:
            NSLog(@"You sent the email.");
            break;
        case MFMailComposeResultSaved:
            NSLog(@"You saved a draft of this email");
            break;
        case MFMailComposeResultCancelled:
            NSLog(@"You cancelled sending this email.");
            break;
        case MFMailComposeResultFailed:
            NSLog(@"Mail failed:  An error occurred when trying to compose this email");
            break;
        default:
            NSLog(@"An error occurred when trying to compose this email");
            break;
    }
    
    [self dismissViewControllerAnimated:YES completion:NULL];
}

-(void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
    //ShMusicAction action;
    //action.m_type = ShMusicActionType_DidFinish;
    //ShMusic::PushAlert( action );
}

static bool isUpdated = false;

- (void)update
{
    // Update the touch
    ShTouch::Update();
    
    if( isShaken )
    {
        // Set the device to indicate it's been shaken
        ShTouch::SetShaken();
        
        // Don't do this again
        isShaken = false;
    }
    
    // Update the project
    myProject.Update();
    
    // Look for programmed events
    [self processEvents];
    
    isUpdated = true;
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    if( isUpdated == true )
    {
        // Empty render targets
        RsImpl::pInstance()->EmptyRenderTargets();
        
        // Render the project
        myProject.Render();
        
        // Render
        RsImpl::pInstance()->Render();
    }
}

@end
