#include <list>
#include "cpe/utils/math_ex.h"
#include "gdpp/app/Log.hpp"
#include "Box2D/Common/b2Math.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite/ComponentGen.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "B2Action_WaitStop.hpp"
#include "B2ObjectExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

static const float WAITING_TIME = 0.5f;
static const float WAITING_DISTANCE = 0.01f;
struct wait_data{
	float waiting_time;
	float waiting_distance;
};

class B2ActionWaitStopImpl : public Fsm::ActionGen<B2Action_WaitStop, B2ActionWaitStopImpl> {

public:	
    B2ActionWaitStopImpl(Fsm::Action & action)
        : ActionBase(action)
		, m_wait_time(0.0f)
		, m_wait_distance(0.0f)
		, m_wasMaxTime(false)
    {
		m_last_pos.x = 0.0f;
		m_last_pos.y = 0.0f;
    }

    B2ActionWaitStopImpl(Fsm::Action & action, B2ActionWaitStopImpl const & o)
        : ActionBase(action)
		, m_wait_time(0.0f)
		, m_wait_distance(0.0f)
		, m_wasMaxTime(false)
    {
		m_last_pos.x = 0.0f;
		m_last_pos.y = 0.0f;
    }

    int enter(void) {
		onStartWaitStop();
        return 0;
    }

    void exit(void) {
		m_waitList.clear();
    }

    void update(float delta) {
		onUpdateWaiting(delta);
	}

private:
	void onStartWaitStop(void){
		P2D::Transform * transform = entity().findComponent<P2D::Transform>();
		m_last_pos = transform->originPos();
		m_wait_time = 0.0f;
		m_wait_distance = 0.0f;
		m_wasMaxTime = false;
		m_waitList.clear();
        syncUpdate(true);
	}

	void onUpdateWaiting(float delta) {
		B2WorldExt & b2World = world().res<B2WorldExt>();
		P2D::Transform * transform = entity().findComponent<P2D::Transform>();
		wait_data data;
		data.waiting_time = delta;

        P2D::Pair curent_pos = transform->originPos();
		data.waiting_distance = cpe_math_distance(curent_pos.x, curent_pos.y,  m_last_pos.x,  m_last_pos.y) / b2World.ptmRatio();
		m_waitList.push_back(data);

		m_wait_time += data.waiting_time;
		m_wait_distance += data.waiting_distance;
		m_last_pos = transform->originPos();
		//APP_CTX_ERROR(app(), "onUpdate_Waiting: size =%d, m_wait_time =%f, m_wait_distance %f", m_waitList.size(), m_wait_time, m_wait_distance);
		while(m_wait_time > WAITING_TIME)
		{
			std::list<wait_data>::iterator iter = m_waitList.begin();
			if(iter == m_waitList.end()){
				APP_CTX_ERROR(app(), "onUpdate_Waiting: no wait list");
				break;
			}

			m_wait_time -= iter->waiting_time;
			m_wait_distance -= iter->waiting_distance;
			m_waitList.pop_front();	
			m_wasMaxTime = true;
		}

		if( m_wasMaxTime && m_wait_distance < WAITING_DISTANCE)
		{
			syncUpdate(false);
		}
	}

private:
	float				 m_wait_time;
	float				 m_wait_distance;
	P2D::Pair			 m_last_pos;
	std::list<wait_data> m_waitList;
	bool				 m_wasMaxTime;
};

void B2Action_WaitStop::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2ActionWaitStopImpl>(repo)
        .on_enter(&B2ActionWaitStopImpl::enter)
        .on_exit(&B2ActionWaitStopImpl::exit)
        .on_update(&B2ActionWaitStopImpl::update)
        ;
}

const char * B2Action_WaitStop::NAME = "b2-wait-stop";

}}}

