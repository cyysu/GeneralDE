#include "NPGUIControl.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_MoveInOut.hpp"
#include "EnvExt.hpp"
#include "cpe/pal/pal_strings.h"
#include "UICenterExt.hpp"

namespace UI { namespace App {

UIAction_MoveInOut::UIAction_MoveInOut(Sprite::Fsm::Action & action)
    : ActionBase(action)
    , m_move_way(MoveWay_In)
{
	bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_MoveInOut::UIAction_MoveInOut(Sprite::Fsm::Action & action, UIAction_MoveInOut const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_page_name(o.m_page_name)
	, m_cotrol_name(o.m_cotrol_name)
	, m_percent_decorator(o.m_percent_decorator)
	, m_speed(o.m_speed)
	, m_policy(o.m_policy)
	, m_move_way(o.m_move_way)
    , m_take_time(o.m_take_time)
{
}

void UIAction_MoveInOut::setMoveWay(const char * moveWay) {
    if (strcmp(moveWay, "in") == 0) {
        m_move_way = MoveWay_In;
    }
    else if (strcmp(moveWay, "out") == 0) {
        m_move_way = MoveWay_Out;
    }
    else {
		APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error, "entity %d(%s): %s: move way %s error",
			entity().id(), entity().name(), name(), moveWay);
    }
}

int UIAction_MoveInOut::enter(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return -1;
	}

    if (m_move_way != MoveWay_In && m_move_way != MoveWay_Out) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: move way %d unknown",
			entity().id(), entity().name(), name(), m_move_way);
		return -1;
    }

	m_runing_time = 0.0f;
	m_target_pos.x = control->GetRenderRealX();
	m_target_pos.y = control->GetRenderRealY();
	if(m_policy == "left"){
		m_origin_pos.x = -1 * control->GetRenderRealW();
		m_origin_pos.y = m_target_pos.y;
	}
	else if(m_policy == "right"){
		m_origin_pos.x = m_env.get().screenSize().x;
		m_origin_pos.y = m_target_pos.y;
	}
	else if(m_policy == "top"){
		m_origin_pos.x = m_target_pos.x;
		m_origin_pos.y = -1 * control->GetRenderRealH();
	}
	else if(m_policy == "buttom"){
		m_origin_pos.x = m_target_pos.x;
		m_origin_pos.y = m_env.get().screenSize().y;
	}
    else {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: move policy %s error",
			entity().id(), entity().name(), name(), m_policy.c_str());
		return -1;
    }

    if (m_speed > 0.0f){
	    m_duration = cpe_math_distance(m_origin_pos.x, m_origin_pos.y, m_target_pos.x, m_target_pos.y) / m_speed;
    }
    else if(m_take_time > 0.0f){
        m_duration = m_take_time;
    }
    else{
        APP_CTX_ERROR(
            app(), "entity %d(%s): %s: enter: speed=%f,take_tim=%f error",
            entity().id(), entity().name(), name(), m_speed, m_take_time);
        return -1;
    }


	if(m_move_way == MoveWay_Out) {
        ::std::swap(m_target_pos, m_origin_pos);
	}

    control->SetRenderRealX(m_origin_pos.x);
    control->SetRenderRealY(m_origin_pos.y);
	control->SetVisible(true);

	startUpdate();

    return 0;
}

void UIAction_MoveInOut::update(float delta) {
    Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: update: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

	m_runing_time += delta;

	float percent = m_runing_time >= m_duration ? 1.0f : m_runing_time / m_duration;

	percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);

	NPVector2 putPos(
        m_origin_pos.x + (m_target_pos.x - m_origin_pos.x) * percent,
        m_origin_pos.y + (m_target_pos.y - m_origin_pos.y) * percent);
	control->SetRenderRealPT(putPos);

	if (m_runing_time >= m_duration) {
		stopUpdate();
	}
}

void UIAction_MoveInOut::exit(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	NPGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: update: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

	if(m_move_way == MoveWay_In){
		control->SetRenderRealPT(NPVector2(m_target_pos.x, m_target_pos.y));
	}
	else if(m_move_way == MoveWay_Out) {
		control->SetRenderRealPT(NPVector2(m_origin_pos.x, m_origin_pos.y));
		control->SetVisible(false);
	}
	else{
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: exit: m_move_way %d eror",
			entity().id(), entity().name(), name(), m_move_way);
	}
}

void UIAction_MoveInOut::setDecotator(const char* def){
	if(ui_percent_decorator_setup(&m_percent_decorator,def, app().em()) != 0){
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error,
			"entity %d(%s): %s: set decorate %s fail",
			entity().id(), entity().name(), name(), def);
	}
}

void UIAction_MoveInOut::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_MoveInOut>(repo)
        .on_enter(&UIAction_MoveInOut::enter)
        .on_exit(&UIAction_MoveInOut::exit)
        .on_update(&UIAction_MoveInOut::update)
        ;
}

const char * UIAction_MoveInOut::NAME = "ui-move-in-out";

}}

