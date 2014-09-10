#include "DeviceExt.hpp"

namespace UI { namespace App {

Device::~Device() {
}

DeviceExt::DeviceExt() {
}

DeviceExt::~DeviceExt() {
}

void DeviceExt::setRenderRect(int x, int y, int w, int h) {
    m_renderRect.lt.x = (float)x;
    m_renderRect.lt.y = (float)y;
    m_renderRect.rb.x = (float)(x + w);
    m_renderRect.rb.y = (float)(y + h);
}

}}

