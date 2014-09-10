#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>
#import "SKPSMTPMessage/SKPSMTPMessage.h"
#import "SKPSMTPMessage/NSData+Base64Additions.h"
#include "gd/app/app_context.h"

@class ViewController;



@interface AppDelegate : UIResponder <UIApplicationDelegate, SKPSMTPMessageDelegate, CLLocationManagerDelegate>
{
    gd_app_context_t m_app;
    double                           _longitude;
    double                           _latitude;
}

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;

@property ( nonatomic, retain ) UIActivityIndicatorView*     indicator;
@property ( nonatomic, retain ) UILabel*                     loadingLabel;

@property (nonatomic, retain) CLLocationManager* locationManager;


-(void)SetInteraction:(BOOL)allow onView:(UIView*)aView;
-(void)sendMail:(NSString*)subject body:(NSString *)messageStr sender:(NSString*)sender recipient:(NSString*)recipients;

@end
