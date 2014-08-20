#ifndef UIPP_APP_UIPOPUPPAGE_H
#define UIPP_APP_UIPOPUPPAGE_H
#include "NPGUIWindow.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/timer/TimerProcessor.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIPopupPage
    : public NPGUIWindow
    , public Gd::Timer::TimerProcessor
{
public:
    UIPopupPage(UICenterExt & center, UIPopupPageDef const & def);
    ~UIPopupPage();

private:
    void onTimeout(Gd::Timer::TimerID timerId);

    UICenterExt & m_center;
    UIPopupPageDef const & m_def;
    Gd::Timer::TimerID m_timerId;
};

}}

#endif
