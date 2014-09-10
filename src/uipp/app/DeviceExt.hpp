#ifndef UIPP_APP_DEVICE_EXT_H
#define UIPP_APP_DEVICE_EXT_H
#include "uipp/app/Device.hpp"
#include "System.hpp"

namespace UI { namespace App {

class DeviceExt : public Device {
public:
    DeviceExt();
    ~DeviceExt();

	virtual Sprite::P2D::Rect const & renderRect(void) const {
        return m_renderRect;
    }

    void setRenderRect(int x, int y, int w, int h);

private:
    Sprite::P2D::Rect m_renderRect;
};

}}

#endif
