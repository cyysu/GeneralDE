#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/app/Page.hpp"
#include "UIAction_PlayAnim.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

class PlayAnimEventHandler : public RGUIEventHandler {
public:
    PlayAnimEventHandler(UIAction_PlayAnim & action)
        : m_action(action)
    {
    }

	virtual void operator () (RGUIEventArgs& args) {
        m_action.stopUpdate();
    }

    UIAction_PlayAnim & m_action;
};

UIAction_PlayAnim::UIAction_PlayAnim(Sprite::Fsm::Action & action)
    : ActionBase(action)
{
}

UIAction_PlayAnim::UIAction_PlayAnim(Sprite::Fsm::Action & action, UIAction_PlayAnim const & o)
    : ActionBase(action)
    , m_env(o.m_env)
    , m_page_name(o.m_page_name)
    , m_control_name(o.m_control_name)
    , m_anim_name(o.m_anim_name)
{
}

int UIAction_PlayAnim::enter(void) {
    UIPageProxyExt * page = findPage();
    if (page == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): page not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return -1;
    }

    if (!page->page().WasVisible()) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): page is not visiable!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return -1;
    }

    RGUIControl * control = page->page().findChild(m_control_name.c_str());
    if (control == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): control not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return -1;
    }

    const RGUIActorKeyData * animData = getAnimData(control);
    if (animData == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): anim is unknown!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return -1;
    }

    if (animData->frameKeyVec.empty()) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): anim not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return -1;
    }

	control->SetVisible(true);
    
	RGUIActorKeyCtrlMove * animCtrl = control->GetMoveAnimCtrl();
    assert(animCtrl);

    animCtrl->Clear();
    animCtrl->SetActorKeyVec(*animData);
    animCtrl->SetPlay(true);
    animCtrl->SetPlayHandler(NULL);
    animCtrl->SetStopHandler(new PlayAnimEventHandler(*this));

    if (lifeCircle() == ui_sprite_fsm_action_life_circle_working) {
        startUpdate();
    }

    return 0;
}

void UIAction_PlayAnim::update(float delta) {
    UIPageProxyExt * page = findPage();
    if (page == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): update: page not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return;
    }

    RGUIControl * control = page->page().findChild(m_control_name.c_str());
    if (control == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): update: control not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return;
    }

	RGUIActorKeyCtrlMove * animCtrl = control->GetMoveAnimCtrl();
    assert(animCtrl);
    PlayAnimEventHandler * handler = dynamic_cast<PlayAnimEventHandler *>(animCtrl->GetStopHandler());

    if (animCtrl->WasPlay()) {
        if (handler && &handler->m_action == this) return;
    }
    
    stopUpdate();
}

void UIAction_PlayAnim::exit(void) {
    UIPageProxyExt * page = findPage();
    if (page == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): exit: page not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return;
    }

    RGUIControl * control = page->page().findChild(m_control_name.c_str());
    if (control == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s) ui-show-anim(%s.%s.%s): exit: control not exist!",
            entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
        return;
    }

	RGUIActorKeyCtrlMove * animCtrl = control->GetMoveAnimCtrl();
    if (animCtrl->WasPlay()) {
        if (PlayAnimEventHandler * handler = dynamic_cast<PlayAnimEventHandler *>(animCtrl->GetStopHandler())) {
            if (&handler->m_action == this) {
                if (entity().debug()) {
                    APP_CTX_ERROR(
                        app(), "entity %d(%s) ui-show-anim(%s.%s.%s): exit: stop playing anim!",
                        entity().id(), entity().name(), m_page_name.c_str(), m_control_name.c_str(), m_anim_name.c_str());
                }

                animCtrl->SetPlay(false);
                animCtrl->SetStopHandler(NULL);
            }
        }
    }
}

const RGUIActorKeyData * 
UIAction_PlayAnim::getAnimData(RGUIControl * control) {
    if (strcmp(m_anim_name.c_str(), "show") == 0) {
        return &control->GetShowAnimData();
    }
    else if (strcmp(m_anim_name.c_str(), "hide") == 0) {
        return &control->GetHideAnimData();
    }
    else if (strcmp(m_anim_name.c_str(), "dead") == 0) {
        return &control->GetDeadAnimData();
    }
    else if (strcmp(m_anim_name.c_str(), "down") == 0) {
        return &control->GetDownAnimData();
    }
    else if (strcmp(m_anim_name.c_str(), "rise") == 0) {
        return &control->GetRiseAnimData();
    }
    else if (strcmp(m_anim_name.c_str(), "user") == 0) {
        return &control->GetUserAnimData();
    }
    else {
        return NULL;
    }
}

UIPageProxyExt * UIAction_PlayAnim::findPage(void) {
    return m_env.get().uiCenter().findPage(m_page_name.c_str());
}

void UIAction_PlayAnim::install(Sprite::Fsm::Repository & repo) {
    Sprite::Fsm::ActionReg<UIAction_PlayAnim>(repo)
        .on_enter(&UIAction_PlayAnim::enter)
        .on_exit(&UIAction_PlayAnim::exit)
        .on_update(&UIAction_PlayAnim::update)
        ;
}

const char * UIAction_PlayAnim::NAME = "ui-play-anim";

}}

