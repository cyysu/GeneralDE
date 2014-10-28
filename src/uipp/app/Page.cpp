#include <map>
#include "RAudioManager.h"
#include "cpe/pal/pal_string.h"
#include "gd/app/app_log.h"
#include "R2DSFrameFileMgr.h"
#include "R2DSImageFileMgr.h"
#include "RGUILabel.h"
#include "RGUIPictureCondition.h"
#include "RGUILabelCondition.h"
#include "RGUIProgressBar.h"
#include "RGUIEditBox.h"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/app/Page.hpp"
#include "uipp/app/Gen/PageEvtSch.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"
#include "UIPageProxyExt.hpp"
#include "uipp/sprite_render/NpUtils.hpp"

namespace UI { namespace App {

Page::Page(Gd::App::Application & app, Cpe::Cfg::Node & cfg) 
    : m_app(app)
    , m_schedule(new PageEvtSch)
    , m_proxy(NULL)
    , m_data_meta(NULL)
    , m_data(NULL)
    , m_data_size(0)
{
}

Page::~Page() {
    assert(m_proxy == NULL);
    delete m_schedule;
}

UICenter & Page::uiCenter(void) {
    return m_proxy->center();
}

UICenter const & Page::uiCenter(void) const {
    return m_proxy->center();
}

Env & Page::env(void) {
    return m_proxy->center().env();
}

Env const & Page::env(void) const {
    return m_proxy->center().env();
}

Gd::App::Application & Page::app(void) {
    return m_app;
}

Gd::App::Application const & Page::app(void) const {
    return m_app;
}

void Page::SetVisible(bool flag) {
    if (WasVisible() != flag) {
        RGUIWindow::SetVisible(flag);
        assert(m_proxy);
        dynamic_cast<UIPageProxyExt*>(m_proxy)->onVisiableUpdate();
    }
}

void Page::sendEvent(LPDRMETA meta, void const * data, size_t data_size) {
    uiCenter().entity().sendEvent(meta, data, data_size);
}

void Page::showPopupPage(const char * message, const char * template_name) {
    uiCenter().showPopupPage(message, template_name);
}

void Page::showPopupPage(LPDRMETA meta, void const * data, size_t data_size, const char * template_name) {
    uiCenter().showPopupPage(meta, data, data_size, template_name);
}

void Page::showPopupErrorMsg(int error, const char * template_name) {
    uiCenter().showPopupErrorMsg(error, template_name);
}

void Page::OnShow(RGUIEventArgs& args) {
    RGUIWindow::OnShow(args);
    m_schedule->triggerByType(PageEvtSch::Trigger::TRIGGERTYPE_CONTROL_SHOW, args);
}

void Page::OnEventMouseClick(RGUIEventArgs& args) {
	RGUIWindow::OnEventMouseClick(args);
	m_schedule->triggerMouseClick(args);
}

void Page::OnEventMouseDown(RGUIEventArgs& args) {
	RGUIWindow::OnEventMouseDown(args);

    if (const char * sep = strrchr(args.sender->GetName().c_str(), '_')) {
        if (const char * audioRes = uiCenter().findAudioByPostfix(sep + 1)) {
            RAudioManager*	audioManager = RAudioManager::GetIns();
            assert(audioManager);

            std::string tmp(audioRes);
			int id = audioManager->AddSFX(tmp);
			if (id < 0) {
                APP_CTX_ERROR(app(), "Page %s: sfx %s not exist", name(), audioRes);
            }
            else {
				audioManager->StopSFX(id);
                audioManager->PlaySFX(id);
            }
        }
    }
}

void Page::OnEventListBoxItemShow(RGUIEventArgs& args) {
    RGUIWindow::OnEventListBoxItemShow(args);
    m_schedule->triggerByType(PageEvtSch::Trigger::TRIGGERTYPE_CONTROL_ITEM_SHOW, args);
}

bool Page::haveTrigger(RGUIControl const * control) const {
    return m_schedule->haveTrigger(control);
}

bool Page::isControlNameEq(RGUIControl * control, const char * name, UICenter const & uiCenter) {
    const char * check_name = control->GetName().c_str();
    size_t name_len = strlen(name);

    if (memcmp(check_name, name, name_len) == 0) {
        if (check_name[name_len] == 0) return true;
        if (check_name[name_len] == '_'
            && uiCenter.findAudioByPostfix(check_name + name_len + 1) != NULL)
        {
            return true;
        }
    }

    return false;
}

bool Page::isControlNameEq(RGUIControl * control, const char * name) const {
    return isControlNameEq(control, name, uiCenter());
}

static RGUIControl * findChildByName(RGUIControl * p, const char * name, UICenter const & uiCenter) {
	std::list<RGUIControl*> & childs = p->GetChildren();

	for(std::list<RGUIControl*>::iterator it = childs.begin();
        it != childs.end();
        ++it)
    {
        RGUIControl* check_control = *it;
        if (Page::isControlNameEq(check_control, name, uiCenter)) {
            return check_control;
        }

		RGUIControl* result = findChildByName(check_control, name, uiCenter);
		if (result) return result;
	}

	return NULL;
}

RGUIControl *
Page::findChild(RGUIControl * control, const char * name, UICenter const & uiCenter) {
    while(const char * sep = strchr(name, '.')) {
        char buf[64];
        size_t len = sep - name;

        strncpy(buf, name, len);
        buf[len] = 0;

        control = findChildByName(control, buf, uiCenter);
        if (control == NULL) return NULL;

        name = sep + 1;
    }

    control = findChildByName(control, name, uiCenter);
    if (control == NULL) return NULL;

    return control;
}

RGUIControl *
Page::findChild(RGUIControl * control, const char * name) {
    return findChild(control, name, uiCenter());
}

void Page::addEventHandler(const char * event, EventHandlerScope scope, ui_sprite_event_process_fun_t fun, void * ctx) {
    assert(m_proxy);
    dynamic_cast<UIPageProxyExt *>(m_proxy)->addEventHandler(event, scope, fun, ctx);
}

void Page::setLabelText(RGUIControl * control, const char * name, const char * text) {
    if (RGUILabel * c = RDynamicCast(RGUILabel, findChild(control, name))) {
        c->SetTextA(text);
    }
}

void Page::setLabelText(RGUIControl * control, const char * name, int value) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", value);
    setLabelText(control, name, buf);
}

void Page::setIndex(RGUIControl * control, const char * name, int index) {
    if (RGUIPictureCondition* pic = RDynamicCast(RGUIPictureCondition, findChild(control, name))) {
        pic->SetIndex(index < 0 ? 0 : index);
    }
	else if (RGUILabelCondition* lab = RDynamicCast(RGUILabelCondition, findChild(control, name))) {
		lab->SetIndex(index < 0 ? 0 : index);
	}
}

void Page::setProgress(RGUIControl * control, const char * name, float percent) {
    if (RGUIProgressBar* progress = RDynamicCast(RGUIProgressBar, findChild(control, name))) {
        progress->SetProgress(percent);
    }
}

void Page::setShowAnimPlay(RGUIControl * control, const char * name) {
    if (RGUIControl * c = findChild(control, name)) {
        setShowAnimPlay(c);
    }
}

void Page::setShowAnimPlay(RGUIControl * control) {
    assert(control);
    control->Hide();
    control->SetShowAnimPlay();
}

void Page::setHideAnimPlay(RGUIControl * control, const char * name) {
    if (RGUIControl * c = findChild(control, name)) {
        setHideAnimPlay(c);
    }
}

void Page::setHideAnimPlay(RGUIControl * control) {
    assert(control);
    control->Show();
    control->SetHideAnimPlay();
}

void Page::setBackFrame(RGUIControl * control, const char * name, const char * resource) {
   if (RGUIControl * c = findChild(control, name)) {
	  UI::Sprite::R::NpUtils::setBackFrame(c,resource);
   }
}

bool Page::isChildOf(RGUIControl* control, const char * name) const{
	for(RGUIControl* p = control->GetParent(); p; p = p->GetParent()) {
		if (strcmp(p->GetName().c_str(), name) == 0) return true;
	}
	return false;
}

bool Page::isControlNameWith(RGUIControl* control, const char * str) const{
	return strstr(control->GetName().c_str(), str) != NULL;
}

void Page::setControlEnable(RGUIControl * control, const char * name, bool is_enable) {
	if (RGUIControl * c = findChild(control, name)) {
        c->SetEnable(is_enable);
    }
}

void Page::setControlVisible(RGUIControl * control, const char * name, bool is_visiable) {
    if (RGUIControl * c = findChild(control, name)) {
        c->SetVisible(is_visiable);
    }
}

void Page::setListCount(RGUIControl * control, const char * name, uint32_t item_count) {
}

::std::string Page::getText(RGUIControl * control, const char * name) {
    if (RGUIEditBox * ebox = RDynamicCast(RGUIEditBox, findChild(control, name))) {
        return ebox->GetTextA();
    }
    else {
        return "";
    }
}

void Page::setUserData(RGUIControl * control, const char * data) {
    control->SetUserText(data);
}

void Page::setUserData(RGUIControl * control, const char * name, const char * data) {
    if (RGUIControl * c = findChild(control, name)) {
        setUserData(c, data);
    }
}

bool Page::readResId(char * res, int & id) {
	if (char * p = strrchr(res, '#')) {
		*p = 0;
		id = atoi(p + 1);
		return true;
	}
	else {
		//assert(false);
		return false;
	}
}

}}
