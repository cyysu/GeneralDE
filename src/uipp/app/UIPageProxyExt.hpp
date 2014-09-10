#ifndef UIPP_APP_UIPAGEPROXY_EXT_H
#define UIPP_APP_UIPAGEPROXY_EXT_H
#include "uipp/app/UIPageProxy.hpp"
#include "uipp/app/Page.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIPageProxyExt : public UIPageProxy {
public:
    virtual void addEventHandler(
        const char * event, Page::EventHandlerScope scope,
        ui_sprite_event_process_fun_t fun, void * ctx) = 0;

    virtual void onVisiableUpdate(void) = 0;

    static ::std::auto_ptr<UIPageProxyExt>
    create(
        UICenterExt & center, Cpe::Cfg::Node const & config);
};

}}

#endif


