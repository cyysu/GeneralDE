#ifndef UIPP_SPRITE_2D_TRANSFORM_H
#define UIPP_SPRITE_2D_TRANSFORM_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite/System.hpp"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace P2D {

class Transform : Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_2d_transform_t () const { return (ui_sprite_2d_transform_t)this; }

    Pair pos(PosPolicy policy) const { return ui_sprite_2d_transform_pos(*this, (uint8_t)policy); }
    void setPos(Pair const & pos) { ui_sprite_2d_transform_set_pos(*this, pos); }

    Pair scale(void) const { return ui_sprite_2d_transform_scale(*this); }
    void setScale(Pair const & scale) { ui_sprite_2d_transform_set_scale(*this, scale); }

    float angle(void) const { return ui_sprite_2d_transform_angle(*this); }
    void setAngle(float angle) { ui_sprite_2d_transform_set_angle(*this, angle); }

    Pair rectLt(void) const { return ui_sprite_2d_transform_rect_lt(*this); }
    Pair rectRb(void) const { return ui_sprite_2d_transform_rect_rb(*this); }
    void mergeRect(Pair const & lt, Pair const & rb) { ui_sprite_2d_transform_merge_rect(*this, lt, rb); }

    uint8_t flipX(void) const { return ui_sprite_2d_transform_flip_x(*this); }
    void setFlipX(uint8_t flip_x) { ui_sprite_2d_transform_set_flip_x(*this, flip_x); }

    uint8_t flipY(void) const { return ui_sprite_2d_transform_flip_y(*this); }
    void setFlipY(uint8_t flip_y) { ui_sprite_2d_transform_set_flip_y(*this, flip_y); }

    static const char * NAME;
};

PosPolicy posPolicyFromStr(const char * str_policy);

}}}

#endif
