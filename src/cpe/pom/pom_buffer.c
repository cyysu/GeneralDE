#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pom/pom_error.h"
#include "pom_buffer.h"
#include "pom_page_head.h"

int pom_buffer_mgr_init(
    struct pom_buffer_mgr * pgm,
    size_t page_size,
    size_t buf_size,
    mem_allocrator_t alloc)
{
    assert(pgm);

    if (page_size < sizeof(struct pom_data_page_head)) return -1;
    if (buf_size < page_size) return -1;

    pgm->m_backend = NULL;
    pgm->m_backend_ctx = NULL;
    pgm->m_page_size = page_size;
    pgm->m_buf_size = buf_size;

    if (cpe_range_mgr_init(&pgm->m_free_pages, alloc) != 0) {
        return -1;
    }

    if (cpe_range_mgr_init(&pgm->m_buffers, alloc) != 0) {
        cpe_range_mgr_fini(&pgm->m_free_pages);
        return -1;
    }

    if (cpe_range_mgr_init(&pgm->m_buffer_ids, alloc) != 0) {
        cpe_range_mgr_fini(&pgm->m_free_pages);
        cpe_range_mgr_fini(&pgm->m_buffers);
        return -1;
    }

    return 0;
}

void pom_buffer_mgr_fini(struct pom_buffer_mgr * pgm) {
    assert(pgm);

    if (pgm->m_backend && pgm->m_backend->clear) {
        struct pom_buffer_id_it it;
        cpe_range_mgr_ranges(&it.m_range_it, &pgm->m_buffer_ids);
        it.m_curent = cpe_range_it_next(&it.m_range_it);
        pgm->m_backend->clear(&it, pgm->m_backend_ctx);
    }

    cpe_range_mgr_fini(&pgm->m_buffer_ids);
    cpe_range_mgr_fini(&pgm->m_buffers);
    cpe_range_mgr_fini(&pgm->m_free_pages);
}

int pom_buffer_mgr_set_backend(
    struct pom_buffer_mgr * pgm,
    pom_backend_t backend,
    void * backend_ctx)
{
    if (!cpe_range_mgr_is_empty(&pgm->m_buffer_ids)) {
        return -1;
    }

    pgm->m_backend = backend;
    pgm->m_backend_ctx = backend_ctx;
    return 0;
}

static int pom_buffer_mgr_reserve_for_new_buffer(struct pom_buffer_mgr * pgm, error_monitor_t em) {
    if (cpe_range_mgr_reserve_for_put(&pgm->m_buffers, 1) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "reserve for buffers fail!");
        return -1;
    }

    if (cpe_range_mgr_reserve_for_put(&pgm->m_free_pages, 1) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "reserve for free pages fail!");
        return -1;
    }

    if (cpe_range_mgr_reserve_for_put(&pgm->m_buffer_ids, 1) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "reserve for buffer ids fail!");
        return -1;
    }

    return 0;
}

static void
pom_buffer_mgr_init_pages(struct pom_buffer_mgr * pgm, void * buf) {
    int leftSize = pgm->m_buf_size;
    char * b = (char *)buf;

    while(leftSize >= (int)pgm->m_page_size) {
        pom_data_page_head_init(b);
        b += pgm->m_page_size;
        leftSize -= pgm->m_page_size;
    }
}

static void *
pom_buffer_mgr_get_buf(
    struct pom_buffer_mgr * pgm,
    pom_buffer_id_t buf_id,
    error_monitor_t em)
{
    void * buf;

    if (pgm->m_backend && pgm->m_backend->buf_get) {
        buf = pgm->m_backend->buf_get(buf_id, pgm->m_backend_ctx);
        if (buf == NULL) {
            CPE_ERROR_EX(em, pom_buffer_get_fail, "backend get buf "FMT_PTR_INT_T" fail!", buf_id);
        }
    }
    else {
        buf = (void *)buf_id;
    }

    return buf;
}

static int pom_buffer_mgr_alloc_new_buffer(
    struct pom_buffer_mgr * pgm,
    error_monitor_t em)
{
    pom_buffer_id_t new_buf_id;
    void * new_buf;

    if (pgm->m_backend == NULL) {
        CPE_ERROR_EX(em, pom_no_buffer, "no backend to alloc new buf!");
        return -1;
    }

    if (pgm->m_backend->buf_alloc == NULL) {
        CPE_ERROR_EX(em, pom_no_buffer, "backend not support alloc new buf!");
        return -1;
    }

    if (pom_buffer_mgr_reserve_for_new_buffer(pgm, em) != 0) return -1;

    new_buf_id = pgm->m_backend->buf_alloc(pgm->m_buf_size, pgm->m_backend_ctx);
    if (new_buf_id == POM_INVALID_BUFFER_ID) {
        CPE_ERROR_EX(em, pom_no_buffer, "backend alloc new buf fail!");
        return -1;
    }

    cpe_range_put_one(&pgm->m_buffer_ids, new_buf_id);

    new_buf = pom_buffer_mgr_get_buf(pgm, new_buf_id, em);
    if (new_buf == NULL) return -1;

    pom_buffer_mgr_init_pages(pgm, new_buf);

    cpe_range_put_range(&pgm->m_buffers, (ptr_int_t)new_buf, ((ptr_int_t)new_buf) + pgm->m_buf_size);
    cpe_range_put_range(&pgm->m_free_pages, (ptr_int_t)new_buf, ((ptr_int_t)new_buf) + pgm->m_buf_size);

    return 0;
}

int pom_buffer_mgr_add_new_buffer(
    struct pom_buffer_mgr * pgm,
    pom_buffer_id_t buf_id,
    error_monitor_t em)
{
    void * new_buf;

    if (pom_buffer_mgr_reserve_for_new_buffer(pgm, em) != 0) return -1;

    new_buf = pom_buffer_mgr_get_buf(pgm, buf_id, em);
    if (new_buf == NULL) return -1;

    pom_buffer_mgr_init_pages(pgm, new_buf);

    cpe_range_put_range(&pgm->m_buffers, (ptr_int_t)new_buf, ((ptr_int_t)new_buf) + pgm->m_buf_size);
    cpe_range_put_range(&pgm->m_free_pages, (ptr_int_t)new_buf, ((ptr_int_t)new_buf) + pgm->m_buf_size);
    cpe_range_put_one(&pgm->m_buffer_ids, buf_id);

    return 0;
}

static int pom_buffer_mgr_reserve_for_attach_buffer(struct pom_buffer_mgr * pgm, error_monitor_t em) {
    if (cpe_range_mgr_reserve_for_put(&pgm->m_buffers, 1) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "reserve for buffers fail!");
        return -1;
    }

    if (cpe_range_mgr_reserve_for_put(&pgm->m_free_pages, (pgm->m_buf_size / pgm->m_page_size) + 1) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "reserve for free pages fail!");
        return -1;
    }

    if (cpe_range_mgr_reserve_for_put(&pgm->m_buffer_ids, 1) != 0) {
        CPE_ERROR_EX(em, pom_no_memory, "reserve for buffer ids fail!");
        return -1;
    }

    return 0;
}

int pom_buffer_mgr_attach_old_buffer(
    struct pom_buffer_mgr * pgm,
    pom_buffer_id_t buf_id,
    error_monitor_t em)
{
    char * buf;
    int leftSize;

    if (pom_buffer_mgr_reserve_for_attach_buffer(pgm, em) != 0) return -1;

    buf = (char *)pom_buffer_mgr_get_buf(pgm, buf_id, em);
    if (buf == NULL) return -1;

    cpe_range_put_range(&pgm->m_buffers, (ptr_int_t)buf, ((ptr_int_t)buf) + pgm->m_buf_size);
    cpe_range_put_one(&pgm->m_buffer_ids, buf_id);

    for(leftSize = pgm->m_buf_size;
        leftSize > (int)pgm->m_page_size;
        leftSize -= pgm->m_page_size, buf += pgm->m_page_size)
    {
        if (!pom_data_page_head_is_valid((struct pom_data_page_head*)buf)) {
            cpe_range_put_range(
                &pgm->m_free_pages,
                (ptr_int_t)buf, ((ptr_int_t)buf) + pgm->m_page_size);
        }
    }

    return 0;
}

void * pom_page_get(struct pom_buffer_mgr * pgm, error_monitor_t em) {
    int tryCount;
    struct cpe_range pageRange;

    assert(pgm);

    tryCount = 0;
    do {
        pageRange = cpe_range_get_range(&pgm->m_free_pages, pgm->m_page_size);

        if (!cpe_range_is_valid(pageRange)) {
            if (pom_buffer_mgr_alloc_new_buffer(pgm, em) != 0) {
                return NULL;
            }
            continue;
        }

        if (cpe_range_size(pageRange) < (int)pgm->m_page_size) {
            continue;
        }

        assert(cpe_range_size(pageRange) == pgm->m_page_size);
        break;
    }
    while(tryCount < 500);

    if (cpe_range_is_valid(pageRange)
        && cpe_range_size(pageRange) == pgm->m_page_size)
    {
        return (void*)(pageRange.m_start);
    }
    else {
        return NULL;
    }
}

void * pom_buffer_mgr_find_page(
    struct pom_buffer_mgr * pgm,
    void * address)
{
    struct cpe_range r;
    char * buffer;
    char * page;

    r = cpe_range_find(&pgm->m_buffers, (ptr_int_t)address);
    if (!cpe_range_is_valid(r)) return NULL;

    buffer = (char*)r.m_start;

    while((buffer + pgm->m_buf_size) <= (char*)address) {
        buffer += pgm->m_buf_size;
    }

    page = buffer +
        ((((char*)address - buffer) / pgm->m_page_size) * pgm->m_page_size);
    if ((page + pgm->m_page_size) > buffer + pgm->m_buf_size) return NULL;
    return page;
}
