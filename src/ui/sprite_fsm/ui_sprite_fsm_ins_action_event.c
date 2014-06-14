#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/timer/timer_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_ins_event_binding_i.h"
#include "ui_sprite_fsm_ins_attr_binding_i.h"
#include "ui_sprite_fsm_action_meta_i.h"

struct ui_sprite_fsm_action_addition_source_ctx {
    UI_SPRITE_FSM_STATE_ENTER_EVENT m_enter_event;
    dr_data_source_t * m_last_source;
    struct dr_data_source m_append_sources[64];
};

static void ui_sprite_fsm_action_append_addition_source(
    ui_sprite_fsm_action_t fsm_action, 
    dr_data_source_t * data_source,
    struct ui_sprite_fsm_action_addition_source_ctx * ctx)
{
    ui_sprite_fsm_state_t fsm_state = fsm_action->m_state;
    ui_sprite_fsm_module_t module = fsm_action->m_state->m_ins->m_module;
    uint8_t append_source_pos = 0;
    dr_data_source_t * insert_pos;

    ctx->m_last_source = data_source;
    while(*ctx->m_last_source) {
        ctx->m_last_source = &(*ctx->m_last_source)->m_next;
    }
    insert_pos = ctx->m_last_source;

    /*数据只有当前action会有 */
    if (fsm_action->m_meta->m_data_meta) {
        /*append action data*/
        ctx->m_append_sources[append_source_pos].m_data.m_meta = fsm_action->m_meta->m_data_meta;
        ctx->m_append_sources[append_source_pos].m_data.m_data = ((char *)ui_sprite_fsm_action_data(fsm_action)) + fsm_action->m_meta->m_data_start;
        ctx->m_append_sources[append_source_pos].m_data.m_size = fsm_action->m_meta->m_data_size;
        ctx->m_append_sources[append_source_pos].m_next = NULL;

        *insert_pos = &ctx->m_append_sources[append_source_pos++];
        insert_pos = &((*insert_pos)->m_next);
    }
        
    /*处理所有action路径上的事件 */
    while(fsm_state) {
        if (append_source_pos + 2 >= CPE_ARRAY_SIZE(ctx->m_append_sources)) break;

        if (fsm_state->m_enter_event) {
            /*append enter_evt common*/
            ctx->m_enter_event.enter_evt_from = fsm_state->m_enter_event->from_entity_id;
            strncpy(ctx->m_enter_event.enter_evt_name, dr_meta_name(fsm_state->m_enter_event->meta), sizeof(ctx->m_enter_event.enter_evt_name));

            ctx->m_append_sources[append_source_pos].m_data.m_meta = module->m_meta_action_enter_event;
            ctx->m_append_sources[append_source_pos].m_data.m_data = &ctx->m_enter_event;
            ctx->m_append_sources[append_source_pos].m_data.m_size = sizeof(ctx->m_enter_event);
            ctx->m_append_sources[append_source_pos].m_next = NULL;

            *insert_pos = &ctx->m_append_sources[append_source_pos++];
            insert_pos = &((*insert_pos)->m_next);

            /*append enter_evt data*/
            ctx->m_append_sources[append_source_pos].m_data.m_meta = fsm_state->m_enter_event->meta;
            ctx->m_append_sources[append_source_pos].m_data.m_data = (void*)fsm_state->m_enter_event->data;
            ctx->m_append_sources[append_source_pos].m_data.m_size = fsm_state->m_enter_event->size;
            ctx->m_append_sources[append_source_pos].m_next = NULL;

            *insert_pos = &ctx->m_append_sources[append_source_pos++];
            insert_pos = &((*insert_pos)->m_next);
        }

        if (fsm_state->m_ins->m_parent) {
            fsm_state = fsm_state->m_ins->m_parent->m_cur_state;
        }
        else {
            break;
        }
    }
}

void ui_sprite_fsm_action_build_and_send_event(
    ui_sprite_fsm_action_t fsm_action,
    const char * event_def, dr_data_source_t data_source)
{
    struct ui_sprite_fsm_action_addition_source_ctx ctx;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);
    
    ui_sprite_component_build_and_send_event(
        ui_sprite_fsm_action_to_component(fsm_action), event_def, data_source);

    *ctx.m_last_source = NULL;
}

ui_sprite_event_t ui_sprite_fsm_action_build_event(
    ui_sprite_fsm_action_t op_action, mem_allocrator_t alloc, const char * def, dr_data_source_t data_source)
{
    struct ui_sprite_fsm_action_addition_source_ctx ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(op_action);
    ui_sprite_event_t r;
    
    ui_sprite_fsm_action_append_addition_source(op_action, &data_source, &ctx);

    r = ui_sprite_entity_build_event(entity, alloc, def, data_source);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_bulk_set_attrs(
    ui_sprite_fsm_action_t fsm_action, const char * def, dr_data_source_t data_source)
{
    struct ui_sprite_fsm_action_addition_source_ctx ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);

    r = ui_sprite_entity_bulk_set_attrs(entity, def, data_source);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_set_attr(
    ui_sprite_fsm_action_t fsm_action,
    const char * path, const char * value, dr_data_source_t data_source)
{
    struct ui_sprite_fsm_action_addition_source_ctx ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    int r;

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &ctx);

    r = ui_sprite_entity_set_attr(entity, path, value, data_source);

    *ctx.m_last_source = NULL;

    return r;
}

int ui_sprite_fsm_action_add_event_handler(
    ui_sprite_fsm_action_t fsm_action, ui_sprite_event_scope_t scope,
    const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_fsm_action_event_binding_t binding;
    ui_sprite_event_handler_t handler;
    ui_sprite_component_t component = ui_sprite_fsm_action_to_component(fsm_action);

    handler = ui_sprite_component_add_event_handler(component, scope, event_name, fun, ctx);
    if (handler == NULL) return -1;

    binding = ui_sprite_fsm_action_event_binding_create(fsm_action, handler);
    if (binding == NULL) {
        ui_sprite_event_handler_free(ui_sprite_fsm_action_to_world(fsm_action), handler);
        return -1;
    }

    return 0;
}

int ui_sprite_fsm_action_add_attr_monitor(
    ui_sprite_fsm_action_t fsm_action,
    const char * attrs, ui_sprite_attr_monitor_fun_t fun, void * ctx)
{
    ui_sprite_fsm_action_attr_binding_t binding;
    ui_sprite_attr_monitor_t handler;
    ui_sprite_component_t component = ui_sprite_fsm_action_to_component(fsm_action);

    handler = ui_sprite_component_add_attr_monitor(component, attrs, fun, ctx);
    if (handler == NULL) return -1;

    binding = ui_sprite_fsm_action_attr_binding_create(fsm_action, handler);
    if (binding == NULL) {
        ui_sprite_attr_monitor_free(ui_sprite_fsm_action_to_world(fsm_action), handler);
        return -1;
    }

    return 0;
}
