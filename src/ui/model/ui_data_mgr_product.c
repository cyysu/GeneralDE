#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "ui/model/ui_data_mgr.h"
#include "ui/model/ui_data_module.h"
#include "ui/model/ui_data_sprite.h"
#include "ui/model/ui_data_action.h"
#include "ui/model/ui_data_layout.h"
#include "ui/model/ui_data_particle.h"
#include "ui_model_internal_types.h"

void ui_data_mgr_product_init(ui_data_mgr_t mgr) {
    struct ui_product_type * product_type;

    bzero(mgr->m_product_types, sizeof(mgr->m_product_types));

    /*module*/
    product_type = mgr->m_product_types + (ui_data_src_type_module - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create = (product_create_fun_t)ui_data_module_create;
    product_type->product_free = (product_free_fun_t)ui_data_module_free;

    /*action*/
    product_type = mgr->m_product_types + (ui_data_src_type_action - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create = (product_create_fun_t)ui_data_action_create;
    product_type->product_free = (product_free_fun_t)ui_data_action_free;

    /*sprite*/
    product_type = mgr->m_product_types + (ui_data_src_type_sprite - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create = (product_create_fun_t)ui_data_sprite_create;
    product_type->product_free = (product_free_fun_t)ui_data_sprite_free;

    /*layout*/
    product_type = mgr->m_product_types + (ui_data_src_type_layout - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create = (product_create_fun_t)ui_data_layout_create;
    product_type->product_free = (product_free_fun_t)ui_data_layout_free;

    /*particle*/
    product_type = mgr->m_product_types + (ui_data_src_type_particle - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create = (product_create_fun_t)ui_data_particle_create;
    product_type->product_free = (product_free_fun_t)ui_data_particle_free;
}

void ui_data_mgr_set_loader(ui_data_mgr_t mgr, ui_data_src_type_t type, product_load_fun_t loader, void * ctx) {
    struct ui_product_type * product_type;
    
    assert(type >= UI_DATA_SRC_TYPE_MIN && (uint8_t)type < UI_DATA_SRC_TYPE_MAX);

    product_type = mgr->m_product_types + (type - UI_DATA_SRC_TYPE_MIN);

    product_type->product_load_ctx = ctx;
    product_type->product_load = loader;
}

void ui_data_mgr_set_saver(ui_data_mgr_t mgr, ui_data_src_type_t type, product_save_fun_t saver, product_remove_fun_t remover, void * ctx) {
    struct ui_product_type * product_type;
    
    assert(type >= UI_DATA_SRC_TYPE_MIN && (uint8_t)type < UI_DATA_SRC_TYPE_MAX);

    product_type = mgr->m_product_types + (type - UI_DATA_SRC_TYPE_MIN);

    product_type->product_save_ctx = ctx;
    product_type->product_save = saver;
    product_type->product_remove = remover;
}
