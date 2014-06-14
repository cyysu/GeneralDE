#ifndef UI_MODEL_INTERNAL_TYPES_H
#define UI_MODEL_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "ui/model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*product_free_fun_t)(void *);
typedef void * (*product_create_fun_t)(ui_data_mgr_t mgr, ui_data_src_t src);

struct ui_product_type {
    product_create_fun_t product_create;
    product_free_fun_t product_free;
    void * product_load_ctx;
    product_load_fun_t product_load;
    void * product_save_ctx;
    product_save_fun_t product_save;
    product_remove_fun_t product_remove;
};

struct ui_data_mgr {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    struct ui_product_type m_product_types[UI_DATA_SRC_TYPE_MAX - UI_DATA_SRC_TYPE_MIN];

    struct mem_buffer m_dump_buffer;

    LPDRMETA m_meta_module;
    LPDRMETA m_meta_img_block;
    LPDRMETA m_meta_frame;
    LPDRMETA m_meta_frame_img;
    LPDRMETA m_meta_actor;
    LPDRMETA m_meta_actor_layer;
    LPDRMETA m_meta_actor_frame;
    LPDRMETA m_meta_particle_emitter;
    LPDRMETA m_meta_particle_mod;
    LPDRMETA m_meta_controls[UI_DATA_CONTROL_TYPE_MAX - UI_DATA_CONTROL_TYPE_MIN];

    /*src相关数据 */
    ui_data_src_t m_src_root;
    struct cpe_hash_table m_srcs;
    struct cpe_hash_table m_srcs_by_id;

    /*module相关数据 */
    struct cpe_hash_table m_img_blocks;

    /*layout相关数据 */
    struct cpe_hash_table m_controls_by_id;
};

#define ui_product_type_of(__mgr, __type) ((__mgr)->m_product_types + ((__type) - UI_DATA_SRC_TYPE_MIN))

#ifdef __cplusplus
}
#endif

#endif
