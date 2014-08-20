#include "cpe/utils/math_ex.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_anim/AnimationBackend.hpp"
#include "B2WorldExt.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2WorldExt::B2WorldExt(WorldRes & world_res)
    : WorldResGen<B2World, B2WorldExt>(world_res)
    , WorldUpdatorGen<B2WorldExt>(world_res.world())
    , m_ptm_ratio(1.0f)
    , m_step_duration(0.1)
    , m_is_debug(false)
    , m_world(b2Vec2(0.0f, 10.0f))
    , m_boundary(NULL)
    , m_left_time(0)
    , m_contact_listener(*this)
    , m_group_masks(0)
    , m_debug_anim_group(UI_SPRITE_INVALID_ANIM_ID)
{
    m_debug_layer[0] = 0;

	m_objTypeMap.insert(std::make_pair( "boundary", 0));
	m_world.SetContactListener(&m_contact_listener);
	m_world.SetAllowSleeping(true);
	//m_world.SetContinuousPhysics(true);
}

B2WorldExt::~B2WorldExt() {
    if (m_debug_anim_group != UI_SPRITE_INVALID_ANIM_ID) {
        if (Anim::AnimationBackend * backend = world().findRes<Anim::AnimationBackend>()) {
            backend->removeGroup(m_debug_anim_group);
        }

        m_debug_anim_group = UI_SPRITE_INVALID_ANIM_ID;
    }
}

void B2World::setGravity(P2D::Pair const & gravity) {
    B2WorldExt & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

    o.m_world.SetGravity(b2Vec2(gravity.x, gravity.y));
}

P2D::Pair B2World::gravity(void) const {
    B2WorldExt const & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

    b2Vec2 g = o.m_world.GetGravity();

    P2D::Pair r = { g.x, g.y };

    return r;
}

void B2World::setPtmRatio(float ratio) {
    B2WorldExt & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

	o.m_ptm_ratio = ratio;
}

float B2World::ptmRatio(void) const {
    B2WorldExt const & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

	return o.m_ptm_ratio;
}

void B2World::setBoundary(P2D::Pair const & input_lt, P2D::Pair const & input_rb) {
    B2WorldExt & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

    if (o.m_boundary) {
        o.m_world.DestroyBody(o.m_boundary);
        o.m_boundary = NULL;
    }

    P2D::Pair lt;
    P2D::Pair rb;

    lt.x = input_lt.x / o.m_ptm_ratio;
    lt.y = input_lt.y / o.m_ptm_ratio;

    rb.x = input_rb.x / o.m_ptm_ratio;
    rb.y = input_rb.y / o.m_ptm_ratio;

	const float halfWidth = (rb.x - lt.x) / 2;
	const float halfHeight = (rb.y - lt.y) / 2;

    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(lt.x + halfWidth, lt.y + halfHeight); // bottom-left corner

    o.m_boundary = o.m_world.CreateBody(&groundBodyDef);

    /*设置四条边 */
    b2PolygonShape groundBox;
	b2Fixture*	   fixture;
    b2Filter filter;
    filter.categoryBits = o.obyType("boundary");
    filter.maskBits = 0xFFFF;

    // bottom
    groundBox.SetAsBox(halfWidth, 0, b2Vec2(0, halfHeight), 0);
    fixture = o.m_boundary->CreateFixture(&groundBox, 0);
    fixture->SetFilterData(filter);

    // top
    groundBox.SetAsBox(halfWidth, 0, b2Vec2(0, -halfHeight), 0);
    fixture = o.m_boundary->CreateFixture(&groundBox, 0);
    fixture->SetFilterData(filter);

    // left
    groundBox.SetAsBox(0, halfHeight, b2Vec2(-halfWidth, 0), 0);
    fixture = o.m_boundary->CreateFixture(&groundBox, 0);
    fixture->SetFilterData(filter);

    // right
    groundBox.SetAsBox(0, halfHeight, b2Vec2(halfWidth, 0), 0);
    fixture = o.m_boundary->CreateFixture(&groundBox, 0);
    fixture->SetFilterData(filter);
}

void B2World::addObjType(::std::string const & type_name) {
    B2WorldExt & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);
    
    if (o.m_objTypeMap.find(type_name) != o.m_objTypeMap.end()) {
        APP_CTX_THROW_EXCEPTION(
            o.world().app(), ::std::runtime_error,
            "B2ObjType '%s' duplicate!", type_name.c_str());
    }

    if (o.m_objTypeMap.size() >= 16) {
        APP_CTX_THROW_EXCEPTION(o.world().app(), ::std::runtime_error, "B2ObjType type is full!");
    }

	uint16_t size = o.m_objTypeMap.size();

	o.m_objTypeMap.insert(std::make_pair(type_name, size));
}

uint16_t B2World::obyType(::std::string const & type_name) const {
    B2WorldExt const & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

    B2WorldExt::ObjTypeMap::const_iterator pos = o.m_objTypeMap.find(type_name);
    if (pos == o.m_objTypeMap.end()) {
        APP_CTX_THROW_EXCEPTION(
            o.world().app(), ::std::runtime_error,
            "B2ObjType '%s' not exist!", type_name.c_str());
    }

    return 1 << pos->second;
}

void B2World::setDebug(bool is_debug) {
    B2WorldExt & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

    o.m_is_debug = is_debug;
}

bool B2World::debug(void) const { 
    B2WorldExt const & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);
    return o.m_is_debug;
}


void B2WorldExt::setStepDuration(float step_duration) {
    B2WorldExt & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);

    assert(step_duration > 0.0001f);
    o.m_step_duration = step_duration;
}

/*b2ContactListener*/
void B2WorldExt::ContactListenerAdp::BeginContact(b2Contact* contact) {
    b2Fixture * fixture_a = contact->GetFixtureA();
    b2Fixture * fixture_b = contact->GetFixtureB();
    if (fixture_a->GetBody() == m_world.m_boundary || fixture_b->GetBody() == m_world.m_boundary) {
        return;
    }

    B2ObjectPartExt * part_a = (B2ObjectPartExt *)fixture_a->GetUserData();
    B2ObjectPartExt * part_b = (B2ObjectPartExt *)fixture_b->GetUserData();

    if (part_a && part_b) {
        part_a->obj().onBeginContact(*part_b, *contact);
        part_b->obj().onBeginContact(*part_a, *contact);
    }
}
    
void B2WorldExt::ContactListenerAdp::EndContact(b2Contact* contact) {
    b2Fixture * fixture_a = contact->GetFixtureA();
    b2Fixture * fixture_b = contact->GetFixtureB();
    if (fixture_a->GetBody() == m_world.m_boundary || fixture_b->GetBody() == m_world.m_boundary) {
        return;
    }

	B2ObjectPartExt * part_a = (B2ObjectPartExt *)fixture_a->GetUserData();
	B2ObjectPartExt * part_b = (B2ObjectPartExt *)fixture_b->GetUserData();;

	if (part_a && part_b) {
		part_a->obj().onEndContact(*part_b, *contact);
		part_b->obj().onEndContact(*part_a, *contact);
	}
}

void B2WorldExt::onWorldUpdate(World & world, float delta) {
    m_left_time += delta;
    while(m_left_time >= m_step_duration) {
        m_world.Step(m_step_duration, 8, 3);
        m_left_time -= m_step_duration;
    }
}

void B2WorldExt::setDebugLayer(const char * layer) {
    strncpy(m_debug_layer, layer, sizeof(m_debug_layer));
}

void B2World::setUpdatorPriority(int8_t priority) {
    WorldUpdatorGen<B2WorldExt> & o = Cpe::Utils::calc_cast<B2WorldExt>(*this);
    o.setUpdatorPriority(priority);
}

uint32_t B2WorldExt::showDebugArea(b2Body & body, b2Shape & shape) {
    Anim::AnimationBackend * backend = world().findRes<Anim::AnimationBackend>();
    if (backend == NULL) {
        APP_CTX_ERROR(world().app(), "B2World: showDebugArea: no animation backend!");
        return UI_SPRITE_INVALID_ANIM_ID;
    }

    if (m_debug_anim_group == UI_SPRITE_INVALID_ANIM_ID) {
        if (m_debug_layer[0] == 0) {
            APP_CTX_ERROR(world().app(), "B2World: showDebugArea: no debug layer!");
            return UI_SPRITE_INVALID_ANIM_ID;
        }

        m_debug_anim_group = backend->createGroup(m_debug_layer, 0, 0);
        if (m_debug_anim_group == UI_SPRITE_INVALID_ANIM_ID) {
            APP_CTX_ERROR(world().app(), "B2World: showDebugArea: create group at layer %s fail!", m_debug_layer);
            return UI_SPRITE_INVALID_ANIM_ID;
        }
    }

    float x_base = body.GetPosition().x;
    float y_base = body.GetPosition().y;

    switch(shape.GetType()) {
    case b2Shape::e_circle: {
			b2CircleShape& circle = (b2CircleShape&)(shape);
			float p_x = x_base + circle.m_p.x;
			float p_y = y_base + circle.m_p.y;
			float radius = circle.m_radius;
			static char buf[256];
			snprintf(
				buf, sizeof(buf), "CIRCLE: p.x=%f, p.y=%f, radius=%f, color=yellow",
				p_x * m_ptm_ratio, p_y * m_ptm_ratio, radius * m_ptm_ratio);
			return backend->startAnimation(m_debug_anim_group, buf);
			}
        break;

    case b2Shape::e_chain: {
			b2ChainShape& chain = (b2ChainShape&)(shape);
			assert(chain.GetChildCount() > 2);

			static char buf[1024];
			snprintf(buf, sizeof(buf), "CHAIN: color=yellow,");
			std::string str = "CHAIN: ";
			for (int i=0; i < chain.GetChildCount(); i++)
			{
				b2Vec2& vec = chain.m_vertices[i];
				char temp[32];
				snprintf(temp, sizeof(temp), "(%f,%f),", (x_base + vec.x) * m_ptm_ratio, (y_base + vec.y) * m_ptm_ratio);
				strcat(buf, temp);
			}
		    
			return backend->startAnimation(m_debug_anim_group, buf);
			break;
		}

    case b2Shape::e_polygon: {
        b2PolygonShape& polygon = (b2PolygonShape&)(shape);
			if(polygon.GetVertexCount() == 4){
				static char buf[256];
				float x0 = x_base + polygon.GetVertex(0).x;
				float y0 = y_base + polygon.GetVertex(0).y;
				float x1 = x_base + polygon.GetVertex(2).x;
				float y1 = y_base + polygon.GetVertex(2).y;

				snprintf(
					buf, sizeof(buf), "BOX: lt.x=%f, lt.y=%f, rb.x=%f, rb.y=%f, color=yellow",
					x0 * m_ptm_ratio, y0 * m_ptm_ratio, x1 * m_ptm_ratio, y1 * m_ptm_ratio);
				return backend->startAnimation(m_debug_anim_group, buf);
			}
			break;
		}
    default:
        break;
    }

    return UI_SPRITE_INVALID_ANIM_ID;
}

void B2WorldExt::removeDebugArea(uint32_t animId) {
    Anim::AnimationBackend * backend = world().findRes<Anim::AnimationBackend>();
    if (backend == NULL) {
        APP_CTX_ERROR(world().app(), "B2World: removeDebugArea: no animation backend!");
        return;
    }

    backend->stopAnimation(animId);
}

B2World::~B2World() {
}

B2World & B2World::install(Sprite::World & world) {
    return B2WorldExt::install(world);
}

const char * B2World::NAME = "B2World";

}}}

