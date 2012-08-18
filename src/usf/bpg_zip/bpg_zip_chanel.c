#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_zip/bpg_zip_chanel.h"
#include "bpg_zip_internal_types.h"

static void bpg_zip_chanel_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_zip_chanel = {
    "usf_bpg_zip_chanel",
    bpg_zip_chanel_clear
};

bpg_zip_chanel_t
bpg_zip_chanel_create(
    gd_app_context_t app,
    const char * name,
    error_monitor_t em)
{
    bpg_zip_chanel_t chanel;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_zip_chanel));
    if (mgr_node == NULL) return NULL;

    chanel = (bpg_zip_chanel_t)nm_node_data(mgr_node);

    chanel->m_app = app;
    chanel->m_alloc = gd_app_alloc(app);
    chanel->m_em = em;
    chanel->m_debug = 0;

    chanel->m_zip_send_to = NULL;
    chanel->m_zip_recv_at = NULL;
    chanel->m_unzip_send_to = NULL;
    chanel->m_unzip_recv_at = NULL;
    chanel->m_mask_bit = 0;
    chanel->m_size_threshold = 0;
    mem_buffer_init(&chanel->m_data_buf, gd_app_alloc(app));

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_zip_chanel);

    return chanel;
}

static void bpg_zip_chanel_clear(nm_node_t node) {
    bpg_zip_chanel_t chanel;
    chanel = (bpg_zip_chanel_t)nm_node_data(node);

    if (chanel->m_zip_send_to != NULL) {
        bpg_pkg_dsp_free(chanel->m_zip_send_to);
        chanel->m_zip_send_to = NULL;
    }

    if (chanel->m_zip_recv_at != NULL) {
        dp_rsp_free(chanel->m_zip_recv_at);
        chanel->m_zip_recv_at = NULL;
    }

    if (chanel->m_unzip_send_to != NULL) {
        bpg_pkg_dsp_free(chanel->m_unzip_send_to);
        chanel->m_unzip_send_to = NULL;
    }

    if (chanel->m_unzip_recv_at != NULL) {
        dp_rsp_free(chanel->m_unzip_recv_at);
        chanel->m_unzip_recv_at = NULL;
    }

    mem_buffer_clear(&chanel->m_data_buf);
}

gd_app_context_t bpg_zip_chanel_app(bpg_zip_chanel_t chanel) {
    return chanel->m_app;
}

void bpg_zip_chanel_free(bpg_zip_chanel_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_zip_chanel) return;
    nm_node_free(mgr_node);
}

bpg_zip_chanel_t
bpg_zip_chanel_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_zip_chanel) return NULL;
    return (bpg_zip_chanel_t)nm_node_data(node);
}

bpg_zip_chanel_t
bpg_zip_chanel_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_zip_chanel) return NULL;
    return (bpg_zip_chanel_t)nm_node_data(node);
}

const char * bpg_zip_chanel_name(bpg_zip_chanel_t chanel) {
    return nm_node_name(nm_node_from_data(chanel));
}

cpe_hash_string_t
bpg_zip_chanel_name_hs(bpg_zip_chanel_t chanel) {
    return nm_node_name_hs(nm_node_from_data(chanel));
}

int bpg_zip_chanel_set_zip_send_to(bpg_zip_chanel_t chanel, cfg_t cfg) {
    if (chanel->m_zip_send_to != NULL) {
        bpg_pkg_dsp_free(chanel->m_zip_send_to);
        chanel->m_zip_send_to = NULL;
    }

    chanel->m_zip_send_to = bpg_pkg_dsp_create(chanel->m_alloc);
    if (chanel->m_zip_send_to == NULL) return -1;

    if (bpg_pkg_dsp_load(chanel->m_zip_send_to, cfg, chanel->m_em) != 0) return -1;

    return 0;
}

int bpg_zip_chanel_zip_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int bpg_zip_chanel_set_zip_recv_at(bpg_zip_chanel_t chanel, const char * name) {
    char sp_name_buf[128];

    if (chanel->m_zip_recv_at != NULL) {
        dp_rsp_free(chanel->m_zip_recv_at);
        chanel->m_zip_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.zip.recv.sp", bpg_zip_chanel_name(chanel));
    chanel->m_zip_recv_at = dp_rsp_create(gd_app_dp_mgr(chanel->m_app), sp_name_buf);
    if (chanel->m_zip_recv_at == NULL) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_zip_chanel_set_zip_recv_at: create rsp fail!",
            bpg_zip_chanel_name(chanel));
        return -1;
    }
    dp_rsp_set_processor(chanel->m_zip_recv_at, bpg_zip_chanel_zip_rsp, chanel);

    if (dp_rsp_bind_string(chanel->m_zip_recv_at, name, chanel->m_em) != 0) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_zip_chanel_set_zip_recv_at: bind rsp to %s fail!",
            bpg_zip_chanel_name(chanel), name);
        dp_rsp_free(chanel->m_zip_recv_at);
        chanel->m_zip_recv_at = NULL;
        return -1;
    }

    return 0;
}

int bpg_zip_chanel_set_unzip_send_to(bpg_zip_chanel_t chanel, cfg_t cfg) {
    if (chanel->m_unzip_send_to != NULL) {
        bpg_pkg_dsp_free(chanel->m_unzip_send_to);
        chanel->m_unzip_send_to = NULL;
    }

    chanel->m_unzip_send_to = bpg_pkg_dsp_create(chanel->m_alloc);
    if (chanel->m_unzip_send_to == NULL) return -1;

    if (bpg_pkg_dsp_load(chanel->m_unzip_send_to, cfg, chanel->m_em) != 0) return -1;

    return 0;
}

int bpg_zip_chanel_unzip_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int bpg_zip_chanel_set_unzip_recv_at(bpg_zip_chanel_t chanel, const char * name) {
    char sp_name_buf[128];

    if (chanel->m_unzip_recv_at != NULL) {
        dp_rsp_free(chanel->m_unzip_recv_at);
        chanel->m_unzip_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.unzip.recv.sp", bpg_zip_chanel_name(chanel));
    chanel->m_unzip_recv_at = dp_rsp_create(gd_app_dp_mgr(chanel->m_app), sp_name_buf);
    if (chanel->m_unzip_recv_at == NULL) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_zip_chanel_set_unzip_recv_at: create rsp fail!",
            bpg_zip_chanel_name(chanel));
        return -1;
    }
    dp_rsp_set_processor(chanel->m_unzip_recv_at, bpg_zip_chanel_unzip_rsp, chanel);

    if (dp_rsp_bind_string(chanel->m_unzip_recv_at, name, chanel->m_em) != 0) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_zip_chanel_set_unzip_recv_at: bind rsp to %s fail!",
            bpg_zip_chanel_name(chanel), name);
        dp_rsp_free(chanel->m_unzip_recv_at);
        chanel->m_unzip_recv_at = NULL;
        return -1;
    }

    return 0;
}

uint32_t bpg_zip_chanel_size_threshold(bpg_zip_chanel_t chanel) {
    return chanel->m_size_threshold;
}

void bpg_zip_chanel_set_size_threshold(bpg_zip_chanel_t chanel, uint32_t threaded) {
    chanel->m_size_threshold = threaded;
}
