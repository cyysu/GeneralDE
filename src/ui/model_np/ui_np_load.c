#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "ui/model/ui_data_mgr.h"
#include "ui/model/ui_data_src.h"
#include "ui/model_np/ui_model_np.h"

static dir_visit_next_op_t ui_data_np_load_on_file(const char * full, const char * base, void * ctx);

struct ui_data_np_load_ctx {
    ui_data_mgr_t m_mgr;
    int m_load_product;
    error_monitor_t m_em;
    struct mem_buffer m_buffer;
};

int ui_data_np_load(ui_data_mgr_t mgr, const char * dir, int load_product, error_monitor_t em) {
    struct dir_visitor dir_walker;
    struct ui_data_np_load_ctx ctx;
    int ret = 0;

    ui_data_init_np_loader(mgr);

    dir_walker.on_dir_enter = NULL;
    dir_walker.on_dir_leave = NULL;
    dir_walker.on_file = ui_data_np_load_on_file;

    ctx.m_mgr = mgr;
    ctx.m_load_product = load_product;
    mem_buffer_init(&ctx.m_buffer, NULL);

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);

        ctx.m_em = em;
        dir_search(&dir_walker, &ctx, dir, 20, em, NULL);

        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);

        ctx.m_em = &logError;
        dir_search(&dir_walker, &ctx, dir, 20, &logError, NULL);
    }

    mem_buffer_clear(&ctx.m_buffer);

    return ret;
}

static int ui_data_np_load_meta_info(ui_data_src_t src, const char * full, struct ui_data_np_load_ctx * ctx) {
    const char * meta_file_name;
    FILE * meta_file;
    char data_buf[64];
    ssize_t data_len;
    uint32_t id;

    mem_buffer_clear_data(&ctx->m_buffer);

    if (mem_buffer_strcat(&ctx->m_buffer, full) != 0
        || mem_buffer_strcat(&ctx->m_buffer, ".meta") != 0
        || (meta_file_name = mem_buffer_make_continuous(&ctx->m_buffer, 0)) == NULL)
    {
        CPE_ERROR(ctx->m_em, "build meta file path fail!");
        return -1;
    }

    meta_file = file_stream_open(meta_file_name, "r", NULL);
    if (meta_file == NULL) {
        if (errno == ENOENT) {
            CPE_INFO(ctx->m_em, "meta file %s not exist!", meta_file_name);
            return 0;
        }
        else {
            CPE_ERROR(
                ctx->m_em, "open meta file %s fail, error=%d (%s)!",
                meta_file_name, errno, strerror(errno));
            return -1;
        }
    }
    
    data_len = file_stream_load_to_buf(data_buf, sizeof(data_buf), meta_file, NULL);
    if (data_len < 0) {
        CPE_ERROR(
            ctx->m_em, "read meta file %s fail, error=%d (%s)!",
            meta_file_name, errno, strerror(errno));
        file_stream_close(meta_file, NULL);
        return -1;
    }
    data_buf[data_len] = 0;
    
    sscanf(data_buf, FMT_UINT32_T, &id);

    if (ui_data_src_set_id(src, id) != 0) {
        CPE_ERROR(ctx->m_em, "set src id %d fail!", id);
        file_stream_close(meta_file, NULL);
        return -1;
    }

    file_stream_close(meta_file, NULL);
    return 0;
}

static dir_visit_next_op_t ui_data_np_load_on_file(const char * full, const char * base, void * input_ctx) {
    struct ui_data_np_load_ctx * ctx = input_ctx;
    ui_data_src_t src;

    const char * suffix = file_name_suffix(base);

    CPE_ERROR_SET_FILE(ctx->m_em, full);
    CPE_ERROR_SET_LINE(ctx->m_em, 0);

    if (strcmp(suffix, "npModule") == 0) {
        if ((src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_module, full))) {
            if (ui_data_np_load_meta_info(src, full, ctx) != 0) {
                ui_data_src_free(src);
                src = NULL;
            }
        }
    }
    else if (strcmp(suffix, "npSprite") == 0) {
        if ((src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_sprite, full))) {
            if (ui_data_np_load_meta_info(src, full, ctx) != 0) {
                ui_data_src_free(src);
                src = NULL;
            }
        }
    }
    else if (strcmp(suffix, "npAction") == 0) {
        if ((src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_action, full))) {
            if (ui_data_np_load_meta_info(src, full, ctx) != 0) {
                ui_data_src_free(src);
                src = NULL;
            }
        }
    }
    else if (strcmp(suffix, "npLayout") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_layout, full);
    }
    else if (strcmp(suffix, "particle") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_particle, full);
    }
    else {
        src = NULL;
    }

    if (ctx->m_load_product && src) {
        ui_data_src_load(src, ctx->m_em);
    }

    CPE_ERROR_SET_FILE(ctx->m_em, NULL);
    CPE_ERROR_SET_LINE(ctx->m_em, -1);

    return dir_visit_next_go;
}

extern int ui_data_np_load_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_np_load_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_np_load_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_np_load_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
extern int ui_data_np_load_particle(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);

int ui_data_init_np_loader(ui_data_mgr_t mgr) {
    ui_data_mgr_set_loader(mgr, ui_data_src_type_module, ui_data_np_load_module, NULL);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_sprite, ui_data_np_load_sprite, NULL);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_action, ui_data_np_load_action, NULL);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_layout, ui_data_np_load_layout, NULL);
    ui_data_mgr_set_loader(mgr, ui_data_src_type_particle, ui_data_np_load_particle, NULL);
    return 0;
}
