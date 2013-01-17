#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_proxy/app_net_proxy.h"
#include "app_net_proxy_internal_ops.h"
#include "protocol/app_net_protocol.h"

static void app_net_proxy_on_read(app_net_proxy_t proxy, net_ep_t ep) {
	app_net_pkg_t req_buf;

    if(proxy->m_debug >= 2) {
        CPE_INFO(
            proxy->m_em, "%s: ep %d: on read",
            app_net_proxy_name(proxy), (int)net_ep_id(ep));
    }

    req_buf = app_net_proxy_req_buf(proxy);
    if (req_buf == NULL) {
       CPE_ERROR(
           proxy->m_em, "%s: ep %d: get req buf fail!",
           app_net_proxy_name(proxy), (int)net_ep_id(ep));
       net_ep_close(ep);
       return;
    }

    while(1) {
        TSPKG_HEAD head_buf;

        char * input_buf;
        size_t input_buf_size;
        size_t input_head_use_size;
        size_t output_size;
        dr_cvt_result_t cvt_result;

        bzero(&head_buf, sizeof(head_buf));

        input_buf_size = net_ep_size(ep);
        if (input_buf_size <= 0) break;

        input_buf = net_ep_peek(ep, NULL, input_buf_size);
        if (input_buf == NULL) {
            CPE_ERROR(
                proxy->m_em, "%s: ep %d: peek data fail, size=%d!",
                app_net_proxy_name(proxy), (int)net_ep_id(ep), (int)input_buf_size);
            net_ep_close(ep);
            break;
        }

        input_head_use_size = input_buf_size;
        output_size = sizeof(head_buf);

        cvt_result =
            dr_cvt_decode(
                proxy->m_cvt,
                proxy->m_head_meta,
                &head_buf, &output_size,
                input_buf, &input_head_use_size, proxy->m_em, proxy->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if(proxy->m_debug) {
                CPE_ERROR(
                    proxy->m_em, "%s: ep %d: not enough head data, input size is %d!",
                app_net_proxy_name(proxy), (int)net_ep_id(ep), (int)input_buf_size);
            }
            break;
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                proxy->m_em, "%s: ep %d: decode package fail, input size is %d!",
                app_net_proxy_name(proxy), (int)net_ep_id(ep), (int)input_buf_size);
            net_ep_close(ep);
            break;
        }

        if (head_buf.bodylen + input_head_use_size < input_buf_size) {
            if(proxy->m_debug) {
                CPE_ERROR(
                    proxy->m_em, "%s: ep %d: not enough head data, input size is %d!",
                app_net_proxy_name(proxy), (int)net_ep_id(ep), (int)input_buf_size);
            }
            break;
        }

        if (head_buf.bodylen > app_net_pkg_data_capacity(req_buf)) {
            CPE_ERROR(
                proxy->m_em, "%s: ep %d: req_buf not enough capacity! capacity=%d, input body size=%d!",
                app_net_proxy_name(proxy), (int)net_ep_id(ep), (int)app_net_pkg_data_capacity(req_buf), (int)input_buf_size);
            net_ep_erase(ep, head_buf.bodylen + input_head_use_size);
            continue;
        }

        memcpy(app_net_pkg_data(req_buf), input_buf + input_head_use_size, head_buf.bodylen);
        app_net_pkg_data_set_size(req_buf, head_buf.bodylen);

        net_ep_erase(ep, head_buf.bodylen + input_head_use_size);

        if(proxy->m_debug >= 2) {
            CPE_INFO(
                proxy->m_em, "%s: ep %d: decode one package, buf-origin-size=%d left-size=%d!",
                app_net_proxy_name(proxy), (int)net_ep_id(ep), (int)input_buf_size, (int)net_ep_size(ep));
        }

        switch(head_buf.cmd) {
        }

		/* switch(app_net_pkg_cmd(req_buf)) { */
		/* case app_net_pkg_req_regist_client: */
		/* 	app_net_proxy_regClient(pInput, output_size, (size_t)net_ep_id(ep) ); */
		/* 	break; */
		/* case TS_CMD_REQ_TRANSFER: */
		/* 	app_net_proxy_transfer( pInput, output_size, (size_t)net_ep_id(ep) ); */
		/* 	break; */
		/* } */


        //dp_dispatch_by_string(
        //app_net_pkg_set_connection_id(req_buf, net_ep_id(ep));
        //if (proxy->m_dispatch_to) {
        //    if (dp_dispatch_by_string(proxy->m_dispatch_to, app_net_pkg_to_dp_req(req_buf), proxy->m_em) != 0) {
        //        CPE_ERROR(
        //            proxy->m_em, "%s: ep %d: dispatch to %s error!",
        //            app_net_proxy_name(proxy), (int)net_ep_id(ep), cpe_hs_data(proxy->m_dispatch_to));

        //        app_net_pkg_set_errno(req_buf, -1);
        //        app_net_proxy_reply(app_net_pkg_to_dp_req(req_buf), proxy, proxy->m_em);
        //    }
        //}
        //else {
        //    if (dp_dispatch_by_numeric(app_net_pkg_cmd(req_buf), app_net_pkg_to_dp_req(req_buf), proxy->m_em) != 0) {
        //        CPE_ERROR(
        //            proxy->m_em, "%s: ep %d: dispatch cmd %d error!",
        //            app_net_proxy_name(proxy), (int)net_ep_id(ep), app_net_pkg_cmd(req_buf));

        //        app_net_pkg_set_errno(req_buf, -1);
        //        app_net_proxy_reply(app_net_pkg_to_dp_req(req_buf), proxy, proxy->m_em);
        //    }
        //}
    }
}

static void app_net_proxy_on_open(app_net_proxy_t proxy, net_ep_t ep) {
    struct cpe_hash_it ep_it;
    app_net_ep_t app_ep;
	app_net_pkg_t req_buf;

    if(proxy->m_debug >= 2) {
        CPE_INFO(
            proxy->m_em, "%s: ep %d: on open",
            app_net_proxy_name(proxy), (int)net_ep_id(ep));
    }

    req_buf = app_net_proxy_req_buf(proxy);
    if (req_buf == NULL) {
       CPE_ERROR(
           proxy->m_em, "%s: ep %d: on open: get req buf fail!",
           app_net_proxy_name(proxy), (int)net_ep_id(ep));
       return;
    }

    if(proxy->m_debug) {
        CPE_INFO(
            proxy->m_em, "%s: ep %d: on open",
            app_net_proxy_name(proxy), (int)net_ep_id(ep));
    }

    cpe_hash_it_init(&ep_it, &proxy->m_eps);

    while((app_ep = cpe_hash_it_next(&ep_it))) {
        app_net_ep_send_regist_req(app_ep, ep);
    }
}

static void app_net_proxy_on_close(app_net_proxy_t proxy, net_ep_t ep, net_ep_event_t event) {
    struct cpe_hash_it ep_it;
    app_net_ep_t app_ep;

    if(proxy->m_debug) {
        CPE_INFO(
            proxy->m_em, "%s: ep %d: on close, event=%d",
            app_net_proxy_name(proxy), (int)net_ep_id(ep), event);
    }

    cpe_hash_it_init(&ep_it, &proxy->m_eps);

    while((app_ep = cpe_hash_it_next(&ep_it))) {
        app_net_ep_set_state(app_ep, app_net_ep_state_init);
    }

    net_ep_free(ep);
}

static void app_net_proxy_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    app_net_proxy_t proxy = (app_net_proxy_t)ctx;

    assert(proxy);

    switch(event) {
    case net_ep_event_read:
        app_net_proxy_on_read(proxy, ep);
        break;
    case net_ep_event_open:
        app_net_proxy_on_open(proxy, ep);
        break;
    default:
        app_net_proxy_on_close(proxy, ep, event);
        break;
    }
}

static void app_net_proxy_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    app_net_proxy_t proxy = (app_net_proxy_t)ctx;

    assert(proxy);

    mem_free(proxy->m_alloc, net_chanel_queue_buf(chanel));
}

int app_net_proxy_ep_init(app_net_proxy_t proxy, net_ep_t ep, size_t read_chanel_size, size_t write_chanel_size) {
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;

    assert(proxy);

    buf_r = mem_alloc(proxy->m_alloc, read_chanel_size);
    buf_w = mem_alloc(proxy->m_alloc, write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto EP_INIT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, read_chanel_size);
    if (chanel_r == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_r, app_net_proxy_free_chanel_buf, proxy);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, write_chanel_size);
    if (chanel_w == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_w, app_net_proxy_free_chanel_buf, proxy);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, app_net_proxy_process, proxy);

    if(proxy->m_debug) {
        CPE_INFO(
            proxy->m_em, "%s: ep %d: init success!",
            app_net_proxy_name(proxy), (int)net_ep_id(ep));
    }

    return 0;
EP_INIT_ERROR:
    if (buf_r) mem_free(proxy->m_alloc, buf_r);
    if (buf_w) mem_free(proxy->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        proxy->m_em, "%s: ep %d: init fail!",
        app_net_proxy_name(proxy), (int)net_ep_id(ep));

    return -1;
}


void app_net_ep_send_regist_req(app_net_ep_t ep, net_ep_t net_ep) {
    TSPKG_HEAD head_buf;
    bzero(&head_buf, sizeof(head_buf));

    head_buf.app_type = ep->m_app_type;
    head_buf.app_id = ep->m_app_id;
    head_buf.cmd = TS_CMD_REQ_REGISTER_CLIENT;

    if (net_ep_send(net_ep, &head_buf, sizeof(head_buf)) != 0) {
        app_net_ep_set_state(ep, app_net_ep_state_registing);

        if (ep->m_proxy->m_debug) {
            CPE_INFO(
                ep->m_proxy->m_em, "%s: app-ep %d.%d: send regist pkg success!",
                app_net_proxy_name(ep->m_proxy), ep->m_app_type, ep->m_app_id);
        }
    }
    else {
        CPE_INFO(
            ep->m_proxy->m_em, "%s: app-ep %d.%d: send regist pkg fail!",
            app_net_proxy_name(ep->m_proxy), ep->m_app_type, ep->m_app_id);
        app_net_ep_set_state(ep, app_net_ep_state_error);
    }
}
