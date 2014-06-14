#ifndef UI_SPRITE_WORLD_I_H
#define UI_SPRITE_WORLD_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/sorted_vector.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_repository_i.h"
#include "ui_sprite_event_i.h"
#include "ui_sprite_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_world {
    ui_sprite_repository_t m_repo;
    uint32_t m_max_entity_id;
    uint8_t m_in_tick;
    struct cpe_hash_table m_entity_by_id;
    struct cpe_hash_table m_entity_by_name;
    struct cpe_hash_table m_entity_protos;
    struct cpe_hash_table m_group_by_id;
    struct cpe_hash_table m_group_by_name;
    struct cpe_hash_table m_resources;
    struct cpe_hash_table m_event_handlers;
    struct cpe_hash_table m_attr_monitor_bindings;
    ui_sprite_entity_list_t m_wait_destory_entities;
    ui_sprite_pending_event_list_t m_pending_events;
    ui_sprite_attr_monitor_list_t m_pending_monitors;
    void * m_evt_processed_entities_buf;
    struct cpe_sorted_vector m_evt_processed_entities;
    tl_manage_t m_tl_mgr;
    cpe_timer_mgr_t m_timer_mgr;
    ui_sprite_world_updator_list_t m_updators;
    ui_sprite_component_list_t m_updating_components;
    tl_time_t m_last_update_time;
};

#ifdef __cplusplus
}
#endif

#endif
