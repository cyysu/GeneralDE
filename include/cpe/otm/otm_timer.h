#ifndef CPE_OTM_TIMER_H
#define CPE_OTM_TIMER_H
#include "otm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

otm_timer_t otm_timer_create(
    otm_manage_t mgr,
    otm_timer_id_t id,
    const char * name,
    otm_timer_id_t span,
    otm_process_fun_t process,
    void * process_ctx);

void otm_timer_free(otm_timer_t timer);

#ifdef __cplusplus
}
#endif

#endif
