#ifndef UIPP_SPRITE_FSM_ACTION_GEN_H
#define UIPP_SPRITE_FSM_ACTION_GEN_H
#include "cpepp/utils/TypeUtils.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "Action.hpp"
#include "Repository.hpp"
#include "uipp/sprite/Entity.hpp"

namespace UI { namespace Sprite { namespace Fsm {

template<typename OuterT, typename T>
struct ActionHandlerTraits {
    static void (OuterT::*s_fun)(T const & evt);
};

template<typename BaseT, typename OuterT> 
class ActionGen : public BaseT {
public:
    typedef ActionGen ActionBase;

	ActionGen(UI::Sprite::Fsm::Action & action) : m_action(action) {
	}

    Gd::App::Application & app(void) { return world().app(); }
    Gd::App::Application const & app(void) const { return world().app(); }

    Entity & entity(void) { return m_action.entity(); }
    Entity const & entity(void) const { return m_action.entity(); }

    World & world(void) { return entity().world(); }
    World const & world(void) const { return entity().world(); }

    const char * name(void) const { return m_action.name(); }

    bool isActive(void) const { return m_action.isActive(); }
    bool isUpdate(void) const { return m_action.isUpdate(); }
    void startUpdate(void) { m_action.startUpdate(); }
    void stopUpdate(void) { m_action.stopUpdate(); }
    void syncUpdate(bool is_update) { m_action.syncUpdate(is_update); }

    /*event operations*/
    void sendEvent(LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_fsm_action_send_event(m_action, meta, data, data_size); }

    template<typename T>
    void sendEvent(T const & data) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void sendEventTo(const char * target, LPDRMETA meta, void const * data, size_t data_size) {
        ui_sprite_fsm_action_send_event_to(m_action, target, meta, data, data_size);
    }

    template<typename T>
    void sendEventTo(const char * target, T const & data) {
        sendEvent(target, Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /* build and send event operations*/
    void buildAndSendEvent(const char * event, dr_data_source_t data_source = NULL) {
        ui_sprite_fsm_action_build_and_send_event(m_action, event, data_source);
    }

    template<typename T1>
    void buildAndSendEvent(const char * event, T1 const & arg1) {
        dr_data_source data_source[1];

        data_source[0].m_data.m_meta = Cpe::Dr::MetaTraits<T1>::META;
        data_source[0].m_data.m_data = (void *)&arg1;
        data_source[0].m_data.m_size = Cpe::Dr::MetaTraits<T1>::data_size(arg1);
        data_source[0].m_next = NULL;

        buildAndSendEvent(event, data_source);
    }

    template<typename T1, typename T2>
    void buildAndSendEvent(const char * event, T1 const & arg1, T2 const & arg2) {
        dr_data_source data_source[2];

        data_source[0].m_data.m_meta = Cpe::Dr::MetaTraits<T1>::META;
        data_source[0].m_data.m_data = (void *)&arg1;
        data_source[0].m_data.m_size = Cpe::Dr::MetaTraits<T1>::data_size(arg1);
        data_source[0].m_next = &data_source[1];

        data_source[1].m_data.m_meta = Cpe::Dr::MetaTraits<T2>::META;
        data_source[1].m_data.m_data = (void *)&arg2;
        data_source[1].m_data.m_size = Cpe::Dr::MetaTraits<T2>::data_size(arg2);
        data_source[1].m_next = NULL;

        buildAndSendEvent(event, data_source);
    }

    template<typename T>
    void addEventHandler(void (OuterT::*fun)(T const & evt), ui_sprite_event_scope_t scope = ui_sprite_event_scope_self) {
        ActionHandlerTraits<OuterT, T>::s_fun = fun;

        if (ui_sprite_fsm_action_add_event_handler(
                m_action, scope, Cpe::Dr::MetaTraits<T>::NAME, process_event<T>, Cpe::Utils::calc_cast<OuterT>(this))
            != 0)
        {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "add event handler of %s fail!", Cpe::Dr::MetaTraits<T>::NAME);
        }
    }

private:
    template<typename T>
    static void process_event(void * ctx, ui_sprite_event_t evt) {
        try {
            (((OuterT *)ctx)->*ActionHandlerTraits<OuterT, T>::s_fun)(*(T const *)evt->data);
        }
        catch(...) {
        }
    }
            
	UI::Sprite::Fsm::Action & m_action;
};

template<typename OuterT, typename T>
void (OuterT::* ActionHandlerTraits<OuterT, T>::s_fun)(T const & evt);

}}}

#endif
