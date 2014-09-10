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

CGRect  GetRenderRectInPixels()
{
    CGRect rt;
    
    GetDeviceType();
    switch (deviceType) {
        case Iphone:
            rt = CGRectMake(0, 0, 480, 320);
            break;
        case IphoneHD:
            rt = CGRectMake(0, 0,  960, 640);
            break;
        case Ipad:
            rt = CGRectMake(0, 0, 1024, 768);
            break;
        case IpadHD:
            rt = CGRectMake(0, 0, 2048, 1536);
            break;
        case Iphone5:
            rt = CGRectMake(0, 0, 1136, 640);
            break; 
        default:
            break;
    }
    return rt;
    
}

CGPoint  ToGamePoint( CGPoint viewPointInPixels )
{
    CGRect rt = GetRenderRectInPixels();
    viewPointInPixels.x -= rt.origin.x;
    viewPointInPixels.y -= rt.origin.y;
    
    return viewPointInPixels;
}

CGPoint  ToViewPointInPixels( CGPoint gamePoint )
{
    CGRect rt = GetRenderRectInPixels();
    
    gamePoint.x += rt.origin.x;
    gamePoint.y += rt.origin.y;
    
    return gamePoint;
}

CGSize ToViewSizeInPixels( CGSize gameSize )
{
    //CGRect rt = GetRenderRectInPixels();
    
    return gameSize;
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

CGRect ToViewRectInPoints( CGRect rect ,float scale )
{
    CGRect rt;
    rt.origin = ToViewPointInPixels(rect.origin);
    rt.origin.x /= scale;
    rt.origin.y /= scale;
    
    rt.size = ToViewSizeInPixels(rect.size);
    rt.size.width /= scale;
    rt.size.height /= scale;
    
    return rt;
}

CGRect ToGameRect( CGRect rect, float scale )
{
    CGRect rt = rect;
    rt.origin.x *= scale;
    rt.origin.y *= scale;
    
    rt.size.width *= scale;
    rt.size.height *= scale;
    
    CGPoint bound = CGPointMake(rt.origin.x + rt.size.width, rt.origin.y + rt.size.height);
    rt.origin = ToGamePoint(rt.origin);
    bound = ToGamePoint(bound);
    rt.size = CGSizeMake(bound.x - rt.origin.x, bound.y - rt.origin.y);
    return rt;
}
