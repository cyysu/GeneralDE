#include <assert.h>
#include "svr/set/share/set_pkg.h"
#include "set_svr_router_ops.h"

set_svr_router_t set_svr_router_create(set_svr_t svr, uint32_t ip, uint16_t port) {
    set_svr_router_t router;

    router = mem_alloc(svr->m_alloc, sizeof(struct set_svr_router));
    if (router == NULL) {
        CPE_ERROR(svr->m_em, "%s: create router %d.%d: malloc fail!", set_svr_name(svr), ip, port);
        return NULL;
    }

    router->m_svr = svr;
    router->m_id = ++svr->m_max_conn_id;
    router->m_ip = ip;
    router->m_port = port;

    router->m_conn = NULL;
    router->m_wb = NULL;

    TAILQ_INIT(&router->m_svr_svrs);

    cpe_hash_entry_init(&router->m_hh_by_id);
    if (cpe_hash_table_insert_unique(&svr->m_routers_by_id, router) != 0) {
        CPE_ERROR(svr->m_em, "%s: router %d-%d.%d: id duplicate!", set_svr_name(svr), router->m_id, ip, port);
        mem_free(svr->m_alloc, router);
        return NULL;
    }

    cpe_hash_entry_init(&router->m_hh_by_addr);
    if (cpe_hash_table_insert_unique(&svr->m_routers_by_addr, router) != 0) {
        CPE_ERROR(svr->m_em, "%s: router %d-%d.%d: addr duplicate!", set_svr_name(svr), router->m_id, ip, port);
        cpe_hash_table_remove_by_ins(&svr->m_routers_by_id, router);
        mem_free(svr->m_alloc, router);
        return NULL;
    }

    return router;
}

void set_svr_router_free(set_svr_router_t router) {
    set_svr_t svr = router->m_svr;

    if (router->m_conn) {
        set_svr_router_conn_free(router->m_conn);
        assert(router->m_conn == NULL);
    }
    
    cpe_hash_table_remove_by_ins(&svr->m_routers_by_id, router);
    cpe_hash_table_remove_by_ins(&svr->m_routers_by_addr, router);

    mem_free(svr->m_alloc, router);
}

void set_svr_router_free_all(set_svr_t svr) {
    struct cpe_hash_it router_it;
    set_svr_router_t router;

    cpe_hash_it_init(&router_it, &svr->m_routers_by_addr);

    router = cpe_hash_it_next(&router_it);
    while(router) {
        set_svr_router_t next = cpe_hash_it_next(&router_it);
        set_svr_router_free(router);
        router = next;
    }
}

set_svr_router_t set_svr_router_find_by_addr(set_svr_t svr, uint32_t ip, uint16_t port) {
    struct set_svr_router key;
    key.m_ip = ip;
    key.m_port = port;
    return cpe_hash_table_find(&svr->m_routers_by_addr, &key);
}

set_svr_router_t set_svr_router_find_by_id(set_svr_t svr, uint32_t id) {
    struct set_svr_router key;
    key.m_id = id;
    return cpe_hash_table_find(&svr->m_routers_by_id, &key);
}

void set_svr_router_clear_data(set_svr_router_t router) {
    if (router->m_conn) {
        set_svr_router_conn_free(router->m_conn);
        assert(router->m_conn == NULL);
    }

    if (router->m_wb) {
        ringbuffer_free(router->m_svr->m_ringbuf, router->m_wb);
        router->m_wb = NULL;
    }
}

void set_svr_router_link_node_w(set_svr_router_t router, ringbuffer_block_t blk) {
    if (router->m_wb) {
		ringbuffer_link(router->m_svr->m_ringbuf, router->m_wb , blk);
	}
    else {
		blk->id = router->m_id;
		router->m_wb = blk;
	}
}

uint32_t set_svr_router_hash_by_addr(set_svr_router_t o) {
    return o->m_ip;
}

int set_svr_router_eq_by_addr(set_svr_router_t l, set_svr_router_t r) {
    return l->m_ip == r->m_ip && l->m_port == r->m_port;
}

uint32_t set_svr_router_hash_by_id(set_svr_router_t o) {
    return o->m_id;
}

int set_svr_router_eq_by_id(set_svr_router_t l, set_svr_router_t r) {
    return l->m_id == r->m_id;
}


LPDRMETA set_svr_get_pkg_meta(set_svr_t svr, dp_req_t head, set_svr_svr_type_t to_svr_type, set_svr_svr_type_t from_svr_type) {
    switch(set_pkg_category(head)) {
    case set_pkg_request:
        if (to_svr_type == NULL) {
            to_svr_type = set_svr_svr_type_find_by_id(svr, set_pkg_to_svr_type(head));
            if (to_svr_type == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: get_pkg_meta: to svr type %d not exist!",
                    set_svr_name(svr), set_pkg_to_svr_type(head));
                return NULL;
            }
        }
        return to_svr_type->m_pkg_meta;
    case set_pkg_response:
    case set_pkg_notify: {
        if (from_svr_type == NULL) {
            from_svr_type = set_svr_svr_type_find_by_id(svr, set_pkg_from_svr_type(head));
            if (from_svr_type == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: get_pkg_meta: from svr type %d not exist!",
                    set_svr_name(svr), set_pkg_from_svr_type(head));
                return NULL;
            }
        }
        return from_svr_type->m_pkg_meta;
    }
    default:
        CPE_ERROR(
            svr->m_em, "%s: get_pkg_meta: category %d is unknown!",
            set_svr_name(svr), set_pkg_category(head));
        return NULL;
    }
}
