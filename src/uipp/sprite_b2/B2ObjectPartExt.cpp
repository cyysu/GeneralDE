#include <cassert>
#include <string>
#include "cpe/utils/math_ex.h"
#include "Box2D/Collision/Shapes/b2ChainShape.h"
#include "Box2D/Collision/Shapes/b2CircleShape.h"
#include "Box2D/Collision/Shapes/b2EdgeShape.h"
#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "uipp/sprite_2d/Transform.hpp"
#include "uipp/sprite_anim/System.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2ObjectExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2ObjectPartExt::B2ObjectPartExt(B2ObjectExt & obj, B2ObjectPartMeta const & meta)
    : m_obj(obj)
    , m_meta(meta)
    , m_shape(NULL)
    , m_b2Fixture(NULL)
    , m_debug_box(UI_SPRITE_INVALID_ANIM_ID)
{
}

B2ObjectPartExt::B2ObjectPartExt(B2ObjectExt & obj, B2ObjectPartMeta const & meta, const B2ObjectPartExt & o)
    : m_obj(obj)
    , m_meta(meta)
    , m_shape(NULL)
    , m_b2Fixture(NULL)
    , m_debug_box(UI_SPRITE_INVALID_ANIM_ID)
{
    if (o.m_shape) {
        createShape(*o.m_shape);
    }
}

B2ObjectPartExt::~B2ObjectPartExt() {
    delete [] (char*)m_shape;
    assert(m_b2Fixture == NULL);
    assert(m_debug_box == UI_SPRITE_INVALID_ANIM_ID);
}

void B2ObjectPartExt::updateCollision(void) {
    assert(m_b2Fixture);

    b2Filter filter;
    filter.categoryBits = m_obj.categories() | m_meta.categories();
    filter.maskBits = m_obj.collisions() | m_meta.collisions();

    Entity & e = m_obj.entity();
    if (e.debug()) {
        char category_buf[64];
        char mask_buf[64];
        APP_CTX_INFO(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: obj-type=%d, update collision: category=%s, mask=%s, group-index=%d, isSensor=%d",
            e.id(), e.name(), m_meta.name().c_str(), m_obj.body()->GetType(),
            cpe_str_mask_uint16(filter.categoryBits, category_buf, sizeof(category_buf)), 
            cpe_str_mask_uint16(filter.maskBits, mask_buf, sizeof(mask_buf)),
            filter.groupIndex, m_b2Fixture->IsSensor() ? 1 : 0);
    }

    m_b2Fixture->SetFilterData(filter);
}

int B2ObjectPartExt::enter(void) {
    Entity & e = m_obj.entity();
    B2WorldExt & world = m_obj.world().res<B2WorldExt>();
    b2Body * body = m_obj.body();

    assert(body);	
    assert(m_b2Fixture == NULL);

    b2FixtureDef fixture_def;
    fixture_def.friction = m_meta.friction();
    fixture_def.restitution = m_meta.restitution();
    fixture_def.density = m_meta.density();
    fixture_def.userData = (static_cast<B2ObjectPartExt*>(this));
    fixture_def.isSensor = m_meta.isSensor();

    fixture_def.shape =  buildB2Shape();
    if (fixture_def.shape == NULL) {
        APP_CTX_ERROR(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: enter: build shape fail!",
            e.id(), e.name(), m_meta.name().c_str());
        return -1;
    }

    m_b2Fixture = body->CreateFixture(&fixture_def);

    delete fixture_def.shape;
    fixture_def.shape = NULL;

    if (m_b2Fixture == NULL) {
        APP_CTX_ERROR(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: create fixture fail!",
            e.id(), e.name(), m_meta.name().c_str());
        return -1;
    }

    b2Filter filter;
    filter.categoryBits = m_obj.categories() | m_meta.categories();
    filter.maskBits = m_obj.collisions() | m_meta.collisions();
    m_b2Fixture->SetFilterData(filter);

    if(world.debug() || e.debug()) {
        char category_buf[64];
        char mask_buf[64];

        APP_CTX_INFO(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: enter: obj-type=%d, category=%s, mask=%s, group-index=%d, isSensor=%d",
            e.id(), e.name(), m_meta.name().c_str(),
            body->GetType(),
            cpe_str_mask_uint16(filter.categoryBits, category_buf, sizeof(category_buf)), 
            cpe_str_mask_uint16(filter.maskBits, mask_buf, sizeof(mask_buf)),
            filter.groupIndex, m_b2Fixture->IsSensor() ? 1 : 0);
    }

    return 0;
}

b2Shape * B2ObjectPartExt::buildB2Shape(void) const {
    Entity const & e = m_obj.entity();

    if (m_shape == NULL) {
        APP_CTX_ERROR(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: build shape: no shap def!",
            e.id(), e.name(), m_meta.name().c_str());
        return NULL;
    }

    P2D::Transform const & transform = e.component<P2D::Transform>();

    float ptm = m_obj.world().res<B2WorldExt>().ptmRatio();

    uint8_t pos_adj_policy = P2D::PosAdjByFlip | P2D::PosAdjByScale;

    switch(m_shape->type) {
    case UI_SPRITE_B2_SHAPE_ENTITY_RECT: {
        P2D::Pair lt = transform.localPos(P2D::TopLeft, 0);
        P2D::Pair rb = transform.localPos(P2D::BottomRight, 0);

        lt.x -= m_shape->data.entity_rect.adj.x;
        lt.y -= m_shape->data.entity_rect.adj.y;
        rb.x += m_shape->data.entity_rect.adj.x;
        rb.y += m_shape->data.entity_rect.adj.y;

        lt = transform.adjLocalPos(lt, pos_adj_policy);
        rb = transform.adjLocalPos(lt, pos_adj_policy);

        b2PolygonShape * b2_shape = new b2PolygonShape();
        b2_shape->SetAsBox(
            fabs(lt.x - rb.x) / ptm / 2.0f,
            fabs(lt.y - rb.y) / ptm / 2.0f,
            b2Vec2((lt.x + rb.x) / ptm/ 2.0f, (lt.y + rb.y) / ptm / 2.0f),
            0.0f);
        return b2_shape;
    }
    case UI_SPRITE_B2_SHAPE_BOX: {
        P2D::Pair lt = { m_shape->data.box.lt.x, m_shape->data.box.lt.y };
        P2D::Pair rb = { m_shape->data.box.rb.x, m_shape->data.box.rb.y };

        lt = transform.adjLocalPos(lt, pos_adj_policy);
        rb = transform.adjLocalPos(rb, pos_adj_policy);

        b2PolygonShape * b2_shape = new b2PolygonShape();
        b2_shape->SetAsBox(
            fabs(lt.x - rb.x) / ptm / 2.0f,
            fabs(lt.y - rb.y) / ptm / 2.0f,
            b2Vec2((lt.x + rb.x) / ptm/ 2.0f, (lt.y + rb.y) / ptm / 2.0f),
            0.0f);
        return b2_shape;
    }
    case UI_SPRITE_B2_SHAPE_CIRCLE: {
		b2CircleShape * b2_shape = new b2CircleShape();

		b2_shape->m_radius = m_shape->data.circle.radius * transform.scale() / ptm;
		b2_shape->m_p.x = m_shape->data.circle.center.x / ptm;
		b2_shape->m_p.y = m_shape->data.circle.center.y / ptm;
        return b2_shape;
	}
    case UI_SPRITE_B2_SHAPE_SECTOR:{
		b2ChainShape * b2_shape = new b2ChainShape();
		assert(m_shape->data.sector.angle_step < 100);
		b2Vec2 vs[100];
		P2D::Pair center;
		center.x = m_shape->data.sector.center.x / ptm;
		center.y = m_shape->data.sector.center.y / ptm;
		P2D::Pair pos = transform.adjLocalPos(center, pos_adj_policy);
		vs[0].Set(pos.x, pos.y);
		for (int i=0; i < m_shape->data.sector.angle_step + 1; i++)
		{
			float rad = (m_shape->data.sector.angle_start + (m_shape->data.sector.angle_range * i) / m_shape->data.sector.angle_step) * M_PI / 180.0f;
			pos.x = (m_shape->data.sector.center.x + m_shape->data.sector.radius * cos(rad)) / ptm;
			pos.y = (m_shape->data.sector.center.y + m_shape->data.sector.radius * sin(rad)) / ptm;
			pos = transform.adjLocalPos(pos, pos_adj_policy);

			vs[i + 1].Set(pos.x, pos.y);
		}
		
		b2_shape->CreateLoop(vs, m_shape->data.sector.angle_step + 2);
        return b2_shape;
	}
    case UI_SPRITE_B2_SHAPE_CHAIN: {
        ::std::vector<b2Vec2> b2_points;
        if (m_shape->data.chain.point_count < 3) {
            APP_CTX_ERROR(
                m_obj.app(), "entity %d(%s): B2Fixture[%s]: build chain shape: point not enouth!",
                e.id(), e.name(), m_meta.name().c_str());
            return NULL;
        }

        for(uint32_t i = 0; i < m_shape->data.chain.point_count; ++i) {
            P2D::Pair pt = { m_shape->data.chain.points[i].x, m_shape->data.chain.points[i].y };
            pt = transform.adjLocalPos(pt, pos_adj_policy);
            b2_points.push_back(b2Vec2(pt.x / ptm, pt.y / ptm));
        }

        b2ChainShape * b2_shape = new b2ChainShape();
        b2_shape->CreateLoop(&b2_points[0], b2_points.size());
        return b2_shape;
    }
    default:
        APP_CTX_ERROR(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: build shape: unknown shap type %d!",
            e.id(), e.name(), m_meta.name().c_str(), m_shape->type);
        return NULL;
    }
}

void B2ObjectPart::createShape(UI_SPRITE_B2_SHAPE const & shap) {
    B2ObjectPartExt & o = Cpe::Utils::calc_cast<B2ObjectPartExt>(*this);

    size_t buf_size;
    switch(shap.type) {
    case UI_SPRITE_B2_SHAPE_CHAIN:
        buf_size = sizeof(shap.data.chain.points[0]) * shap.data.chain.point_count + sizeof(shap);
        break;
    default:
        buf_size = sizeof(shap);
        break;
    }

    char * buf = new char[buf_size];

    if (o.m_shape) delete [] (char*)o.m_shape;

    o.m_shape = (UI_SPRITE_B2_SHAPE *)buf;

    memcpy(o.m_shape, &shap, buf_size);
}

void B2ObjectPartExt::exit(void) {
    Entity & e = m_obj.entity();
    B2WorldExt & world = m_obj.world().res<B2WorldExt>();

    if (m_b2Fixture == NULL) return;

    b2Body * body = m_obj.body();
    assert(body);	
    body->DestroyFixture(m_b2Fixture);
    m_b2Fixture = NULL;

    if (m_debug_box != UI_SPRITE_INVALID_ANIM_ID) {
        world.removeDebugArea(m_debug_box);
        m_debug_box = UI_SPRITE_INVALID_ANIM_ID;
    }

    if(world.debug() || e.debug()) {
        APP_CTX_INFO(
            m_obj.app(), "entity %d(%s): B2Fixture[%s]: exit",
            e.id(), e.name(), m_meta.name().c_str());
    }
} 

void B2ObjectPartExt::updateDebugArea(B2WorldExt & world) {
    if (m_debug_box != UI_SPRITE_INVALID_ANIM_ID) {
        world.removeDebugArea(m_debug_box);
        m_debug_box = UI_SPRITE_INVALID_ANIM_ID;
    }

    if (m_shape == NULL) return;

    b2Body * body = m_obj.body();
    assert(body);
    m_debug_box = m_obj.world().res<B2WorldExt>().showDebugArea(*body, *m_b2Fixture->GetShape());
}

B2ObjectPart::~B2ObjectPart() {
}

}}}
