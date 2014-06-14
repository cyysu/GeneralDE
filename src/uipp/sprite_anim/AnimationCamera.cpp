#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "ui/sprite_anim/ui_sprite_anim_module.h"
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_anim/AnimationCamera.hpp"

namespace UI { namespace Sprite { namespace Anim {

const char * AnimationCamera::NAME = UI_SPRITE_ANIM_CAMERA_TYPE_NAME;

AnimationCamera &
AnimationCamera::install(World & world, P2D::Pair const & screan_size, ui_sprite_anim_module_t module) {
    if (module == NULL) {
        module = ui_sprite_anim_module_find_nc(world.app(), NULL);
        if (module == NULL) {
            APP_CTX_THROW_EXCEPTION(
                world.app(),
                ::std::runtime_error,
                "AnimationCamera::install: default ui_sprite_anim_module module not exist!");
        }
    }

    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_create(module, world);
    if (camera == NULL) {
        APP_CTX_THROW_EXCEPTION(
            world.app(),
            ::std::runtime_error,
            "AnimationCamera::install: create camera fail!");
    }

    return *(AnimationCamera*)camera;
}

}}}
