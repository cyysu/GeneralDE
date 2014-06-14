#ifndef UIPP_SPRITE_ANIM_BACKEND_H
#define UIPP_SPRITE_ANIM_BACKEND_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_anim/ui_sprite_anim_backend.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Anim {

class AnimationBackend : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_anim_backend_t () const { return (ui_sprite_anim_backend_t)this; }

    WorldRes & worldRes(void);
    WorldRes const & worldRes(void) const;

    World & world(void);
    World const & world(void) const;

    UI_SPRITE_2D_PAIR screenSize(void) const {
        ui_sprite_anim_backend_def const * p = def();
        return p->m_screen_size_fun(p->m_ctx);
    }

    uint32_t createGroup(const char * layer) {
        ui_sprite_anim_backend_def const * p = def();
        return p->m_create_group_fun(p->m_ctx, layer);
    }

    void removeGroup(uint32_t group_id) {
        ui_sprite_anim_backend_def const * p = def();
        p->m_remove_group_fun(p->m_ctx, group_id);
    }

    void setPos(uint32_t group_id, UI_SPRITE_2D_PAIR const & new_pos) {
        ui_sprite_anim_backend_def const * p = def();
        p->m_pos_update_fun(p->m_ctx, group_id, new_pos);
    }

    void setScale(uint32_t group_id, UI_SPRITE_2D_PAIR const & new_scale) {
        ui_sprite_anim_backend_def const * p = def();
        p->m_scale_update_fun(p->m_ctx, group_id, new_scale);
    }

    void setAngle(uint32_t group_id, float new_angle) {
        ui_sprite_anim_backend_def const * p = def();
        p->m_angle_update_fun(p->m_ctx, group_id, new_angle);
    }

    uint32_t startAnimation(uint32_t group_id, const char * res, uint8_t is_loop = 0, int32_t start = -1, int32_t end = -1) {
        ui_sprite_anim_backend_def const * p = def();
        return p->m_start_fun(p->m_ctx, group_id, res, is_loop, start, end);
    }

    void stopAnimation(uint32_t anim_id) {
        ui_sprite_anim_backend_def const * p = def();
        p->m_stop_fun(p->m_ctx, anim_id);
    }

    bool isAnimationRuning(uint32_t anim_id) const {
        ui_sprite_anim_backend_def const * p = def();
        return p->m_is_runing_fun(p->m_ctx, anim_id) ? true : false;
    }

    ui_sprite_anim_backend_def const * def(void) const {
        return ui_sprite_anim_backend_op(*this);
    }

    static const char * NAME;
};

}}}

#endif
