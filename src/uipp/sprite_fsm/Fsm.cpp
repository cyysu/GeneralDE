#include "cpepp/utils/MemBuffer.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "uipp/sprite_fsm/Fsm.hpp"

namespace UI { namespace Sprite { namespace Fsm {

const char * Fsm::path(Cpe::Utils::MemBuffer & buff) const {
    return ui_sprite_fsm_dump_path(*this, buff);
}

const char * ComponentFsm::NAME = UI_SPRITE_FSM_COMPONENT_FSM_NAME;

const char * ActionFsm::NAME = UI_SPRITE_FSM_ACTION_FSM_NAME;

}}}
