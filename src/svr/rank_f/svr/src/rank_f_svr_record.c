#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_f_svr_ops.h"

static int rank_f_svr_record_cmp(rank_f_svr_index_info_t index_info, SVR_RANK_F_RECORD * l, SVR_RANK_F_RECORD * r) {
    uint8_t i;
    for(i = 0; i < index_info->m_sorter_count; ++i) {
        struct rank_f_svr_index_sorter * sorter = index_info->m_sorters + i;
        int ret = sorter->m_sort_fun(
            ((char *)(l + 1)) + sorter->m_data_start_pos,
            ((char *)(r + 1)) + sorter->m_data_start_pos);
        if (ret) return ret;
    }

    return 0;
}

static SVR_RANK_F_RECORD ** rank_f_svr_record_bsearch(
    rank_f_svr_index_info_t index_info, SVR_RANK_F_RECORD * key,
    SVR_RANK_F_RECORD ** begin, SVR_RANK_F_RECORD ** end)
{
    while(begin != end) {
        SVR_RANK_F_RECORD ** middle;
        int r;

        middle = begin + ((end - begin) / 2);
        assert(middle != end);
        r = rank_f_svr_record_cmp(index_info, *middle, key);
        if (r == 0) {
            return middle;
        }
        else if (r < 0) {
            end = middle;
        }
        else {
            begin = middle + 1;
        }
    }

    return NULL;
}

int rank_f_svr_record_update(rank_f_svr_t svr, rank_f_svr_index_t index, SVR_RANK_F_RECORD * record) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t buf;
    rank_f_svr_index_buf_t last_buf;
    int record_count;
    uint16_t insert_pos;
    SVR_RANK_F_RECORD * place_record;

    index_info = &svr->m_index_infos[index->m_index_id];
    assert(index_info->m_svr);

    insert_pos = 0;
    buf = index->m_bufs;
    record_count = index->m_record_count;

    while(buf) {
        SVR_RANK_F_RECORD ** last_record =
            buf->m_records
            + ((record_count > RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               ? RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               : record_count)
               - 1);
        int r = rank_f_svr_record_cmp(index_info, *last_record, record);
        if (r == 0) { /*已经找到 */
            memcpy(*last_record, record, sizeof(*record));
            return 0;
        }
        else if (r < 0) { /*在当前块 */
            SVR_RANK_F_RECORD ** begin = buf->m_records;
            SVR_RANK_F_RECORD ** end = last_record;

            while(begin != end) {
                SVR_RANK_F_RECORD ** middle;
                int r;

                middle = begin + ((end - begin) / 2);
                assert(middle != end);
                r = rank_f_svr_record_cmp(index_info, *middle, record);
                if (r == 0) {
                    memcpy(*middle, record, sizeof(*record));
                    return 0;
                }
                else if (r < 0) {
                    end = middle;
                }
                else {
                    begin = middle + 1;
                }
            }

            insert_pos = begin - buf->m_records;
            break;
        }
        else { /*在下一块 */
            if (buf->m_next == NULL) {
                if (record_count >= RANK_F_SVR_INDEX_BUF_RECORD_COUNT) {
                    buf = NULL;
                    insert_pos = 0;
                }
                else {
                    insert_pos = record_count;
                }
                break;
            }
            else {
                buf = buf->m_next;
                record_count -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT;
            }
        }
    }

    place_record = aom_obj_alloc(svr->m_record_mgr);
    if (place_record == NULL) {
        CPE_ERROR(svr->m_em, "%s: record update: alloc new record fail!", rank_f_svr_name(svr));
        return -1;
    }

    last_buf = NULL;
    while(buf) {
        if (record_count >= RANK_F_SVR_INDEX_BUF_RECORD_COUNT) { /*当前块放满了 */
            SVR_RANK_F_RECORD * save_record;

            assert(insert_pos < RANK_F_SVR_INDEX_BUF_RECORD_COUNT);

            save_record = buf->m_records[RANK_F_SVR_INDEX_BUF_RECORD_COUNT - 1];

            if (insert_pos != RANK_F_SVR_INDEX_BUF_RECORD_COUNT - 1) {
                memmove(
                    buf->m_records + insert_pos + 1,
                    buf->m_records + insert_pos,
                    sizeof(buf->m_records[0]) * (RANK_F_SVR_INDEX_BUF_RECORD_COUNT - 1 - insert_pos));
            }
            
            buf->m_records[insert_pos] = place_record;

            place_record = save_record;
            insert_pos = 0;
            last_buf = buf;
            buf = buf->m_next;
            record_count -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT;
        }
        else {
             /*有空余位置（应该是最后一块), 直接插入 */
            assert(buf->m_next == NULL);

            if (insert_pos < record_count) {
                memmove(
                    buf->m_records + insert_pos + 1,
                    buf->m_records + insert_pos,
                    sizeof(buf->m_records[0]) * (RANK_F_SVR_INDEX_BUF_RECORD_COUNT - 1 - insert_pos));
            }

            assert(insert_pos <= record_count);

            buf->m_records[insert_pos] = place_record;
            ++index->m_record_count;
            return 0; 
        }
    }

    /*需要构造一个新块并插入 */
    assert(place_record);
    assert(buf == NULL);
    assert(insert_pos == 0);

    buf = rank_f_svr_index_buf_alloc(svr);
    if (buf == NULL) {
        CPE_ERROR(svr->m_em, "%s: record update: alloc new buff fail!", rank_f_svr_name(svr));
        aom_obj_free(svr->m_record_mgr, place_record);
        return -1;
    }

    buf->m_records[insert_pos] = place_record;

    if (last_buf) {
        /*已经有过块了，则最后一块一定是满的 */
        assert(last_buf->m_next == NULL);
        assert((index->m_record_count % RANK_F_SVR_INDEX_BUF_RECORD_COUNT) == 0);

        last_buf->m_next = buf;
    }
    else {
        assert(index->m_bufs == NULL);
        index->m_bufs = buf;
    }

    ++index->m_record_count;

    return 0;
}

int rank_f_svr_record_remove(rank_f_svr_t svr, rank_f_svr_index_t gid_index, SVR_RANK_F_RECORD * key) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t buf;
    rank_f_svr_index_buf_t * link_at;
    int record_count;
    uint8_t remove_pos;

    assert(gid_index->m_index_id == 0);

    index_info = &svr->m_index_infos[gid_index->m_index_id];
    assert(index_info->m_svr);

    remove_pos = 0;
    buf = gid_index->m_bufs;
    record_count = gid_index->m_record_count;

    link_at = &gid_index->m_bufs;
    while(buf) {
        SVR_RANK_F_RECORD ** last_record =
            buf->m_records
            + ((record_count > RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               ? RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               : record_count)
               - 1);
        int r = rank_f_svr_record_cmp(index_info, *last_record, key);
        if (r == 0) { /*已经找到 */
            remove_pos = last_record - buf->m_records;
            break;
        }
        else if (r < 0) { /*在当前块 */
            SVR_RANK_F_RECORD ** begin = buf->m_records;
            SVR_RANK_F_RECORD ** end = last_record;

            while(begin != end) {
                SVR_RANK_F_RECORD ** middle;
                int r;

                middle = begin + ((end - begin) / 2);
                assert(middle != end);
                r = rank_f_svr_record_cmp(index_info, *middle, key);
                if (r == 0) {
                    remove_pos = middle - buf->m_records;
                    break;
                }
                else if (r < 0) {
                    end = middle;
                }
                else {
                    begin = middle + 1;
                }
            }

            if (begin == end) {
                if (rank_f_svr_record_cmp(index_info, *begin, key) == 0) {
                    remove_pos = begin - buf->m_records;
                }
                else {
                    return SVR_RANK_F_ERROR_RECORD_NOT_EXIST;
                }
            }

            break;
        }
        else { /*在下一块 */
            if (buf->m_next == NULL) {
                return SVR_RANK_F_ERROR_RECORD_NOT_EXIST;
            }
            else {
                link_at = &buf->m_next;
                buf = buf->m_next;
                record_count -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT;
            }
        }
    }

    assert(buf);
    assert(remove_pos < record_count);
    assert(remove_pos < RANK_F_SVR_INDEX_BUF_RECORD_COUNT);

    aom_obj_free(svr->m_record_mgr, buf->m_records[remove_pos]);

    while(buf->m_next) {
        assert(record_count > RANK_F_SVR_INDEX_BUF_RECORD_COUNT);

        memmove(
            buf->m_records + remove_pos,
            buf->m_records + remove_pos + 1,
            sizeof(buf->m_records[0]) * (RANK_F_SVR_INDEX_BUF_RECORD_COUNT - remove_pos - 1));

        buf->m_records[RANK_F_SVR_INDEX_BUF_RECORD_COUNT - 1] = buf->m_next->m_records[0];

        remove_pos = 0;
        link_at = &buf->m_next;
        buf = buf->m_next;
        record_count -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT;
    }

    assert(buf);
    assert(buf->m_next == NULL);
    assert(remove_pos < record_count);
    assert(record_count < RANK_F_SVR_INDEX_BUF_RECORD_COUNT);

    memmove(
        buf->m_records + remove_pos,
        buf->m_records + remove_pos + 1,
        sizeof(buf->m_records[0]) * (record_count - remove_pos - 1));

    if (record_count == 1) {
        *link_at = NULL;
        rank_f_svr_index_buf_free(svr, buf);
    }

    gid_index->m_record_count --;

    return 0;
}

SVR_RANK_F_RECORD *
rank_f_svr_record_find(rank_f_svr_t svr, rank_f_svr_index_t index, SVR_RANK_F_RECORD * key) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t buf;
    int record_count;

    index_info = &svr->m_index_infos[index->m_index_id];
    assert(index_info->m_svr);

    for(buf = index->m_bufs, record_count = index->m_record_count;
        buf && record_count > 0;
        buf = buf->m_next, record_count -= RANK_F_SVR_INDEX_BUF_RECORD_COUNT)
    {
        SVR_RANK_F_RECORD ** last_record =
            buf->m_records
            + ((record_count > RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               ? RANK_F_SVR_INDEX_BUF_RECORD_COUNT
               : record_count)
               - 1);
        int r = rank_f_svr_record_cmp(index_info, *last_record, key);
        if (r == 0) { /*已经找到 */
            return *last_record;
        }
        else if (r < 0) { /*在当前块 */
            SVR_RANK_F_RECORD ** p = rank_f_svr_record_bsearch(index_info, key, buf->m_records, last_record);
            return p ? *p : NULL;
        }
        else { /*在下一块 */
            //goto next buf
        }
    }

    return NULL;
}

static rank_f_svr_index_info_t g_index_info_for_sort;
static int rank_f_svr_record_qsort_cmp(void const * l, void const * r) {
    return rank_f_svr_record_cmp(g_index_info_for_sort, *(SVR_RANK_F_RECORD**)l, *(SVR_RANK_F_RECORD**)r);
}

int rank_f_svr_record_sort(rank_f_svr_t svr, rank_f_svr_index_t index, rank_f_svr_index_t records) {
    rank_f_svr_index_info_t index_info;
    rank_f_svr_index_buf_t buf;
    SVR_RANK_F_RECORD * sortbuf[records->m_record_count];
    uint8_t write_pos = 0;
    rank_f_svr_index_buf_t * link_to;

    assert(index->m_bufs == NULL);

    index_info = &svr->m_index_infos[index->m_index_id];
    assert(index_info->m_svr);

    /*将现有的record统一放入 */
    for(buf = records->m_bufs; buf; buf = buf->m_next) {
        uint8_t rp;
        for(rp = 0; rp < RANK_F_SVR_INDEX_BUF_RECORD_COUNT && write_pos < records->m_record_count; ++rp) {
            assert(buf->m_records[rp]);
            sortbuf[write_pos++] = buf->m_records[rp];
        }
    }

    /*排序 */
    g_index_info_for_sort = index_info;
    qsort(sortbuf, write_pos, sizeof(sortbuf[0]), rank_f_svr_record_qsort_cmp);

    /*将排序结果回填 */
    link_to = &index->m_bufs;
    while(index->m_record_count < records->m_record_count) {
        uint8_t i;

        buf = rank_f_svr_index_buf_alloc(svr);
        if (buf == NULL) {
            CPE_ERROR(svr->m_em, "%s: record sort: alloc new buf fail!", rank_f_svr_name(svr));
            return -1;
        }

        *link_to = buf;
        link_to = &buf->m_next;
        
        for(i = 0;
            i <  RANK_F_SVR_INDEX_BUF_RECORD_COUNT && index->m_record_count < records->m_record_count;
            ++i, ++index->m_record_count)
        {
            buf->m_records[i] = sortbuf[index->m_record_count];
        }
    }

    return 0;
}

