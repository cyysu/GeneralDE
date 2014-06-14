#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_module_i.h"
#include "ui_data_src_i.h"

ui_data_module_t ui_data_module_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_module_t module;

    if (src->m_type != ui_data_src_type_module) {
        CPE_ERROR(
            mgr->m_em, "create module at %s: src not module!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create module at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    module = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_module));
    if (module == NULL) {
        CPE_ERROR(
            mgr->m_em, "create module at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    module->m_mgr = mgr;
    module->m_src = src;
    TAILQ_INIT(&module->m_img_blocks);
    bzero(&module->m_data, sizeof(module->m_data));

    src->m_product = module;

    return module;
}

void ui_data_module_free(ui_data_module_t module) {
    ui_data_mgr_t mgr = module->m_mgr;

    while(!TAILQ_EMPTY(&module->m_img_blocks)) {
        ui_data_img_block_free(TAILQ_FIRST(&module->m_img_blocks));
    }

    assert(module->m_src->m_product == module);
    module->m_src->m_product = NULL;

    mem_free(mgr->m_alloc, module);
}

UI_MODULE * ui_data_module_data(ui_data_module_t module) {
    return &module->m_data;
}

LPDRMETA ui_data_module_meta(ui_data_mgr_t data_mgr) {
    return data_mgr->m_meta_module;
}
