#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "protocol/base/base_package.h"
#include "bpg_pkg_internal_types.h"

bpg_pkg_t
bpg_pkg_create(
    bpg_pkg_manage_t mgr,
    size_t pkg_capacity,
    LPDRMETA carry_data_meta,
    size_t carry_data_capacity)
{
    dp_req_t dp_req;
    bpg_pkg_t bpg_pkg;

    if (pkg_capacity < sizeof (BASEPKG)) return NULL;

    dp_req = dp_req_create(
        gd_app_dp_mgr(mgr->m_app),
        bpg_pkg_type_name,
        sizeof(struct bpg_pkg) + carry_data_capacity + pkg_capacity);
    if (dp_req == NULL) return NULL;

    bzero(dp_req_data(dp_req), dp_req_capacity(dp_req));

    bpg_pkg = (bpg_pkg_t)dp_req_data(dp_req);

    bpg_pkg->m_mgr = mgr;
    bpg_pkg->m_connection_id = BPG_INVALID_CONNECTION_ID;
    bpg_pkg->m_carry_data_meta = carry_data_meta;
    bpg_pkg->m_carry_data_size = 0;
    bpg_pkg->m_carry_data_capacity = carry_data_capacity;
    bpg_pkg->m_dp_req = dp_req;

    bpg_pkg_init(bpg_pkg);

    return bpg_pkg;
}

void bpg_pkg_free(bpg_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

uint32_t bpg_pkg_connection_id(bpg_pkg_t pkg) {
    return pkg->m_connection_id;
}

void bpg_pkg_set_connection_id(bpg_pkg_t pkg, uint32_t connection_id) {
    pkg->m_connection_id = connection_id;
}

LPDRMETA bpg_pkg_carry_data_meta(bpg_pkg_t req) {
    return req->m_carry_data_meta;
}

void * bpg_pkg_carry_data(bpg_pkg_t req) {
    return req + 1;
}

size_t bpg_pkg_carry_data_capacity(bpg_pkg_t req) {
    return req->m_carry_data_capacity;
}

size_t bpg_pkg_carry_data_size(bpg_pkg_t req) {
    return req->m_carry_data_size;
}

int bpg_pkg_carry_data_set_size(bpg_pkg_t req, size_t size) {
    if (size > req->m_carry_data_capacity) return -1;
    req->m_carry_data_size = size;
    return 0;
}

bpg_pkg_manage_t bpg_pkg_mgr(bpg_pkg_t req) {
    return req->m_mgr;
}

size_t bpg_pkg_pkg_data_capacity(bpg_pkg_t req) {
    return dp_req_capacity(req->m_dp_req) - sizeof(struct bpg_pkg) - req->m_carry_data_capacity;
}

void * bpg_pkg_pkg_data(bpg_pkg_t req) {
    return ((char *)(req + 1)) + req->m_carry_data_capacity;
}

size_t bpg_pkg_pkg_data_size(bpg_pkg_t req) {
    return dp_req_size(req->m_dp_req) - sizeof(struct bpg_pkg) - req->m_carry_data_capacity;
}

int bpg_pkg_pkg_data_set_size(bpg_pkg_t req, size_t size) {
    size_t total_size = size + sizeof(struct bpg_pkg) + req->m_carry_data_capacity;
    if (total_size > dp_req_capacity(req->m_dp_req)) return -1;
    return dp_req_set_size(req->m_dp_req, total_size);
}

void bpg_pkg_init(bpg_pkg_t bpg_pkg) {
    BASEPKG_HEAD * head;
    size_t old_data_size;

    old_data_size = dp_req_size(bpg_pkg->m_dp_req);
    if (old_data_size > sizeof(struct bpg_pkg)) {
        bzero(bpg_pkg + 1, old_data_size - sizeof(struct bpg_pkg));
    }

    bpg_pkg_pkg_data_set_size(bpg_pkg, sizeof(BASEPKG_HEAD));

    bpg_pkg->m_connection_id = BPG_INVALID_CONNECTION_ID;

    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(bpg_pkg);
}

void bpg_pkg_clear_data(bpg_pkg_t bpg_pkg) {
    BASEPKG_HEAD * head;

    bpg_pkg_pkg_data_set_size(bpg_pkg, sizeof(BASEPKG_HEAD));

    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(bpg_pkg);
    bzero(head, sizeof(*head));
    head->headlen = sizeof(BASEPKG_HEAD);
}

uint32_t bpg_pkg_cmd(bpg_pkg_t req) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    return head->cmd;
}

void bpg_pkg_set_cmd(bpg_pkg_t req, uint32_t cmd) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    head->cmd = cmd;
}

uint32_t bpg_pkg_sn(bpg_pkg_t req) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    return head->sn;
}

void bpg_pkg_set_sn(bpg_pkg_t req, uint32_t sn) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    head->sn = sn;
}

uint32_t bpg_pkg_flags(bpg_pkg_t req) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    return head->flags;
}

int bpg_pkg_flag_enable(bpg_pkg_t pkg, bpg_pkg_flag_t flag) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);

    return ((head->flags & (uint32_t)flag) == (uint32_t)flag) ? 1 : 0;
}

void bpg_pkg_flag_set_enable(bpg_pkg_t pkg, bpg_pkg_flag_t flag, int is_enable) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);

    if (is_enable) {
        head->flags |= (uint32_t)flag;
    }
    else {
        head->flags &= ~(uint32_t)flag;
    }
}

void bpg_pkg_set_flags(bpg_pkg_t req, uint32_t flags) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    head->flags = flags;
}

uint32_t bpg_pkg_errno(bpg_pkg_t req) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    return head->errorNo;
}

void bpg_pkg_set_errno(bpg_pkg_t req, uint32_t errorNo) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    head->errorNo = errorNo;
}

dp_req_t bpg_pkg_to_dp_req(bpg_pkg_t req) {
    return req->m_dp_req;
}

uint64_t bpg_pkg_client_id(bpg_pkg_t req) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    return head->clientId;
}

void bpg_pkg_set_client_id(bpg_pkg_t req, uint64_t client_id) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(req);

    head->clientId = client_id;
}


bpg_pkg_t bpg_pkg_from_dp_req(dp_req_t req) {
    if (cpe_hs_cmp(dp_req_type_hs(req), bpg_pkg_type_name) != 0) return NULL;
    return (bpg_pkg_t)dp_req_data(req);
}

dr_cvt_t bpg_pkg_data_cvt(bpg_pkg_t pkg) {
    return pkg->m_mgr->m_data_cvt;
}

dr_cvt_t bpg_pkg_base_cvt(bpg_pkg_t pkg) {
    return pkg->m_mgr->m_base_cvt;
}

LPDRMETALIB bpg_pkg_data_meta_lib(bpg_pkg_t pkg) {
    return dr_ref_lib(pkg->m_mgr->m_metalib_ref);
}

LPDRMETA bpg_pkg_base_meta(bpg_pkg_t pkg) {
    LPDRMETALIB metalib;

    metalib = dr_ref_lib(pkg->m_mgr->m_metalib_basepkg_ref);
    return metalib ? dr_lib_find_meta_by_name(metalib, "basepkg") : NULL;
}

LPDRMETA bpg_pkg_main_data_meta(bpg_pkg_t pkg, error_monitor_t em) {
    return bpg_pkg_manage_find_meta_by_cmd(pkg->m_mgr, bpg_pkg_cmd(pkg));
}

LPDRMETA bpg_pkg_append_data_meta(bpg_pkg_t pkg, bpg_pkg_append_info_t append_info, error_monitor_t em) {
    LPDRMETALIB metalib;
    LPDRMETA data_meta;

    metalib = bpg_pkg_manage_data_metalib(pkg->m_mgr);
    if (metalib == NULL) {
        CPE_ERROR(
            em, "%s: bpg_pkg_append_data_meta:  data meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return NULL;
    }

    data_meta = dr_lib_find_meta_by_id(metalib, bpg_pkg_append_info_id(append_info));
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "%s: bpg_pkg_append_data_meta:  meta of id %d not exist in lib %s!",
            bpg_pkg_manage_name(pkg->m_mgr), bpg_pkg_append_info_id(append_info), dr_lib_name(metalib));
        return NULL;
    }

    return data_meta;
}

uint32_t bpg_pkg_body_total_len(bpg_pkg_t pkg) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);
    return head->bodytotallen;
}

void bpg_pkg_set_body_total_len(bpg_pkg_t pkg, uint32_t totallen) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);
    head->bodytotallen = totallen;

    bpg_pkg_pkg_data_set_size(pkg, sizeof(BASEPKG_HEAD) + totallen);
}

void * bpg_pkg_body_data(bpg_pkg_t pkg) {
    return ((char *)bpg_pkg_pkg_data(pkg)) + sizeof(BASEPKG_HEAD);
}
    
uint32_t bpg_pkg_body_len(bpg_pkg_t pkg) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);

    return head->bodylen;
}

int32_t bpg_pkg_append_info_count(bpg_pkg_t pkg) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);
    return head->appendInfoCount;
}

bpg_pkg_append_info_t bpg_pkg_append_info_at(bpg_pkg_t pkg, int32_t pos) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);

    return (pos >= 0 && pos < head->appendInfoCount)
        ? (bpg_pkg_append_info_t)(&head->appendInfos[pos])
        : NULL;
}

uint32_t bpg_pkg_append_info_id(bpg_pkg_append_info_t append_info) {
    return ((APPENDINFO *)append_info)->id;
}

uint32_t bpg_pkg_append_info_size(bpg_pkg_append_info_t append_info) {
    return ((APPENDINFO *)append_info)->size;
}

bpg_pkg_debug_level_t bpg_pkg_debug_level(bpg_pkg_t req) {
    return bpg_pkg_manage_debug_level(req->m_mgr, bpg_pkg_cmd(req));
}

const char * bpg_pkg_dump(bpg_pkg_t req, mem_buffer_t buffer) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    LPDRMETALIB metalib;
    LPDRMETA meta;
    BASEPKG_HEAD * head;
    const char * data;
    int i;

    mem_buffer_clear_data(buffer);

    data = bpg_pkg_pkg_data(req);

    head = (BASEPKG_HEAD *)data;
    
    stream_printf(((write_stream_t)&stream), "head: ");

    metalib = dr_ref_lib(req->m_mgr->m_metalib_basepkg_ref);
    if ((meta = metalib ? dr_lib_find_meta_by_name(metalib, "basepkg_head") : NULL)) {
        dr_json_print((write_stream_t)&stream, head, sizeof(BASEPKG_HEAD), meta, DR_JSON_PRINT_MINIMIZE, 0);
    }
    else {
        stream_printf((write_stream_t)&stream, "[no meta] cmd=%d", head->cmd);
    }
    data += sizeof(BASEPKG_HEAD);

    stream_printf(((write_stream_t)&stream), "\nbody: ");
    if (head->bodylen > 0) {
        if ((meta = bpg_pkg_main_data_meta(req, NULL))) {
            stream_printf(((write_stream_t)&stream), " %s", dr_meta_name(meta));
            dr_json_print((write_stream_t)&stream, data, head->bodylen, meta, DR_JSON_PRINT_MINIMIZE, 0);
        }
        else {
            stream_printf((write_stream_t)&stream, "[no meta] bodylen=%d", head->bodylen);
        }

        data += head->bodylen;
    }
    else {
        stream_printf((write_stream_t)&stream, "[no data]");
    }

    metalib = dr_ref_lib(req->m_mgr->m_metalib_ref);
    for(i = 0; i < head->appendInfoCount; ++i) {
        APPENDINFO const * append_info = &head->appendInfos[i];

        if ((meta = metalib ? dr_lib_find_meta_by_id(metalib, append_info->id) : NULL)) {
            stream_printf((write_stream_t)&stream, "\nappend %d(%s): ", append_info->id, dr_meta_name(meta));

            dr_json_print((write_stream_t)&stream, data, append_info->size, meta, DR_JSON_PRINT_MINIMIZE, 0);
        }
        else {
            stream_printf(
                (write_stream_t)&stream, "\nappend [no meta]: id=%d, size=%d, origin-size=%d", 
                append_info->id, append_info->size, append_info->size);
        }

        data += append_info->size;
    }

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}


CPE_HS_DEF_VAR(bpg_pkg_type_name, "bpg_pkg_type");
