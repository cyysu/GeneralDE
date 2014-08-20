#include "NPGUIEventArg.h"
#include "NPGUIListBoxAdvItem.h"
#include "NPGUIListBoxRow.h"
#include "NPGUIListBoxCol.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/app/Gen/PageEvtSch.hpp"

namespace UI { namespace App {

PageEvtSch::PageEvtSch() : m_max_id(0) {
}

PageEvtSch::~PageEvtSch() {
    Gd::Timer::TimerCenter::instance(Gd::App::Application::instance())
        .unregisterTimer(*this);
}

void PageEvtSch::addTrigger(Trigger const & trigger, ProcessFun const & fun) {
    TriggerNode node = { 0, trigger, fun, 0 };

    if (trigger.type == Trigger::TRIGGERTYPE_CONTROL_NOOP) {
        if (trigger.delay_ms) {
            node.timer = 
                Gd::Timer::TimerCenter::instance(Gd::App::Application::instance())
                .registerTimer(
                    *this, &PageEvtSch::on_timer,
                    trigger.delay_ms, trigger.delay_ms, 1);
            node.id = ++m_max_id;
            //APP_INFO("triggers push_back %d = ", node.id);
            m_triggers.push_back(node);
        }
        else {
            fun.call(node, NULL);
        }
    }
    else {
        assert(trigger.control != NULL);
        node.id = ++m_max_id;
        m_triggers.push_back(node);
    }
}

void PageEvtSch::triggerAnimStop(NPGUIEventArgs & args) {
    NPGUIAnimationEventArgs& animEventArgs = static_cast<NPGUIAnimationEventArgs&>(args);

    TriggerContainer need_executes;

    for(TriggerContainer::iterator it =  m_triggers.begin();
        it != m_triggers.end();
        )
    {
        if (it->trigger.control != args.sender) {
            ++it;
            continue;
        }

        if (
            (it->trigger.type == Trigger::TRIGGERTYPE_CONTROL_HIDEN && animEventArgs.fire == NPGUIEVENT_HIDE)
            || (it->trigger.type == Trigger::TRIGGERTYPE_CONTROL_SHOW && animEventArgs.fire == NPGUIEVENT_SHOW)
            )
        {
            if (check_execute(*it, need_executes, &args)) {
                it = m_triggers.erase(it);
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
    
    for(TriggerContainer::iterator it = need_executes.begin(); it != need_executes.end(); ++it) {
        it->fun.call(*it, &args);
        //APP_INFO("triggers erase %d == ", it->id);
    }
}

void PageEvtSch::triggerByType(Trigger::TriggerType type, NPGUIEventArgs & args) {
    TriggerContainer need_executes;

    for(TriggerContainer::iterator it = m_triggers.begin(); it != m_triggers.end();) {
        if (it->trigger.control != args.sender) {
            ++it;
            continue;
        }

        if (it->trigger.type == type) {
            if (check_execute(*it, need_executes, &args)) {
                it = m_triggers.erase(it);
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
    
    for(TriggerContainer::iterator it = need_executes.begin(); it != need_executes.end(); ++it) {
        it->fun.call(*it, &args);
        //APP_INFO("triggers erase %d == ", it->id);
    }
}

void PageEvtSch::triggerMouseClick(NPGUIEventArgs & args) {
    TriggerContainer need_executes;

    for(TriggerContainer::iterator it = m_triggers.begin(); it != m_triggers.end();) {
        if (it->trigger.type == Trigger::TRIGGERTYPE_CONTROL_CLICK) {
            if (it->trigger.control != args.sender) {
                ++it;
                continue;
            }

            if (check_execute(*it, need_executes, &args)) {
                it = m_triggers.erase(it);
            }
            else {
                ++it;
            }
        }
        else if (it->trigger.type == Trigger::TRIGGERTYPE_CONTROL_ITEM_CLICK) {
            if (it->trigger.control_name && it->trigger.control_name != args.sender->GetName()) {
                ++it;
                continue;
            }

			NPGUIListBoxAdvItem* list_item = NULL;
            if (NPGUIListBoxCol * list_control = NPDynamicCast(NPGUIListBoxCol, it->trigger.control)) {
				list_item =  NPDynamicCast(NPGUIListBoxAdvItem, list_control->GetItem(args.sender) ) ;
			}
			else if (NPGUIListBoxRow * list_control = NPDynamicCast(NPGUIListBoxRow, it->trigger.control)) {
				list_item =  NPDynamicCast(NPGUIListBoxAdvItem, list_control->GetItem(args.sender) ) ;
			}

            assert(list_item);

            args.lParam = list_item;
            args.wParam = (void*)list_item->GetIndex();
            if (check_execute(*it, need_executes, &args)) {
                it = m_triggers.erase(it);
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
    
    for(TriggerContainer::iterator it = need_executes.begin(); it != need_executes.end(); ++it) {
        it->fun.call(*it, &args);
    }
}

void PageEvtSch::doTriggers() {
    TriggerContainer need_executes;

    for(TriggerContainer::iterator it = m_triggers.begin(); it != m_triggers.end();) {
        if (it->trigger.category == Trigger::ONCE) {
            need_executes.push_back(*it);
            it = m_triggers.erase(it);
        }
        else {
            ++it;
        }
    }

    for(TriggerContainer::iterator it = need_executes.begin(); it != need_executes.end(); ++it) {
        it->fun.call(*it, NULL);
    }
}
    
void PageEvtSch::clearTriggers() {
    for(TriggerContainer::iterator it = m_triggers.begin(); it != m_triggers.end();) {
        if (it->trigger.category == Trigger::ONCE) {
            it = m_triggers.erase(it);
        }
        else {
            ++it;
        }
    }
}

bool PageEvtSch::check_execute(TriggerNode & node, TriggerContainer & need_executes, NPGUIEventArgs * args) {
    if (node.trigger.delay_ms == 0) {
        need_executes.push_back(node);
        return node.trigger.category == Trigger::ONCE ? true : false;
    }
    else {
        node.timer =
            Gd::Timer::TimerCenter::instance(Gd::App::Application::instance())
            .registerTimer(
                *this, &PageEvtSch::on_timer,
                node.trigger.delay_ms, node.trigger.delay_ms, 1);
        return false;
    }
}

bool PageEvtSch::haveTrigger(NPGUIControl const * control) const {
    for(TriggerContainer::const_iterator it =  m_triggers.begin();
        it != m_triggers.end();
        ++it)
    {
        if (it->trigger.control == control) return true;
    }

    return false;
}

void PageEvtSch::on_timer(Gd::Timer::TimerID timerId) {
    for(TriggerContainer::iterator it =  m_triggers.begin();
        it != m_triggers.end();
        ++it)
    {
        if (it->timer != timerId) continue;

        TriggerNode node = *it;
        //APP_INFO("triggers erase %d ==", node.id);
        m_triggers.erase(it);

        node.fun.call(node, NULL);

        break;
    }
}

NPGUIListBoxAdvItem * PageEvtSch::_cvt_to_list_item(NPGUIControl * control) {
    return NPDynamicCast(NPGUIListBoxAdvItem, control);
}

}}
