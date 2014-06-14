#ifndef UIPP_SPRITE_B2_OBJECT_PART_H
#define UIPP_SPRITE_B2_OBJECT_PART_H
#include "cpepp/utils/ClassCategory.hpp"
#include "Box2D/Collision/Shapes/b2ChainShape.h"
#include "Box2D/Collision/Shapes/b2CircleShape.h"
#include "Box2D/Collision/Shapes/b2EdgeShape.h"
#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPart : public Cpe::Utils::Noncopyable {
public:
    b2ChainShape & createChainShap(void);
    b2CircleShape & createCircleShape(void);
    b2EdgeShape & createEdgeShape(void);
    b2PolygonShape & createPolygonShape(void);

    ~B2ObjectPart();
};

}}}

#endif

