#ifndef UI_DATA_LAYOUT_INTERNAL_H
#define UI_DATA_LAYOUT_INTERNAL_H
#include "cpe/utils/hash.h"
#include "ui/model/ui_data_layout.h"
#include "ui_model_internal_types.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_control_list, ui_data_control) ui_data_control_list_t;

struct ui_data_layout {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    ui_data_control_t m_root;
};

struct ui_data_control {
    ui_data_layout_t m_layout;

    ui_data_control_t m_parent;
    ui_data_control_list_t m_childs;
    TAILQ_ENTRY(ui_data_control) m_next_for_parent;
    struct cpe_hash_entry m_hh_for_mgr;

    ui_data_control_type_t m_type;
};

uint32_t ui_data_control_hash(const ui_data_control_t control);
int ui_data_control_eq(const ui_data_control_t l, const ui_data_control_t r);

#ifdef __cplusplus
}
#endif

#endif
