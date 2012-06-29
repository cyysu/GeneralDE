#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_ref.h"
#include "usf/logic/logic_executor_build.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "bpg_rsp_internal_ops.h"

bpg_rsp_t bpg_rsp_create(bpg_rsp_manage_t mgr, const char * name) {
    char * buf;
    size_t name_len;
    bpg_rsp_t rsp;
    
    assert(name);

    name_len = strlen(name) + 1;

    buf = mem_alloc(mgr->m_alloc, sizeof(struct bpg_rsp) + name_len);
    if (buf == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp %s: alloc fail", bpg_rsp_manage_name(mgr), name) ;
        return NULL;
    }

    memcpy(buf, name, name_len);

    rsp = (bpg_rsp_t)(buf + name_len);
    rsp->m_mgr = mgr;
    rsp->m_name = buf;
    rsp->m_queue_info = NULL;
    rsp->m_flags = 0;

    TAILQ_INIT(&rsp->m_ctx_to_pdu);
    TAILQ_INSERT_TAIL(&mgr->m_rsps, rsp, m_next);

    return rsp;
}

void bpg_rsp_free(bpg_rsp_t rsp) {
    dp_rsp_t dp_rsp;

    TAILQ_REMOVE(&rsp->m_mgr->m_rsps, rsp, m_next);

    bpg_rsp_copy_info_clear(rsp);

    if (rsp->m_executor_ref) {
        logic_executor_ref_dec(rsp->m_executor_ref);
        rsp->m_executor_ref = NULL;
    }

    dp_rsp = dp_rsp_find_by_name(gd_app_dp_mgr(rsp->m_mgr->m_app), bpg_rsp_name(rsp));
    if (dp_rsp) {
        dp_rsp_free(dp_rsp);
    }

    mem_free(rsp->m_mgr->m_alloc, (void*)rsp->m_name);
}

void bpg_rsp_set_executor(bpg_rsp_t rsp, logic_executor_ref_t executor) {
    if (rsp->m_executor_ref) logic_executor_ref_dec(rsp->m_executor_ref);
    if (executor) logic_executor_ref_inc(executor);
    rsp->m_executor_ref = executor;
}

int bpg_rsp_set_queue(bpg_rsp_t rsp, const char * queue_name) {
    struct bpg_rsp_queue_info *  queue_info = NULL;
    
    if (queue_name) {
        char name_buf[128];
        cpe_hs_init((cpe_hash_string_t)name_buf, sizeof(name_buf), queue_name);
        queue_info = bpg_rsp_queue_info_find(rsp->m_mgr, (cpe_hash_string_t)name_buf);
        if (queue_info == NULL) {
            CPE_ERROR(
                rsp->m_mgr->m_em, "%s: rsp %s: logic queue %s not exist!", 
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), queue_name) ;
            return -1;
        }
    }

    rsp->m_queue_info = queue_info;

    return 0;
}


const char * bpg_rsp_name(bpg_rsp_t rsp) {
    return rsp->m_name;
}

uint32_t bpg_rsp_flags(bpg_rsp_t rsp) {
    return rsp->m_flags;
}

void bpg_rsp_flags_set(bpg_rsp_t rsp, uint32_t flag) {
    rsp->m_flags = flag;
}

void bpg_rsp_flag_enable(bpg_rsp_t rsp, bpg_rsp_flag_t flag) {
    rsp->m_flags |= flag;
}

void bpg_rsp_flag_disable(bpg_rsp_t rsp, bpg_rsp_flag_t flag) {
    rsp->m_flags &= ~((uint32_t)flag);
}

int bpg_rsp_flag_is_enable(bpg_rsp_t rsp, bpg_rsp_flag_t flag) {
    return rsp->m_flags & flag;
}
