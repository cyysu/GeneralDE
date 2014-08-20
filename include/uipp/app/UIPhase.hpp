#ifndef UIPP_APP_UIPHASE_H
#define UIPP_APP_UIPHASE_H
#include "System.hpp"

namespace UI { namespace App {

class UIPhase {
public:
    virtual const char * name(void) const = 0;
    virtual ~UIPhase();
};

}}

#endif
