#include "uipp/sprite/WorldRes.hpp"
#include "uipp/sprite_anim/AnimationBackend.hpp"

namespace UI { namespace Sprite { namespace Anim {

WorldRes & AnimationBackend::worldRes(void) {
    return *(WorldRes*)ui_sprite_world_res_from_data(*this);
}

WorldRes const & AnimationBackend::worldRes(void) const {
    return *(WorldRes const *)ui_sprite_world_res_from_data(*this);
}

World & AnimationBackend::world(void) {
    return worldRes().world();
}

World const & AnimationBackend::world(void) const {
    return worldRes().world();
}

const char * AnimationBackend::NAME = UI_SPRITE_ANIM_BACKEND_NAME;

}}}
