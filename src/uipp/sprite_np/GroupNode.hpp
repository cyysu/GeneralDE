#ifndef UIPP_SPRITE_GROUP_NODE_H
#define UIPP_SPRITE_GROUP_NODE_H
#include "uipp/sprite_np/System.hpp"
#include "NP2DSTransRef.h"

namespace UI { namespace Sprite { namespace NP {

class LayerExt;
class GroupNode : public NP2DSTransRef {
public:
	explicit GroupNode(World & world, LayerExt & layer, uint32_t entity_id, float priority);

    uint32_t entityId(void) const { return m_entity_id; }
    World & world(void) { return m_world; }

    World & m_world;
    LayerExt & m_layer;
    uint32_t m_entity_id;
    float m_priority;
};

}}}

#endif
