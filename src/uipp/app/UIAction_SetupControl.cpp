#include "NPGUIControl.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_SetupControl.hpp"
#include "EnvExt.hpp"
#include "cpe/pal/pal_strings.h"
#include "UICenterExt.hpp"

namespace UI { namespace App {

UIAction_SetupControl::UIAction_SetupControl(Sprite::Fsm::Action & action)
    : ActionBase(action)
    , m_cfg_restore(0)
    , m_cfg_visiable(1)
{
}

UIAction_SetupControl::UIAction_SetupControl(Sprite::Fsm::Action & action, UIAction_SetupControl const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_page_name(o.m_page_name)
	, m_cotrol_name(o.m_cotrol_name)
    , m_cfg_restore(o.m_cfg_restore)
    , m_cfg_visiable(o.m_cfg_visiable)
{
}

int UIAction_SetupControl::enter(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return -1;
	}

    m_saved_visiable = control->WasVisible() ? 1 : 0;
    control->SetVisible(m_cfg_visiable ? true : false);

    return 0;
}

void UIAction_SetupControl::exit(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: exit: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

    if (m_cfg_restore) {
        control->SetVisible(m_saved_visiable ? true : false);
    }
}

void UIAction_SetupControl::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_SetupControl>(repo)
        .on_enter(&UIAction_SetupControl::enter)
        .on_exit(&UIAction_SetupControl::exit)
        ;
}

const char * UIAction_SetupControl::NAME = "ui-setup-control";

}}

