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

CGSize GetScreenSizeInPixels();

#endif
