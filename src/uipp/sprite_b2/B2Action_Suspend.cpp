#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite/ComponentGen.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "B2Action_Suspend.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_Suspend::B2Action_Suspend(Fsm::Action & action)
    : ActionBase(action)
    , m_resume(true)
    , m_change_to_type(OBJECTTYPE_DYNAMIC)
    , m_change_to_runing_mode(RUNINGMODE_PASSIVE)
{
}

B2Action_Suspend::B2Action_Suspend(Fsm::Action & action, B2Action_Suspend const & o)
    : ActionBase(action)
    , m_resume(o.m_resume)
    , m_change_to_type(o.m_change_to_type)
    , m_change_to_runing_mode(o.m_change_to_runing_mode)
{
}

int B2Action_Suspend::enter(void) {
    B2ObjectExt & b2Object = entity().component<B2ObjectExt>();		

    b2Object.suspend(m_change_to_type, m_change_to_runing_mode, m_resume);

    return 0;
}

void B2Action_Suspend::exit(void) {
    B2ObjectExt & b2Object = entity().component<B2ObjectExt>();
    b2Object.resume();
}

void B2Action_Suspend::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_Suspend>(repo)
        .on_enter(&B2Action_Suspend::enter)
        .on_exit(&B2Action_Suspend::exit)
        ;
}

const char * B2Action_Suspend::NAME = "b2-suspend";

}}}

