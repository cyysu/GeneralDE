#ifndef UIPP_SPRITE_ANIM_SCH_H
#define UIPP_SPRITE_ANIM_SCH_H
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "uipp/sprite/ComponentSimulate.hpp"
#include "AnimationDef.hpp"

namespace UI { namespace Sprite { namespace Anim {

class AnimationSch : public ComponentSimulate {
public:
    operator ui_sprite_anim_sch_t () const { return (ui_sprite_anim_sch_t)this; }


    AnimationDef & createAnimationDef(const char * anim_name, const char * res, bool auto_start);

    AnimationDef const * findAnimationDef(const char * anim_name) const {
        return (AnimationDef const * )ui_sprite_anim_def_find(*this, anim_name);
    }


    AnimationDef const & animationDef(const char * anim_name) const;

    AnimationDefIterator animationDefs(void) const {
        AnimationDefIterator it;
        ui_sprite_anim_sch_defs(&it.m_it, *this);
        return it;
    }

    uint32_t startAnimation(const char * res, bool is_loop, const char * group = "") {
        return ui_sprite_anim_sch_start_anim(*this, group, res, is_loop, -1 , -1);
    }

    uint32_t startAnimation(const char * res, uint16_t start, uint16_t end, const char * group = "") {
        return ui_sprite_anim_sch_start_anim(*this, group, res, 0, start, end);
    }

    void stopAnimation(uint32_t anim_id) { ui_sprite_anim_sch_stop_anim(*this, anim_id); }

    void setDefaultLayer(const char * layer) { ui_sprite_anim_sch_set_default_layer(*this, layer); }

    static const char * NAME;
};

}}}

#endif
