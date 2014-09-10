#ifndef UIPP_APP_ACTION_ALPHA_IN_OUT_H
#define UIPP_APP_ACTION_ALPHA_IN_OUT_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/utils/ui_percent_decorator.h"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_AlphaInOut : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_AlphaInOut> {
public:
    enum AlphaWay {
        AlphaWay_In = 1,
        AlphaWay_Out = 2,
    };

    UIAction_AlphaInOut(Sprite::Fsm::Action & action);
    UIAction_AlphaInOut(Sprite::Fsm::Action & action, UIAction_AlphaInOut const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setEnv(EnvExt & env) { m_env = env; }

	void setPageName(const char * page_name) { m_page_name = page_name; }
	const char * pageName(void) const { return m_page_name.c_str(); }

	void setControlName(const char * cotrol_name) { m_cotrol_name = cotrol_name; }
	const char * cotrolName(void) const { return m_cotrol_name.c_str(); }

    void setOriginAlpha(float alpha) { m_cfg_origin_alpha = alpha; }

	void setTakeTime(const float take_time) { m_take_time = take_time; }
	void setAlphaWay(const char * alphaWay);

	void setDecotator(const char* def);
    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
	Cpe::Utils::ObjRef<EnvExt> m_env;
	::std::string m_page_name;
	::std::string m_cotrol_name;
	ui_percent_decorator m_percent_decorator;
	AlphaWay m_alpha_way;
    float m_cfg_origin_alpha;
	float m_take_time;
    
	float m_target_alpha;
	float m_origin_alpha;
	float m_runing_time;
};

}}

#endif
