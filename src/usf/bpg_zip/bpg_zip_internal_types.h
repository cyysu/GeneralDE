#ifndef USF_BPG_ZIP_INTERNAL_TYPES_H
#define USF_BPG_ZIP_INTERNAL_TYPES_H
#include "usf/bpg_pkg/bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_zip_chanel {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    dp_rsp_t m_zip_recv_at;
    bpg_pkg_dsp_t m_zip_send_to;

    dp_rsp_t m_unzip_recv_at;
    bpg_pkg_dsp_t m_unzip_send_to;

    uint32_t m_size_threshold;
    uint8_t m_mask_bit;

    struct mem_buffer m_data_buf;
    
    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
