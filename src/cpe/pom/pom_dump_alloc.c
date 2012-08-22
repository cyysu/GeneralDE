#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/bitarry.h"
#include "cpe/pom/pom_manage.h"
#include "pom_internal_ops.h"

static void pom_mgr_dump_alloc_class_info(write_stream_t stream, struct pom_class_mgr * class_mgr, int level) {
    size_t i;

    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "class info:\n");

    for(i = 0; i < (sizeof(class_mgr->m_classes) / sizeof(class_mgr->m_classes[0])); ++i) {
        struct pom_class * pom_class = &class_mgr->m_classes[i];

        stream_putc_count(stream, ' ', (level + 1) << 2);
        stream_printf(stream, "[%d]: ", (int)i);

        if (pom_class->m_id == POM_INVALID_CLASSID) {
            stream_printf(stream, "INVALID\n");
            continue;
        }

        stream_printf(
            stream, "%s: object-size=%d, page-size=%d, alloc-buf-capacity=%d, begin-in-page=%d\n",
            cpe_hs_data(pom_class->m_name),
            (int)pom_class->m_object_size,
            (int)pom_class->m_page_size,
            (int)pom_class->m_object_buf_begin_in_page);
    }
}

static void pom_mgr_dump_alloc_buf_info(write_stream_t stream, struct pom_buffer_mgr * buf_mgr, int level) {
    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "buf info:\n");
}

void pom_mgr_dump_alloc_info(write_stream_t stream, pom_mgr_t mgr, int level) {
    stream_putc_count(stream, ' ', level << 2);
    stream_printf(stream, "pom_mgr alloc info:\n");

    pom_mgr_dump_alloc_class_info(stream, &mgr->m_classMgr, level + 1);
    pom_mgr_dump_alloc_buf_info(stream, &mgr->m_bufMgr, level + 1);
}
