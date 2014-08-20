#include "NPGUIControl.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_ScaleInOut.hpp"
#include "EnvExt.hpp"
#include "cpe/pal/pal_strings.h"
#include "UICenterExt.hpp"

namespace UI { namespace App {

UIAction_ScaleInOut::UIAction_ScaleInOut(Sprite::Fsm::Action & action)
    : ActionBase(action)
	, m_zoom_way(ZoomWay_In)
{
	bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_ScaleInOut::UIAction_ScaleInOut(Sprite::Fsm::Action & action, UIAction_ScaleInOut const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_page_name(o.m_page_name)
	, m_cotrol_name(o.m_cotrol_name)
	, m_percent_decorator(o.m_percent_decorator)
	, m_zoom_way(o.m_zoom_way)
    , m_take_time(o.m_take_time)
{
}

void UIAction_ScaleInOut::setZoomWay(const char * zoom_way) {
	if (strcmp(zoom_way, "in") == 0) {
		m_zoom_way = ZoomWay_In;
	}
	else if (strcmp(zoom_way, "out") == 0) {
		m_zoom_way = ZoomWay_Out;
	}
	else {
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error, "entity %d(%s): %s: scale way %s error",
			entity().id(), entity().name(), name(), zoom_way);
	}
}

int UIAction_ScaleInOut::enter(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return -1;
	}

	m_runing_time = 0.0f;

	if (m_take_time < 0.0f) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: duration %f error",
			entity().id(), entity().name(), name(), m_take_time);
		return -1;
	}

	m_pivot.x = control->GetPivot().x;
	m_pivot.y = control->GetPivot().y;
	control->SetPivot(NPVector2(0.5f, 0.5f));

	if(m_zoom_way == ZoomWay_In){
		control->SetScale(NPVector2(0.0f, 0.0f));
	}
    control->SetVisible(true);

	startUpdate();

    return 0;
}

void UIAction_ScaleInOut::update(float delta) {
    Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: update: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

	m_runing_time += delta;

	float percent = m_runing_time >= m_take_time ? 1.0f : m_runing_time / m_take_time;

	if(m_zoom_way == ZoomWay_In){
		percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);
	}
	else if(m_zoom_way == ZoomWay_Out){
		percent = 1.0f - ui_percent_decorator_decorate(&m_percent_decorator, percent);
	}
	else{
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: update: m_move_way %d eror",
			entity().id(), entity().name(), name(), m_zoom_way);
		return;
	}

	control->SetScale(NPVector2(percent, percent));

	if (m_runing_time >= m_take_time) {
		stopUpdate();
	}
}

void UIAction_ScaleInOut::exit(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: update: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

	control->SetPivot(NPVector2(m_pivot.x, m_pivot.y));
	control->SetScale(NPVector2(1.0f, 1.0f));
	if(m_zoom_way == ZoomWay_Out){
		control->SetVisible(false);
	}
}

void UIAction_ScaleInOut::setDecotator(const char* def){
	if(ui_percent_decorator_setup(&m_percent_decorator,def, app().em()) != 0){
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error,
			"entity %d(%s): %s: set decorate %s fail",
			entity().id(), entity().name(), name(), def);
	}
}

void UIAction_ScaleInOut::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_ScaleInOut>(repo)
        .on_enter(&UIAction_ScaleInOut::enter)
        .on_exit(&UIAction_ScaleInOut::exit)
        .on_update(&UIAction_ScaleInOut::update)
        ;
}

const char * UIAction_ScaleInOut::NAME = "ui-scale-in-out";

}}

