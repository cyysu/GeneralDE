#ifndef Application_common_h
#define Application_common_h

#pragma once

#import <UIKit/UIKit.h> 

//rechardchen: for universal version
enum DeviceType {
    Unknown,
    Iphone,
    IphoneHD,
    Ipad,
    IpadHD,
    Iphone5,
};

DeviceType GetDeviceType();
CGRect GetRenderRectInPixels();
CGPoint ToGamePoint( CGPoint viewPointInPixels );
CGPoint ToViewPointInPixels( CGPoint gamePoint );
CGSize ToViewSizeInPixels( CGSize gameSize );

CGRect ToViewRectInPoints( CGRect rect , float scale = 1.f);
CGRect ToGameRect( CGRect rect, float scale = 1.f );//rect is in point

CGSize GetScreenSizeInPixels();

#endif
