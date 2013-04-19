#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/utils/stream.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

extern char g_metalib_svr_center_pro[];
static fsm_def_machine_t
center_agent_center_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

int center_agent_center_init(center_agent_t agent, struct center_agent_center * center) {
    center->m_agent = agent;
    center->m_cvt = NULL;
    center->m_connector = NULL;
    center->m_read_chanel_size = 4 * 1024;
    center->m_write_chanel_size = 1024;
    center->m_process_count_per_tick = 10;
    center->m_max_pkg_size = 1024 * 1024 * 5;

    center->m_fsm_timer_id = GD_TIMER_ID_INVALID;

    center->m_pkg_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_pkg");
    if (center->m_pkg_meta == NULL) {
        CPE_ERROR(agent->m_em, "%s: create find pkg meta fail!", center_agent_name(agent));
        return -1;
    }

    center->m_fsm_def = center_agent_center_create_fsm_def(center_agent_name(agent), agent->m_alloc, agent->m_em);
    if (center->m_fsm_def == NULL) {
        CPE_ERROR(agent->m_em, "%s: create fsm def fail!", center_agent_name(agent));
        return -1;
    }

    mem_buffer_init(&center->m_incoming_pkg_buf, agent->m_alloc);
    mem_buffer_init(&center->m_outgoing_encode_buf, agent->m_alloc);

    if (fsm_machine_init(&center->m_fsm, center->m_fsm_def, "disconnected", center, 1) != 0) {
        CPE_ERROR(agent->m_em, "%s: init fsm fail!", center_agent_name(agent));
        fsm_def_machine_free(center->m_fsm_def);
        return -1;
    }

    return 0;
}

void center_agent_center_clear(struct center_agent_center * center) {
    fsm_machine_fini(&center->m_fsm);
    assert(center->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    fsm_def_machine_free(center->m_fsm_def);
    center->m_fsm_def = NULL;

    mem_buffer_clear(&center->m_incoming_pkg_buf);
    mem_buffer_clear(&center->m_outgoing_encode_buf);

    if (center->m_cvt) {
        dr_cvt_free(center->m_cvt);
        center->m_cvt = NULL;
    }

    if (center->m_connector) {
        net_connector_free(center->m_connector);
        center->m_connector = NULL;
    }
}

int center_agent_center_set_cvt(struct center_agent_center * center, const char * cvt_name) {
    if (center->m_cvt) dr_cvt_free(center->m_cvt);

    center->m_cvt = dr_cvt_create(center->m_agent->m_app, cvt_name);
    if (center->m_cvt == NULL) {
        CPE_ERROR(center->m_agent->m_em, "%s: set cvt %s fail!", center_agent_name(center->m_agent), cvt_name);
        return -1;
    } 

    return 0;
}

int center_agent_center_set_svr(struct center_agent_center * center, const char * ip, short port) {
    if (center->m_connector) {
        net_connector_free(center->m_connector);
        center->m_connector = NULL;
    }
    
    center->m_connector = 
        net_connector_create_with_ep(gd_app_net_mgr(center->m_agent->m_app), center_agent_name(center->m_agent), ip, port);
    if (center->m_connector == NULL) {
        CPE_ERROR(
            center->m_agent->m_em, "%s: set svr to %s:%d: create connector fail!",
            center_agent_name(center->m_agent), ip, (int)port);
        return -1;
    }
 
    if (net_connector_add_monitor(
            center->m_connector,
            center_agent_center_connector_state_monitor, center) != 0)
    {
        net_connector_free(center->m_connector);
        center->m_connector = NULL;
        CPE_ERROR(
            center->m_agent->m_em, "%s: set svr to %s:%d: add connector monitor fail!",
            center_agent_name(center->m_agent), ip, (int)port);
        return -1;
    }

    if (center_agent_center_ep_init(center, net_connector_ep(center->m_connector)) != 0) {
        net_connector_free(center->m_connector);
        center->m_connector = NULL;
        CPE_ERROR(
            center->m_agent->m_em, "%s: set svr to %s:%d: init ep fail!",
            center_agent_name(center->m_agent), ip, (int)port);
        return -1;
    }

    return 0;
}

int center_agent_center_set_reconnect_span_ms(struct center_agent_center * center, uint32_t span_ms) {
    if (center->m_connector == NULL) return -1;
    net_connector_set_reconnect_span_ms(center->m_connector, span_ms);
    return 0;
}

static void center_agent_center_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct center_agent_fsm_evt * evt = input_event;
    switch(evt->m_type) {
    case center_agent_fsm_evt_pkg:
        stream_printf(s, "package: ");
        break;
    case center_agent_fsm_evt_timeout:
        stream_printf(s, "timeout");
        break;
    case center_agent_fsm_evt_connected:
        stream_printf(s, "connected");
        break;
    case center_agent_fsm_evt_disconnected:
        stream_printf(s, "disconnected");
        break;
    }
}

fsm_def_machine_t
center_agent_center_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    fsm_def_machine_t fsm_def = fsm_def_machine_create(name, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "center_agent_create_fsm_def: create fsm def fail!");
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, center_agent_center_dump_event);

    if (center_agent_center_fsm_create_disconnected(fsm_def, em) != 0
        || center_agent_center_fsm_create_idle(fsm_def, em) != 0
        || center_agent_center_fsm_create_join(fsm_def, em) != 0
        || center_agent_center_fsm_create_syncing(fsm_def, em) != 0)
    {
        CPE_ERROR(em, "center_agent_create_fsm_def: init fsm fail!");
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

void center_agent_center_close(struct center_agent_center * center) {
    net_ep_close(net_connector_ep(center->m_connector));
}

void center_agent_center_apply_evt(struct center_agent_center * center, enum center_agent_fsm_evt_type type) {
    struct center_agent_fsm_evt evt;
    evt.m_type = type;
    evt.m_pkg = NULL;
    fsm_machine_apply_event(&center->m_fsm, &evt);
}

void center_agent_center_apply_pkg(struct center_agent_center * center, SVR_CENTER_PKG * pkg) {
    struct center_agent_fsm_evt evt;
    evt.m_type = center_agent_fsm_evt_pkg;
    evt.m_pkg = pkg;
    fsm_machine_apply_event(&center->m_fsm, &evt);
}

void center_agent_center_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    struct center_agent_center * center = ctx;
    assert(center->m_fsm_timer_id == timer_id);
    center_agent_center_apply_evt(center, center_agent_fsm_evt_timeout);
}

int center_agent_center_start_state_timer(struct center_agent_center * center, tl_time_span_t span) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(center->m_agent->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            center->m_agent->m_em, "%s: start state timer: get default timer manager fail!",
            center_agent_name(center->m_agent));
        return -1;
    }

    assert(center->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &center->m_fsm_timer_id, center_agent_center_state_timeout, center, NULL, NULL, span, span, -1) != 0) {
        assert(center->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(center->m_agent->m_em, "%s: start state timer: regist timer fail!", center_agent_name(center->m_agent));
        return -1;
    }

    assert(center->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void center_agent_center_stop_state_timer(struct center_agent_center * center) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(center->m_agent->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(center->m_agent->m_em, "%s: start state timer: get default timer manager fail!", center_agent_name(center->m_agent));
        return;
    }

    assert(center->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, center->m_fsm_timer_id);
    center->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}

