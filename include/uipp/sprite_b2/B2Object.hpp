#ifndef UIPP_SPRITE_B2_OBJECT_H
#define UIPP_SPRITE_B2_OBJECT_H
#include "Box2D/Dynamics/b2Body.h"
#include "B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2Object {
public:
    RuningMode runingMode(void) const;
    ObjectType type(void) const;
    void setTypeAndMode(ObjectType type, RuningMode mode);

    void setMass(float mass);
    float mass(void) const;

    float gravityScale(void) const;
    void setGravityScale(float v);

    uint8_t bullet(void) const;
    void setBullet(uint8_t v);

    uint8_t fixedRotation(void) const;
    void setFixedRotation(uint8_t v);

    P2D::Pair lineerVelocity(void) const;
    P2D::Pair pos(void) const;

	B2ObjectPart & addPart(const char * type_name);

    ~B2Object();

    static const char * NAME;
};

}}}

#endif

