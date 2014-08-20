#ifndef UIPP_SPRITE_B2_OBJECTPART_META_H
#define UIPP_SPRITE_B2_OBJECTPART_META_H
#include <string>
#include "uipp/sprite_b2/B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartMeta {
public:
    B2ObjectPartMeta(
        ::std::string const & name,
        uint16_t categories, uint16_t collisions,
        float friction, float restitution, float density, bool isSensor);

    ::std::string const & name(void) const { return m_name; }
    
    /*摩擦力 */
    float friction(void) const { return m_friction; }

    /*回弹力 */
    float restitution(void) const { return m_restitution; }

    /*密度 */
    float density(void) const { return m_density; }

    /*传感器 */
    bool isSensor(void) const { return m_isSensor; }

    uint16_t categories(void) const { return m_categories; }
    uint16_t collisions(void) const { return m_collisions; }
private:
    ::std::string m_name;
    uint16_t m_categories;
    uint16_t m_collisions;
    float m_friction;
    float m_restitution;
    float m_density;
	bool m_isSensor;
};

}}}

#endif
