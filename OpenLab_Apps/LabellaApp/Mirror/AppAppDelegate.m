//
//  AppAppDelegate.m
//

#import "AppAppDelegate.h"
#import "AppViewController.h"

@implementation AppAppDelegate

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{
    switch (buttonIndex) {
        case 1:
           //[[GAI sharedInstance] setOptOut:YES];
            break;
        case 0:
           // [[GAI sharedInstance] setOptOut:NO];
            break;
            
        default:
            break;
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        self.viewController = [[AppViewController alloc] initWithNibName:@"AppViewController_iPhone" bundle:nil];
    } else {
        self.viewController = [[AppViewController alloc] initWithNibName:@"AppViewController_iPad" bundle:nil];
    }
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    
/*
    // 1
    [GAI sharedInstance].trackUncaughtExceptions = NO;
    
    // 2
    [[GAI sharedInstance].logger setLogLevel:kGAILogLevelVerbose];
    
    // 3
    [GAI sharedInstance].dispatchInterval = 20;
    
    // 4
    id<GAITracker> tracker = [[GAI sharedInstance] trackerWithTrackingId:@"UA-51824187-1"];
    (void)tracker;

    NSString *top = @"The developers of this game would like to record how you play this game";
    NSString *bot = @"We won’t record pictures, sound or anything personal. Is that ok?";
    
    UIAlertView *av = [[UIAlertView alloc] initWithTitle:top message:bot delegate:self cancelButtonTitle:@"Yes" otherButtonTitles:@"No", nil];
    [av show];
*/
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
