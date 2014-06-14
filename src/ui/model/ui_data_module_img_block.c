#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/model/ui_data_src.h"
#include "ui_data_module_i.h"
#include "ui_data_src_i.h"

ui_data_img_block_t ui_data_img_block_create(ui_data_module_t module) {
    ui_data_mgr_t mgr = module->m_mgr;
    ui_data_img_block_t block;

    block = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_img_block));
    if (block == NULL) {
        CPE_ERROR(
            mgr->m_em, "create img in module %s: alloc fail !",
            ui_data_src_path_dump(&mgr->m_dump_buffer, module->m_src));
        return NULL;
    }

    block->m_module = module;
    bzero(&block->m_data, sizeof(block->m_data));
    block->m_data.id = (uint32_t)-1;

    TAILQ_INSERT_TAIL(&module->m_img_blocks, block, m_next_for_module);

    return block;
}

void ui_data_img_block_free(ui_data_img_block_t block) {
    ui_data_module_t module = block->m_module;
    ui_data_mgr_t mgr = module->m_mgr;

    TAILQ_REMOVE(&module->m_img_blocks, block, m_next_for_module);

    if (block->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_img_blocks, block);
    }

    mem_free(mgr->m_alloc, block);
}

int ui_data_img_block_set_id(ui_data_img_block_t block, uint32_t id) {
    ui_data_mgr_t mgr = block->m_module->m_mgr;
    uint32_t old_id;

    old_id = block->m_data.id;

    if (block->m_data.id != (uint32_t)-1) {
        cpe_hash_table_remove_by_ins(&mgr->m_img_blocks, block);
    }

    block->m_data.id = id;

    if (block->m_data.id != (uint32_t)-1) {
        cpe_hash_entry_init(&block->m_hh_for_mgr);
        if (cpe_hash_table_insert_unique(&mgr->m_img_blocks, block) != 0) {
            block->m_data.id = old_id;
            if (old_id != (uint32_t)-1) {
                cpe_hash_table_insert_unique(&mgr->m_img_blocks, block);
            }
            return -1;
        }
    }

    return 0;
}

static ui_data_img_block_t ui_data_img_block_in_module_next(struct ui_data_img_block_it * it) {
    ui_data_img_block_t * data = (ui_data_img_block_t *)(it->m_data);
    ui_data_img_block_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_module);

    return r;
}

void ui_data_img_block_in_module(ui_data_img_block_it_t it, ui_data_module_t module) {
    *(ui_data_img_block_t *)(it->m_data) = TAILQ_FIRST(&module->m_img_blocks);
    it->next = ui_data_img_block_in_module_next;
}

UI_IMG_BLOCK * ui_data_img_block_data(ui_data_img_block_t img_block) {
    return &img_block->m_data;
}

LPDRMETA ui_data_img_block_meta(ui_data_mgr_t data_mgr) {
    return data_mgr->m_meta_img_block;
}

uint32_t ui_data_img_block_hash(const ui_data_img_block_t block) {
    return block->m_module->m_src->m_id & block->m_data.id;
}

int ui_data_img_block_eq(const ui_data_img_block_t l, const ui_data_img_block_t r) {
    return l->m_data.id == r->m_data.id && l->m_module == r->m_module;
}
