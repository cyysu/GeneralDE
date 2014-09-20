#import "common.h"

static DeviceType deviceType = Unknown;

DeviceType GetDeviceType()
{
    if ( Unknown == deviceType )
    {
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
        {
            if ( [UIScreen mainScreen].scale > 1.f )
            {
                if ( [[UIScreen mainScreen] bounds].size.height > 480 )
                    deviceType = Iphone5;
                else
                    deviceType = IphoneHD;
            }
            else
                deviceType = Iphone;
        }
        else if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        {
            if ( [UIScreen mainScreen].scale > 1.f )
            {
                deviceType = IpadHD;
            }
            else
            {
                deviceType = Ipad;
            }
        }
    }
    return deviceType;
}

//TODO clean
CGSize GetScreenSizeInPixels()
{
    CGSize sz;
    
    GetDeviceType();
    switch (deviceType) {
        case Iphone:
            sz = CGSizeMake(480, 320);
            break;
        case IphoneHD:
            sz = CGSizeMake(960, 640);
            break;
        case Ipad:
            sz = CGSizeMake(1024, 768);
            break;
        case IpadHD:
            sz = CGSizeMake(2048, 1536);
            break;
        case Iphone5:
            sz = CGSizeMake(1136, 640);
            break;
        default:
            break;
    }
    return sz;
}
