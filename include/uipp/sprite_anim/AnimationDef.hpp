#ifndef UIPP_SPRITE_ANIM_DEF_H
#define UIPP_SPRITE_ANIM_DEF_H
#include "ui/sprite_anim/ui_sprite_anim_def.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Anim {

class AnimationDef : public ComponentSimulate {
public:
    operator ui_sprite_anim_def_t () const { return (ui_sprite_anim_def_t)this; }

    uint8_t autoStart(void) const { return ui_sprite_anim_def_auto_start(*this); }

    const char * animName(void) const { return ui_sprite_anim_def_anim_name(*this); }

    const char * animRes(void) const { return ui_sprite_anim_def_anim_res(*this); }
};

class AnimationDefIterator {
public:
    AnimationDefIterator() { m_it.next = NULL; }

    AnimationDef const * next(void) { return (AnimationDef const *)ui_sprite_anim_def_it_next(&m_it); }

private:
    ui_sprite_anim_def_it m_it;

friend class AnimationSch;
};

}}}

#endif
