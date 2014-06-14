#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/file.h"
#include "ui/model/ui_data_mgr.h"
#include "ui/model/ui_data_src.h"
#include "ui/model_np/ui_model_np.h"

#define ui_data_np_add_processing_src(src) \
    if (processing_src_count + 1 >= processing_src_capacity) { \
        uint32_t new_capacity = processing_src_capacity ? processing_src_capacity * 2 : 128; \
        ui_data_src_t * new_buf = mem_alloc(NULL, sizeof(processing_srcs[0]) * new_capacity); \
        if (new_buf == NULL) {                                          \
            CPE_ERROR(em, "ui_data_np_save: alloc buf fail, capacity = %d", new_capacity); \
            goto SAVE_COMPLETE;                                         \
        }                                                               \
        if (processing_srcs) {                                          \
            memcpy(new_buf, processing_srcs, sizeof(processing_srcs[0]) * processing_src_count); \
            mem_free(NULL, processing_srcs);                            \
        }                                                               \
        processing_srcs = new_buf;                                      \
        processing_src_capacity = new_capacity;                         \
    }                                                                   \
    processing_srcs[processing_src_count++] = src

int ui_data_np_save(ui_data_mgr_t mgr, const char * dir, error_monitor_t em) {
    ui_data_src_t * processing_srcs = NULL;
    uint32_t processing_src_capacity = 0;
    uint32_t processing_src_count = 0;
    ui_data_src_t root = ui_data_mgr_src_root(mgr);
    struct mem_buffer path_buff;

    mem_buffer_init(&path_buff, NULL);

    if (dir == NULL) dir = ui_data_src_data(root);

    ui_data_init_np_saver(mgr);

    ui_data_np_add_processing_src(root);

    while(processing_src_count > 0) {
        ui_data_src_t src = processing_srcs[--processing_src_count];

        switch(ui_data_src_type(src)) {
        case ui_data_src_type_dir: {
            struct ui_data_src_it src_it;
            ui_data_src_t child;

            ui_data_src_childs(&src_it, src);
            while((child = ui_data_src_it_next(&src_it))) {
                ui_data_np_add_processing_src(child);
            }
            break;
        }
        case ui_data_src_type_module:
        case ui_data_src_type_sprite:
        case ui_data_src_type_action:
        case ui_data_src_type_layout:
        case ui_data_src_type_particle:
            if (ui_data_src_load_state(src) == ui_data_src_state_loaded) {
                ui_data_src_save(src, dir, em);
            }
            break;
        default:
            break;
        }
    }

SAVE_COMPLETE:
    if (processing_srcs) {
        mem_free(NULL, processing_srcs);
    }

    mem_buffer_clear(&path_buff);

    return 0;
}

extern int ui_data_np_save_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_np_save_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_np_save_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_np_save_layout(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_np_save_particle(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
extern int ui_data_np_remove_file(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

int ui_data_init_np_saver(ui_data_mgr_t mgr) {
    ui_data_mgr_set_saver(mgr, ui_data_src_type_module, ui_data_np_save_module, ui_data_np_remove_file, NULL);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_sprite, ui_data_np_save_sprite, ui_data_np_remove_file, NULL);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_action, ui_data_np_save_action, ui_data_np_remove_file, NULL);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_layout, ui_data_np_save_layout, ui_data_np_remove_file, NULL);
    ui_data_mgr_set_saver(mgr, ui_data_src_type_particle, ui_data_np_save_particle, ui_data_np_remove_file, NULL);
    return 0;
}
