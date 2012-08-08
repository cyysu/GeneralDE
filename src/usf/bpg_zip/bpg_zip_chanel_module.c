#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cvt.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_zip/bpg_zip_chanel.h"
#include "bpg_zip_internal_types.h"

EXPORT_DIRECTIVE
int bpg_zip_chanel_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_zip_chanel_t bpg_zip_chanel;
    const char * zip_recv_at;
    cfg_t zip_send_to;
    const char * unzip_recv_at;
    cfg_t unzip_send_to;

    zip_recv_at = cfg_get_string(cfg, "zip-recv-at", NULL);
    zip_send_to = cfg_find_cfg(cfg, "zip-send-to");
    unzip_recv_at = cfg_get_string(cfg, "unzip-recv-at", NULL);
    unzip_send_to = cfg_find_cfg(cfg, "unzip-send-to");

    if (zip_send_to && zip_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: zip-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (zip_recv_at && zip_send_to == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: zip-send-to not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (unzip_send_to && unzip_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: unzip-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (unzip_recv_at && unzip_send_to == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: unzip-send-to not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (zip_recv_at == NULL && unzip_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: not configure any operation!",
            gd_app_module_name(module));
        return -1;
    }

    bpg_zip_chanel =
        bpg_zip_chanel_create(
            app, gd_app_module_name(module),
            gd_app_em(app));
    if (bpg_zip_chanel == NULL) return -1;

    if (zip_recv_at && bpg_zip_chanel_set_zip_recv_at(bpg_zip_chanel, zip_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set zip-recv-at %s fail!",
            gd_app_module_name(module),
            zip_recv_at);
        bpg_zip_chanel_free(bpg_zip_chanel);
        return -1;
    }

    if (zip_send_to && bpg_zip_chanel_set_zip_send_to(bpg_zip_chanel, zip_send_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set zip-send-to fail!",
            gd_app_module_name(module));
        bpg_zip_chanel_free(bpg_zip_chanel);
        return -1;
    }

    if (unzip_recv_at && bpg_zip_chanel_set_unzip_recv_at(bpg_zip_chanel, unzip_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set unzip-recv-at %s fail!",
            gd_app_module_name(module),
            unzip_recv_at);
        bpg_zip_chanel_free(bpg_zip_chanel);
        return -1;
    }

    if (unzip_send_to && bpg_zip_chanel_set_unzip_send_to(bpg_zip_chanel, unzip_send_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set unzip-send-to fail!",
            gd_app_module_name(module));
        bpg_zip_chanel_free(bpg_zip_chanel);
        return -1;
    }

    bpg_zip_chanel->m_mask_bit = cfg_get_int32(cfg, "mask-bit", 128);
    if (bpg_zip_chanel->m_mask_bit >= sizeof(uint32_t) * 8) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: mask-bit not configured!",
            gd_app_module_name(module));
        bpg_zip_chanel_free(bpg_zip_chanel);
    }

    bpg_zip_chanel->m_size_threshold = cfg_get_uint32(cfg, "size-threshold", bpg_zip_chanel->m_size_threshold);

    bpg_zip_chanel->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (bpg_zip_chanel->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done, zip(%s), unzip(%s), mask-bit=%d .",
            gd_app_module_name(module), zip_recv_at ? "ok" : "none", unzip_recv_at ? "ok" : "none", bpg_zip_chanel->m_debug);
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_zip_chanel_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_zip_chanel_t bpg_zip_chanel;

    bpg_zip_chanel = bpg_zip_chanel_find_nc(app, gd_app_module_name(module));
    if (bpg_zip_chanel) {
        bpg_zip_chanel_free(bpg_zip_chanel);
    }
}
