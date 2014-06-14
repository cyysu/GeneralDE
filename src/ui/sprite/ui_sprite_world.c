#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/timer/timer_manage.h"
#include "gd/app/app_context.h"
#include "ui_sprite_world_i.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_group_i.h"
#include "ui_sprite_world_res_i.h"
#include "ui_sprite_event_i.h"
#include "ui_sprite_attr_monitor_i.h"
#include "ui_sprite_event_meta_i.h"
#include "ui_sprite_updator_i.h"
#include "ui_sprite_component_i.h"
#include "ui_sprite_component_meta_i.h"

ui_sprite_world_t ui_sprite_world_create(ui_sprite_repository_t repo) {
    ui_sprite_world_t world;

    world = mem_alloc(repo->m_alloc, sizeof(struct ui_sprite_world));
    if (world == NULL) {
        CPE_ERROR(repo->m_em, "create world: alloc fail!");
        return NULL;
    }

    world->m_repo = repo;
    world->m_max_entity_id = 0;
    world->m_in_tick = 0;
    TAILQ_INIT(&world->m_wait_destory_entities);
    TAILQ_INIT(&world->m_pending_monitors);
    TAILQ_INIT(&world->m_pending_events);
    TAILQ_INIT(&world->m_updators);
    TAILQ_INIT(&world->m_updating_components);
    world->m_last_update_time = 0;
    world->m_evt_processed_entities_buf = NULL;

    world->m_tl_mgr = tl_manage_create(repo->m_alloc);
    if (world->m_tl_mgr == NULL) {
        CPE_ERROR(repo->m_em, "create world: create tl manager fail!");
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    world->m_timer_mgr = cpe_timer_mgr_create(world->m_tl_mgr, repo->m_alloc, repo->m_em);
    if (world->m_timer_mgr == NULL) {
        CPE_ERROR(repo->m_em, "create world: create timer mgr fail!");
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_entity_by_id,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_entity_id_hash,
            (cpe_hash_cmp_t) ui_sprite_entity_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_entity, m_hh_for_id),
            -1) != 0)
    {
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_entity_by_name,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_entity_name_hash,
            (cpe_hash_cmp_t) ui_sprite_entity_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_entity, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_entity_protos,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_entity_name_hash,
            (cpe_hash_cmp_t) ui_sprite_entity_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_entity, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_resources,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_world_res_hash,
            (cpe_hash_cmp_t) ui_sprite_world_res_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_world_res, m_hh_for_world),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_event_handlers,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_event_handler_hash,
            (cpe_hash_cmp_t) ui_sprite_event_handler_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_event_handler, m_hh_for_world),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_attr_monitor_bindings,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_attr_monitor_binding_hash,
            (cpe_hash_cmp_t) ui_sprite_attr_monitor_binding_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_attr_monitor_binding, m_hh_for_world),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_group_by_name,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_group_name_hash,
            (cpe_hash_cmp_t) ui_sprite_group_name_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_group, m_hh_for_name),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (cpe_hash_table_init(
            &world->m_group_by_id,
            repo->m_alloc,
            (cpe_hash_fun_t) ui_sprite_group_id_hash,
            (cpe_hash_cmp_t) ui_sprite_group_id_eq,
            CPE_HASH_OBJ2ENTRY(ui_sprite_group, m_hh_for_id),
            -1) != 0)
    {
        cpe_hash_table_fini(&world->m_group_by_name);
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    if (ui_sprite_world_start_tick(world) != 0) {
        cpe_hash_table_fini(&world->m_group_by_id);
        cpe_hash_table_fini(&world->m_group_by_name);
        cpe_hash_table_fini(&world->m_attr_monitor_bindings);
        cpe_hash_table_fini(&world->m_event_handlers);
        cpe_hash_table_fini(&world->m_resources);
        cpe_hash_table_fini(&world->m_entity_protos);
        cpe_hash_table_fini(&world->m_entity_by_id);
        cpe_hash_table_fini(&world->m_entity_by_name);
        cpe_timer_mgr_free(world->m_timer_mgr);
        tl_manage_free(world->m_tl_mgr);
        mem_free(repo->m_alloc, world);
        return NULL;
    }

    return world;
}

static void ui_sprite_world_clear_event_handler(ui_sprite_world_t world) {
    struct cpe_hash_it event_handler_it;
    ui_sprite_event_handler_t event_handler;

    cpe_hash_it_init(&event_handler_it, &world->m_event_handlers);

    event_handler = cpe_hash_it_next(&event_handler_it);
    while (event_handler) {
        ui_sprite_event_handler_t next = cpe_hash_it_next(&event_handler_it);

        ui_sprite_event_handler_free(world, event_handler);

        event_handler = next;
    }
}

static void ui_sprite_world_clear_attr_monitor(ui_sprite_world_t world) {
    while(cpe_hash_table_count(&world->m_attr_monitor_bindings) > 0) {
        struct cpe_hash_it attr_monitor_binding_it;
        ui_sprite_attr_monitor_binding_t attr_monitor_binding;

        cpe_hash_it_init(&attr_monitor_binding_it, &world->m_attr_monitor_bindings);

        attr_monitor_binding = cpe_hash_it_next(&attr_monitor_binding_it);
        assert(attr_monitor_binding);

        ui_sprite_attr_monitor_free(world, attr_monitor_binding->m_monitor);
    }
}

gd_app_context_t ui_sprite_world_app(ui_sprite_world_t world) {
    return world->m_repo->m_app;
}

void ui_sprite_world_free(ui_sprite_world_t world) {
    ui_sprite_repository_t repo = world->m_repo;

    ui_sprite_world_stop_tick(world);

    ui_sprite_entity_free_all(world);
    ui_sprite_entity_proto_free_all(world);
    ui_sprite_group_free_all(world);
    ui_sprite_world_res_free_all(world);
    ui_sprite_world_clear_event_handler(world);
    ui_sprite_world_clear_attr_monitor(world);
    ui_sprite_world_update_free_all(world);

    assert(TAILQ_EMPTY(&world->m_pending_monitors));

    while(!TAILQ_EMPTY(&world->m_pending_events)) {
        ui_sprite_pending_event_free(world, TAILQ_FIRST(&world->m_pending_events));
    }

    assert(TAILQ_EMPTY(&world->m_wait_destory_entities));

    assert(TAILQ_EMPTY(&world->m_pending_events));
    assert(TAILQ_EMPTY(&world->m_updating_components));

    cpe_hash_table_fini(&world->m_entity_by_id);
    cpe_hash_table_fini(&world->m_entity_by_name);
    cpe_hash_table_fini(&world->m_entity_protos);
    cpe_hash_table_fini(&world->m_resources);
    cpe_hash_table_fini(&world->m_event_handlers);
    cpe_hash_table_fini(&world->m_group_by_id);
    cpe_hash_table_fini(&world->m_group_by_name);

    cpe_timer_mgr_free(world->m_timer_mgr);
    tl_manage_free(world->m_tl_mgr);

    if (world->m_evt_processed_entities_buf) {
        mem_free(repo->m_alloc, world->m_evt_processed_entities_buf);
    }

    mem_free(repo->m_alloc, world);
}

ui_sprite_repository_t ui_sprite_world_repository(ui_sprite_world_t world) {
    return world->m_repo;
}

tl_manage_t ui_sprite_world_tl_mgr(ui_sprite_world_t world) {
    return world->m_tl_mgr;
}

tl_time_t ui_sprite_world_time(ui_sprite_world_t world) {
    return tl_manage_time(world->m_tl_mgr);
}

cpe_timer_mgr_t ui_sprite_world_timer_mgr(ui_sprite_world_t world) {
    return world->m_timer_mgr;
}

void ui_sprite_world_send_event(ui_sprite_world_t world, LPDRMETA meta, void const * data, size_t size) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_pending_event_t processing_evt =
        ui_sprite_event_enqueue(
            world, NULL, 
            ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);
    if (processing_evt == NULL) {
        CPE_ERROR(repo->m_em, "world send event: event enqueue fail!");
    }

    processing_evt->m_target_count = 1;
    processing_evt->m_targets[0].m_type = ui_sprite_event_target_type_world;
}

void ui_sprite_world_send_event_to(ui_sprite_world_t world, const char * input_targets, LPDRMETA meta, void const * data, size_t size) {
    ui_sprite_repository_t repo = world->m_repo;
    ui_sprite_pending_event_t processing_evt;
    char * targets;

    processing_evt = ui_sprite_event_enqueue(
        world, NULL,
        ui_sprite_repository_event_debug_level(repo, meta), meta, data, size);
    if (processing_evt == NULL) {
        CPE_ERROR(repo->m_em, "world send event: event enqueue fail!");
		return;
    }

    targets = cpe_str_mem_dup(repo->m_alloc, input_targets);

    if (ui_sprite_event_analize_targets(processing_evt, world, NULL,  targets) != 0) {
        ui_sprite_pending_event_free(world, processing_evt);
    }

    mem_free(repo->m_alloc, targets);
}

ui_sprite_event_handler_t
ui_sprite_world_add_event_handler(
    ui_sprite_world_t world, const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx)
{
    ui_sprite_repository_t repo = world->m_repo;

    ui_sprite_event_handler_t handler =
        ui_sprite_event_handler_create(world, NULL, event_name, 0, fun, ctx);
    if (handler == NULL) {
        CPE_ERROR(repo->m_em, "world add handler of event %s fail!", event_name);
        return NULL;
    }

    return handler;
}

void ui_sprite_world_clear_event_handler_by_ctx(ui_sprite_world_t world, void * ctx) {
    struct cpe_hash_it event_handler_it;
    ui_sprite_event_handler_t event_handler;

    cpe_hash_it_init(&event_handler_it, &world->m_event_handlers);

    event_handler = cpe_hash_it_next(&event_handler_it);
    while (event_handler) {
        ui_sprite_event_handler_t next = cpe_hash_it_next(&event_handler_it);

        if (event_handler->m_ctx == ctx) {
            ui_sprite_event_handler_free(world, event_handler);
        }

        event_handler = next;
    }
}

int ui_sprite_world_add_updator(ui_sprite_world_t world, ui_sprite_world_update_fun_t fun, void * ctx) {
    ui_sprite_world_updator_t updator = ui_sprite_world_update_create(world, fun, ctx);
    return updator ? 0 : -1;
}

void ui_sprite_world_remove_updator(ui_sprite_world_t world, void * ctx) {
    ui_sprite_world_updator_t updator;

    for(updator = TAILQ_FIRST(&world->m_updators);
        updator != TAILQ_END(&world->m_updators);
        )
    {
        ui_sprite_world_updator_t next = TAILQ_NEXT(updator, m_next);

        if (updator->m_ctx == ctx) {
            ui_sprite_world_update_free(world, updator);
        }

        updator = next;
    }
}

static ptr_int_t ui_sprite_world_tick(void * ctx, ptr_int_t arg) {
    ui_sprite_world_updator_t updator;
    ui_sprite_component_t component;
    ui_sprite_world_t world = ctx;
    tl_time_t new_time;
    tl_time_span_t delta;
    float delta_s;
    ptr_int_t processed_count;

    processed_count = tl_manage_tick(world->m_tl_mgr, 100);

    new_time = tl_manage_time(world->m_tl_mgr);

    delta =
        world->m_last_update_time <= 0
        ? 0
        : (world->m_last_update_time > new_time
           ? 0
           : new_time - world->m_last_update_time);

    world->m_last_update_time = new_time;

    delta_s = ((float)delta) / 1000.0f;

    /*首先更新环境 */
    for(updator = TAILQ_FIRST(&world->m_updators);
        updator != TAILQ_END(&world->m_updators);
        )
    {
        ui_sprite_world_updator_t next = TAILQ_NEXT(updator, m_next);

        updator->m_fun(world, updator->m_ctx, delta_s);
        ++processed_count;

        updator = next;
    }
    
    /*然后更新环境中的对象 */
    for(component = TAILQ_FIRST(&world->m_updating_components);
        component != TAILQ_END(&world->m_updating_components);
        )
    {
        ui_sprite_component_t next = TAILQ_NEXT(component, m_next_for_updating);

        assert(component->m_meta->m_update_fun);
        component->m_meta->m_update_fun(component, component->m_meta->m_update_fun_ctx, delta_s);
        
        component = next;
    }

    /*统一执行所有事件 */
    ui_sprite_event_dispatch(world);

    return processed_count;
}

uint8_t ui_sprite_world_is_tick_start(ui_sprite_world_t world) {
    return world->m_in_tick;
}

int ui_sprite_world_start_tick(ui_sprite_world_t world) {
    int r;

    if (world->m_in_tick) return 0;

    r = gd_app_tick_add(world->m_repo->m_app, ui_sprite_world_tick, world, 0);

    if (r == 0) world->m_in_tick = 1;

    return r;
}

void ui_sprite_world_stop_tick(ui_sprite_world_t world) {
    if (!world->m_in_tick) return;

    if (gd_app_tick_remove(world->m_repo->m_app, ui_sprite_world_tick, world) == 0) {
        world->m_in_tick = 0;
    }
}
