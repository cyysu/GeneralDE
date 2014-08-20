#ifndef UIPP_SPRITE_NP_MODULE_EXT_H
#define UIPP_SPRITE_NP_MODULE_EXT_H
#include "uipp/sprite_np/Module.hpp"

namespace UI { namespace Sprite { namespace NP {

class ContextExt;
class uipp_sprite_np_ext : public uipp_sprite_np {
public:
    virtual Gd::App::Application & app(void)  = 0;
    virtual Gd::App::Application const & app(void) const = 0;

    virtual void registerContext(ContextExt & ctx) = 0;
    virtual void unregisterContext(ContextExt & ctx) = 0;

    virtual ~uipp_sprite_np_ext();
};

}}}

#endif
