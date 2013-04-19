#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/center/center_agent.h"
#include "svr/center/center_agent_pkg.h"
#include "center_agent_internal_ops.h"

static void center_agent_svr_cb(EV_P_ ev_io *w, int revents);

int center_agent_svr_init(center_agent_t agent, struct center_agent_svr * svr, uint16_t port) {
    struct ev_loop * loop = net_mgr_ev_loop(gd_app_net_mgr(agent->m_app));
    struct sockaddr_in addr;

    svr->m_agent = agent;
    svr->m_port = port;
    svr->m_pkg_meta = NULL;
    svr->m_cvt = NULL;

    svr->m_incoming_capacity = 2048;
    svr->m_incoming_pkg_buf = NULL;
    svr->m_dispatch_to = NULL;

    svr->m_fd = cpe_socket_open(PF_INET, SOCK_DGRAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(svr->m_fd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        CPE_ERROR(agent->m_em, "%s: svr_init: bind fail!", center_agent_name(agent));
        return -1;
    }

    svr->m_watcher.data = svr;
    ev_io_init(&svr->m_watcher, center_agent_svr_cb, svr->m_fd, EV_READ);
    ev_io_start(loop, &svr->m_watcher);

    return 0;
}

void center_agent_svr_clear(struct center_agent_svr * svr) {
    struct ev_loop * loop = net_mgr_ev_loop(gd_app_net_mgr(svr->m_agent->m_app));
    ev_io_stop(loop, &svr->m_watcher);
    cpe_sock_close(svr->m_fd);

    if (svr->m_incoming_pkg_buf) {
        center_agent_pkg_free(svr->m_incoming_pkg_buf);
        svr->m_incoming_pkg_buf = NULL;
    }

    if (svr->m_dispatch_to) {
        mem_free(svr->m_agent->m_alloc, svr->m_dispatch_to);
        svr->m_dispatch_to = NULL;
    }
}

int center_agent_svr_set_dispatch_to(struct center_agent_svr * svr, const char * dispatch_to) {
    cpe_hash_string_t new_dispatch_to = cpe_hs_create(svr->m_agent->m_alloc, dispatch_to);
    if (new_dispatch_to == NULL) return -1;

    if (svr->m_dispatch_to) mem_free(svr->m_agent->m_alloc, svr->m_dispatch_to);
    svr->m_dispatch_to = new_dispatch_to;

    return 0;
}

void center_agent_svr_set_recv_capacity(struct center_agent_svr * svr, size_t capacity) {
    svr->m_incoming_capacity = capacity;
}

static center_agent_pkg_t center_agent_svr_incoming_pkg_buf(struct center_agent_svr * svr, uint32_t capacity) {
    if (svr->m_incoming_pkg_buf && center_agent_pkg_capacity(svr->m_incoming_pkg_buf) < capacity) {
        center_agent_pkg_free(svr->m_incoming_pkg_buf);
        svr->m_incoming_pkg_buf = NULL;
    }

    if (svr->m_incoming_pkg_buf == NULL) {
        svr->m_incoming_pkg_buf = center_agent_pkg_create(svr->m_agent, capacity);
        if (svr->m_incoming_pkg_buf == NULL) {
            CPE_ERROR(svr->m_agent->m_em, "%s: crate incoming pkg buf fail!", center_agent_name(svr->m_agent));
        }
    }

    return svr->m_incoming_pkg_buf;
}

static void center_agent_svr_cb(EV_P_ ev_io *w, int revents) {
    struct center_agent_svr * svr = w->data;
    ssize_t recv_size;
    center_agent_pkg_t pkg = center_agent_svr_incoming_pkg_buf(svr, svr->m_incoming_capacity);
    if (pkg == NULL) {
        CPE_ERROR(svr->m_agent->m_em, "%s: svr: get center_agent_pkg fail!", center_agent_name(svr->m_agent));
        return;
    }

    if (svr->m_pkg_meta && svr->m_cvt) {
        dr_cvt_result_t cvt_result;
        size_t output_capacity;
        size_t input_capacity;

        mem_buffer_set_size(&svr->m_agent->m_dump_buffer, svr->m_incoming_capacity);
        recv_size = recvfrom(
            svr->m_fd,
            mem_buffer_make_continuous(&svr->m_agent->m_dump_buffer, 0),
            mem_buffer_size(&svr->m_agent->m_dump_buffer), 0, NULL, NULL);
        if (recv_size < 0) {
            CPE_ERROR(
                svr->m_agent->m_em, "%s: svr: recv error: error=%d(%s)!",
                center_agent_name(svr->m_agent), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            return;
        }

    SVR_DECODE_TRY_AGAIN:
        output_capacity = center_agent_pkg_capacity(pkg);
        input_capacity = (size_t)recv_size;
        cvt_result = dr_cvt_decode(
            svr->m_cvt,
            svr->m_pkg_meta,
            center_agent_pkg_data(pkg), &output_capacity,
            mem_buffer_make_continuous(&svr->m_agent->m_dump_buffer, 0), &input_capacity,
            svr->m_agent->m_em, svr->m_agent->m_debug);

        if (cvt_result == dr_cvt_result_not_enough_input) {
            pkg = center_agent_svr_incoming_pkg_buf(svr, center_agent_pkg_capacity(pkg) * 2);
            if (pkg == NULL) {
                CPE_ERROR(svr->m_agent->m_em, "%s: svr: get center_agent_pkg (again) fail!", center_agent_name(svr->m_agent));
                return;
            }
            goto SVR_DECODE_TRY_AGAIN;
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                svr->m_agent->m_em, "%s: svr: decode package fail, input size is %d!",
                center_agent_name(svr->m_agent), (int)recv_size);
            return;
        }
        else {
            center_agent_pkg_set_size(pkg, output_capacity);
        }
    }
    else {
        recv_size = recvfrom(svr->m_fd, center_agent_pkg_data(pkg), center_agent_pkg_capacity(pkg), 0, NULL, NULL);
        if (recv_size < 0) {
            CPE_ERROR(
                svr->m_agent->m_em, "%s: svr: on recv: recv error: error=%d(%s)!",
                center_agent_name(svr->m_agent), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            return;
        }
        center_agent_pkg_set_size(pkg, recv_size);
    }

    if (svr->m_agent->m_debug) {
        if (svr->m_pkg_meta) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_agent->m_dump_buffer);
            mem_buffer_clear_data(&svr->m_agent->m_dump_buffer);

            dr_json_print((write_stream_t)&stream, center_agent_pkg_data(pkg), center_agent_pkg_size(pkg), svr->m_pkg_meta, 0, NULL);
            stream_putc((write_stream_t)&stream, 0);

            CPE_INFO(
                svr->m_agent->m_em, "%s: svr: ==> recv one pkg, recv-size=%d, pkg-size=%d\n%s",
                center_agent_name(svr->m_agent),
                (int)recv_size, (int)center_agent_pkg_size(pkg),
                (const char *)mem_buffer_make_continuous(&svr->m_agent->m_dump_buffer, 0));
        }
        else {
            CPE_INFO(
                svr->m_agent->m_em, "%s: svr: ==> recv one pkg, recv-size=%d, pkg-size=%d",
                center_agent_name(svr->m_agent), (int)recv_size, (int)center_agent_pkg_size(pkg));
        }
    }

    if (svr->m_dispatch_to == NULL) {
        CPE_ERROR(svr->m_agent->m_em, "%s: on recv: no dispatch-to configured!", center_agent_name(svr->m_agent));
        return;
    }

    if (dp_dispatch_by_string(svr->m_dispatch_to, center_agent_pkg_to_dp_req(pkg), svr->m_agent->m_em) != 0) {
        CPE_ERROR(svr->m_agent->m_em, "%s: on recv: dispatch-to %s!", center_agent_name(svr->m_agent), cpe_hs_data(svr->m_dispatch_to));
        return;
    }
}
