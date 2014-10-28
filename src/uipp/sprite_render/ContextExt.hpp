#ifndef UIPP_SPRITE_R_CONTEXT_EXT_H
#define UIPP_SPRITE_R_CONTEXT_EXT_H
#include <memory>
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite_render/Context.hpp"
#include "ModuleExt.hpp"

namespace UI { namespace Sprite { namespace R {
class RControlNode;
class ContextExt : public Context {
public:
    virtual uipp_sprite_render_ext & module(void) = 0;
    virtual uipp_sprite_render_ext const & module(void) const = 0;

    virtual void destory(void) = 0;
    virtual void render(void) = 0;
};

}}}

#endif
