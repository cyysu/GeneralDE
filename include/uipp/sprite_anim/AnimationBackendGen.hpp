#ifndef UIPP_SPRITE_ANIM_BACKEND_GEN_H
#define UIPP_SPRITE_ANIM_BACKEND_GEN_H
#include "cpe/pal/pal_strings.h"
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "ui/sprite_anim/ui_sprite_anim_module.h"
#include "AnimationBackend.hpp"

namespace UI { namespace Sprite { namespace Anim {

template<typename OuterT> 
class AnimationBackendGen: public Cpe::Utils::Noncopyable {
public:
    static UI_SPRITE_2D_PAIR screen_size(void * ctx) {
        return ((OuterT*)ctx)->screenSize();
    }

    static uint32_t start_fun(
        void * ctx, uint32_t group_id, const char * res, 
        uint8_t is_loop, int32_t start, int32_t end)
    {
        OuterT * o = (OuterT*)ctx;
        try {
            return o->startAnimation(group_id, res, is_loop, start, end);
        }
        catch(...) {
            return UI_SPRITE_INVALID_ANIM_ID;
        }
    }

    static uint32_t create_group(void * ctx, const char * layer) {
        OuterT * o = (OuterT*)ctx;
        try {
            return o->createGroup(layer);
        }
        catch(...) {
            return UI_SPRITE_INVALID_ANIM_ID;
        }
    }

    static void remove_group(void * ctx, uint32_t group_id) {
        OuterT * o = (OuterT*)ctx;
        try {
            o->removeGroup(group_id);
        }
        catch(...) {
        }
    }

    static void stop_fun(void * ctx, uint32_t anim_id) {
        try {
            ((OuterT*)ctx)->stopAnimation(anim_id);
        }
        catch(...) {
        }
    }

    static uint8_t is_runing_fun(void * ctx, uint32_t anim_id) {
        OuterT * o = (OuterT*)ctx;
        try {
            return o->isAnimationRuning(anim_id) ? 1 : 0;
        }
        catch(...) {
            return 0;
        }
    }

    static int set_template_value_fun(void * ctx, uint32_t anim_id, const char * ctrl_name, const char * attr_name, const char * value) {
        OuterT * o = (OuterT*)ctx;
        try {
            return o->setTemplateValue(anim_id, ctrl_name, attr_name, value);
        }
        catch(...) {
            return -1;
        }
    }

    static void on_pos_update(void * ctx, uint32_t group_id, UI_SPRITE_2D_PAIR new_pos) {
        OuterT * o = (OuterT*)ctx;
        try {
            o->onGroupPosUpdate(group_id, new_pos);
        }
        catch(...) {
        }
    }

    static void on_scale_update(void * ctx, uint32_t group_id, UI_SPRITE_2D_PAIR new_scale) {
        OuterT * o = (OuterT*)ctx;
        try {
            o->onGroupScaleUpdate(group_id, new_scale);
        }
        APP_CTX_CATCH_EXCEPTION(o->world().app(), "on_scale_update: ");
    }

    static void on_angle_update(void * ctx, uint32_t group_id, float new_angle) {
        OuterT * o = (OuterT*)ctx;
        try {
            o->onGroupAngleUpdate(group_id, new_angle);
        }
        APP_CTX_CATCH_EXCEPTION(o->world().app(), "on_angle_update: ");
    }

    static void on_camera_update(void * ctx, UI_SPRITE_2D_PAIR pos, UI_SPRITE_2D_PAIR scale) {
        OuterT * o = (OuterT*)ctx;
        try {
            o->onCameraUpdate(pos, scale);
        }
        APP_CTX_CATCH_EXCEPTION(o->world().app(), "on_camera_update: ");
    }

    AnimationBackendGen(UI::Sprite::World & world, OuterT * p, const char * module_name = NULL) : m_world(world) {
        ui_sprite_anim_backend_t backend = ui_sprite_anim_backend_find(world);
        if (backend == NULL) {
			APP_CTX_THROW_EXCEPTION(
                world.app(),
                ::std::runtime_error,
                "anim backend install fail: backend not exist!");
        }

        ui_sprite_anim_backend_def def = {
            (void*)p,
            screen_size,
            create_group,
            remove_group,
            on_pos_update,
            on_scale_update,
            on_angle_update,
            start_fun,
            stop_fun,
            is_runing_fun,
            set_template_value_fun,
            on_camera_update,
        };

        ui_sprite_anim_backend_set_op(backend, &def);
    }

    ~AnimationBackendGen() {
        ui_sprite_anim_backend_t backend = ui_sprite_anim_backend_find(m_world);
        if (backend) {
            ui_sprite_anim_backend_def def;
            bzero(&def, sizeof(def));
            ui_sprite_anim_backend_set_op(backend, &def);
        }
    }

private:
    ui_sprite_world_t m_world;
};

}}}

#endif
