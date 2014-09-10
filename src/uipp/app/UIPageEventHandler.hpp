#ifndef UIPP_APP_UIPAGEEVENTHANDLER_H
#define UIPP_APP_UIPAGEEVENTHANDLER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/app/Page.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIPageEventHandler : public Cpe::Utils::Noncopyable {
public:
    UIPageEventHandler(
        const char * event, Page::EventHandlerScope scope,
        ui_sprite_event_process_fun_t fun, void * ctx);
    ~UIPageEventHandler();

    bool isActive(void) const { return m_handler != NULL; }
    Page::EventHandlerScope scope(void) const { return m_scope; }

    void active(Sprite::Entity & entity);
    void deactive(Sprite::Entity & entity);


private:
    ::std::string m_event;
    Page::EventHandlerScope m_scope;
    ui_sprite_event_process_fun_t m_fun;
    void * m_ctx;
    ui_sprite_event_handler_t m_handler;
};

}}

#endif
