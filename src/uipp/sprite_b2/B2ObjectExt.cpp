#include <cassert>
#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartMeta.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2WorldExt.hpp"
#include "B2Collision.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2ObjectExt::B2ObjectExt(Component & component)
    : ComponentBase(component)
    , m_runingMode(RUNINGMODE_ACTIVE)
	, m_type(OBJECTTYPE_DYNAMIC)
    , m_gravityScale(1.0f)
    , m_mass(0.0f)
    , m_fixedRotation(1)
    , m_bullet(0)
    , m_categories(0)
    , m_default_collisions(0)
    , m_default_part_meta(NULL)
    , m_body(NULL)
    , m_saved_ref_count(0)
    , m_resume_state(false)
{
}

B2ObjectExt::B2ObjectExt(Component & component, B2ObjectExt const & o)
    : ComponentBase(component)
    , m_runingMode(o.m_runingMode)
	, m_type(o.m_type)
    , m_gravityScale(o.m_gravityScale)
    , m_mass(o.m_mass)
    , m_fixedRotation(o.m_fixedRotation)
    , m_bullet(o.m_bullet)
    , m_categories(o.m_categories)
    , m_default_collisions(o.m_default_collisions)
    , m_default_part_meta(NULL)
    , m_body(NULL)
    , m_saved_ref_count(0)
    , m_resume_state(false)
{
    if (o.m_default_part_meta) {
        m_default_part_meta = new B2ObjectPartMeta(*o.m_default_part_meta);
    }

    for(::std::vector<B2ObjectPartMeta *>::const_iterator it = o.m_part_metas.begin();
        it != o.m_part_metas.end();
        ++it)
    {
        m_part_metas.push_back(new B2ObjectPartMeta(**it));
    }

    for(PartList::const_iterator part_it = o.m_parts.begin();
        part_it != o.m_parts.end();
        ++part_it)
    {
        B2ObjectPartMeta * meta = findMeta((*part_it)->meta().name().c_str());
        assert(meta);
        m_parts.push_back(new B2ObjectPartExt(*this, *meta, **part_it));
    }
}

B2ObjectExt::~B2ObjectExt() {
    if (m_body) {
        exit();
        assert(m_body == NULL);
    }

    for(PartList::reverse_iterator it = m_parts.rbegin();
        it != m_parts.rend();
        ++it)
    {
        delete *it;
    }

    m_parts.clear();

    if (m_default_part_meta) {
        delete m_default_part_meta;
        m_default_part_meta = NULL;
    }

    for(::std::vector<B2ObjectPartMeta *>::iterator it = m_part_metas.begin();
        it != m_part_metas.end();
        ++it)
    {
        delete *it;
    }

    m_part_metas.clear();
}

void B2ObjectExt::onBeginContact(B2ObjectPartExt & other, b2Contact & contact) {
    if (m_actions.empty()) return;

    float ptm = world().res<B2WorldExt>().ptmRatio();

    b2WorldManifold manifold;
    contact.GetWorldManifold(&manifold);

    UI_SPRITE_B2_COLLISION_DATA data;
    data.collision_state = UI_SPRITE_B2_COLLISION_STATE_BEGIN;
    data.collision_entity_id = other.obj().entity().id();
    strncpy(data.collision_part_name, other.meta().name().c_str(), sizeof(data.collision_part_name));

    data.collision_pos.x = manifold.points[0].x * ptm;
    data.collision_pos.y = manifold.points[0].y * ptm;

    // printf(
    //     "xxxx: entity %d(%s): begin contact, pos[0]=(%f,%f), pos[1]=(%f,%f)\n",
    //     entity().id(), entity().name(),
    //     manifold.points[0].x * ptm , manifold.points[0].y * ptm, 
    //     manifold.points[1].x * ptm , manifold.points[1].y * ptm);

    uint16_t categories = other.meta().categories() | other.obj().categories();
    Entity & e = entity();
    Entity & other_e = other.obj().entity();

    for(::std::vector<B2Collision *>::iterator it = m_actions.begin();
        it != m_actions.end();
        ++it)
    {
        B2Collision & action = **it;

        if (e.debug()) {
            char buf1[64];
            char buf2[64];
            APP_CTX_INFO(
                app(), "entity %d(%s): %s: Collision begin: with %d(%s)[%s]: require mask %s, part mask is %s",
                e.id(), e.name(), action.name(), other_e.id(), other_e.name(), other.meta().name().c_str(),
                cpe_str_mask_uint16(action.collisions(), buf1, sizeof(buf1)),
                cpe_str_mask_uint16(categories, buf2, sizeof(buf2)));
        }

        if (action.collisions() & categories) {
            action.onCollision(data);
        }
    }
}

void B2ObjectExt::onEndContact(B2ObjectPartExt & other, b2Contact & contact) {
    if (m_actions.empty()) return;

    float ptm = world().res<B2WorldExt>().ptmRatio();

    b2WorldManifold manifold;
    contact.GetWorldManifold(&manifold);


    UI_SPRITE_B2_COLLISION_DATA data;
    data.collision_state = UI_SPRITE_B2_COLLISION_STATE_END;
    data.collision_entity_id = other.obj().entity().id();
    strncpy(data.collision_part_name, other.meta().name().c_str(), sizeof(data.collision_part_name));

    data.collision_pos.x = manifold.points[0].x * ptm;
    data.collision_pos.y = manifold.points[0].y * ptm;

    uint16_t categories = other.meta().categories() | other.obj().categories();
    Entity & e = entity();
    Entity & other_e = other.obj().entity();

    for(::std::vector<B2Collision *>::iterator it = m_actions.begin();
        it != m_actions.end();
        ++it)
    {
        B2Collision & action = **it;

        if (e.debug()) {
            char buf1[64];
            char buf2[64];
            APP_CTX_INFO(
                app(), "entity %d(%s): %s: Collision end: with %d(%s)[%s]: require mask %s, part mask is %s",
                e.id(), e.name(), action.name(), other_e.id(), other_e.name(), other.meta().name().c_str(),
                cpe_str_mask_uint16(action.collisions(), buf1, sizeof(buf1)),
                cpe_str_mask_uint16(categories, buf2, sizeof(buf2)));
        }

        if (action.collisions() & categories) {
            action.onCollision(data);
        }
    }
}

int B2ObjectExt::enter(void) {
    Entity & e = entity();

    if (m_body) {
        APP_CTX_ERROR(
            app(), "entity %d(%s): B2Object: enter: is already entered!",
            e.id(), e.name());
        return -1;
    }

    B2WorldExt & world = this->world().res<B2WorldExt>();
    b2World & bw = world.bw();
	
	b2BodyDef bodyDef;
    bodyDef.type =(b2BodyType)m_type;
    bodyDef.fixedRotation = m_fixedRotation;
    bodyDef.bullet = m_bullet ? true : false;
    bodyDef.gravityScale = m_gravityScale;

    if (P2D::Transform * transform = e.findComponent<P2D::Transform>()) {
        P2D::Pair curent_pos = transform->originPos();

        bodyDef.position.Set(
            curent_pos.x / world.ptmRatio(), 
			curent_pos.y / world.ptmRatio());

		bodyDef.angle = transform->angle() * b2_pi / 180.f;
    }	

    m_body = bw.CreateBody(&bodyDef);

    if (m_body == NULL) {
        APP_CTX_ERROR(
            app(), "entity %d(%s): B2Object: enter: create body fail!",
            e.id(), e.name());
        return -1;
    }

    for(int pos = 0; pos < m_parts.size(); ++pos) {
        B2ObjectPartExt * part = m_parts[pos];
		if (part->enter()) {
			APP_CTX_ERROR(
				app(), "entity %d(%s): B2Object: enter: part enter fail!",
				e.id(), e.name());

			for(--pos; pos >= 0; --pos) {
				part = m_parts[pos];
				part->exit();
			}
			bw.DestroyBody(m_body);
			m_body = NULL;

			return -1;
		}
    }

    updateMass();
    updateMovePolicy();

    if(world.debug() && e.debug()) {
        updateDebugArea(world);
    }

    addAttrMonitor<99>(
        "setter.linear_velocity_angle,setter.linear_velocity_value",
        &B2ObjectExt::onSetLinearVelocity);
    onSetLinearVelocity();

    return 0;
}

void B2ObjectExt::exit(void) {
    if (m_body == NULL) return;

    B2WorldExt  & world = this->world().res<B2WorldExt>();

    b2World & bw = world.bw();

    for(PartList::reverse_iterator it = m_parts.rbegin();
        it != m_parts.rend();
        ++it)
    {
        B2ObjectPartExt * part = *it;
        part->exit();
    }

    bw.DestroyBody(m_body);
    m_body = NULL;
}

void B2ObjectExt::update(float delta) {
    Entity & e = entity();

    assert(m_body);

	B2WorldExt & world = this->world().res<B2WorldExt>();
    P2D::Transform * transform = e.findComponent<P2D::Transform>();
    if (transform == NULL) return;

    float ptm = world.ptmRatio();

    P2D::Pair pos = { m_body->GetPosition().x * ptm, m_body->GetPosition().y * ptm };
    float angle = m_body->GetAngle() * 180.f / b2_pi;

    P2D::Pair oldPos = transform->originPos();
    float oldAngle = transform->angle();

    if (fabs(pos.x - oldPos.x) > 0.01f || fabs(pos.y - oldPos.y) > 0.01f || fabs(angle - oldAngle) > 0.001f) {
        transform->setOriginPos(pos);
        transform->setAngle(angle);

        if(world.debug() && e.debug()) {
            updateDebugArea(world);
        }
    }
}

float B2Object::mass(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
    return o.m_mass;
}

void B2Object::setMass(float mass) {
    B2ObjectExt & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);

    o.m_mass = mass;

    if (o.isActive()) o.updateMass();
}

float B2Object::gravityScale(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
    return o.m_gravityScale;
}

void B2Object::setGravityScale(float v) {
    B2ObjectExt & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);

    o.m_gravityScale = v;

    if (o.m_body) o.m_body->SetGravityScale(o.m_gravityScale);
}

uint8_t B2Object::bullet(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
    return o.m_bullet;
}

void B2Object::setBullet(uint8_t v) {
    B2ObjectExt & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);

    o.m_bullet = v;

    if (o.m_body) o.m_body->SetBullet(o.m_bullet ? true : false);
}

uint8_t B2Object::fixedRotation(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
    return o.m_fixedRotation;
}

void B2Object::setFixedRotation(uint8_t v) {
    B2ObjectExt & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);

    o.m_fixedRotation = v;

    if (o.m_body) o.m_body->SetFixedRotation(o.m_fixedRotation ? true : false);
}

ObjectType B2Object::type(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
	return o.m_type;
}

RuningMode B2Object::runingMode(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
	return o.m_runingMode;
}

void B2Object::setTypeAndMode(ObjectType type, RuningMode mode) {
    B2ObjectExt & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);

	o.m_type = type;
    o.m_runingMode = mode;

	if (o.isActive()) {
        o.m_body->SetType((b2BodyType)type);
        o.updateMovePolicy();
        o.updateMass();
	}
}

void B2ObjectExt::applyLinearImpulseToCenter(const UI_SPRITE_2D_PAIR& pair_impuls) {
	if (m_body == NULL) {
		APP_INFO("entity %d(%s): applyLinearImpulseToCenter: no body!", entity().id(), entity().name());
		return;
	}

    float ptm = world().res<B2WorldExt>().ptmRatio();

	m_body->ApplyLinearImpulse(b2Vec2(pair_impuls.x / ptm, pair_impuls.y / ptm), m_body->GetWorldCenter(), true);
}

void B2ObjectExt::applyLinearImpulseToCenter(float angle, float impuls) {
	if (m_body == NULL) {
		APP_INFO("entity %d(%s): applyLinearImpulseToCenter: no body!", entity().id(), entity().name());
		return;
	}

	float  rad = angle * b2_pi / 180.f;
    float ptm = world().res<B2WorldExt>().ptmRatio();

    impuls /= ptm;

	m_body->ApplyLinearImpulse(b2Vec2(cos(rad) * impuls, sin(rad) * impuls), m_body->GetWorldCenter(), true);
}

void B2ObjectExt::setLinearVelocity(const UI_SPRITE_2D_PAIR& pair_velocity) {
	if (m_body == NULL) {
		APP_INFO("entity %d(%s): setLinearVelocity: no body!", entity().id(), entity().name());
		return;
	}

    float ptm = world().res<B2WorldExt>().ptmRatio();

	m_body->SetLinearVelocity(b2Vec2(pair_velocity.x / ptm, pair_velocity.y / ptm));
}

void B2ObjectExt::setLinearVelocity(float angle, float velocity) {
	if (m_body == NULL) {
		APP_INFO("entity %d(%s): setLinearVelocity: no body!", entity().id(), entity().name());
		return;
	}

    if (velocity == 0) {
        m_body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    }
    else {
        float  rad = angle * b2_pi / 180.f;
        float ptm = world().res<B2WorldExt>().ptmRatio();
        
        velocity /= ptm;
        
        m_body->SetLinearVelocity(b2Vec2(cos(rad) * velocity, sin(rad) * velocity));
    }
}

void B2ObjectExt::setTransform(const UI_SPRITE_2D_PAIR& pos, float angle) {
	if (m_body == NULL) {
		APP_INFO("error: this body is null while applyForce!!!");
		return;
	}
    
    float ptm = world().res<B2WorldExt>().ptmRatio();

	m_body->SetTransform(b2Vec2(pos.x / ptm, pos.y / ptm), angle * b2_pi / 180.f);
}

B2ObjectPartMeta * B2ObjectExt::findMeta(const char * partName) {
    if (m_default_part_meta) return m_default_part_meta;

    for(::std::vector<B2ObjectPartMeta *>::iterator it = m_part_metas.begin();
        it != m_part_metas.end();
        ++it)
    {
        B2ObjectPartMeta * check_meta = *it;
        if (strcmp(check_meta->name().c_str(), partName) == 0) {
            return check_meta;
        }
    }

    return NULL;
}

B2ObjectPart & B2Object::addPart(const char * name) {
    B2ObjectExt & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);

    Entity & e = o.entity();

    B2ObjectPartMeta * meta = o.findMeta(name);
    if (meta == NULL) {
        APP_CTX_THROW_EXCEPTION(
            o.app(), ::std::runtime_error,
            "entity %d(%s): B2Object: part %s is unknown!",
            e.id(), e.name(), name);
    }

    o.m_parts.push_back(new B2ObjectPartExt(o, *meta));

    return *o.m_parts.back();
}

uint16_t B2ObjectExt::collisions(void) const {
    uint16_t collisions = m_default_collisions;

    for(::std::vector<B2Collision *>::const_iterator action_it = m_actions.begin();
        action_it != m_actions.end();
        ++action_it)
    {
        B2Collision const & action = **action_it;
        collisions |= action.collisions();
    }

    return collisions;
}

void B2ObjectExt::setCategories(uint16_t categories) {
    m_categories = categories;

    if (isActive()) {
        updateCollision();
    }
}

void B2ObjectExt::setCollisions(uint16_t categories) {
    m_default_collisions = categories;

    if (isActive()) {
        updateCollision();
    }
}

void B2ObjectExt::setDefaultFixtureMeta(uint16_t categories, uint16_t collisions, float friction, float restitution, float density, bool isSensor) {
    if (!m_part_metas.empty()) {
		APP_CTX_THROW_EXCEPTION(
			app(),
			::std::runtime_error,
			"B2Object: can`t set default fixture when have fixtures!");
    }

    if (m_default_part_meta) {
        delete m_default_part_meta;
    }

    m_default_part_meta = new B2ObjectPartMeta("", categories, collisions, friction, restitution, density, isSensor);
}

void B2ObjectExt::addFixtureMeta(::std::string const & name, uint16_t categories, uint16_t collisions, float friction, float restitution, float density, bool isSensor) {
    if (m_default_part_meta) {
		APP_CTX_THROW_EXCEPTION(
			app(),
			::std::runtime_error,
			"B2Object: can`t set add fixture when have default fixture!");
    }

    m_part_metas.push_back(new B2ObjectPartMeta(name, categories, collisions, friction, restitution, density, isSensor));
}

void B2ObjectExt::updateMass(void) {
    assert(m_body);

	b2MassData massData;
    m_body->GetMassData(&massData);
	massData.mass = m_mass;
	m_body->SetMassData(&massData);
}

void B2ObjectExt::addCollision(B2Collision & collision) {
    m_actions.push_back(&collision);

    if (isActive()) {
        updateCollision();
    }
}

void B2ObjectExt::removeCollision(B2Collision & collision) {
    for(::std::vector<B2Collision *>::iterator it = m_actions.begin();
        it != m_actions.end();
        ++it)
    {
        if (*it == &collision) {
            m_actions.erase(it);
            break;
        }
    }

    if (isActive()) {
        updateCollision();
    }
}

void B2ObjectExt::updateCollision(void) {
    for(PartList::const_iterator part_it = m_parts.begin();
        part_it != m_parts.end();
        ++part_it)
    {
        B2ObjectPartExt & part = **part_it;
        part.updateCollision();
    }
}

void B2ObjectExt::updateMovePolicy(void) {
    clearAttrMonitors();

    addAttrMonitor<0>("transform.scale,transform.flip_x,transform.flip_y", &B2ObjectExt::onShapUpdate);

    if (m_runingMode == RUNINGMODE_ACTIVE && m_type != OBJECTTYPE_STATIC) {
        syncUpdate(true);
    }
    else {
        syncUpdate(false);

        addAttrMonitor<1>("transform.pos,transform.angle", &B2ObjectExt::onLocationUpdate);
    }
}

P2D::Pair B2Object::pos(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
    assert(o.m_body);

    float ptm = o.world().res<B2WorldExt>().ptmRatio();

    P2D::Pair pos = { o.m_body->GetPosition().x * ptm, o.m_body->GetPosition().y * ptm };

    return pos;
}

P2D::Pair B2Object::lineerVelocity(void) const {
    B2ObjectExt const & o = Cpe::Utils::calc_cast<B2ObjectExt>(*this);
    assert(o.m_body);

    const b2Vec2& v = o.m_body->GetLinearVelocity();

    float ptm = o.world().res<B2WorldExt>().ptmRatio();

    P2D::Pair r = { v.x * ptm , v.y * ptm };
    return r;
}

bool B2ObjectExt::isCollisionWith(uint16_t mask) const {
    assert(m_body);

    for (b2ContactEdge const * edge = m_body->GetContactList(); edge; edge = edge->next) {
        b2Contact const * contact = edge->contact;
        if (!contact->IsTouching()) continue;

        b2Fixture const * other = contact->GetFixtureA();
        if (other->GetBody() == m_body) other = contact->GetFixtureB();

        if (other->GetFilterData().categoryBits & mask) {
            return true;
        }
    }

    return false;
}

void B2ObjectExt::updateDebugArea(B2WorldExt & world) {
    for(PartList::const_iterator part_it = m_parts.begin();
        part_it != m_parts.end();
        ++part_it)
    {
        B2ObjectPartExt & part = **part_it;
        part.updateDebugArea(world);
    }
}

void B2ObjectExt::onLocationUpdate(void) {
    P2D::Transform const & transform = entity().component<P2D::Transform>();

    setTransform(transform.originPos(), transform.angle());
    m_body->SetAwake(true);

    B2WorldExt & world = this->world().res<B2WorldExt>();
    
    if(world.debug() && entity().debug()) {
        updateDebugArea(world);
    }
}

void B2ObjectExt::onSetLinearVelocity(void) {
    UI_SPRITE_B2_OBJ_DATA & d = const_cast<UI_SPRITE_B2_OBJ_DATA &>(attrData());
    if (d.setter.linear_velocity_angle.setted || d.setter.linear_velocity_value.setted) {
        P2D::Pair old_v = lineerVelocity();

        float angle;
        if (d.setter.linear_velocity_angle.setted) {
            angle = d.setter.linear_velocity_angle.value;
            d.setter.linear_velocity_angle.setted = 0;
        }
        else {
            angle = cpe_math_angle(0, 0, old_v.x, old_v.y);
        }

        float value;
        if (d.setter.linear_velocity_value.setted) {
            value = d.setter.linear_velocity_value.value;
            d.setter.linear_velocity_value.setted = 0;
        }
        else {
            value = cpe_math_distance(0, 0, old_v.x, old_v.y);
        }

        setLinearVelocity(angle, value);
    }
}

void B2ObjectExt::onShapUpdate(void) {
    Entity & e = entity();

    for(PartList::const_iterator part_it = m_parts.begin();
        part_it != m_parts.end();
        ++part_it)
    {
        B2ObjectPartExt & part = **part_it;
        part.exit();
    }

    for(PartList::const_iterator part_it = m_parts.begin();
        part_it != m_parts.end();
        ++part_it)
    {
        B2ObjectPartExt & part = **part_it;
		if (part.enter() != 0) {
			APP_CTX_ERROR(app(), "entity %d(%s): B2Object: onShapUpdate: part enter fail!", e.id(), e.name());
		}
    }

    updateMass();

    B2WorldExt & world = this->world().res<B2WorldExt>();
    if(world.debug() && e.debug()) {
        updateDebugArea(world);
    }

    m_body->SetAwake(true);
}

float B2ObjectExt::adjAngleByMoving(float angle) const {
    if (m_body == NULL) return angle;

    b2Vec2 const & v = m_body->GetLinearVelocity();

    if (v.x == 0.0f && v.y == 0.0f) return angle;

    return cpe_math_angle(0, 0, v.x, v.y) + angle;
}

void B2ObjectExt::suspend(ObjectType type, RuningMode runingMode, bool resume_state) {
    if (m_saved_ref_count == 0) {
        m_saved_type = m_type;
        m_saved_runingMode = m_runingMode;
        m_saved_liner_velocity = m_body->GetLinearVelocity();
        m_saved_angular_velocity = m_body->GetAngularVelocity();
    }

    m_resume_state = resume_state;

    setTypeAndMode(type, runingMode);
    m_body->SetGravityScale(0.0f);

    m_body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    m_body->SetAngularVelocity(0.0f);

    ++m_saved_ref_count;
}

void B2ObjectExt::resume(void) {
    assert(m_saved_ref_count > 0);
    --m_saved_ref_count;

    if (m_saved_ref_count > 0) return;

    m_body->SetGravityScale(m_gravityScale);
    setTypeAndMode(m_saved_type, m_saved_runingMode);
    m_body->SetAwake(true);

    if (m_resume_state) {
        m_body->SetLinearVelocity(m_saved_liner_velocity);
        m_body->SetAngularVelocity(m_saved_angular_velocity);
    }
}

void B2ObjectExt::install(Repository & repo) {
    ComponentReg<B2ObjectExt>(repo)
        .with_data()
        .on_enter(&B2ObjectExt::enter)
        .on_exit(&B2ObjectExt::exit)
        .on_update(&B2ObjectExt::update)
        ;
}

B2Object::~B2Object() {
}

P2D::Pair const V_ZERO = { 0.0f, 0.0f };

const char * B2Object::NAME = "B2Object";

}}}

