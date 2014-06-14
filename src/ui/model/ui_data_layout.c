#include <assert.h>
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_layout_t ui_data_layout_create(ui_data_mgr_t mgr, ui_data_src_t src) {
    ui_data_layout_t layout;

    if (src->m_type != ui_data_src_type_layout) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: src not layout!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    if (src->m_product) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: product already loaded!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    layout = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_layout));
    if (layout == NULL) {
        CPE_ERROR(
            mgr->m_em, "create layout at %s: alloc fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return NULL;
    }

    layout->m_mgr = mgr;
    layout->m_src = src;
    layout->m_root = NULL;

    src->m_product = layout;

    return layout;
}

void ui_data_layout_free(ui_data_layout_t layout) {
    ui_data_mgr_t mgr = layout->m_mgr;

    assert(layout->m_src->m_product == layout);
    layout->m_src->m_product = NULL;

    if (layout->m_root) {
        ui_data_control_free(layout->m_root);
        assert(layout->m_root == NULL);
    }

    mem_free(mgr->m_alloc, layout);
}

ui_data_control_t ui_data_layout_root(ui_data_layout_t layout) {
    return layout->m_root;
}
