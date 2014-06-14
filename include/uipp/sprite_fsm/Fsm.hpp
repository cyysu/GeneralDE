#ifndef UIPP_SPRITE_FSM_INS_H
#define UIPP_SPRITE_FSM_INS_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/MemBuffer.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class Fsm : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_fsm_ins_t () const { return (ui_sprite_fsm_ins_t)this; }

    Entity & entity(void) { return *(Entity*)ui_sprite_fsm_to_entity(*this); }
    Entity const & entity(void) const { return *(Entity*)ui_sprite_fsm_to_entity(*this); }

    World & world(void) { return *(World*)ui_sprite_fsm_to_world(*this); }
    World const & world(void) const { return *(World*)ui_sprite_fsm_to_world(*this); }

    State * findState(uint16_t id) { return (State *)ui_sprite_fsm_state_find_by_id(*this, id); }
    State const * findState(uint16_t id) const { return (State const *)ui_sprite_fsm_state_find_by_id(*this, id); }

    State * findState(const char * name) { return (State *)ui_sprite_fsm_state_find_by_name(*this, name); }
    State const * findState(const char * name) const { return (State const *)ui_sprite_fsm_state_find_by_name(*this, name); }

    State * defaultState(void) { return (State *)ui_sprite_fsm_default_state(*this); }
    State const * defaultState(void) const { return (State const *)ui_sprite_fsm_default_state(*this); }
    void setDefaultState(const char * name);

    State * currentState(void) { return (State *)ui_sprite_fsm_current_state(*this); }
    State const * currentState(void) const { return (State const *)ui_sprite_fsm_current_state(*this); }

    bool isInState(const char * path) const { return ui_sprite_fsm_is_in_state(*this, path) ? true : false; }

    const char * path(Cpe::Utils::MemBuffer & buff) const;
};

class ComponentFsm : public Fsm {
public:
    static const char * NAME;
};

class ActionFsm : public Fsm {
public:
    static const char * NAME;
};

}}}

#endif
