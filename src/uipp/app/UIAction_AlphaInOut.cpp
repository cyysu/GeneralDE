#include "RGUIControl.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_AlphaInOut.hpp"
#include "EnvExt.hpp"
#include "cpe/pal/pal_strings.h"
#include "UICenterExt.hpp"

namespace UI { namespace App {

UIAction_AlphaInOut::UIAction_AlphaInOut(Sprite::Fsm::Action & action)
    : ActionBase(action)
    , m_alpha_way(AlphaWay_In)
    , m_cfg_origin_alpha(0.0f)
    , m_take_time(0.0f)
{
	bzero(&m_percent_decorator, sizeof(m_percent_decorator));
}

UIAction_AlphaInOut::UIAction_AlphaInOut(Sprite::Fsm::Action & action, UIAction_AlphaInOut const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_page_name(o.m_page_name)
	, m_cotrol_name(o.m_cotrol_name)
	, m_percent_decorator(o.m_percent_decorator)
	, m_alpha_way(o.m_alpha_way)
    , m_cfg_origin_alpha(o.m_cfg_origin_alpha)
    , m_take_time(o.m_take_time)
{
}

void UIAction_AlphaInOut::setAlphaWay(const char * alphaWay) {
    if (strcmp(alphaWay, "in") == 0) {
        m_alpha_way = AlphaWay_In;
    }
    else if (strcmp(alphaWay, "out") == 0) {
        m_alpha_way = AlphaWay_Out;
    }
    else {
		APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error, "entity %d(%s): %s: alpha way %s error",
			entity().id(), entity().name(), name(), alphaWay);
    }
}

int UIAction_AlphaInOut::enter(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	RGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return -1;
	}

    if (m_alpha_way != AlphaWay_In && m_alpha_way != AlphaWay_Out) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: enter: alpha way %d unknown",
			entity().id(), entity().name(), name(), m_alpha_way);
		return -1;
    }

	m_runing_time = 0.0f;
	m_origin_alpha = m_cfg_origin_alpha;
	m_target_alpha = control->GetAlpha();

	if(m_alpha_way == AlphaWay_Out){
        ::std::swap(m_origin_alpha, m_target_alpha);
	}

    control->SetAlpha(m_origin_alpha);
	control->SetVisible(true);

	startUpdate();

    return 0;
}

void UIAction_AlphaInOut::update(float delta) {
    Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	RGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: alpha: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

	m_runing_time += delta;

	float percent = m_runing_time >= m_take_time ? 1.0f : m_runing_time / m_take_time;
    percent = ui_percent_decorator_decorate(&m_percent_decorator, percent);

	float alpha = m_origin_alpha + (m_target_alpha - m_origin_alpha) * percent;
	control->SetAlpha(alpha);

	if (m_runing_time >= m_take_time) {
		stopUpdate();
	}
}

void UIAction_AlphaInOut::exit(void) {
	Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
	RGUIControl * control = page.findChild(m_cotrol_name.c_str());
	if (control == NULL) {
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: update: control %s not exist",
			entity().id(), entity().name(), name(), m_cotrol_name.c_str());
		return;
	}

	if(m_alpha_way == AlphaWay_In){
		control->SetAlpha(m_target_alpha);
	}
	else if(m_alpha_way == AlphaWay_Out) {
		control->SetAlpha(m_origin_alpha);
		control->SetVisible(false);
	}
	else{
		APP_CTX_ERROR(
			app(), "entity %d(%s): %s: exit: m_alpha_way %d eror",
			entity().id(), entity().name(), name(), m_alpha_way);
	}
}

void UIAction_AlphaInOut::setDecotator(const char* def){
	if(ui_percent_decorator_setup(&m_percent_decorator,def, app().em()) != 0){
		APP_CTX_THROW_EXCEPTION(
			app(), ::std::runtime_error,
			"entity %d(%s): %s: set decorate %s fail",
			entity().id(), entity().name(), name(), def);
	}
}

void UIAction_AlphaInOut::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_AlphaInOut>(repo)
        .on_enter(&UIAction_AlphaInOut::enter)
        .on_exit(&UIAction_AlphaInOut::exit)
        .on_update(&UIAction_AlphaInOut::update)
        ;
}

const char * UIAction_AlphaInOut::NAME = "ui-alpha-in-out";

}}

