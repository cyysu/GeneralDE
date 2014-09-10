#ifndef UIPP_APP_DEVICE_H
#define UIPP_APP_DEVICE_H
#include "System.hpp"
#include "uipp/sprite_2d/System.hpp"

namespace UI { namespace App {

class Device {
public:
	virtual Sprite::P2D::Rect const & renderRect(void) const = 0;

    virtual ~Device();
};

}}

#endif
