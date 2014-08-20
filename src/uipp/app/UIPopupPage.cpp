#include "gdpp/app/Log.hpp"
#include "gdpp/timer/TimerCenter.hpp"
#include "NPGUIDesktop.h"
#include "UIPopupPage.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"
#include "UIPopupPageDef.hpp"

namespace UI { namespace App {

UIPopupPage::UIPopupPage(UICenterExt & center, UIPopupPageDef const & def) 
    : m_center(center)
    , m_def(def)
    , m_timerId(0)
{
    if (!LoadBinFile(def.res())) {
        APP_CTX_THROW_EXCEPTION(
            center.env().app(), ::std::runtime_error,
            "load popup page %s: load-from %s fail!", def.name(), def.res());
    }
    SetAlwaysTop(true);
    BringToFront();
    Show();

    if (def.duration() > 0.0f) {
        tl_time_span_t delay = (def.duration() * 1000);

        m_timerId =
            Gd::Timer::TimerCenter::instance(center.env().app())
            .registerTimer(*this, &UIPopupPage::onTimeout, delay, 0, 1);
        if (m_timerId == 0) {
            APP_CTX_THROW_EXCEPTION(
                center.env().app(), ::std::runtime_error,
                "load popup page %s: register time (duration=%d) fail!", def.name(), (int)delay);
        }
    }

    NPGUIDesktop::GetIns()->AddWindow(this);

    center.addPopupPage(*this);
}

UIPopupPage::~UIPopupPage() {
    m_center.removePopupPage(*this);

    NPGUIDesktop::GetIns()->DelChild(this, false);

    if (m_timerId != 0) {
        Gd::Timer::TimerCenter::instance(m_center.env().app())
            .unregisterTimer(m_timerId);
        m_timerId = 0;
    }
}

void UIPopupPage::onTimeout(Gd::Timer::TimerID timerId) {
    m_center.stopPopupPage(this);
}

}}

