#ifndef UIPP_SPRITE_ANIM_CAMERA_H
#define UIPP_SPRITE_ANIM_CAMERA_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Anim {

class AnimationCamera : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_anim_camera_t () const { return (ui_sprite_anim_camera_t)this; }

    P2D::Pair screenSize(void) const { return  ui_sprite_anim_camera_screen_size(*this); }
    void setScreeSize(P2D::Pair const & screenSize) { ui_sprite_anim_camera_set_screen_size(*this, screenSize); }

    /*control */
    P2D::Pair pos(void) const { return ui_sprite_anim_camera_pos(*this); }
    float scale(void) const { return ui_sprite_anim_camera_scale(*this); }
    P2D::Pair scalePair(void) const { return ui_sprite_anim_camera_scale_pair(*this); }
    void setPosAndScale(P2D::Pair const & pos, float scale) { ui_sprite_anim_camera_set_pos_and_scale(*this, pos, scale); }

    /*limit*/
    P2D::Pair limitLT(void) const { return ui_sprite_anim_camera_limit_lt(*this); }
    P2D::Pair limitBR(void) const { return ui_sprite_anim_camera_limit_rb(*this); }
    void setLimit(P2D::Pair const & lt, P2D::Pair const & br) { ui_sprite_anim_camera_set_limit(*this, lt, br); }

    /*operation*/
    P2D::Pair centerPos(void) const { return ui_sprite_anim_camera_center_pos(*this); }

    P2D::Pair screenToWorld(P2D::Pair const & pos) const { return ui_sprite_anim_camera_screen_to_world(*this, pos); }
    P2D::Pair worldToScreen(P2D::Pair const & pos) const { return ui_sprite_anim_camera_world_to_screen(*this, pos); }

    P2D::Pair calcPosFromPosOnScreen(
        P2D::Pair const & pos_in_world, P2D::Pair const & pos_on_screen, float target_scale) const
    {
        return ui_sprite_anim_camera_calc_pos_from_pos_in_screen(*this, pos_in_world, pos_on_screen, target_scale);
    }

    /*static */
    static const char * NAME;

    static AnimationCamera & install(World & world, P2D::Pair const & screan_size, ui_sprite_anim_module_t module = NULL);
};

}}}

#endif
