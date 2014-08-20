#ifndef UIPP_SPRITE_B2_COLLISION_H
#define UIPP_SPRITE_B2_COLLISION_H
#include <set>
#include <string>
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_b2/B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2WorldExt;
class B2ObjectPartExt;
class B2Collision : public Cpe::Utils::Noncopyable {
public:
    typedef void (*on_collision_fun_t)(void * ctx, UI_SPRITE_B2_COLLISION_DATA const & data);

    B2Collision(const char * name, on_collision_fun_t fun, void * ctx);
    B2Collision(const char * name, on_collision_fun_t fun, void * ctx, B2Collision const & o);

    const char * name(void) const { return m_name; }
    uint16_t collisions(void) const { return m_collisions; }

    void addPart(::std::string const & part);

    bool isAssoc(B2ObjectPartExt & other);
    void addCollision(B2WorldExt & world, ::std::string const & collision);

    void onCollision(UI_SPRITE_B2_COLLISION_DATA const & data) {
        m_on_collision(m_ctx, data);
    }

private:
    const char * m_name;
    uint16_t m_collisions;
    ::std::set< ::std::string> m_parts;
    on_collision_fun_t m_on_collision;
    void * m_ctx;
};

}}}

#endif
