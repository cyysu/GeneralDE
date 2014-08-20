#include "cpe/dr/dr_data.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/State.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_ShowPage.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

UIAction_ShowPage::UIAction_ShowPage(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_ShowPage::UIAction_ShowPage(Sprite::Fsm::Action & action, UIAction_ShowPage const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_page_name(o.m_page_name)
{
}

int UIAction_ShowPage::enter(void) {
    Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();

    if (page.pageData()) {
        if (ui_sprite_event_t enterEvt = ui_sprite_fsm_state_enter_event(state())) {
            if (dr_meta_copy_same_entry_part(
                    page.pageData(), page.pageDataSize(), page.pageDataMeta(),
                    enterEvt->data, enterEvt->size, enterEvt->meta,
                    NULL, 0, app().em())
                < 0)
            {
                APP_CTX_ERROR(
                    app(), "entity %d(%s) ui-show-page(%s): enter: copy data fail!",
                    entity().id(), entity().name(), m_page_name.c_str());
                return -1;
            }
        }
    }

    page.Show();
    page.BringToFront();

    if (lifeCircle() == ui_sprite_fsm_action_life_circle_working) {
        startUpdate();
    }

    return 0;
}

void UIAction_ShowPage::update(float delta) {
    Page & page = m_env.get().uiCenter().page(m_page_name.c_str()).page();
    if (!page.WasVisible()) {
        stopUpdate();
    }
}

void UIAction_ShowPage::exit(void) {
    m_env.get().uiCenter().page(m_page_name.c_str()).page().Hide();
}

void UIAction_ShowPage::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_ShowPage>(repo)
        .on_enter(&UIAction_ShowPage::enter)
        .on_exit(&UIAction_ShowPage::exit)
        .on_update(&UIAction_ShowPage::update)
        ;
}

const char * UIAction_ShowPage::NAME = "ui-show-page";

}}

