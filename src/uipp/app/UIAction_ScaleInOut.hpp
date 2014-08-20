#ifndef UIPP_APP_ACTION_SCALE_IN_OUT_H
#define UIPP_APP_ACTION_SCALE_IN_OUT_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/utils/ui_percent_decorator.h"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_ScaleInOut : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_ScaleInOut> {
public:
	enum ZoomWay {
		ZoomWay_In = 1,
		ZoomWay_Out = 2,
	};

    UIAction_ScaleInOut(Sprite::Fsm::Action & action);
    UIAction_ScaleInOut(Sprite::Fsm::Action & action, UIAction_ScaleInOut const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setEnv(EnvExt & env) { m_env = env; }

	void setPageName(const char * page_name) { m_page_name = page_name; }
	const char * pageName(void) const { return m_page_name.c_str(); }

	void setControlName(const char * cotrol_name) { m_cotrol_name = cotrol_name; }
	const char * cotrolName(void) const { return m_cotrol_name.c_str(); }

	void setTakeTime(const float take_time) { m_take_time = take_time; }

	void setZoomWay(const char * zoom_way);

	void setDecotator(const char* def);
    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
	Cpe::Utils::ObjRef<EnvExt> m_env;
	::std::string m_page_name;
	::std::string m_cotrol_name;
	ui_percent_decorator m_percent_decorator;

	Sprite::P2D::Pair m_pivot;
	ZoomWay	  m_zoom_way;
	float m_runing_time;
	//float m_duration;
    float m_take_time;
};

}}

#endif
