#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_SwitchPhase.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

UIAction_SwitchPhase::UIAction_SwitchPhase(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_SwitchPhase::UIAction_SwitchPhase(Sprite::Fsm::Action & action, UIAction_SwitchPhase const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_phase_name(o.m_phase_name)
{
}

int UIAction_SwitchPhase::enter(void) {
    m_env.get().uiCenter().phaseSwitch(m_phase_name.c_str());
    return 0;
}

void UIAction_SwitchPhase::exit(void) {
}

void UIAction_SwitchPhase::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_SwitchPhase>(repo)
        .on_enter(&UIAction_SwitchPhase::enter)
        .on_exit(&UIAction_SwitchPhase::exit)
        ;
}

const char * UIAction_SwitchPhase::NAME = "ui-switch-phase";

}}

