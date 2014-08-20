#include "cpe/utils/string_utils.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "B2Action_WaitCollision.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_WaitCollision::B2Action_WaitCollision(Fsm::Action & action)
    : ActionBase(action)
    , m_collision(action.name(), callWaitCollision, this)
{
}

B2Action_WaitCollision::B2Action_WaitCollision(Fsm::Action & action, B2Action_WaitCollision const & o)
    : ActionBase(action)
    , m_collision(action.name(), callWaitCollision, this, o.m_collision)
{
}

int B2Action_WaitCollision::enter(void) {
    entity().component<B2ObjectExt>().addCollision(m_collision);
    startUpdate();
    return 0;
}

void B2Action_WaitCollision::exit(void) {
    entity().component<B2ObjectExt>().removeCollision(m_collision);
}

void B2Action_WaitCollision::update(float delta) {
    if (entity().component<B2ObjectExt>().isCollisionWith(m_collision.collisions())) {
        stopUpdate();
    }
}

void B2Action_WaitCollision::onCollision(UI_SPRITE_B2_COLLISION_DATA const & data) {
    if (entity().component<B2ObjectExt>().isCollisionWith(m_collision.collisions())) {
        stopUpdate();
    }
}

void B2Action_WaitCollision::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_WaitCollision>(repo)
        .on_enter(&B2Action_WaitCollision::enter)
        .on_exit(&B2Action_WaitCollision::exit)
        .on_update(&B2Action_WaitCollision::update)
        ;
}

void B2Action_WaitCollision::callWaitCollision(void * ctx, UI_SPRITE_B2_COLLISION_DATA const & data) {
    ((B2Action_WaitCollision*)ctx)->onCollision(data);
}

const char * B2Action_WaitCollision::NAME = "b2-wait-collision";

}}}

