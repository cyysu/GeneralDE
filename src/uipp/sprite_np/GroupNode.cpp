#include "GroupNode.hpp"

namespace UI { namespace Sprite { namespace NP {

GroupNode::GroupNode(World & world, LayerExt & layer, uint32_t entity_id, float priority)
    : m_world(world)
    , m_layer(layer)
    , m_entity_id(entity_id)
    , m_priority(priority)
{
}

}}}

