#ifndef UIPP_APP_UIPHASENODE_EXT_H
#define UIPP_APP_UIPHASENODE_EXT_H
#include "uipp/app/UIPhaseNode.hpp"
#include "System.hpp"
#include "UIPhaseExt.hpp"
#include "UIPageProxyExt.hpp"

namespace UI { namespace App {

class UIPhaseNodeExt : public UIPhaseNode {
public:
    virtual UIPhaseExt const & phase(void) const = 0;

    virtual void saveState(void) = 0;
    virtual bool enter(void) = 0;
    virtual void exit(void) = 0;

    virtual void addVisiablePage(UIPageProxyExt & page) = 0;
    virtual void removeVisiablePage(UIPageProxyExt & page) = 0;
    virtual UIPageProxyExt const & visiablePageAt(size_t pos) const = 0;

    static ::std::auto_ptr<UIPhaseNodeExt> create(UIPhaseExt & phase);
};

}}

#endif


