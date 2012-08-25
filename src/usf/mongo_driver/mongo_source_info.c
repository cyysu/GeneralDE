#include <assert.h>
#include "cpe/dp/dp_responser.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

struct mongo_source_info *
mongo_source_info_create(mongo_driver_t driver, int32_t source, const char * incoming_dispatch_to, const char * outgoing_recv_at) {
    size_t dispatch_to_len = cpe_hs_len_to_binary_len(strlen(incoming_dispatch_to));
    struct mongo_source_info * source_info;
    char name_buf[128];

    source_info = mem_alloc(driver->m_alloc, sizeof(struct mongo_source_info) + dispatch_to_len);
    if (source_info == NULL) return NULL;

    cpe_hs_init((cpe_hash_string_t)(source_info + 1), dispatch_to_len, incoming_dispatch_to);

    source_info->m_driver = driver;
    source_info->m_source = source;
    source_info->m_incoming_dsp_to = (cpe_hash_string_t)(source_info + 1);

    cpe_hash_entry_init(&source_info->m_hh);
    if (!cpe_hash_table_insert_unique(&driver->m_source_infos, source_info) != 0) {
        CPE_ERROR(driver->m_em, "%s: create source info: source %d duplicate!", mongo_driver_name(driver), source);
        mem_free(driver->m_alloc, source_info);
        return NULL;
    }
    
    snprintf(name_buf, sizeof(name_buf), "%s.%d.send-rsp", mongo_driver_name(driver), source);
    source_info->m_outgoing_rsp = dp_rsp_create(gd_app_dp_mgr(driver->m_app), name_buf);
    if (source_info->m_outgoing_rsp == NULL) {
        CPE_ERROR(driver->m_em, "%s: create source info: create send rsp %s fail!", mongo_driver_name(driver), name_buf);
        cpe_hash_table_remove_by_ins(&driver->m_source_infos, source_info);
        mem_free(driver->m_alloc, source_info);
        return NULL;
    }
    dp_rsp_set_processor(source_info->m_outgoing_rsp, mongo_driver_send, source_info);

    return source_info;
}

void mongo_source_info_free(struct mongo_source_info * source_info) {
    dp_rsp_free(source_info->m_outgoing_rsp);
    source_info->m_outgoing_rsp = NULL;

    cpe_hash_table_remove_by_ins(&source_info->m_driver->m_source_infos, source_info);
    mem_free(source_info->m_driver->m_alloc, source_info);
}

uint32_t mongo_source_info_hash(const struct mongo_source_info * source_info) {
    return source_info->m_source;
}

int mongo_source_info_eq(const struct mongo_source_info * l, const struct mongo_source_info * r) {
    return l->m_source == r->m_source;
}

void mongo_source_info_free_all(mongo_driver_t driver) {
    struct cpe_hash_it source_info_it;
    struct mongo_source_info * source_info;

    cpe_hash_it_init(&source_info_it, &driver->m_source_infos);

    source_info = cpe_hash_it_next(&source_info_it);
    while(source_info) {
        struct mongo_source_info * next = cpe_hash_it_next(&source_info_it);
        mongo_source_info_free(source_info);
        source_info = next;
    }
}
