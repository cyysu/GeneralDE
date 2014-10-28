#ifndef UIPP_SPRITE_R_MODULE_H
#define UIPP_SPRITE_R_MODULE_H
#include "gdpp/app/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "System.hpp"

namespace UI { namespace Sprite { namespace R {

class uipp_sprite_render : public Cpe::Nm::Object {
public:
    virtual void render(void) = 0;

    virtual void registerRender(const char * name, node_build_fun_t build_fun, void * build_ctx) = 0;
    virtual void unregisterRender(const char * name) = 0;

    virtual ~uipp_sprite_render();

    static uipp_sprite_render & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}}

#endif
