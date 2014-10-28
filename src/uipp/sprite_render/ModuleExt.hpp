#ifndef UIPP_SPRITE_R_MODULE_EXT_H
#define UIPP_SPRITE_R_MODULE_EXT_H
#include "uipp/sprite_render/Module.hpp"

namespace UI { namespace Sprite { namespace R {

struct node_builder {
    char m_name[64];
    node_build_fun_t m_build_fun;
    void * m_build_ctx;
};

class ContextExt;
class uipp_sprite_render_ext : public uipp_sprite_render {
public:
    virtual Gd::App::Application & app(void)  = 0;
    virtual Gd::App::Application const & app(void) const = 0;

    virtual node_builder const * findNodeBuilder(const char * name, size_t name_len) const = 0;

    virtual void registerContext(ContextExt & ctx) = 0;
    virtual void unregisterContext(ContextExt & ctx) = 0;

    virtual ~uipp_sprite_render_ext();
};

}}}

#endif
