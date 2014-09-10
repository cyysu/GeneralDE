#ifndef UIPP_APP_PAGE_GEN_H
#define UIPP_APP_PAGE_GEN_H
#include <cassert>
#include <sstream>
#include "cpepp/utils/TypeUtils.hpp"
#include "RRTTI.h"
#include "RGUIInclude.h"
#include "cpepp/utils/TypeUtils.hpp"
#include "../Page.hpp"
#include "../Env.hpp"
#include "PageEvtSch.hpp"

namespace UI { namespace App {

class TriggerStub {
public:
    TriggerStub(
        Page & owner,
        PageEvtSch::Trigger const & trigger)
        : m_owner(owner)
        , m_trigger(trigger)
    {
    }

protected:
    Page & m_owner;
    PageEvtSch::Trigger m_trigger;
};

template<typename OuterT, typename T>
struct PageHandlerTraits {
    static void (OuterT::*s_fun)(T const & evt);
};


template<class OuterT>
class PageGen : public Page, public PageEvtProcessor {
public:
    typedef PageEvtSch::Trigger Trigger;
    typedef PageGen Base;

    PageGen(Gd::App::Application & app, Cpe::Cfg::Node & cfg)
        : Page(app, cfg)
    {
    }

    class TriggerStubT : public TriggerStub {
    public:
        TriggerStubT(OuterT & outer, Trigger const & trigger)
            : TriggerStub(outer, trigger)
        {
        }

        //using TriggerStub::call;

        TriggerStubT & call(void (OuterT::*fun)()) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun));
            return *this;
        }

        TriggerStubT & call(void (OuterT::*fun)(Trigger const & trigger)) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun));
            return *this;
        }

        TriggerStubT & call(void (OuterT::*fun)(RGUIEventArgs & args)) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun));
            return *this;
        }

        TriggerStubT & call(void (OuterT::*fun)(RGUIListBoxAdvItem * item, uint32_t idx)) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun));
            return *this;
        }

        template<typename ControlT>
        TriggerStubT & call(void (OuterT::*fun)(ControlT *)) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun));
            return *this;
        }

        template<typename ArgT, typename ArgT2>
        TriggerStubT & call(void (OuterT::*fun)(ArgT), ArgT2 arg) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun, arg));
            return *this;
        }

        template<typename ArgT>
        TriggerStubT & call(void (OuterT::*fun)(ArgT const &), ArgT const & arg) {
            m_owner.schedule().addTrigger(m_trigger, m_owner.schedule()._make_fun(outer(), fun, arg));
            return *this;
        }

        TriggerStubT & delay(tl_time_span_t delay_ms) {
            m_trigger.delay_ms = delay_ms;
            return *this;
        }

    private:
        OuterT & outer(void) { return Cpe::Utils::calc_cast<OuterT>(m_owner); }
        OuterT const & outer(void) const { return Cpe::Utils::calc_cast<OuterT>(m_owner); }
    };

    template<typename ControlT>
    class P {
    public:
        P(PageGen & parent, const char * const name)
            : m_parent(parent)
            , m_name(name)
            , m_control(NULL)
        {
        }
        
        ControlT * operator->() {
            if (m_control == NULL) {
                m_control = (ControlT*)ControlT::DynamicCast(&ControlT::sRTTI, findChild(&m_parent, m_name, m_parent.uiCenter()));
                assert(m_control);
            }
            
            return m_control;
        }
        
        TriggerStubT bind_show() { return m_parent.bind_show(operator->()); }
        TriggerStubT bind_click(void) { return m_parent.bind_click(operator->()); }
        TriggerStubT bind_list_item_show(void) { return m_parent.bind_list_item_show(operator->()); }
        TriggerStubT bind_list_item_click(const char * name) { return m_parent.bind_list_item_click(operator->(), name); }

        operator RGUIControl * () {
            return m_control;
        }

    private:
        PageGen & m_parent;
        const char * const m_name;
        ControlT * m_control;
    };

    template<typename T>
    void addEventHandler(void (OuterT::*fun)(T const & evt), EventHandlerScope scope = EventHandlerScopeVisiable) {
        PageHandlerTraits<OuterT, T>::s_fun = fun;

        Page::addEventHandler(Cpe::Dr::MetaTraits<T>::NAME, scope, process_event<T>, Cpe::Utils::calc_cast<OuterT>(this));
    }

protected:
    TriggerStubT on_hidden(RGUIControl * control) {
        assert(control);
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_HIDEN, control, NULL, 0, Trigger::ONCE };
        return TriggerStubT(outer(), trigger);
    }

    TriggerStubT on_show(RGUIControl * control) {
        assert(control);
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_SHOW, control, NULL, 0, Trigger::ONCE };
        return TriggerStubT(outer(), trigger);
    }

    TriggerStubT bind_show(RGUIControl * control) {
        assert(control);
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_SHOW, control, NULL, 0, Trigger::BINDING };
        return TriggerStubT(outer(), trigger);
    }

    TriggerStubT bind_click(const char * control_name) {
        return bind_click(findChild(this, control_name));
    }

    TriggerStubT bind_click(RGUIControl * control) {
        assert(control);
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_CLICK, control, NULL, 0, Trigger::BINDING };
        return TriggerStubT(outer(), trigger);
    }

    TriggerStubT bind_list_item_show(const char * control_name) {
        return bind_list_item_show(findChild(this, control_name));
    }

    TriggerStubT bind_list_item_show(RGUIControl * control) {
        assert(control);
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_ITEM_SHOW, control, NULL, 0, Trigger::BINDING };
        return TriggerStubT(outer(), trigger);
    }

    TriggerStubT bind_list_item_click(const char * control_name, const char * name) {
        return bind_list_item_click(findChild(this, control_name), name);
    }

    TriggerStubT bind_list_item_click(RGUIControl * control, const char * name) {
        assert(control);
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_ITEM_CLICK, control, name, 0, Trigger::BINDING };
        return TriggerStubT(outer(), trigger);
    }

    TriggerStubT delay(tl_time_span_t delay_ms) {
        Trigger trigger = { Trigger::TRIGGERTYPE_CONTROL_NOOP, NULL, NULL, delay_ms, Trigger::ONCE };
        return TriggerStubT(outer(), trigger);
    }
protected:


private:
    template<typename T>
    static void process_event(void * ctx, ui_sprite_event_t evt) {
        try {
            (((OuterT *)ctx)->*PageHandlerTraits<OuterT, T>::s_fun)(*(T const *)evt->data);
        }
        catch(...) {
        }
    }
            

    OuterT & outer(void) { return Cpe::Utils::calc_cast<OuterT>(*this); }
    OuterT const & outer(void) const { return Cpe::Utils::calc_cast<OuterT>(*this); }
};

template<typename OuterT, typename T>
void (OuterT::* PageHandlerTraits<OuterT, T>::s_fun)(T const & evt);

}}

#endif
