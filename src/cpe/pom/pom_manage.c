#include <assert.h>
#include "cpe/pom/pom_manage.h"
#include "pom_manage_i.h"

pom_mgr_t
pom_mgr_create(
    mem_allocrator_t alloc,
    size_t page_size,
    size_t buffer_size)
{
    pom_mgr_t omm = (pom_mgr_t)mem_alloc(alloc, sizeof(struct pom_mgr));
    if (omm == NULL) return NULL;

    omm->m_alloc = alloc;

    if (
        pom_buffer_mgr_init(
            &omm->m_bufMgr, page_size, buffer_size, alloc)
        != 0)
    {
        mem_free(alloc, omm);
        return NULL;
    }

    if (pom_class_mgr_init(&omm->m_classMgr, alloc) != 0) {
        pom_buffer_mgr_fini(&omm->m_bufMgr);
        mem_free(alloc, omm);
        return NULL;
    }

    return omm;
}

void pom_mgr_free(pom_mgr_t omm) {
    if (omm == NULL) return;

    pom_class_mgr_fini(&omm->m_classMgr);
    pom_buffer_mgr_fini(&omm->m_bufMgr);
    
    mem_free(omm->m_alloc, omm);
}

int pom_mgr_set_backend(pom_mgr_t mgr, pom_backend_t backend, void * backend_ctx) {
    assert(mgr);
    return pom_buffer_mgr_set_backend(&mgr->m_bufMgr, backend, backend_ctx);
    
}

pom_class_id_t
pom_mgr_add_class(
    pom_mgr_t omm,
    const char * className,
    size_t object_size,
    size_t align,
    error_monitor_t em)
{
    assert(omm);
    assert(className);

    return pom_class_add(
        &omm->m_classMgr,
        className,
        object_size, 
        omm->m_bufMgr.m_page_size,
        align,
        em);
}

int pom_mgr_add_class_with_id(
    pom_mgr_t omm,
    pom_class_id_t classId,
    const char * className,
    size_t object_size,
    size_t align,
    error_monitor_t em)
{
    assert(omm);
    assert(className);

    return pom_class_add_with_id(
        &omm->m_classMgr,
        classId,
        className,
        object_size, 
        omm->m_bufMgr.m_page_size,
        align,
        em);
}

pom_class_t
pom_mgr_get_class(pom_mgr_t omm, pom_class_id_t classId) {
    assert(omm);
    return pom_class_get(&omm->m_classMgr, classId);
}

pom_class_t
pom_mgr_find_class(pom_mgr_t omm, cpe_hash_string_t className) {
    assert(omm);
    return pom_class_find(&omm->m_classMgr, className);
}

size_t pom_mgr_page_size(pom_mgr_t omm) {
    assert(omm);
    return omm->m_bufMgr.m_page_size;
}

size_t pom_mgr_buf_size(pom_mgr_t omm) {
    assert(omm);
    return omm->m_bufMgr.m_buf_size;
}

void pom_mgr_buffers(struct pom_buffer_it * it, pom_mgr_t omm) {
    assert(it);
    assert(omm);

    it->m_buf_size = omm->m_bufMgr.m_buf_size;
    cpe_range_mgr_ranges(&it->m_range_it, &omm->m_bufMgr.m_buffers);
    it->m_curent = cpe_range_it_next(&it->m_range_it);
}

void * pom_next_buffer(struct pom_buffer_it * it) {
    void * r;

    assert(it);

    if (cpe_range_size(it->m_curent) <= 0) {
        return NULL;
    }

    assert((cpe_range_size(it->m_curent) % it->m_buf_size) == 0);

    r = (void*)it->m_curent.m_start;
    it->m_curent.m_start += it->m_buf_size;

    if (cpe_range_size(it->m_curent) <= 0) {
        it->m_curent = cpe_range_it_next(&it->m_range_it);
    }

    return r;
}

void pom_mgr_buffer_ids(struct pom_buffer_id_it * it, pom_mgr_t omm) {
    assert(it);
    assert(omm);

    cpe_range_mgr_ranges(&it->m_range_it, &omm->m_bufMgr.m_buffer_ids);
    it->m_curent = cpe_range_it_next(&it->m_range_it);
}

pom_buffer_id_t
pom_next_buffer_id(struct pom_buffer_id_it * it) {
    pom_buffer_id_t r;
    assert(it);

    if (cpe_range_size(it->m_curent) <= 0) {
        return POM_INVALID_BUFFER_ID;
    }

    r = it->m_curent.m_start++;

    if (cpe_range_size(it->m_curent) <= 0) {
        it->m_curent = cpe_range_it_next(&it->m_range_it);
    }

    return r;
}

int pom_mgr_add_new_buffer(pom_mgr_t omm, pom_buffer_id_t buf_id, error_monitor_t em) {
    return pom_buffer_mgr_add_new_buffer(&omm->m_bufMgr, buf_id, em);
}

int pom_mgr_attach_old_buffer(pom_mgr_t omm, pom_buffer_id_t buf_id, error_monitor_t em) {
    return pom_buffer_mgr_attach_old_buffer(&omm->m_bufMgr, buf_id, em);
}
