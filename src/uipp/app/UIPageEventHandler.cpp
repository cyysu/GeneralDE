#include "cpe/pal/pal_strings.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_event.h"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/World.hpp"
#include "UIPageEventHandler.hpp"

namespace UI { namespace App {

UIPageEventHandler::UIPageEventHandler(
    const char * event, Page::EventHandlerScope scope,
    ui_sprite_event_process_fun_t fun, void * ctx)
    : m_event(event)
    , m_scope(scope)
    , m_fun(fun)
    , m_ctx(ctx)
    , m_handler(NULL)
{
}

UIPageEventHandler::~UIPageEventHandler() {
    assert(m_handler == NULL);
}


void UIPageEventHandler::active(Sprite::Entity & entity) {
    assert(m_handler == NULL);

    m_handler = ui_sprite_entity_add_event_handler(entity, ui_sprite_event_scope_self, m_event.c_str(), m_fun, m_ctx);
    if (m_handler == NULL) {
        APP_CTX_THROW_EXCEPTION(
            entity.world().app(), ::std::runtime_error,
            "UIPageEventHandler: register handler %s fail", m_event.c_str());
    }
}

void UIPageEventHandler::deactive(Sprite::Entity & entity) {
    assert(m_handler);
    ui_sprite_event_handler_free(entity.world(), m_handler);
    m_handler = NULL;
}

}}

