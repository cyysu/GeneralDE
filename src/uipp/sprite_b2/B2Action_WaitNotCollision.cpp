#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "B2Action_WaitNotCollision.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_WaitNotCollision::B2Action_WaitNotCollision(Fsm::Action & action)
    : ActionBase(action)
    , m_categories(0)
{
}

B2Action_WaitNotCollision::B2Action_WaitNotCollision(Fsm::Action & action, B2Action_WaitNotCollision const & o)
    : ActionBase(action)
    , m_categories(o.m_categories)
{
}

int B2Action_WaitNotCollision::enter(void) {
    if (entity().component<B2ObjectExt>().isCollisionWith(m_categories)) {
        startUpdate();
    }

    return 0;
}

void B2Action_WaitNotCollision::exit(void) {
}

void B2Action_WaitNotCollision::update(float delta) {
    if (!entity().component<B2ObjectExt>().isCollisionWith(m_categories)) {
        stopUpdate();
        return;
    }
}

void B2Action_WaitNotCollision::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_WaitNotCollision>(repo)
        .on_enter(&B2Action_WaitNotCollision::enter)
        .on_exit(&B2Action_WaitNotCollision::exit)
        .on_update(&B2Action_WaitNotCollision::update)
        ;
}

const char * B2Action_WaitNotCollision::NAME = "b2-wait-not-collision";

}}}

