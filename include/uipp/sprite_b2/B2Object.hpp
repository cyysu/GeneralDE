#ifndef UIPP_SPRITE_B2_OBJECT_H
#define UIPP_SPRITE_B2_OBJECT_H
#include "Box2D/Dynamics/b2Body.h"
#include "B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2Object {
public:
    enum RuningMode {
        Passive,
        Active
    };

    RuningMode runintMode(void) const;
    void setRuningMode(RuningMode mode);

    void setMass(float mass);
	void setType(b2BodyType type);

	B2ObjectPart & addPart(const char * name);

    ~B2Object();

    static const char * NAME;
};

}}}

#endif

