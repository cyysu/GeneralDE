#ifndef UIPP_SPRITE_B2_WORLDEXT_H
#define UIPP_SPRITE_B2_WORLDEXT_H
#include <map>
#include "Box2D/Box2D.h"
#include "uipp/sprite/WorldResGen.hpp"
#include "uipp/sprite/WorldUpdatorGen.hpp"
#include "uipp/sprite_b2/B2World.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectExt;
class B2WorldExt
    : public WorldResGen<B2World, B2WorldExt>
	, public WorldUpdatorGen<B2WorldExt>
{
    class ContactListenerAdp : public b2ContactListener {
    public:
        ContactListenerAdp(B2WorldExt & world) : m_world(world) {}

        void BeginContact(b2Contact* contact);
        void EndContact(b2Contact* contact);
    private:
        B2WorldExt & m_world;
    };

public:
    B2WorldExt(WorldRes & world_res);
    ~B2WorldExt();

    b2World & bw(void) { return m_world; }

    float stepDuration(void) const { return m_step_duration; }
    void setStepDuration(float step_duration);

    void setDebugLayer(const char * layer);

	void onWorldUpdate(World & world, float delta);

    uint32_t showDebugArea(b2Body & body, b2Shape & shape);
    void removeDebugArea(uint32_t animId);

    void setGroundMasks(uint16_t masks) { m_group_masks = masks; }
    uint16_t groundMasks(void) const { return m_group_masks; }

private:
	typedef std::map< ::std::string, uint16_t> ObjTypeMap;

    float m_ptm_ratio;
    float m_step_duration;
    bool m_is_debug;
	ObjTypeMap m_objTypeMap;

    class b2World m_world;
    b2Body * m_boundary;
    float m_left_time;
    ContactListenerAdp m_contact_listener;

    uint16_t m_group_masks;

    char m_debug_layer[64];
    uint32_t m_debug_anim_group;

friend class B2World;
};

}}}

#endif
