#include "B2Collision.hpp"
#include "B2WorldExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2ObjectExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Collision::B2Collision(const char * name, on_collision_fun_t fun, void * ctx)
    : m_name(name)
    , m_collisions(0)
    , m_on_collision(fun)
    , m_ctx(ctx)
{
}

B2Collision::B2Collision(const char * name, on_collision_fun_t fun, void * ctx, B2Collision const & o)
    : m_name(name)
    , m_collisions(o.m_collisions)
    , m_parts(o.m_parts)
    , m_on_collision(fun)
    , m_ctx(ctx)
{
}

void B2Collision::addCollision(B2WorldExt & world, ::std::string const & collision) {
    if (collision == "*") {
        m_collisions = 0xFFFF;
    }
    else {
        m_collisions |= world.obyType(collision);
    }
}

void B2Collision::addPart(::std::string const & part) {
    m_parts.insert(part);
}

bool B2Collision::isAssoc(B2ObjectPartExt & other) {
    if (!(other.obj().categories() & m_collisions)) return false;

    return m_parts.find(other.meta().name()) != m_parts.end()
        || m_parts.find("*") != m_parts.end();
}


}}}
