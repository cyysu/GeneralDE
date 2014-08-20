#ifndef UIPP_SPRITE_B2_OBJECT_EXT_H
#define UIPP_SPRITE_B2_OBJECT_EXT_H
#include <vector>
#include <set>
#include <string>
#include "Box2D/Dynamics/b2Body.h"
#include "uipp/sprite/ComponentWithDataGen.hpp"
#include "uipp/sprite/ComponentReg.hpp"
#include "uipp/sprite_b2/B2Object.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2WorldExt;
class B2ObjectPartExt;
class B2ObjectPartMeta;
class B2Collision;
class B2ObjectExt : public ComponentWithDataGen<B2Object, B2ObjectExt, UI_SPRITE_B2_OBJ_DATA> {
public:
    typedef ::std::vector<B2ObjectPartExt *> PartList;

    B2ObjectExt(Component & component);
    B2ObjectExt(Component & component, B2ObjectExt const & o);
    ~B2ObjectExt();

	void setTransform(const UI_SPRITE_2D_PAIR& transform, float angle);

	void applyLinearImpulseToCenter(const UI_SPRITE_2D_PAIR& pair_force);
	void applyLinearImpulseToCenter(float angle, float force);

    void setLinearVelocity(const UI_SPRITE_2D_PAIR& pair_velocity);
    void setLinearVelocity(float angle, float velocity);

    b2Body * body(void) { return  m_body; }

    uint16_t categories(void) const { return m_categories; }
    uint16_t collisions(void) const;

    void onBeginContact(B2ObjectPartExt & other, b2Contact & contact);
    void onEndContact(B2ObjectPartExt & other, b2Contact & contact);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setCategories(uint16_t categories);
    void setCollisions(uint16_t collisions);

    void setDefaultFixtureMeta(
        uint16_t categories, uint16_t collisions,
        float friction, float restitution, float density, bool isSensor);

    void addFixtureMeta(
        ::std::string const & name,
        uint16_t categories, uint16_t collisions,
        float friction, float restitution, float density, bool isSensor);

    void addCollision(B2Collision & collision);
    void removeCollision(B2Collision & collision);

    bool isCollisionWith(uint16_t mask) const;

    float adjAngleByMoving(float angle) const;

    PartList & parts(void) { return m_parts; }

    static void install(Repository & repo);

    void suspend(ObjectType type, RuningMode runingMode, bool resume_state);
    void resume(void);

private:
    B2ObjectPartMeta * findMeta(const char * partName);

    void updateMass(void);
    void updateCollision(void);
    void updateMovePolicy(void);
    void updateDebugArea(B2WorldExt & world);

    void updateParts(void);

    void onLocationUpdate(void);
    void onShapUpdate(void);
    void onSetLinearVelocity(void);

    RuningMode m_runingMode;
	ObjectType m_type;
    PartList m_parts;
    float m_gravityScale;
    float m_mass;
    uint8_t m_fixedRotation;
    uint8_t m_bullet;
    uint16_t m_categories;
    uint16_t m_default_collisions;
    B2ObjectPartMeta * m_default_part_meta;
    ::std::vector<B2ObjectPartMeta *> m_part_metas;

    b2Body * m_body;
    ::std::vector<B2Collision *> m_actions;

    uint8_t m_saved_ref_count;
    bool m_resume_state;
    RuningMode m_saved_runingMode;
	ObjectType m_saved_type;
    b2Vec2 m_saved_liner_velocity;
    float m_saved_angular_velocity;

friend class B2Object;
};

}}}

#endif
