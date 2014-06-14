#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_anim/AnimationSch.hpp"

namespace UI { namespace Sprite { namespace Anim {

AnimationDef & AnimationSch::createAnimationDef(const char * anim_name, const char * res, bool auto_start) {
    ui_sprite_anim_def_t anim_def = ui_sprite_anim_def_create(*this, anim_name, res, auto_start ? 1 : 0);

    if (anim_def == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().app(),
            ::std::runtime_error,
            "addAnimation: %s => %s add error!", anim_name, res);
    }

    return *(AnimationDef*)anim_def;
}

AnimationDef const & AnimationSch::animationDef(const char * anim_name) const {
    AnimationDef const * def = findAnimationDef(anim_name);

    if (def == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world().app(),
            ::std::runtime_error,
            "addAnimation: %s not exist!", anim_name);
    }

    return *def;
}

const char * AnimationSch::NAME = UI_SPRITE_ANIM_SCH_NAME;

}}}
