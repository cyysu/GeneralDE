#ifndef UIPP_APP_PAGE_CTRLSCHEDULE_H
#define UIPP_APP_PAGE_CTRLSCHEDULE_H
#include <vector>
#include "NPType.h"
#include "NPGUIEventArg.h"
#include "gdpp/timer/TimerCenter.hpp"
#include "../System.hpp"

namespace UI { namespace App {

class PageEvtProcessor {
};

class PageEvtSch : public Gd::Timer::TimerProcessor {
    struct TriggerNode;
public:

    struct Trigger {
        enum TriggerType {
            TRIGGERTYPE_CONTROL_NOOP
            , TRIGGERTYPE_CONTROL_HIDEN
            , TRIGGERTYPE_CONTROL_SHOW
            , TRIGGERTYPE_CONTROL_CLICK
            , TRIGGERTYPE_CONTROL_ITEM_SHOW
            , TRIGGERTYPE_CONTROL_ITEM_CLICK
        } type;

        enum TriggerCategory {
            ONCE,
            BINDING
        };

        NPGUIControl const * control;
        const char * control_name;
        tl_time_span_t delay_ms;
        TriggerCategory category;
    };

    typedef void (PageEvtProcessor::*ProcessFun1)(void);
    typedef void (PageEvtProcessor::*ProcessFun2)(Trigger const & trigger);
    typedef void (PageEvtProcessor::*ProcessFun3)(NPGUIEventArgs & args);

    struct ProcessFun {
        PageEvtProcessor * processor;
        char data[32];
        void (*call)(TriggerNode const & node, NPGUIEventArgs * args);
    };

    PageEvtSch();
    ~PageEvtSch();

    void addTrigger(Trigger const & trigger, ProcessFun const & fun);

    bool haveTrigger(NPGUIControl const * control) const;

    void triggerAnimStop(NPGUIEventArgs & args);
    void triggerByType(Trigger::TriggerType type, NPGUIEventArgs& args);
    void triggerMouseClick(NPGUIEventArgs & args);

    void doTriggers();
    void clearTriggers();

    template<typename ProcessorT>
    static ProcessFun _make_fun(ProcessorT & processor, void (ProcessorT::*f)(void)) {
        ProcessFun fun;

        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(void);
            ProcessorT * use_processor;
        };

        char __check_size[sizeof(ProcessFunUserDef) > sizeof(fun.data) ? -1 : 1];
        (void)__check_size;

        fun.processor = &processor;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&fun.data;

        fun_data->fun = f;
        fun_data->use_processor = &processor;

        fun.call = &call_no_arg<ProcessorT>;

        return fun;
    }

    template<typename ProcessorT, typename ControlT>
    static ProcessFun _make_fun(ProcessorT & processor, void (ProcessorT::*f)(ControlT * control)) {
        ProcessFun fun;

        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(ControlT & control);
            ProcessorT * use_processor;
        };

        char __check_size[sizeof(ProcessFunUserDef) > sizeof(fun.data) ? -1 : 1];
        (void)__check_size;

        fun.processor = &processor;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&fun.data;

        fun_data->fun = f;
        fun_data->use_processor = &processor;

        fun.call = &call_with_control<ProcessorT, ControlT>;

        return fun;
    }

    template<typename ProcessorT>
    static ProcessFun _make_fun(ProcessorT & processor, void (ProcessorT::*f)(NPGUIListBoxAdvItem * item, uint32_t idx)) {
        ProcessFun fun;

        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(NPGUIListBoxAdvItem * item, uint32_t idx);
            ProcessorT * use_processor;
        };

        char __check_size[sizeof(ProcessFunUserDef) > sizeof(fun.data) ? -1 : 1];
        (void)__check_size;

        fun.processor = &processor;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&fun.data;

        fun_data->fun = f;
        fun_data->use_processor = &processor;

        fun.call = &call_with_list_item<ProcessorT>;

        return fun;
    }

    template<typename ProcessorT>
    static ProcessFun _make_fun(ProcessorT & processor, void (ProcessorT::*f)(NPGUIEventArgs * args)) {
        ProcessFun fun;

        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(NPGUIEventArgs * args);
            ProcessorT * use_processor;
        };

        char __check_size[sizeof(ProcessFunUserDef) > sizeof(fun.data) ? -1 : 1];
        (void)__check_size;

        fun.processor = &processor;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&fun.data;

        fun_data->fun = f;
        fun_data->use_processor = &processor;

        fun.call = &call_with_evt_arg<ProcessorT>;

        return fun;
    }

    template<typename ProcessorT, typename ArgT, typename ArgT2>
    static ProcessFun _make_fun(ProcessorT & processor, void (ProcessorT::*f)(ArgT args), ArgT2 args) {
        ProcessFun fun;

        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(ArgT args);
            ProcessorT * use_processor;
            ArgT args;
        };

        char __check_size[sizeof(ProcessFunUserDef) > sizeof(fun.data) ? -1 : 1];
        (void)__check_size;

        fun.processor = &processor;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&fun.data;

        fun_data->fun = f;
        fun_data->use_processor = &processor;
        fun_data->args = args;

        fun.call = &call_with_arg<ProcessorT, ArgT>;

        return fun;
    }

    template<typename ProcessorT, typename ArgT>
    static ProcessFun _make_fun(ProcessorT & processor, void (ProcessorT::*f)(ArgT const & args), ArgT const & args) {
        ProcessFun fun;

        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(ArgT const & args);
            ProcessorT * use_processor;
            ArgT args;
        };;

        char __check_size[sizeof(ProcessFunUserDef) > sizeof(fun.data) ? -1 : 1];
        (void)__check_size;

        fun.processor = &processor;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&fun.data;
        fun_data->fun = f;
        fun_data->use_processor = &processor;
        fun_data->args = args;

        fun.call = &call_with_arg_ref<ProcessorT, ArgT>;

        return fun;
    }

private:
    template<typename ProcessorT>
    static PageEvtProcessor * _make_use_processor(ProcessorT & processor) {
#ifdef _MSC_VER
        return (PageEvtProcessor*)((void*)&processor);
#else
        return &processor;
#endif
    }

    template<typename ProcessorT>
    static void call_no_arg(TriggerNode const & node, NPGUIEventArgs * args) {
        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(void);
            ProcessorT * use_processor;
        };

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&node.fun.data;
        (fun_data->use_processor->*fun_data->fun)();
    }

    template<typename ProcessorT>
    static void call_with_evt_arg(TriggerNode const & node, NPGUIEventArgs * args) {
        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(NPGUIEventArgs * args);
            ProcessorT * use_processor;
        };

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&node.fun.data;
        (fun_data->use_processor->*fun_data->fun)(*args);
    }

    template<typename ProcessorT, typename ControlT>
    static void call_with_control(TriggerNode const & node, NPGUIEventArgs * args) {
        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(ControlT * control);
            ProcessorT * use_processor;
        };

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&node.fun.data;
        (fun_data->use_processor->*fun_data->fun)(
            (ControlT*)ControlT::DynamicCast(&ControlT::sRTTI, args->sender));
    }

    template<typename ProcessorT>
    static void call_with_list_item(TriggerNode const & node, NPGUIEventArgs * args) {
        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(NPGUIListBoxAdvItem * item, uint32_t idx);
            ProcessorT * use_processor;
        };

        NPGUIListBoxAdvItem * item = _cvt_to_list_item((NPGUIControl *)args->lParam);
        if (item == NULL) return;

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&node.fun.data;
        (fun_data->use_processor->*fun_data->fun)(item, (uint32_t)(ptr_uint_t)args->wParam);
    }

    template<typename ProcessorT, typename ArgT>
    static void call_with_arg(TriggerNode const & node, NPGUIEventArgs * args) {
        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(ArgT args);
            ProcessorT * use_processor;
            ArgT args;
        };

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&node.fun.data;
        (fun_data->use_processor->*fun_data->fun)(fun_data->args);
    }
    
    template<typename ProcessorT, typename ArgT>
    static void call_with_arg_ref(TriggerNode const & node, NPGUIEventArgs * args) {
        struct ProcessFunUserDef {
            void (ProcessorT::*fun)(ArgT args);
            ProcessorT * use_processor;
            ArgT args;
        };

        ProcessFunUserDef * fun_data = (ProcessFunUserDef *)(void*)&node.fun.data;
        (fun_data->use_processor->*fun_data->fun)(fun_data->args);
    }
    
private:
    struct TriggerNode {
        uint32_t id;
        Trigger trigger;
        ProcessFun fun;
        Gd::Timer::TimerID timer;
    };

    typedef ::std::vector<TriggerNode> TriggerContainer;

    bool check_execute(TriggerNode & node, TriggerContainer & waitExecutes, NPGUIEventArgs * args);

    void on_timer(Gd::Timer::TimerID timerId);
    uint32_t m_max_id;
    TriggerContainer m_triggers;

    static NPGUIListBoxAdvItem * _cvt_to_list_item(NPGUIControl * control);
};

}}

#endif
