#ifndef UIPP_SPRITE_B2_ACTION_WAITSTOP_H
#define UIPP_SPRITE_B2_ACTION_WAITSTOP_H
#include "uipp/sprite_b2/B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2Action_WaitStop {
public:
    static const char * NAME;
    static void install(Fsm::Repository & repo);
};

}}}

#endif
