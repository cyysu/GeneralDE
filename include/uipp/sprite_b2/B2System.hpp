#ifndef UIPP_SPRITE_B2_SYSTEM_H
#define UIPP_SPRITE_B2_SYSTEM_H
#include "uipp/sprite/System.hpp"
#include "uipp/sprite_fsm/System.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "protocol/ui/sprite_b2/ui_sprite_b2_data.h"

namespace UI { namespace Sprite { namespace B2 {

enum RuningMode {
    RUNINGMODE_PASSIVE = UI_SPRITE_B2_OBJ_RUNING_MODE_PASSIVE,
    RUNINGMODE_ACTIVE = UI_SPRITE_B2_OBJ_RUNING_MODE_ACTIVE
};

enum ObjectType {
	OBJECTTYPE_STATIC = UI_SPRITE_B2_OBJ_TYPE_STATIC,
	OBJECTTYPE_KINEMATIC = UI_SPRITE_B2_OBJ_TYPE_KINEMATIC,
    OBJECTTYPE_DYNAMIC = UI_SPRITE_B2_OBJ_TYPE_DYNAMIC,
};

class B2System;
class B2Object;
class B2ObjectPart;

extern P2D::Pair const V_ZERO;

}}}

#endif
