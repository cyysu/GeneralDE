#include "gdpp/app/Log.hpp"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "B2Action_OnCollision.hpp"
#include "B2ObjectExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2WorldExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_OnCollision::B2Action_OnCollision(Fsm::Action & action)
    : ActionBase(action)
    , m_collision(action.name(), callOnCollision, this)
{
}

B2Action_OnCollision::B2Action_OnCollision(Fsm::Action & action, B2Action_OnCollision const & o)
    : ActionBase(action)
    , m_collision(action.name(), callOnCollision, this, o.m_collision)
    , m_on_collision_begin(o.m_on_collision_begin)
    , m_on_collision_end(o.m_on_collision_end)
{
}

int B2Action_OnCollision::enter(void) {
    entity().component<B2ObjectExt>().addCollision(m_collision);
    return 0;
}

void B2Action_OnCollision::exit(void) {
    entity().component<B2ObjectExt>().removeCollision(m_collision);
}

void B2Action_OnCollision::onCollision(UI_SPRITE_B2_COLLISION_DATA const & data) {
    const char * evt = NULL;

    if (data.collision_state == UI_SPRITE_B2_COLLISION_STATE_BEGIN) {
        if (!m_on_collision_begin.empty()) {
            evt = m_on_collision_begin.c_str();
        }
    }
    else {
        assert(data.collision_state == UI_SPRITE_B2_COLLISION_STATE_END);

        if (!m_on_collision_end.empty()) {
            evt = m_on_collision_end.c_str();
        }
    }

    if (evt) {
        struct dr_data_source data_source_buf[64];
        dr_data_source_t data_source = data_source_buf;

        data_source->m_data.m_meta = Cpe::Dr::metaOf(data);
        data_source->m_data.m_data = (void*)&data;
        data_source->m_data.m_size = sizeof(data);
        data_source->m_next = NULL;

        if (Entity * otherEntity = world().findEntity(data.collision_entity_id)) {
            data_source->m_next =
                ui_sprite_entity_build_data_source(*otherEntity, data_source_buf + 1, sizeof(data_source_buf) - 1);
        }

        buildAndSendEvent(evt, data_source_buf);
    }
}

void B2Action_OnCollision::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_OnCollision>(repo)
        .on_enter(&B2Action_OnCollision::enter)
        .on_exit(&B2Action_OnCollision::exit)
        ;
}

void B2Action_OnCollision::setOnCollisionBegin(::std::string const & on_begin) {
    m_on_collision_begin = on_begin;
}

void B2Action_OnCollision::setOnCollisionEnd(::std::string const & on_end) {
    m_on_collision_end = on_end;
}

void B2Action_OnCollision::callOnCollision(void * ctx, UI_SPRITE_B2_COLLISION_DATA const & data) {
    ((B2Action_OnCollision*)ctx)->onCollision(data);
}

const char * B2Action_OnCollision::NAME = "b2-on-collision";

}}}

