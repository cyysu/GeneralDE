#ifndef UIPP_SPRITE_R_CONTEXT_EXT_H
#define UIPP_SPRITE_R_CONTEXT_EXT_H
#include <memory>
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite_np/Context.hpp"

namespace UI { namespace Sprite { namespace R {
class RControlNode;
class ContextExt : public Context {
public:
    virtual void destory(void) = 0;
    virtual void render(void) = 0;
};

}}}

#endif
