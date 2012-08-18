#include "zlib.h"
#include "cpe/dp/dp_request.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_zip/bpg_zip_chanel.h"
#include "bpg_zip_internal_types.h"

int bpg_zip_chanel_zip_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    struct bpg_zip_chanel * chanel;
    bpg_pkg_t pkg;
    void * input_buf;
    size_t input_size;
    size_t output_size;
    uint32_t old_flags;
    uint32_t flags;

    chanel = (struct bpg_zip_chanel *)ctx;

    pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        if (bpg_pkg_dsp_pass(chanel->m_zip_send_to, req, em) != 0) {
            CPE_ERROR(
                em, "%s: zip: forward %s pkg fail!",
                bpg_zip_chanel_name(chanel), dp_req_type(req));
            return -1;
        }
        else {
            return 0;
        }
    }

    old_flags = bpg_pkg_flags(pkg);
    if (old_flags & (1 << chanel->m_mask_bit)) {
        if (chanel->m_debug) {
            CPE_INFO(
                em, "%s: zip: compress ignore, flags=%d, bit %d alread set!",
                bpg_zip_chanel_name(chanel), old_flags, chanel->m_mask_bit);
        }
        goto DISPATCH;
    }

    input_size = bpg_pkg_body_total_len(pkg);
    if (mem_buffer_set_size(&chanel->m_data_buf, input_size) != 0
        || (input_buf = mem_buffer_make_continuous(&chanel->m_data_buf, 0)) == NULL)
    {
        CPE_ERROR(
            em, "%s: zip: alloc buf fail, size=%d!",
            bpg_zip_chanel_name(chanel), (int)input_size);
        return -1;
    }

    if (input_size < chanel->m_size_threshold) {
        if (chanel->m_debug) {
            CPE_INFO(
                em, "%s: zip: compress ignore, size=%d, threaded=%d!",
                bpg_zip_chanel_name(chanel), (int)input_size, (int)chanel->m_size_threshold);
        }
        goto DISPATCH;
    }

    memcpy(input_buf, bpg_pkg_body_data(pkg), input_size);

    output_size = bpg_pkg_pkg_data_capacity(pkg);
    if (compress(
            bpg_pkg_body_data(pkg), &output_size, 
            input_buf, input_size) != 0)
    {
        CPE_ERROR(
            em, "%s: zip: compress fail, output_size=%d, input_size=%d!",
            bpg_zip_chanel_name(chanel), (int)output_size, (int)input_size);
        return -1;
    }

    bpg_pkg_set_body_total_len(pkg, output_size);

    flags = old_flags | (1 << chanel->m_mask_bit);
    bpg_pkg_set_flags(pkg, flags);

    if (chanel->m_debug) {
        CPE_INFO(
            em, "%s: zip: compress success, size: %d ==> %d, flags 0x%x ==> 0x%x!",
            bpg_zip_chanel_name(chanel), (int)input_size, (int)output_size, old_flags, flags);
    }

DISPATCH:
    if (bpg_pkg_dsp_dispatch(chanel->m_zip_send_to, pkg, chanel->m_em) != 0) {
        CPE_ERROR(
            em, "%s: zip: compress success, dispatch fail!",
            bpg_zip_chanel_name(chanel));
        return -1;
    }

    return 0;
}

int bpg_zip_chanel_unzip_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    struct bpg_zip_chanel * chanel;
    bpg_pkg_t pkg;
    void * input_buf;
    size_t input_size;
    size_t output_size;
    uint32_t old_flags;
    uint32_t flags;

    chanel = (struct bpg_zip_chanel *)ctx;

    pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        if (bpg_pkg_dsp_pass(chanel->m_unzip_send_to, req, em) != 0) {
            CPE_ERROR(
                em, "%s: unzip: forward %s pkg fail!",
                bpg_zip_chanel_name(chanel), dp_req_type(req));
            return -1;
        }
        else {
            return 0;
        }
    }

    old_flags = bpg_pkg_flags(pkg);
    if (!(old_flags & (1 << chanel->m_mask_bit))) {
        if (chanel->m_debug) {
            CPE_INFO(
                em, "%s: unzip: ignore, flags=%d, bit %d not set!",
                bpg_zip_chanel_name(chanel), old_flags, chanel->m_mask_bit);
        }
        goto DISPATCH;
    }

    input_size = bpg_pkg_body_total_len(pkg);
    if (mem_buffer_set_size(&chanel->m_data_buf, input_size) != 0
        || (input_buf = mem_buffer_make_continuous(&chanel->m_data_buf, 0)) == NULL)
    {
        CPE_ERROR(
            em, "%s: unzip: alloc buf fail, size=%d!",
            bpg_zip_chanel_name(chanel), (int)input_size);
        return -1;
    }

    memcpy(input_buf, bpg_pkg_body_data(pkg), input_size);

    output_size = bpg_pkg_pkg_data_capacity(pkg);
    if (uncompress(
            bpg_pkg_body_data(pkg), &output_size, 
            input_buf, input_size) != 0)
    {
        CPE_ERROR(
            em, "%s:J unzip: uncompress fail, output_size=%d, input_size=%d!",
            bpg_zip_chanel_name(chanel), (int)output_size, (int)input_size);
        return -1;
    }

    bpg_pkg_set_body_total_len(pkg, output_size);

    flags = old_flags & (~ (1 << chanel->m_mask_bit));
    bpg_pkg_set_flags(pkg, flags);

    if (chanel->m_debug) {
        CPE_INFO(
            em, "%s: unzip: uncompress success, size: %d ==> %d, flags 0x%x ==> 0x%x!",
            bpg_zip_chanel_name(chanel), (int)input_size, (int)output_size, old_flags, flags);
    }

DISPATCH:
    if (bpg_pkg_dsp_dispatch(chanel->m_unzip_send_to, pkg, chanel->m_em) != 0) {
        CPE_ERROR(
            em, "%s: unzip: uncompress success, dispatch fail!",
            bpg_zip_chanel_name(chanel));
        return -1;
    }

    return 0;
}
