#ifndef UIPP_SPRITE_R_LAYER_H
#define UIPP_SPRITE_R_LAYER_H
#include "System.hpp"

namespace UI { namespace Sprite { namespace R {

class Layer {
public:
    virtual const char * name(void) const = 0;
    virtual ~Layer();
};

}}}

#endif
