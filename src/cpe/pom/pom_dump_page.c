#include "cpe/pal/pal_stdio.h"
#include "cpe/pom/pom_manage.h"
#include "pom_internal_ops.h"

static void pom_mgr_dump_page_info_one_buf(write_stream_t stream, char const * buf, size_t size, int level) {
    
}

void pom_mgr_dump_page_info(write_stream_t stream, pom_mgr_t mgr, int level) {
    struct cpe_range_it buf_range_it;
    struct cpe_range buf_range;
    struct pom_buffer_mgr * buf_mgr;

    buf_mgr = &mgr->m_bufMgr;

    cpe_range_mgr_ranges(&buf_range_it, &buf_mgr->m_buffer_ids);

    for(buf_range = cpe_range_it_next(&buf_range_it);
        cpe_range_is_valid(buf_range);
        buf_range = cpe_range_it_next(&buf_range_it))
    {
        for(; buf_range.m_start != buf_range.m_end; ++buf_range.m_start) {
            char * page_buf = (char *)pom_buffer_mgr_get_buf(buf_mgr, buf_range.m_start, NULL);
            stream_putc_count(stream, ' ', level << 2);
            if (page_buf == NULL) {
                stream_printf(stream, "buf "FMT_UINT64_T": no data\n", buf_range.m_start);
                continue;
            }
            else {
                stream_printf(
                    stream, "buf "FMT_UINT64_T": %p ~ %p (size="FMT_SIZE_T")\n",
                    buf_range.m_start, page_buf, page_buf + buf_mgr->m_buf_size, buf_mgr->m_buf_size);
                pom_mgr_dump_page_info_one_buf(stream, page_buf, buf_mgr->m_buf_size, level + 1);
            }
        }
    }
}
