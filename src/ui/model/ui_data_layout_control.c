#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_i.h"

ui_data_control_t ui_data_control_create(ui_data_layout_t layout, ui_data_control_t parent, ui_data_control_type_t control_type) {
    ui_data_mgr_t mgr = layout->m_mgr;
    ui_data_control_t control;
    LPDRMETA data_meta;
    size_t data_size;

    data_meta = ui_data_control_meta(mgr, control_type);
    assert(data_meta);

    data_size = dr_meta_size(data_meta);

    if (parent ==  NULL) {
        if (layout->m_root != NULL) {
            CPE_ERROR(
                mgr->m_em, "create img in layout %s: no parent!",
                ui_data_src_path_dump(&mgr->m_dump_buffer, layout->m_src));
            return NULL;
        }
    }

    control = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_control) + data_size);
    if (control == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in layout %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, layout->m_src));
        return NULL;
    }

    control->m_layout = layout;
    control->m_parent = parent;
    control->m_type = control_type;
    TAILQ_INIT(&control->m_childs);

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_childs, control, m_next_for_parent);
    }
    else {
        assert(layout->m_root == NULL);
        layout->m_root = control;
    }

    bzero(control + 1, data_size);
    dr_meta_set_defaults(control + 1, data_size, data_meta, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);

    return control;
}

void ui_data_control_free(ui_data_control_t control) {
    ui_data_layout_t layout = control->m_layout;
    ui_data_mgr_t mgr = layout->m_mgr;
    UI_CONTROL_BASIC * data_basic = ui_data_control_data(control);

    while(!TAILQ_EMPTY(&control->m_childs)) {
        ui_data_control_free(TAILQ_FIRST(&control->m_childs));
    }

    if (control->m_parent) {
        TAILQ_REMOVE(&control->m_parent->m_childs, control, m_next_for_parent);
    }

    if (control == layout->m_root) {
        layout->m_root = NULL;
    }

    if (data_basic->id) {
        cpe_hash_table_remove_by_ins(&layout->m_mgr->m_controls_by_id, control);
    }

    mem_free(mgr->m_alloc, control);
}

ui_data_control_type_t ui_data_control_type(ui_data_control_t control) {
    return control->m_type;
}

LPDRMETA ui_data_control_meta(ui_data_mgr_t mgr, ui_data_control_type_t control_type) {
    assert((uint8_t)control_type >= UI_DATA_CONTROL_TYPE_MIN && (uint8_t)control_type < UI_DATA_CONTROL_TYPE_MAX);
    return mgr->m_meta_controls[control_type - UI_DATA_CONTROL_TYPE_MIN];
}

void * ui_data_control_data(ui_data_control_t control) {
    return control + 1;
}

ui_data_control_t ui_data_control_parent(ui_data_control_t control) {
    return control->m_parent;
}

static ui_data_control_t ui_data_control_child_next(struct ui_data_control_it * it) {
    ui_data_control_t * data = (ui_data_control_t *)(it->m_data);
    ui_data_control_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_parent);

    return r;
}

void ui_data_control_childs(ui_data_control_it_t it, ui_data_control_t control) {
    *(ui_data_control_t *)(it->m_data) = TAILQ_FIRST(&control->m_childs);
    it->next = ui_data_control_child_next;
}

ui_data_control_t ui_data_control_find_by_id(ui_data_layout_t layout, uint32_t id) {
    char key_buf[sizeof(struct ui_data_control) + sizeof(UI_CONTROL_BASIC)];
    ui_data_control_t key = (ui_data_control_t)&key_buf;

    key->m_layout = layout;
    ((UI_CONTROL_BASIC*)(key + 1))->id = id;

    return cpe_hash_table_find(&layout->m_mgr->m_controls_by_id, key);
}

void ui_data_control_set_id(ui_data_control_t control, uint32_t id) {
    ui_data_layout_t layout = control->m_layout;
    UI_CONTROL_BASIC * data_basic = ui_data_control_data(control);

    assert(layout);
    assert(data_basic);

    if (data_basic->id) {
        cpe_hash_table_remove_by_ins(&layout->m_mgr->m_controls_by_id, control);
    }

    data_basic->id = id;
    if (data_basic->id) {
        cpe_hash_entry_init(&control->m_hh_for_mgr);
        cpe_hash_table_insert(&layout->m_mgr->m_controls_by_id, control);
    }
}

uint32_t ui_data_control_id(ui_data_control_t control) {
    UI_CONTROL_BASIC * data_basic = ui_data_control_data(control);

    assert(data_basic);

    return data_basic->id;
}

uint32_t ui_data_control_hash(const ui_data_control_t control) {
    return control->m_layout->m_src->m_id & ((UI_CONTROL_BASIC*)(control + 1))->id;
}

int ui_data_control_eq(const ui_data_control_t l, const ui_data_control_t r) {
    return ((UI_CONTROL_BASIC*)(l + 1))->id == ((UI_CONTROL_BASIC*)(r + 1))->id
        && l->m_layout == r->m_layout;
}
