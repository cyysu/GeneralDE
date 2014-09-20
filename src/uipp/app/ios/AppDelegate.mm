#import "gd/app/app_context.h"
#import "gd/app/app_module.h"
#import "gdpp/app/Log.hpp"
#import "gdpp/app/Application.hpp"
#import "AppDelegate.h"
#import "ViewController.h"
#import "m3eImeController.h"
#import "common.h"
#import "UncaughtExceptionHandler.h"
#import "EAGLView.h"
#import "../EnvExt.hpp"

char g_AppPath[256];
static void (*s_mailSuccessCallback)() = NULL;
static void (*s_mailFailCallback)() = NULL;
static void (*s_lbsSuccessCallback)() = NULL;
static void (*s_lbsFailCallback)(int errorCode) = NULL;


@implementation AppDelegate

@synthesize window = _window;
@synthesize viewController = _viewController;
@synthesize locationManager = _locationManager;

-(id) init
{
    if ( self = [super init] )
    {
        m_app = NULL;
        _longitude = 0.0f;
        _latitude = 0.0f;
    }
    return self;
}

- (void) dealloc
{
    [_window release];
    [_viewController release];
    self.indicator = nil;
    self.loadingLabel = nil;
    [super dealloc];
}

-(void) initAppPath
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	CFBundleRef bundle;
	CFURLRef bundle_url;
	
	if( !(bundle = CFBundleGetMainBundle()) )
	{
		return;
	}
	
	// Get URL of the Resource directory of the .app bundle
	bundle_url = CFBundleCopyResourcesDirectoryURL(bundle);
	if (!bundle_url)
	{
		CFRelease(bundle_url);
		return;
	}
	
	// Try to convert the URL to an absolute path
	UInt8 buf[FILENAME_MAX];
	if (!CFURLGetFileSystemRepresentation(bundle_url, true, buf, sizeof(buf)))
	{
		CFRelease(bundle_url);
		return;
	}
	
	//save current application absolute path
	int length = strlen((const char *)buf);
	strcpy(g_AppPath, (const char *)buf);
	
	if (buf[length - 1] != '/')
		strcat(g_AppPath, "/");
	
	chdir( g_AppPath );
	
#ifdef DEBUG
	NSLog(@"application path = %s" , g_AppPath);
#endif
	[pool release];
}

- (void) initAppEnv
{
    char prog_name[] = "PlayerShareData";
    char * argv[] = { prog_name};

    m_app = gd_app_context_create_main(NULL, 0, 1, argv);
    if (m_app == NULL) {
        assert(false);
        throw ::std::runtime_error("create app fail!");
    }

    gd_app_set_debug(m_app, 1);
    gd_app_ins_set(m_app);

    if (gd_app_cfg_reload(m_app) != 0) {
        APP_CTX_ERROR(m_app, "PlayerShareData: load cfg fail!");
        gd_app_context_free(m_app);
        assert(false);
        throw ::std::runtime_error("create app fail!");
    }

    if (gd_app_modules_load(m_app) != 0) {
        gd_app_context_free(m_app);
        APP_CTX_ERROR(m_app, "PlayerShareData: create app load module fail!");
        assert(false);
        throw ::std::runtime_error("create app fail!");
    }
}

extern void InstallUncaughtExceptionHandler();

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    //    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    //    // Override point for customization after application launch.
    //    self.window.backgroundColor = [UIColor whiteColor];
    //    [self.window makeKeyAndVisible];
    //    return YES;
    @autoreleasepool {
        chdir([[[NSBundle mainBundle] bundlePath]UTF8String]);
    }
    
    InstallUncaughtExceptionHandler();
    
    [self initAppPath];
    
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    
    EAGLView *__glView = [[EAGLView alloc] initWithFrame: [self.window bounds] ];

    //                                  pixelFormat: kEAGLColorFormatRGBA8
    //                                  depthFormat: GL_DEPTH24_STENCIL8_OES
    //                           preserveBackbuffer: NO
    //                                   sharegroup: nil
    //                                multiSampling: NO
    //                              numberOfSamples: 0];

    self.viewController = [[ViewController alloc] initWithNibName:nil bundle:nil];
    self.viewController.wantsFullScreenLayout = YES;
    self.viewController.view = __glView;

    [self.viewController viewDidLoad];

    self.window.rootViewController = self.viewController;
    //    [self.window addSubview:self.viewController.view];
    [self.window makeKeyAndVisible];
    
    //enable multi touch
    self.viewController.view.multipleTouchEnabled = YES;
    [self.window addSubview:[m3eImeController sharedInstance].view];
    
    [self initAppEnv];

    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    CGSize sz = GetScreenSizeInPixels();

    UI::Sprite::P2D::Pair const & screenBaseSize = env.screenBaseSize();
    if ((screenBaseSize.x > screenBaseSize.y && sz.width < sz.height)
        || (screenBaseSize.x < screenBaseSize.y && sz.width > sz.height))
    {
        int32_t t = sz.width;
        sz.width = sz.height ;
        sz.height = t;
    }

    env.runing().init();
    env.runing().setSize(sz.width, sz.height);
    env.device().setRenderRect(0, 0, sz.width, sz.height);
    glViewport(0, 0, sz.width, sz.height);
    // CGRect rt = GetRenderRectInPixels();
    // glViewport(rt.origin.x, rt.origin.y, rt.size.width, rt.size.height);

    application.applicationIconBadgeNumber = 0;
    
    [ application cancelAllLocalNotifications ];
    
    //make screen always lighting in game
    [ UIApplication sharedApplication].idleTimerDisabled = YES;
    //if ( GetDeviceType() == Iphone || GetDeviceType() == IphoneHD )
    //[ application setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft];
    self.locationManager = [[CLLocationManager alloc] init];
    self.locationManager.delegate = self;
    self.locationManager.desiredAccuracy = kCLLocationAccuracyBest;
    self.locationManager.distanceFilter = 5.0;

    env.uiCenter().initPhase();

    env.runing().update();
    env.runing().rend();
	
    return YES;
}


- ( void ) application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
    application.applicationIconBadgeNumber = 0;
    //    NSString* identifier = [ notification.userInfo objectForKey: @"CallBackGame" ];
    //    if (identifier)
    //    {
    //        notification.fireDate   =   [ NSDate dateWithTimeIntervalSinceNow:10 ];
    //        [ [ UIApplication sharedApplication ] scheduleLocalNotification: notification ];
    //    }
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */

    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    env.runing().pause();

    [self.viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
    int iCount = [ [ [ UIApplication sharedApplication ] scheduledLocalNotifications ] count ];
    NSLog( @"Location Number: %d\n", iCount );
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
    //    m3eFrameWorkResume();
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
    [self.viewController startAnimation];
    application.applicationIconBadgeNumber = 0;

    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

    env.runing().resume();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
    gd_app_context_free(m_app);
    m_app = NULL;
    //    [[m3eImeController sharedInstance] release];
}

- ( void ) application : ( UIApplication* )application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
    
}

- ( void ) application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
    NSString* desc = [ error description ];
    NSLog( @"Fail Register Notificaction: %@", desc );
    
}

-(void) SetInteraction:(BOOL)allow onView:(UIView*) aView
{
    aView.userInteractionEnabled = allow;
    for( UIView* v in aView.subviews )
    {
        [self SetInteraction:allow onView:v];
    }
}

- (void)sendMail:(NSString*)subject body:(NSString *)messageStr sender:(NSString*)sender recipient:(NSString*)recipients
{
	SKPSMTPMessage *test_smtp_message = [[SKPSMTPMessage alloc] init];
    test_smtp_message.delegate = self;
    
    NSMutableArray *parts_to_send = [NSMutableArray array];
    
	
	NSDictionary *plainPart = [NSDictionary dictionaryWithObjectsAndKeys:
							   @"text/plain;charset=UTF-8;", kSKPSMTPPartContentTypeKey,
							   [messageStr stringByAppendingString:@"\n"], kSKPSMTPPartMessageKey,
							   nil];
	
    [parts_to_send addObject:plainPart];
    
    test_smtp_message.parts = parts_to_send;
	
    [test_smtp_message send];
}

- (void)getLocation
{
    if([CLLocationManager locationServicesEnabled])
        [self.locationManager startUpdatingLocation];
    else
    {
        if(s_lbsFailCallback)
        {
            s_lbsFailCallback(0);
            s_lbsFailCallback = NULL;
            s_lbsSuccessCallback = NULL;
        }
    }
}

- (double)getLongitude
{
    return _longitude;
}

- (double)getLatitude
{
    return _latitude;
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
    return false;//[WGInterface HandleOpenURL:url];
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    return false;//[WGInterface  HandleOpenURL:url];
}


#pragma mark SKPSMTPMessage Delegate Methods
- (void)messageSent:(SKPSMTPMessage *)SMTPmessage

{
    //[SMTPmessage release];
    if (s_mailSuccessCallback)
    {
        s_mailSuccessCallback();
        s_mailSuccessCallback = NULL;
        s_mailFailCallback = NULL;
    }
}
- (void)messageFailed:(SKPSMTPMessage *)SMTPmessage error:(NSError *)error
{
    //[SMTPmessage release];
    if (s_mailFailCallback)
    {
        s_mailFailCallback();
        s_mailSuccessCallback = NULL;
        s_mailFailCallback = NULL;
    }
}

#pragma mark CLLocationManager Delegate Methods
- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
    CLLocationCoordinate2D coordinate = newLocation.coordinate;
    [self.locationManager stopUpdatingLocation];
    _longitude = coordinate.longitude;
    _latitude = coordinate.latitude;
    if(s_lbsSuccessCallback)
    {
        s_lbsSuccessCallback();
        s_lbsSuccessCallback = NULL;
        s_lbsFailCallback = NULL;
    }
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
    [self.locationManager stopUpdatingLocation];
    if(s_lbsFailCallback)
    {
        if(error.code == kCLErrorDenied)
        {
            s_lbsFailCallback(0);
        }
        else
        {
            s_lbsFailCallback(1);
        }
        s_lbsFailCallback = NULL;
        s_lbsSuccessCallback = NULL;
    }
}
@end
/*s
 void DeviceGetLocation(void (*successCallback)(), void (*failCallback)(int))
 {
 AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
 s_lbsSuccessCallback   = successCallback;
 s_lbsFailCallback      = failCallback;
 [appDelegate getLocation];
 
 }
 
 double GetLongitude()
 {
 AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
 return [appDelegate getLongitude];
 }
 
 double GetLatitude()
 {
 AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
 return [appDelegate getLatitude];
 }
 
 
 
 void feedback(std::string subject, std::string message, std::string sender, std::string recipients, void (*successCallback)(), void (*failCallback)())
 {
 s_mailSuccessCallback = successCallback;
 s_mailFailCallback = failCallback;
 AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
 NSString* nsMessage = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
 NSString* nsSubject = [NSString stringWithCString:subject.c_str() encoding:NSUTF8StringEncoding];
 NSString* nsSender = [NSString stringWithCString:sender.c_str() encoding:NSUTF8StringEncoding];
 NSString* nsRecipients = [NSString stringWithCString:recipients.c_str() encoding:NSUTF8StringEncoding];
 
 [appDelegate sendMail:nsSubject body:nsMessage sender:nsSender recipient:nsRecipients];
 }
 */
