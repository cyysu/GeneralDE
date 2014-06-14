#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "ui/model/ui_data_mgr.h"
#include "ui/model/ui_data_src.h"
#include "ui_ed_search_i.h"

ui_ed_search_t ui_ed_search_create(ui_ed_mgr_t ed_mgr) {
    ui_ed_search_t search;

    search = mem_alloc(ed_mgr->m_alloc, sizeof(struct ui_ed_search));
    if (search == NULL) {
        CPE_ERROR(ed_mgr->m_em, "create search: alloc fail!");
        return NULL;
    }

    search->m_ed_mgr = ed_mgr;
    search->m_search_state = ui_ed_search_init;
    search->m_search_types = 0;
    search->m_cur_src = NULL;
    search->m_cur_obj = NULL;

    TAILQ_INIT(&search->m_search_roots);

    return search;
}

void ui_ed_search_free(ui_ed_search_t search) {
    ui_ed_mgr_t ed_mgr = search->m_ed_mgr;

    while(!TAILQ_EMPTY(&search->m_search_roots)) {
        ui_ed_search_root_t search_root = TAILQ_FIRST(&search->m_search_roots);
        TAILQ_REMOVE(&search->m_search_roots, search_root, m_next);
        mem_free(ed_mgr->m_alloc, search_root);
    }

    mem_free(ed_mgr->m_alloc, search);
}

int ui_ed_search_add_obj_type(ui_ed_search_t search, ui_ed_obj_type_t obj_type) {
    cpe_ba_set(&search->m_search_types, obj_type, cpe_ba_true);
    return 0;
}

int ui_ed_search_add_root(ui_ed_search_t search, const char * path) {
    ui_ed_mgr_t ed_mgr = search->m_ed_mgr;
    ui_ed_search_root_t search_root;
    size_t path_len = strlen(path) + 1;

    search_root = mem_alloc(ed_mgr->m_alloc, sizeof(struct ui_ed_search_root) + path_len);
    if (search_root == NULL) {
        CPE_ERROR(ed_mgr->m_em, "ed_search add root: alloc fail!");
        return -1;
    }

    memcpy((void*)(search_root + 1), path, path_len);
    search_root->m_root = (const char *)(search_root + 1);

    TAILQ_INSERT_TAIL(&search->m_search_roots, search_root, m_next);

    return 0;
}

static int ui_ed_search_need_process_src(ui_ed_search_t search, ui_data_src_t src) {
    switch(ui_data_src_type(src)) {
    case ui_data_src_type_module:
        return (search->m_search_types == 0
                || cpe_ba_get(&search->m_search_types, ui_ed_obj_img_block) == cpe_ba_true);
    case ui_data_src_type_sprite:
        return (search->m_search_types == 0
                || cpe_ba_get(&search->m_search_types, ui_ed_obj_frame) == cpe_ba_true);
    case ui_data_src_type_action:
        return (search->m_search_types == 0
                || cpe_ba_get(&search->m_search_types, ui_ed_obj_actor) == cpe_ba_true
                || cpe_ba_get(&search->m_search_types, ui_ed_obj_actor_layer) == cpe_ba_true
                || cpe_ba_get(&search->m_search_types, ui_ed_obj_actor_frame) == cpe_ba_true);
    case ui_data_src_type_layout:
        return (search->m_search_types == 0);
    default:
        return 0;
    }

    return 1;
}

static int ui_ed_search_need_process_obj(ui_ed_search_t search, ui_ed_obj_t ed_obj) {
    return (search->m_search_types == 0
            || cpe_ba_get(&search->m_search_types, ed_obj->m_meta->m_type) == cpe_ba_true);
}

static int ui_ed_search_need_process_child(ui_ed_search_t search, ui_ed_obj_t ed_obj) {
    return 1;
}

ui_ed_obj_t ui_ed_obj_search_next(ui_ed_search_t search) {
    ui_ed_mgr_t ed_mgr = search->m_ed_mgr;
    ui_data_mgr_t data_mgr = ed_mgr->m_data_mgr;

    if (search->m_search_state == ui_ed_search_completed) return NULL;

    if (search->m_search_state == ui_ed_search_init) {
        search->m_search_state = ui_ed_search_processing;

        if (TAILQ_EMPTY(&search->m_search_roots)) {
            ui_data_src_all_childs(&search->m_src_it, ui_data_mgr_src_root(data_mgr));
            search->m_cur_root = NULL;
        }
        else {
            for(search->m_cur_root = TAILQ_FIRST(&search->m_search_roots);
                search->m_cur_root;
                search->m_cur_root = TAILQ_NEXT(search->m_cur_root, m_next))
            {
                ui_data_src_t root_src = 
                    ui_data_src_child_find_by_path(
                        ui_data_mgr_src_root(data_mgr), search->m_cur_root->m_root);
                if (root_src == NULL) {
                    CPE_INFO(ed_mgr->m_em, "search: find src at %s fail!", search->m_cur_root->m_root);
                }
                else {
                    ui_data_src_all_childs(&search->m_src_it, root_src);
                    break;
                }
            }

            if (search->m_cur_root == NULL) {
                search->m_search_state = ui_ed_search_completed;
                return NULL;
            }
        }

        search->m_cur_src = NULL;
        search->m_cur_obj = NULL;
    }

SEARCH_NEXT_SRC:
    if (search->m_cur_src == NULL) {
        ui_data_src_t src;
        while((src = ui_data_src_it_next(&search->m_src_it))) {
            if (ui_ed_search_need_process_src(search, src)) {
                search->m_cur_src = ui_ed_src_find_by_data(ed_mgr, src);
                if (search->m_cur_src == NULL) {
                    CPE_ERROR(ed_mgr->m_em, "search: load %s fail!", ui_data_src_path_dump(&ed_mgr->m_dump_buffer, src));
                }
                else {
                    break;
                }
            }
        }

        if (search->m_cur_src == NULL) {
            while(search->m_cur_root) {
                search->m_cur_root = TAILQ_NEXT(search->m_cur_root, m_next);
                if (search->m_cur_root) {
                    ui_data_src_t root_src = 
                        ui_data_src_child_find_by_path(
                            ui_data_mgr_src_root(data_mgr), search->m_cur_root->m_root);
                    if (root_src == NULL) {
                        CPE_INFO(ed_mgr->m_em, "search: find src at %s fail!", search->m_cur_root->m_root);
                        continue;
                    }
                    else {
                        ui_data_src_all_childs(&search->m_src_it, root_src);
                        goto SEARCH_NEXT_SRC;
                    }
                }
            }
            search->m_search_state = ui_ed_search_completed;
            return NULL;
        }

        search->m_cur_obj = NULL;
    }

    if (search->m_cur_obj == NULL) {
        assert(search->m_cur_src);
        search->m_cur_obj = ui_ed_src_root_obj(search->m_cur_src);
        if (ui_ed_search_need_process_obj(search, search->m_cur_obj)) {
            return search->m_cur_obj;
        }
    }

    assert(search->m_cur_obj);

    while(search->m_cur_obj) {
        if (!TAILQ_EMPTY(&search->m_cur_obj->m_childs)
            && ui_ed_search_need_process_child(search, search->m_cur_obj))
        {
            search->m_cur_obj = TAILQ_FIRST(&search->m_cur_obj->m_childs);
        }
        else {
            while (search->m_cur_obj->m_parent) {
                ui_ed_obj_t next_obj = TAILQ_NEXT(search->m_cur_obj, m_next_for_parent);
                if (next_obj) {
                    search->m_cur_obj = next_obj;
                    break;
                }
                else {
                    search->m_cur_obj = search->m_cur_obj->m_parent;
                }
            }

            if (search->m_cur_obj->m_parent == NULL) {
                search->m_cur_obj = NULL;
            }

            if (search->m_cur_obj == NULL) {
                search->m_cur_src = NULL;
                goto SEARCH_NEXT_SRC;
            }
        }

        assert(search->m_cur_obj);
        if (ui_ed_search_need_process_obj(search, search->m_cur_obj)) break;
    }

    return search->m_cur_obj;
}

