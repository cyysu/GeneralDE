#ifndef USF_LOGIC_USE_REQUIRE_QUEUE_H
#define USF_LOGIC_USE_REQUIRE_QUEUE_H
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

logic_require_queue_t
logic_require_queue_create(
    gd_app_context_t app,
    mem_allocrator_t alloc,
    error_monitor_t em,
    const char * name,
    logic_manage_t logic_manage);

void logic_require_queue_free(logic_require_queue_t require_queue);

int logic_require_queue_require_count(logic_require_queue_t queue);

int logic_require_queue_add_require_id(logic_require_queue_t queue, logic_require_id_t id);
int logic_require_queue_remove_require_id(logic_require_queue_t queue, logic_require_id_t id);
void logic_require_queue_notify_all(logic_require_queue_t queue, int32_t error);

logic_require_t logic_require_queue_notify(logic_require_queue_t queue, logic_require_id_t id, int32_t error);

#ifdef __cplusplus
}
#endif

#endif
