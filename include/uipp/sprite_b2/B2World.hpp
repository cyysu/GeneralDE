#ifndef UIPP_SPRITE_B2_WORLD_H
#define UIPP_SPRITE_B2_WORLD_H
#include <string>
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite/WorldRes.hpp"
#include "B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2World : public Cpe::Utils::Noncopyable {
public:
    bool debug(void) const;
    void setDebug(bool is_debug);

    P2D::Pair gravity(void) const;
    void setGravity(P2D::Pair const & g);

    float ptmRatio(void) const;
	void setPtmRatio(float ratio);

    void setBoundary(P2D::Pair const & lt, P2D::Pair const & bt);

    void addObjType(::std::string const & type_name);
    uint16_t obyType(::std::string const & type_name) const;

    ~B2World();

    static const char * NAME;

    static B2World & install(Sprite::World & world);
};

}}}

#endif
