#ifndef UIPP_SPRITE_B2_ACTION_WAIT_NOT_COLLISION_H
#define UIPP_SPRITE_B2_ACTION_WAIT_NOT_COLLISION_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartExt;
class B2Action_WaitNotCollision : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_WaitNotCollision> {
public:
    B2Action_WaitNotCollision(Fsm::Action & action);
    B2Action_WaitNotCollision(Fsm::Action & action, B2Action_WaitNotCollision const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setCategories(uint16_t categories) { m_categories = categories; }

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    uint16_t m_categories;
};

}}}

#endif
