#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/Fsm.hpp"

namespace UI { namespace Sprite { namespace Fsm {

Action & State::createAction(const char * name, const char * type_name) {
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_create(*this, name, type_name);

    if (action == NULL) {
        APP_CTX_THROW_EXCEPTION(
            fsm().entity().world().app(), ::std::runtime_error,
            "action %s(%s) create fail!", name, type_name);
    }
    
    return *(Action*)action;
}

}}}
