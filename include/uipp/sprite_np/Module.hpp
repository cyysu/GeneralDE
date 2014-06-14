#ifndef UIPP_SPRITE_NP_MODULE_H
#define UIPP_SPRITE_NP_MODULE_H
#include "gdpp/app/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "System.hpp"

namespace UI { namespace Sprite { namespace NP {

class uipp_sprite_np : public Cpe::Nm::Object {
public:
    virtual void render(void) = 0;

    virtual ~uipp_sprite_np();

    static uipp_sprite_np & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}}

#endif
