#ifndef UIPP_APP_UIPHASE_H
#define UIPP_APP_UIPHASE_H
#include "System.hpp"

namespace UI { namespace App {

class UIPhase {
public:
    virtual const char * name(void) const = 0;
    virtual ui_cache_group_t group(void) = 0;
    virtual float fps(void) const = 0;

    virtual ~UIPhase();
};

}}

#endif
