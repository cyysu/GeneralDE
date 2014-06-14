#ifndef UIPP_SPRITE_WORLD_UPDATOR_GEN_H
#define UIPP_SPRITE_WORLD_UPDATOR_GEN_H
#include "cpepp/utils/TypeUtils.hpp"
#include "World.hpp"

namespace UI { namespace Sprite {

template<typename OuterT>
class WorldUpdatorGen {
    static void do_update(ui_sprite_world_t world, void * ctx, float delta) {
        try {
            ((OuterT*)ctx)->onWorldUpdate(*(World*)world, delta);
        }
        catch(...) {
        }
    }

public:
    WorldUpdatorGen(World & world) : m_world(world) {
        world.addUpdator(do_update, (void*)Cpe::Utils::calc_cast<OuterT>(this));
    }

    ~WorldUpdatorGen() {
        m_world.removeUpdator((void*)Cpe::Utils::calc_cast<OuterT>(this));
    }

    World & m_world;
};

}}

#endif
