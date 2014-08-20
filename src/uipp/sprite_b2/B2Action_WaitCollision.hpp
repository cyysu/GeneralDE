#ifndef UIPP_SPRITE_B2_ACTION_WAIT_COLLISION_H
#define UIPP_SPRITE_B2_ACTION_WAIT_COLLISION_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "B2Collision.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartExt;
class B2Action_WaitCollision : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_WaitCollision> {
public:
    B2Action_WaitCollision(Fsm::Action & action);
    B2Action_WaitCollision(Fsm::Action & action, B2Action_WaitCollision const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    B2Collision & collision(void) { return m_collision; }

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    void onCollision(UI_SPRITE_B2_COLLISION_DATA const & data);

    B2Collision m_collision;

    static void callWaitCollision(void * ctx, UI_SPRITE_B2_COLLISION_DATA const & data);
};

}}}

#endif
