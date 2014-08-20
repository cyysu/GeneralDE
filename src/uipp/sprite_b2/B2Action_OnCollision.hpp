#ifndef UIPP_SPRITE_B2_ACTION_ON_COLLISION_H
#define UIPP_SPRITE_B2_ACTION_ON_COLLISION_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "B2Collision.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartExt;
class B2Action_OnCollision : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_OnCollision> {
public:
    B2Action_OnCollision(Fsm::Action & action);
    B2Action_OnCollision(Fsm::Action & action, B2Action_OnCollision const & o);

    int enter(void);
    void exit(void);

    B2Collision & collision(void) { return m_collision; }

    void setOnCollisionBegin(::std::string const & on_begin);
    void setOnCollisionEnd(::std::string const & on_end);

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    void onCollision(UI_SPRITE_B2_COLLISION_DATA const & data);

    B2Collision m_collision;
    ::std::string m_on_collision_begin;
    ::std::string m_on_collision_end;

    static void callOnCollision(void * ctx, UI_SPRITE_B2_COLLISION_DATA const & data);
};

}}}

#endif
