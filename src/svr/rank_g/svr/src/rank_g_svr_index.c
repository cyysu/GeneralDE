#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_json.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_rank_tree.h"

rank_g_svr_index_t rank_g_svr_index_find(rank_g_svr_t svr, uint16_t id) {
    if (id >= (sizeof(svr->m_indexs) / sizeof(svr->m_indexs[0]))) {
        CPE_ERROR(svr->m_em, "%s: find index %d: id overflow!", rank_g_svr_name(svr), id);
        return NULL;
    }
    
    return svr->m_indexs[id].m_svr == NULL
        ? NULL
        : &svr->m_indexs[id];
}

rank_g_svr_index_t
rank_g_svr_index_create(rank_g_svr_t svr, uint16_t id, const char * entry_path) {
    rank_g_svr_index_t index;
    int start_pos;

    if (id >= (sizeof(svr->m_indexs) / sizeof(svr->m_indexs[0]))) {
        CPE_ERROR(svr->m_em, "%s: create index %d: id overflow!", rank_g_svr_name(svr), id);
        return NULL;
    }

    if (svr->m_indexs[id].m_svr != NULL) {
        CPE_ERROR(svr->m_em, "%s: create index %d: id duplicate!", rank_g_svr_name(svr), id);
        return NULL;
    }

    index = svr->m_indexs + id;

    index->m_rank_entry = dr_meta_find_entry_by_path_ex(svr->m_record_meta, entry_path, &start_pos);
    if (index->m_rank_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create index %d: entry %s not exist!", rank_g_svr_name(svr), id, entry_path);
        return NULL;
    }
    index->m_rank_start_pos = start_pos;

    index->m_svr = svr;
    index->m_id = id;
    index->m_rank_tree = NULL;
    if (id + 1 > svr->m_index_count) {
        svr->m_index_count = id + 1;
    }

    return index;
}

int rank_g_svr_index_update(rank_g_svr_index_t index, uint32_t record_id) {
    rank_g_svr_t svr = index->m_svr;
    void const * record;
    uint32_t rank_value;

    record = aom_obj_get(svr->m_record_mgr, record_id);
    if (record == NULL) {
        CPE_ERROR(svr->m_em, "%s: index %d: get record %d fail", rank_g_svr_name(svr), index->m_id, record_id);
        return -1;
    }

    if (dr_entry_try_read_uint32(
            &rank_value,
            ((const char *)record) + index->m_rank_start_pos, index->m_rank_entry, svr->m_em)
        != 0)
    {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, NULL);
        
        CPE_ERROR(
            svr->m_em, "%s: index %d: update record %s: read sort attr fail!",
            rank_g_svr_name(svr), index->m_id,
            dr_json_dump(&buffer, record, svr->m_record_size, svr->m_record_meta));

        mem_buffer_clear(&buffer);

        return -1;
    }

    if (index->m_record_to_rank_pos[record_id] != INVALID_RANK_TREE_NODE_POS) { /*已经记录在排行榜中 */
        rt_node_t rank_node;

        rank_node = rt_get(index->m_rank_tree, index->m_record_to_rank_pos[record_id]);
        assert(rank_node);
        assert(rt_node_record_id(rank_node) == record_id);

        if (rt_node_value(rank_node) == rank_value) return 0;

        rt_erase(index->m_rank_tree, rank_node);
        index->m_record_to_rank_pos[record_id] = INVALID_RANK_TREE_NODE_POS;
    }

    if (rt_size(index->m_rank_tree) < rt_capacity(index->m_rank_tree)) {
        /*排行榜没有满，新插入一个 */
        rt_node_t rank_node = rt_insert(index->m_rank_tree, rank_value, record_id);
        assert(rank_node);

        index->m_record_to_rank_pos[record_id] = rt_idx(index->m_rank_tree, rank_node);
        return 0;
    }
    else {
        /*和最后一个比较 */
        rt_node_t last_node = rt_last(index->m_rank_tree);
        assert(last_node);

        if (rt_node_value(last_node) > rank_value) {
            /*超出范围，则返回排行榜满了 */
            return SVR_RANK_G_ERROR_FULL;
        }
        else {
            /*否则，替换最后一个 */
            index->m_record_to_rank_pos[rt_node_record_id(last_node)] = INVALID_RANK_TREE_NODE_POS;
            rt_erase(index->m_rank_tree, last_node);


            /*插入新节点 */
            last_node = rt_insert(index->m_rank_tree, rank_value, record_id);
            assert(last_node);
            index->m_record_to_rank_pos[record_id] = rt_idx(index->m_rank_tree, last_node);

            return 0;
        }
    }
}

void rank_g_svr_index_remove(rank_g_svr_index_t index, uint32_t record_id) {
    if (index->m_record_to_rank_pos[record_id] != INVALID_RANK_TREE_NODE_POS) {
        /*已经记录在排行榜中 */
        rt_node_t rank_node;

        rank_node = rt_get(index->m_rank_tree, index->m_record_to_rank_pos[record_id]);
        assert(rank_node);
        assert(rt_node_record_id(rank_node) == record_id);

        rt_erase(index->m_rank_tree, rank_node);
        index->m_record_to_rank_pos[record_id] = INVALID_RANK_TREE_NODE_POS;
    }

    index->m_record_to_rank_pos[record_id] = INVALID_RANK_TREE_NODE_POS;
}
