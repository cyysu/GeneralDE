#ifndef UIPP_APP_ACTION_MOVE_IN_OUT_H
#define UIPP_APP_ACTION_MOVE_IN_OUT_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/utils/ui_percent_decorator.h"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_MoveInOut : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_MoveInOut> {
public:
    enum MoveWay {
        MoveWay_In = 1,
        MoveWay_Out = 2,
    };

    UIAction_MoveInOut(Sprite::Fsm::Action & action);
    UIAction_MoveInOut(Sprite::Fsm::Action & action, UIAction_MoveInOut const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setEnv(EnvExt & env) { m_env = env; }

	void setPageName(const char * page_name) { m_page_name = page_name; }
	const char * pageName(void) const { return m_page_name.c_str(); }

	void setControlName(const char * cotrol_name) { m_cotrol_name = cotrol_name; }
	const char * cotrolName(void) const { return m_cotrol_name.c_str(); }

	void setSpeed(const float speed) { m_speed = speed; }
    void setTakeTime(const float take_time) { m_take_time = take_time; }

	void setPolicy(const char * policy) { m_policy = policy; }
	void setMoveWay(const char * moveWay);

	void setDecotator(const char* def);
    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
	Cpe::Utils::ObjRef<EnvExt> m_env;
	::std::string m_page_name;
	::std::string m_cotrol_name;
	ui_percent_decorator m_percent_decorator;
	::std::string	  m_policy;
	MoveWay m_move_way;

	Sprite::P2D::Pair m_origin_pos;
	Sprite::P2D::Pair m_target_pos;
	float m_runing_time;
	float m_duration;
	float m_speed;
    float m_take_time;
};

}}

#endif
