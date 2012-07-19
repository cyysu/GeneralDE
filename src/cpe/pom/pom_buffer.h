#ifndef CPE_POM_PAGE_H
#define CPE_POM_PAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/range.h"
#include "cpe/pom/pom_types.h"
#include "pom_page_head.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_class_mgr;

struct pom_buffer_mgr {
    pom_backend_t m_backend;
    void * m_backend_ctx;
    size_t m_page_size;
    size_t m_buf_size;
    struct cpe_range_mgr m_free_pages;
    struct cpe_range_mgr m_buffers;
    struct cpe_range_mgr m_buffer_ids;
};

int pom_buffer_mgr_init(
    struct pom_buffer_mgr * pgm,
    size_t page_size,
    size_t buf_size,
    mem_allocrator_t alloc);

int pom_buffer_mgr_set_backend(
    struct pom_buffer_mgr * pgm,
    pom_backend_t backend,
    void * backend_ctx);

int pom_buffer_mgr_add_new_buffer(
    struct pom_buffer_mgr * pgm,
    pom_buffer_id_t buf_id,
    error_monitor_t em);

int pom_buffer_mgr_attach_old_buffer(
    struct pom_buffer_mgr * pgm,
    struct pom_class_mgr * classMgr,
    pom_buffer_id_t buf_id,
    error_monitor_t em);

void * pom_buffer_mgr_find_page(
    struct pom_buffer_mgr * pgm,
    void * address);

void pom_buffer_mgr_fini(struct pom_buffer_mgr * pgm);

void * pom_page_get(struct pom_buffer_mgr * pgm, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif


