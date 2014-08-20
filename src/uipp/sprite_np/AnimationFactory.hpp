#ifndef UIPP_SPRITE_NP_ANIMATIONFACTORY_H
#define UIPP_SPRITE_NP_ANIMATIONFACTORY_H
#include "uipp/sprite_np/System.hpp"
#include "NPNode.h"

namespace UI { namespace Sprite { namespace NP {

class ContextExt;
class AnimationFactory {
public:
    virtual NPNode *
    createAnimation(
        Gd::App::Application & app,
        ContextExt * contextExt,
        const char * res, 
        const uint8_t is_loop, const int32_t start, const int32_t end) = 0;
    
    virtual ~AnimationFactory();

    static AnimationFactory & instance(void);
};

}}}

#endif
