#ifndef UIPP_APP_PAGEPHASENODE_H
#define UIPP_APP_PAGEPHASENODE_H
#include "System.hpp"

namespace UI { namespace App {

class UIPhaseNode {
public:
    virtual UIPhase const & phase(void) const = 0;

    virtual size_t visiablePageCount(void) const = 0;
    virtual UIPageProxy const & visiablePageAt(size_t pos) const = 0;

    virtual ~UIPhaseNode(); 
};

}}

#endif
