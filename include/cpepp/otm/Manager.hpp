#ifndef CPEPP_OTM_MANAGER_H
#define CPEPP_OTM_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/otm/otm_manage.h"
#include "System.hpp"

namespace Cpe { namespace Otm {

class Manager : public Cpe::Utils::SimulateObject {
public:
    operator otm_manage_t (void) const { return (otm_manage_t)(this); }

    template<typename T>
    void registerTimer(
        otm_timer_id_t id,
        const char * name,
        T & r,
        void (T::*fun)(Memo & memo, tl_time_t cur_exec_time, void * obj_ctx),
        tl_time_span_t span)
    {
#ifdef _MSC_VER
        this->registerTimer(
            id, name, r, static_cast<TimerProcessFun>(fun), span
            , *((TimerProcessor*)((void*)&r)));
#else
        this->registerTimer(
            id, name,
            static_cast<TimerProcessor&>(r), static_cast<TimerProcessFun>(fun),
            span);
#endif
    }

    /*VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
      所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	void registerTimer(
        otm_timer_id_t id,
        const char * name,
        TimerProcessor& realResponser, TimerProcessFun fun
        tl_time_span_t span,
#ifdef _MSC_VER
        , TimerProcessor& useResponser
#endif
        );

	void unregisterTimer(otm_timer_id_t id);
	void unregisterTimer(TimerProcessor const & processor);
};

}}

#endif
