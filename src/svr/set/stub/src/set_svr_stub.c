#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/agent/center_agent_svr_type.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/share/set_repository.h"
#include "svr/set/stub/set_svr_stub.h"
#include "set_svr_stub_internal_ops.h"

static void set_svr_stub_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_set_svr_stub = {
    "svr_set_svr_stub",
    set_svr_stub_clear
};

set_svr_stub_t
set_svr_stub_create(
    gd_app_context_t app,
    const char * name,
    center_agent_t agent,
    center_agent_svr_type_t svr_type, uint16_t svr_id,
    mem_allocrator_t alloc, error_monitor_t em)
{
    set_svr_stub_t svr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct set_svr_stub));
    if (mgr_node == NULL) return NULL;

    svr = (set_svr_stub_t)nm_node_data(mgr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_debug = 0;
    svr->m_agent = agent;
    svr->m_pidfile_fd = -1;

    svr->m_svr_type = svr_type;
    svr->m_svr_id = svr_id;
    svr->m_process_count_per_tick = 10;

    svr->m_request_dispatch_to = NULL;
    svr->m_response_dispatch_to = NULL;
    svr->m_dispatch_info_count = 0;
    svr->m_dispatch_infos = NULL;
    svr->m_outgoing_recv_at = NULL;

    svr->m_incoming_buf = NULL;

    svr->m_chanel = NULL;

    if (gd_app_tick_add(app, set_svr_stub_tick, svr, 0) != 0) {
        CPE_ERROR(em, "%s: create: add tick fail!", name);
        nm_node_free(mgr_node);
        return NULL;
    }

    mem_buffer_init(&svr->m_dump_buffer_head, alloc);
    mem_buffer_init(&svr->m_dump_buffer_carry, alloc);
    mem_buffer_init(&svr->m_dump_buffer_body, alloc);

    nm_node_set_type(mgr_node, &s_nm_node_type_set_svr_stub);
    
    return svr;
}

static void set_svr_stub_clear(nm_node_t node) {
    set_svr_stub_t svr;

    svr = (set_svr_stub_t)nm_node_data(node);

    if (svr->m_pidfile_fd == -1) {
        close(svr->m_pidfile_fd);
        svr->m_pidfile_fd = -1;
    }

    gd_app_tick_remove(svr->m_app, set_svr_stub_tick, svr);

    if (svr->m_incoming_buf) {
        dp_req_free(svr->m_incoming_buf);
        svr->m_incoming_buf = NULL;
    }

    if (svr->m_request_dispatch_to) {
        mem_free(svr->m_alloc, svr->m_request_dispatch_to);
        svr->m_request_dispatch_to = NULL;
    }

    if (svr->m_response_dispatch_to) {
        mem_free(svr->m_alloc, svr->m_response_dispatch_to);
        svr->m_response_dispatch_to = NULL;
    }

    if (svr->m_dispatch_infos) {
        size_t i;
        for(i = 0; i < svr->m_dispatch_info_count; ++i) {
            if (svr->m_dispatch_infos[i].m_notify_dispatch_to) {
                mem_free(svr->m_alloc, svr->m_dispatch_infos[i].m_notify_dispatch_to);
            }

            if (svr->m_dispatch_infos[i].m_response_dispatch_to) {
                mem_free(svr->m_alloc, svr->m_dispatch_infos[i].m_response_dispatch_to);
            }
        }

        mem_free(svr->m_alloc, svr->m_dispatch_infos);
        svr->m_dispatch_infos = NULL;
        svr->m_dispatch_info_count = 0;
    }

    if (svr->m_outgoing_recv_at) {
        dp_rsp_free(svr->m_outgoing_recv_at);
        svr->m_outgoing_recv_at = NULL;
    }

    if (svr->m_chanel) {
        set_repository_chanel_detach(svr->m_chanel, svr->m_em);
        svr->m_chanel = NULL;
    }

    mem_buffer_clear(&svr->m_dump_buffer_head);
    mem_buffer_clear(&svr->m_dump_buffer_carry);
    mem_buffer_clear(&svr->m_dump_buffer_body);
}

void set_svr_stub_free(set_svr_stub_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_set_svr_stub) return;
    nm_node_free(mgr_node);
}

set_svr_stub_t
set_svr_stub_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_svr_stub) return NULL;
    return (set_svr_stub_t)nm_node_data(node);
}

set_svr_stub_t
set_svr_stub_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_svr_stub) return NULL;
    return (set_svr_stub_t)nm_node_data(node);
}

gd_app_context_t set_svr_stub_app(set_svr_stub_t mgr) {
    return mgr->m_app;
}

const char * set_svr_stub_name(set_svr_stub_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t set_svr_stub_name_hs(set_svr_stub_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

cpe_hash_string_t set_svr_stub_request_dispatch_to(set_svr_stub_t svr) {
    return svr->m_request_dispatch_to;
}

void set_svr_stub_set_chanel(set_svr_stub_t svr, set_chanel_t chanel) {
    svr->m_chanel = chanel;
}

int set_svr_stub_set_request_dispatch_to(set_svr_stub_t svr, const char * request_dispatch_to) {
    cpe_hash_string_t new_request_dispatch_to = cpe_hs_create(svr->m_alloc, request_dispatch_to);
    if (new_request_dispatch_to == NULL) return -1;

    if (svr->m_request_dispatch_to) mem_free(svr->m_alloc, svr->m_request_dispatch_to);
    svr->m_request_dispatch_to = new_request_dispatch_to;

    return 0;
}

cpe_hash_string_t set_svr_stub_response_dispatch_to(set_svr_stub_t svr) {
    return svr->m_response_dispatch_to;
}

int set_svr_stub_set_response_dispatch_to(set_svr_stub_t svr, const char * response_dispatch_to) {
    cpe_hash_string_t new_response_dispatch_to = cpe_hs_create(svr->m_alloc, response_dispatch_to);
    if (new_response_dispatch_to == NULL) return -1;

    if (svr->m_response_dispatch_to) mem_free(svr->m_alloc, svr->m_response_dispatch_to);
    svr->m_response_dispatch_to = new_response_dispatch_to;

    return 0;
}

cpe_hash_string_t set_svr_stub_svr_response_dispatch_to(set_svr_stub_t svr, uint16_t svr_type) {
    struct set_svr_stub_dispach_info * dispatch_info = 
        set_svr_stub_find_dispatch_info(svr, svr_type);

    return dispatch_info ? dispatch_info->m_response_dispatch_to : NULL;
}

int set_svr_stub_set_svr_response_dispatch_to(set_svr_stub_t svr, uint16_t svr_type, const char * dispatch_to) {
    cpe_hash_string_t new_value;
    struct set_svr_stub_dispach_info * dispatch_info;

    new_value = cpe_hs_create(svr->m_alloc, dispatch_to);
    if (new_value == NULL) return -1;

    dispatch_info = set_svr_stub_find_dispatch_info_check_create(svr, svr_type);
    if (dispatch_info == NULL) {
        mem_free(svr->m_alloc, new_value);
        return -1;
    }

    if (dispatch_info->m_response_dispatch_to) {
        mem_free(svr->m_alloc, dispatch_info->m_response_dispatch_to);
    }
    dispatch_info->m_response_dispatch_to = new_value;

    return 0;
}

cpe_hash_string_t set_svr_stub_svr_notify_dispatch_to(set_svr_stub_t svr, uint16_t svr_type) {
    struct set_svr_stub_dispach_info * dispatch_info = 
        set_svr_stub_find_dispatch_info(svr, svr_type);

    return dispatch_info ? dispatch_info->m_notify_dispatch_to : NULL;
}

int set_svr_stub_set_svr_notify_dispatch_to(set_svr_stub_t svr, uint16_t svr_type, const char * dispatch_to) {
    cpe_hash_string_t new_value;
    struct set_svr_stub_dispach_info * dispatch_info;

    new_value = cpe_hs_create(svr->m_alloc, dispatch_to);
    if (new_value == NULL) return -1;

    dispatch_info = set_svr_stub_find_dispatch_info_check_create(svr, svr_type);
    if (dispatch_info == NULL) {
        mem_free(svr->m_alloc, new_value);
        return -1;
    }

    if (dispatch_info->m_notify_dispatch_to) {
        mem_free(svr->m_alloc, dispatch_info->m_notify_dispatch_to);
    }
    dispatch_info->m_notify_dispatch_to = new_value;

    return 0;
}

int set_svr_stub_set_outgoing_recv_at(set_svr_stub_t svr, const char * outgoing_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.outgoing-recv-rsp", set_svr_stub_name(svr));

    if (svr->m_outgoing_recv_at) dp_rsp_free(svr->m_outgoing_recv_at);

    svr->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), name_buf);
    if (svr->m_outgoing_recv_at == NULL) return -1;

    dp_rsp_set_processor(svr->m_outgoing_recv_at, set_svr_stub_outgoing_recv, svr);

    if (dp_rsp_bind_string(svr->m_outgoing_recv_at, outgoing_recv_at, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: set outgoing_recv_at: bind to %s fail!",
            set_svr_stub_name(svr), outgoing_recv_at);
        dp_rsp_free(svr->m_outgoing_recv_at);
        svr->m_outgoing_recv_at = NULL;
        return -1;
    }

    return 0;
}

struct set_svr_stub_dispach_info * set_svr_stub_find_dispatch_info(set_svr_stub_t svr, uint16_t svr_type) {
    if (svr_type >= svr->m_dispatch_info_count) return NULL;
    return &svr->m_dispatch_infos[svr_type];
}

struct set_svr_stub_dispach_info * set_svr_stub_find_dispatch_info_check_create(set_svr_stub_t svr, uint16_t svr_type) {
    if (svr_type >= svr->m_dispatch_info_count) {
        size_t new_capacity = ((svr_type >> 4) + 1) << 4;
        struct set_svr_stub_dispach_info * infos;

        infos = mem_alloc(svr->m_alloc, sizeof(struct set_svr_stub_dispach_info) * new_capacity);
        if (infos == NULL) return NULL;

        if (svr->m_dispatch_info_count) {
            assert(svr->m_dispatch_infos);
            memcpy(infos, svr->m_dispatch_infos, sizeof(struct set_svr_stub_dispach_info) * svr->m_dispatch_info_count);
            mem_free(svr->m_alloc, svr->m_dispatch_infos);
        }

        bzero(
            infos + svr->m_dispatch_info_count, 
            sizeof(struct set_svr_stub_dispach_info) * (new_capacity - svr->m_dispatch_info_count));

        svr->m_dispatch_infos = infos;
        svr->m_dispatch_info_count = new_capacity;
    }

    return &svr->m_dispatch_infos[svr_type];
}

int set_svr_stub_write_pidfile(set_svr_stub_t svr, const char * pidfile) {
    char buf[16];  
  
    /* 打开放置记录锁的文件 */
    svr->m_pidfile_fd = open(pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  
    if (svr->m_pidfile_fd < 0) {  
        CPE_ERROR(
            svr->m_em, "%s: write pidfile: can`t open %s, error=%d (%s)!",
            set_svr_stub_name(svr), pidfile, errno, strerror(errno));
        return -1;
    }  

    /*试图对文件fd加锁，*/
    svr->m_pidfile_lock.l_type = F_WRLCK;  
    svr->m_pidfile_lock.l_start = 0;  
    svr->m_pidfile_lock.l_whence = SEEK_SET;  
    svr->m_pidfile_lock.l_len = 0;  

    if (fcntl(svr->m_pidfile_fd, F_SETLK, &svr->m_pidfile_lock) < 0) {      /*如果加锁失败的话 */
        /* 如果是因为权限不够或资源暂时不可用，则返回1  */
        if (EACCES == errno || EAGAIN == errno) {  
            CPE_ERROR(
                svr->m_em, "%s: write pidfile: can`t lock %s, error=%d (%s)!",
                set_svr_stub_name(svr), pidfile, errno, strerror(errno));

            close(svr->m_pidfile_fd);
            svr->m_pidfile_fd = -1;
            return -1;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: write pidfile: can`t lock %s, error=%d (%s)!",
                set_svr_stub_name(svr), pidfile, errno, strerror(errno));

            close(svr->m_pidfile_fd);
            svr->m_pidfile_fd = -1;
            return -1;
        }
    }  
  
    /* 先将文件fd清空，然后再向其中写入当前的进程号 */
    ftruncate(svr->m_pidfile_fd, 0);
    snprintf(buf, sizeof(buf), "%d", (int)getpid());
    write(svr->m_pidfile_fd, buf, strlen(buf));

    return 0;
}
