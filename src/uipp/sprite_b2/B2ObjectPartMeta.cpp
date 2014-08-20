#include "B2ObjectPartMeta.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2ObjectPartMeta::B2ObjectPartMeta(
    ::std::string const & name, uint16_t categories, uint16_t collisions,
    float friction, float restitution, float density, bool isSensor)
    : m_name(name)
    , m_categories(categories)
    , m_collisions(collisions)
    , m_friction(friction)
    , m_restitution(restitution)
    , m_density(density)
    , m_isSensor(isSensor)
{
}

}}}
