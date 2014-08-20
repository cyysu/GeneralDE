#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "Box2D/Common/b2Math.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/sprite/ComponentGen.hpp"
#include "uipp/sprite_2d/Transform.hpp"
#include "B2Action_RunTo.hpp"
#include "B2ObjectExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_RunTo::B2Action_RunTo(Fsm::Action & action)
    : ActionBase(action)
    , m_group_masks(0)
{
    bzero(&m_state_data, sizeof(m_state_data));
}

B2Action_RunTo::B2Action_RunTo(Fsm::Action & action, B2Action_RunTo const & o)
    : ActionBase(action)
    , m_group_masks(o.m_group_masks)
{
    bzero(&m_state_data, sizeof(m_state_data));
}

int B2Action_RunTo::enter(void) {
    addEventHandler(&B2Action_RunTo::onRunToEntity);
    addEventHandler(&B2Action_RunTo::onFollowEntity);
    return 0;
}

void B2Action_RunTo::exit(void) {
    bzero(&m_state_data, sizeof(m_state_data));
}

void B2Action_RunTo::update(float delta) {
    Entity & e = entity();
    B2ObjectExt & b2 = e.component<B2ObjectExt>();

    assert(m_state_data.max_speed > 0.0f);

    P2D::Pair curentPos = b2.pos();
    updateMovingData(curentPos, delta);
    if (m_state_data.not_move_duration > 1.0f) {
        if (stopOnArrive()) {
            if (e.debug()) {
                APP_CTX_INFO(
                    app(), "entity %d(%s): b2-run-to: %fs not move, stop!",
                    e.id(), e.name(), m_state_data.not_move_duration);
            }

            stopUpdate();
        }

        return;
    }
    
    /*不在地面，不做任何处理 */
    if (!isOnGround()) return;

    //printf("entity %d(%s): bbbb start\n", e.id(), e.name());

    enqueueLastPos(curentPos);

    /*已经运行到最远距离，不用再运动了*/
    if (m_state_data.max_distance && m_state_data.moved_distance >= m_state_data.max_distance) {
        if (e.debug()) {
            APP_CTX_INFO(
                app(), "entity %d(%s): b2-run-to: max_distance %f reached, moved_distance=%f, stop!",
                e.id(), e.name(), m_state_data.max_distance, m_state_data.moved_distance);
        }

        stopUpdate();
        return;
    }

    P2D::Pair targetPos = this->targetPos();
    float to_target_distance = cpe_math_distance(targetPos.x, targetPos.y, curentPos.x, curentPos.y);
    float to_stop_distance = to_target_distance - m_state_data.near_distance;

    /*计算当前运动的速度 */
    P2D::Pair v_pair = b2.lineerVelocity();
    float v = cpe_math_distance(0.0f, 0.0f, v_pair.x, v_pair.y);
    float v_angle = v > 1.0f ? cpe_math_angle(0.0f, 0.0f, v_pair.x, v_pair.y) : 0;

    //printf("curentPos=(%f,%f), targetPos=(%f,%f)\n", curentPos.x, curentPos.y, targetPos.x, targetPos.y);
    //printf("to_target_distance=%f, to_stop_distance=%f\n", to_target_distance, to_stop_distance);
    //printf("v_pair=(%f,%f), v=%f, v_angle=%f\n", v_pair.x, v_pair.y, v, v_angle);

    /*计算加速（或者减速）方向 */
    float force_angle;
    if (m_state_data.trace_pos_count >= 2) {
        force_angle = movingAngleFromPos();
    }
    else {
        if(v < 1.0f) { /*从静止开始处理，当前速度不可用，单独拿出来处理 */
            if (to_target_distance < 0.5f) {
                force_angle = 0; /*静止并且和对象中心点重合此时选择任何一个方向运动都是可以的,默认选择向左运动 */
            }
            else if (to_stop_distance > 0.0f) { /*还没有到达 */
                force_angle = curentPos.x < targetPos.x ? 0 : -180.0f;
            }
            else {
                assert(to_stop_distance < 0.0f);
                force_angle = curentPos.x > targetPos.x ? 0 : -180.0f;
            }
        }
        else {
            force_angle = v_pair.x > 0.0f ? 0 : -180.0f ;
        }
    }
    //printf("force_angle = %f\n", force_angle);

    if (v <= 1.0f) { /*原来是静止的 */
        /*下一次开始减速后，停止的点 */
        float next_v = incSpeed(0, delta);
        float next_stop_distance = next_v * delta + stopDistance(next_v);
        P2D::Pair nextStopPos = curentPos;
        nextStopPos.x += next_stop_distance * cos(force_angle);
        nextStopPos.y += next_stop_distance * sin(force_angle);

        if (checkIsNextPosGood(curentPos, nextStopPos, targetPos)) {
            //printf("git init speed: v=%f, v_angle=%f\n", next_v, force_angle);
            b2.setLinearVelocity(force_angle, next_v);
        }
        else {
            if (stopOnArrive()) {
                if (e.debug()) {
                    APP_CTX_INFO(app(), "entity %d(%s): b2-run-to: in best pos (v=0), stop!", e.id(), e.name());
                }
                stopUpdate();
                return;
            }
        }
    }
    else {
         /*原来是运动的，将运来的速度分解到施力方向和非施力方向，然后只处理施力方向，只处理施力方向，最后再合成回来 */
        float force_angle_to_v = cpe_math_angle_diff(v_angle, force_angle);
        float v_in_force = v * cos(force_angle_to_v);
        float v_not_in_force = v * sin(force_angle_to_v);
        float cur_stop_distance = stopDistance(v_in_force);

        //printf("force_angle_to_v=%f, v_in_force=%f, v_not_in_force=%f\n", force_angle_to_v, v_in_force, v_not_in_force);

        //printf("max_distance=%f, moved_distance=%f, stopDistance=%f\n",
        //     m_state_data.max_distance, m_state_data.moved_distance, cur_stop_distance);

        if (m_state_data.max_distance && m_state_data.moved_distance + cur_stop_distance > m_state_data.max_distance) {
            updateSpeed(decSpeed(v_in_force, delta), v_not_in_force, force_angle);
            return;
        }

        /*从当前开始减速后，停止的点 */
        P2D::Pair curentStopPos = curentPos;
        curentStopPos.x += cur_stop_distance * cos(force_angle);
        curentStopPos.y += cur_stop_distance * sin(force_angle);

        /*下一次开始减速后，停止的点 */
        float next_v_in_force = incSpeed(v_in_force, delta);
        float next_stop_distance = next_v_in_force * delta + stopDistance(next_v_in_force);
        P2D::Pair nextStopPos = curentPos;
        nextStopPos.x += next_stop_distance * cos(force_angle);
        nextStopPos.y += next_stop_distance * sin(force_angle);

        if (!checkIsNextPosGood(curentStopPos, nextStopPos, targetPos)) {
            v_in_force = decSpeed(v_in_force, delta);
            updateSpeed(v_in_force, v_not_in_force, force_angle);
            if (stopOnArrive() && v_in_force < 1.0f) {
                if (e.debug()) {
                    APP_CTX_INFO(app(), "entity %d(%s): b2-run-to: in best pos (next pos is not good), stop!", e.id(), e.name());
                }
                stopUpdate();
            }
        }
        else {
            updateSpeed(incSpeed(v_in_force, delta), v_not_in_force, force_angle);
        }
    }

    return;
}

void B2Action_RunTo::updateSpeed(float v_in_force, float v_not_in_force, float force_angle) {
    B2ObjectExt & b2 = entity().component<B2ObjectExt>();

    float v = cpe_math_distance(0.0f, 0.0f, v_in_force, v_not_in_force);
    if (v > 1.0f ) {
        b2.setLinearVelocity(
            cpe_math_angle_add(cpe_math_angle(0.0f, 0.0f, v_in_force, v_not_in_force), force_angle),
            v);
    }
    else {
        b2.setLinearVelocity(V_ZERO);
    }
}

bool B2Action_RunTo::checkIsNextPosGood(
    P2D::Pair const & curPos, P2D::Pair const & nextPos, P2D::Pair const & targetPos) const
{
    float cur_to_target_distance = cpe_math_distance(targetPos.x, targetPos.y, curPos.x, curPos.y);
    float cur_to_stop_distance = cur_to_target_distance - m_state_data.near_distance;

    float next_to_target_distance = cpe_math_distance(targetPos.x, targetPos.y, nextPos.x, nextPos.y);
    float next_to_stop_distance = next_to_target_distance - m_state_data.near_distance;

    return fabs(next_to_stop_distance) < fabs(cur_to_stop_distance);
}

float B2Action_RunTo::decSpeed(float base_speed, float delta) const {
    if (m_state_data.dec_delta_speed > 0) {
        base_speed -= m_state_data.dec_delta_speed * delta;
        if (base_speed < 0.0f) base_speed = 0.0f;
    }
    else {
        base_speed = 0.0f;
    }

    return base_speed;
}

float B2Action_RunTo::incSpeed(float base_speed, float delta) const {
    if (m_state_data.inc_delta_speed > 0) {
        base_speed += m_state_data.inc_delta_speed * delta;
        if (base_speed > m_state_data.max_speed) base_speed = m_state_data.max_speed;
    }
    else {
        base_speed = m_state_data.max_speed;
    }

    return base_speed;
}

float B2Action_RunTo::stopDistance(float curent_speed) const {
    if (m_state_data.dec_delta_speed <= 0.1) return 0;

    float stop_time = curent_speed / m_state_data.dec_delta_speed;

    return m_state_data.dec_delta_speed * stop_time * stop_time;
}

void B2Action_RunTo::updateMovingData(P2D::Pair const & pos, float delta) {
    if (pos.x == m_state_data.pre_pos.x && pos.y == m_state_data.pre_pos.y) {
        m_state_data.not_move_duration += delta;
    }
    else {
        m_state_data.pre_pos.x = pos.x;
        m_state_data.pre_pos.y = pos.y;
    }
}

void B2Action_RunTo::enqueueLastPos(P2D::Pair const & pos) {
    if (m_state_data.trace_pos_count == 0) {
        UI_SPRITE_B2_PAIR & new_pos = m_state_data.trace_poses[m_state_data.trace_pos_count++];
        new_pos.x = pos.x;
        new_pos.y = pos.y;
        return;
    }

    UI_SPRITE_B2_PAIR & pre_pos = m_state_data.trace_poses[m_state_data.trace_pos_count - 1];
    if (fabs(pos.x - pre_pos.x) < 1.0f && fabs(pos.y - pre_pos.y) < 1.0f) return;

    m_state_data.moved_distance += cpe_math_distance(pre_pos.x, pre_pos.y, pos.x, pos.y);

    if (m_state_data.trace_pos_count + 1 < CPE_ARRAY_SIZE(m_state_data.trace_poses)) {
        memmove(
            m_state_data.trace_poses,
            m_state_data.trace_poses + 1,
            sizeof(m_state_data.trace_poses[0]) * (m_state_data.trace_pos_count - 1));
        m_state_data.trace_pos_count--;
    }

    UI_SPRITE_B2_PAIR & new_pos = m_state_data.trace_poses[m_state_data.trace_pos_count++];
    new_pos.x = pos.x;
    new_pos.y = pos.y;
    return;
}

float B2Action_RunTo::movingAngleFromPos(void) const {
    assert(m_state_data.trace_pos_count >= 2);

    UI_SPRITE_B2_PAIR const & pos_0 = m_state_data.trace_poses[m_state_data.trace_pos_count - 2];
    UI_SPRITE_B2_PAIR const & pos_1 = m_state_data.trace_poses[m_state_data.trace_pos_count - 1];

    return cpe_math_angle(pos_0.x, pos_0.y, pos_1.x, pos_1.y);
}

P2D::Pair B2Action_RunTo::entityPos(uint32_t entity_id, const char * entity_name) const {
    if (entity_id > 0) {
        return world().entity(entity_id).component<P2D::Transform>().originPos();
    }
    else if (entity_name[0]) {
        return world().entity(entity_name).component<P2D::Transform>().originPos();
    }
    else {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "posOfEntity: no entity info!");
    }
}

P2D::Pair B2Action_RunTo::targetPos(void) const {
    if (m_state_data.target_policy == UI_SPRITE_B2_RUN_TARGET_POLICY_FOLLOW_ENTITY) {
        return entityPos(m_state_data.target.follow_entity.entity_id, m_state_data.target.follow_entity.entity_name);
    }
    else if (m_state_data.target_policy == UI_SPRITE_B2_RUN_TARGET_POLICY_TO_POS) {
        P2D::Pair pt = { m_state_data.target.to_pos.target_pos.x, m_state_data.target.to_pos.target_pos.y };
        return pt;
    }
    else {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "posOfEpos: no entity info!");
    }
}

void B2Action_RunTo::onRunToEntity(UI_SPRITE_EVT_B2_RUN_TO_ENTITY const & evt) {
    Entity & e = entity();

    if (evt.max_speed == 0.0f) {
        APP_CTX_ERROR(app(), "entity %d(%s): onRunToEntity: no max speed!\n", e.id(), e.name());
        return;
    }

    UI_SPRITE_B2_RUN_DATA new_state_data;
    new_state_data.target_policy = UI_SPRITE_B2_RUN_TARGET_POLICY_TO_POS;

    P2D::Pair targetPos = entityPos(evt.entity_id, evt.entity_name);
    new_state_data.target.to_pos.target_pos.x = targetPos.x;
    new_state_data.target.to_pos.target_pos.y = targetPos.y;

    new_state_data.max_speed = evt.max_speed;
    new_state_data.max_distance = evt.max_distance;
    new_state_data.near_distance = evt.near_distance;
    new_state_data.inc_delta_speed = evt.inc_delta_speed;
    new_state_data.dec_delta_speed = evt.dec_delta_speed;
    new_state_data.trace_pos_count = 0;
    new_state_data.moved_distance = 0.0f;
    new_state_data.not_move_duration = 0.0f;

    P2D::Pair curPos = e.component<P2D::Transform>().originPos();
    new_state_data.pre_pos.x = curPos.x;
    new_state_data.pre_pos.y = curPos.y;

    m_state_data = new_state_data;

    if (isOnGround()) enqueueLastPos(curPos);

    syncUpdate(true);
}

void B2Action_RunTo::onFollowEntity(UI_SPRITE_EVT_B2_RUN_FOLLOW_ENTITY const & evt) {
    Entity & e = entity();

    if (evt.max_speed == 0.0f) {
        APP_CTX_ERROR(app(), "entity %d(%s): onFollowEntity: no max speed!\n", e.id(), e.name());
        return;
    }

    UI_SPRITE_B2_RUN_DATA new_state_data;
    new_state_data.target_policy = UI_SPRITE_B2_RUN_TARGET_POLICY_FOLLOW_ENTITY;
    new_state_data.target.follow_entity.entity_id = evt.entity_id;
    strncpy(
        new_state_data.target.follow_entity.entity_name, evt.entity_name,
        sizeof(new_state_data.target.follow_entity.entity_name));

    new_state_data.max_speed = evt.max_speed;
    new_state_data.max_distance = evt.max_distance;
    new_state_data.near_distance = evt.near_distance;
    new_state_data.inc_delta_speed = evt.inc_delta_speed;
    new_state_data.dec_delta_speed = evt.dec_delta_speed;
    new_state_data.trace_pos_count = 0;
    new_state_data.moved_distance = 0.0f;
    new_state_data.not_move_duration = 0.0f;

    P2D::Pair curPos = e.component<P2D::Transform>().originPos();
    new_state_data.pre_pos.x = curPos.x;
    new_state_data.pre_pos.y = curPos.y;

    m_state_data = new_state_data;

    if (isOnGround()) enqueueLastPos(curPos);

    syncUpdate(true);
}

bool B2Action_RunTo::isOnGround(void) const {
    return entity().component<B2ObjectExt>()
        .isCollisionWith(
            world().res<B2WorldExt>().groundMasks() | m_group_masks);
}

void B2Action_RunTo::setGroundMasks(uint16_t ground_masks) {
    m_group_masks = ground_masks;
}

void B2Action_RunTo::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_RunTo>(repo)
        .on_enter(&B2Action_RunTo::enter)
        .on_exit(&B2Action_RunTo::exit)
        .on_update(&B2Action_RunTo::update)
        ;
}

const char * B2Action_RunTo::NAME = "b2-run-to";

}}}

