#ifndef UIPP_APP_UIENTER_EXT_H
#define UIPP_APP_UIENTER_EXT_H
#include "uipp/app/UICenter.hpp"
#include "UIPhaseNodeExt.hpp"
#include "UIPageProxyExt.hpp"

namespace UI { namespace App {

class UICenterExt : public UICenter {
public:
    virtual UIPageProxyExt & page(const char * name) = 0;
    virtual UIPageProxyExt const & page(const char * name) const = 0;
    virtual UIPageProxyExt * findPage(const char * name) = 0;
    virtual UIPageProxyExt const * findPage(const char * name) const = 0;

    virtual UIPhaseNodeExt & curentPhase(void) = 0;
    virtual UIPhaseNode const & curentPhase(void) const = 0;

    virtual void stopPopupPage(UIPopupPage * page) = 0;

    virtual void addPopupPage(UIPopupPage & page) = 0;
    virtual void removePopupPage(UIPopupPage & page) = 0;

    static ::std::auto_ptr<UICenterExt> create(EnvExt & env, Cpe::Cfg::Node const & config);
};

}}

#endif
