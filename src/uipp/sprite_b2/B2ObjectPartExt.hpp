#ifndef UIPP_SPRITE_B2_OBJECTPART_EXT_H
#define UIPP_SPRITE_B2_OBJECTPART_EXT_H
#include <string>
#include <vector>
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_b2/B2ObjectPart.hpp"
#include "Box2D/Dynamics/b2Fixture.h"
#include "B2ObjectPartMeta.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2WorldExt;
class B2ObjectExt;
class B2ObjectPartExt : public B2ObjectPart {
public:
    B2ObjectPartExt(B2ObjectExt & obj, B2ObjectPartMeta const & meta);
    B2ObjectPartExt(B2ObjectExt & obj, B2ObjectPartMeta const & meta, const B2ObjectPartExt & o);

    ~B2ObjectPartExt();

    B2ObjectPartMeta const & meta(void) const { return m_meta; }
    B2ObjectExt & obj(void) { return m_obj; }

    void updateCollision(void);

    int enter(void);
    void exit(void);

    void updateDebugArea(B2WorldExt & world);

	b2Fixture * fixture(void) { return m_b2Fixture; }

private:
    b2Shape * buildB2Shape(void) const;

    /*config */
    B2ObjectExt & m_obj;
    B2ObjectPartMeta const & m_meta;
    UI_SPRITE_B2_SHAPE * m_shape;

    /*runtime */
	b2Fixture * m_b2Fixture;
    uint32_t m_debug_box;

friend class B2ObjectPart;
};

}}}

#endif
