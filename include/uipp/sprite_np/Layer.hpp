#ifndef UIPP_SPRITE_NP_LAYER_H
#define UIPP_SPRITE_NP_LAYER_H
#include "System.hpp"

namespace UI { namespace Sprite { namespace NP {

class Layer {
public:
    virtual const char * name(void) const = 0;
    virtual ~Layer();
};

}}}

#endif
