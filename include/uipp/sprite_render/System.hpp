#ifndef UIPP_SPRITE_R_SYSTEM_H
#define UIPP_SPRITE_R_SYSTEM_H
#include "uipp/sprite/System.hpp"
class RNode;

namespace UI { namespace Sprite { namespace R {

class Layer;
class Context;
class AnimationBackend;

typedef RNode * (*node_build_fun_t)(ui_sprite_world_t world, void * ctx, const char * args);

}}}

#endif
