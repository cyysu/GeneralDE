#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

static void mongo_server_fsm_disable_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct mongo_server * server = fsm_machine_context(fsm);
    mongo_driver_t driver = server->m_driver;

    mongo_server_disconnect(server);

    mongo_driver_check_update_state(driver);
}


static uint32_t mongo_server_fsm_disable_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct mongo_server_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case mongo_server_fsm_evt_start:
        return mongo_server_state_connecting;
    case mongo_server_fsm_evt_stop:
        return FSM_KEEP_STATE;
    default:
        return FSM_INVALID_STATE;
    }
}

int mongo_server_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disable", mongo_server_state_disable);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disable: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, mongo_server_fsm_disable_enter, NULL);

    if (fsm_def_state_add_transition(s, mongo_server_fsm_disable_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disable: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

