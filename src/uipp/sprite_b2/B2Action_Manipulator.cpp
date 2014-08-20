#include "cpe/utils/math_ex.h"
#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "B2Action_Manipulator.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_Manipulator::B2Action_Manipulator(Fsm::Action & action)
    : ActionBase(action)
    , m_rand_ctx(NULL)
{
}

B2Action_Manipulator::B2Action_Manipulator(Fsm::Action & action, B2Action_Manipulator const & o)
    : ActionBase(action)
    , m_rand_ctx(NULL)
{
}

B2Action_Manipulator::~B2Action_Manipulator() {
    delete m_rand_ctx;
}

int B2Action_Manipulator::enter(void) {
    addEventHandler(&B2Action_Manipulator::onStop);
    addEventHandler(&B2Action_Manipulator::onSetAwake);
    addEventHandler(&B2Action_Manipulator::onSetToEntity);
    addEventHandler(&B2Action_Manipulator::onPushByForceAngle);
    addEventHandler(&B2Action_Manipulator::onPushByForcePair);
    addEventHandler(&B2Action_Manipulator::onSetLinearVelocityPair);
    addEventHandler(&B2Action_Manipulator::onSetLinearVelocityAngle);
    addEventHandler(&B2Action_Manipulator::onRandLineearVelocityAngle);
    return 0;
}

void B2Action_Manipulator::exit(void) {
}

void B2Action_Manipulator::onStop(UI_SPRITE_EVT_B2_STOP const & evt) {
    B2ObjectExt & b2Object = entity().component<B2ObjectExt>();		
    b2Object.body()->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    b2Object.body()->SetAngularVelocity(0.0f);
}

void B2Action_Manipulator::onSetToEntity(UI_SPRITE_EVT_B2_SET_TO_ENTITY const & evt) {
    Entity * move_to_entity;

    if (evt.entity_id) {
        move_to_entity = world().findEntity(evt.entity_id);
        if (move_to_entity == NULL) {
            APP_CTX_ERROR(app(), "onSetToEntity: can not find entity %d not exist", evt.entity_id);
            return;
        }
    }
    else {
        move_to_entity = world().findEntity(evt.entity_name);
        if (move_to_entity == NULL) {
            APP_CTX_ERROR(app(), "onSetToEntity: can not find entity %s not exist", evt.entity_name);
            return;
        }
    }

    P2D::Transform * transform = move_to_entity->findComponent<P2D::Transform>();
    if (transform == NULL) {
        APP_CTX_ERROR(
            app(), "onSetToEntity: entity %d(%s) no transform",
            move_to_entity->id(), move_to_entity->name());
        return;
    }

    entity().component<B2ObjectExt>()
        .setTransform(
            transform->worldPos(P2D::posPolicyFromStr(evt.pos_of_entity), P2D::PosAdjAll),
            transform->angle());
}

void B2Action_Manipulator::onPushByForceAngle(UI_SPRITE_EVT_B2_PUSH_BY_IMPULSE_ANGLE const & evt) {
    Entity & e = entity();
    B2ObjectExt & b2Object = e.component<B2ObjectExt>();

    float angle = evt.angle;

    switch(evt.base_policy) {
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_FLIP:
        angle = e.component<P2D::Transform>().adjAngleByFlip(angle);
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_ANGLE:
        angle = e.component<P2D::Transform>().angle() + angle;
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_MOVING:
        angle = b2Object.adjAngleByMoving(angle);
        break;
    }

    b2Object.applyLinearImpulseToCenter(angle, evt.impulse);
}

void B2Action_Manipulator::onPushByForcePair(UI_SPRITE_EVT_B2_PUSH_BY_IMPULSE_PAIR const & evt) {
    Entity & e = entity();
    B2ObjectExt & b2Object = e.component<B2ObjectExt>();

    UI_SPRITE_2D_PAIR impulse = { evt.impulse.x, evt.impulse.y };

    if (impulse.x == 0.0f && impulse.y == 0.0f) return;

    if (evt.base_policy == 0) {
        b2Object.applyLinearImpulseToCenter(impulse);
        return;
    }

    float angle = cpe_math_angle(0.0f, 0.0f, impulse.x, impulse.y);
    float distance = cpe_math_distance(0.0f, 0.0f, impulse.x, impulse.y);

    switch(evt.base_policy) {
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_FLIP:
        angle = e.component<P2D::Transform>().adjAngleByFlip(angle);
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_ANGLE:
        angle = e.component<P2D::Transform>().angle() + angle;
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_MOVING:
        angle = b2Object.adjAngleByMoving(angle);
        break;
    }

    b2Object.applyLinearImpulseToCenter(angle, distance);
}

void B2Action_Manipulator::onSetLinearVelocityAngle(UI_SPRITE_EVT_B2_SET_LINEAR_VELOCITY_ANGLE const & evt) {
    Entity & e = entity();
    B2ObjectExt & b2Object = e.component<B2ObjectExt>();

    float angle = evt.angle;

    switch(evt.base_policy) {
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_FLIP:
        angle = e.component<P2D::Transform>().adjAngleByFlip(angle);
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_ANGLE:
        angle = e.component<P2D::Transform>().angle() + angle;
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_MOVING:
        angle = b2Object.adjAngleByMoving(angle);
        break;
    }

    b2Object.setLinearVelocity(angle, evt.velocity);
}

void B2Action_Manipulator::onRandLineearVelocityAngle(UI_SPRITE_EVT_B2_RAND_LINEAR_VELOCITY_ANGLE const & evt) {
    Entity & e = entity();
    B2ObjectExt & b2Object = e.component<B2ObjectExt>();

    float angle = evt.angle_min + (evt.angle_max - evt.angle_min) * prand();
    float velocity = evt.velocity_min + (evt.velocity_max - evt.velocity_min) * prand();

    switch(evt.base_policy) {
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_FLIP:
        angle = e.component<P2D::Transform>().adjAngleByFlip(angle);
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_ANGLE:
        angle = e.component<P2D::Transform>().angle() + angle;
        break;
    case UI_SPRITE_EVT_B2_OP_BASE_OBJECT_MOVING:
        angle = b2Object.adjAngleByMoving(angle);
        break;
    }

    b2Object.setLinearVelocity(angle, velocity);
}
 
void B2Action_Manipulator::onSetLinearVelocityPair(UI_SPRITE_EVT_B2_SET_LINEAR_VELOCITY_PAIR const & evt) {
    Entity & e = entity();
    B2ObjectExt & b2Object = e.component<B2ObjectExt>();

    UI_SPRITE_2D_PAIR velocity = { evt.velocity.x, evt.velocity.y };

    if ((velocity.x == 0 && velocity.y == 0) || evt.base_policy == 0) {
        b2Object.setLinearVelocity(velocity);
        return;
    }

    float angle = cpe_math_angle(0.0f, 0.0f, velocity.x, velocity.y);
    float distance = cpe_math_distance(0.0f, 0.0f, velocity.x, velocity.y);
    
    b2Object.setLinearVelocity(angle, distance);
}

void B2Action_Manipulator::onSetAwake(UI_SPRITE_EVT_B2_SET_AWAKE const & evt) {
    B2ObjectExt & b2Object = entity().component<B2ObjectExt>();
    b2Object.body()->SetAwake(evt.awake ? true : false);
}

double B2Action_Manipulator::prand(void) {
    if (m_rand_ctx == NULL) {
        m_rand_ctx = new cpe_prand_ctx;
        cpe_prand_init_gasdev(m_rand_ctx, rand());
    }

    return cpe_prand(m_rand_ctx);
}

void B2Action_Manipulator::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_Manipulator>(repo)
        .on_enter(&B2Action_Manipulator::enter)
        .on_exit(&B2Action_Manipulator::exit)
        ;
}

const char * B2Action_Manipulator::NAME = "b2-manipulator";

}}}

