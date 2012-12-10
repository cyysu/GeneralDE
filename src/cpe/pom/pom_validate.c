#include <assert.h>
#include "cpe/utils/error.h"
#include "cpe/pom/pom_manage.h"
#include "pom_internal_ops.h"
#include "pom_page_head.h"

struct pom_validating_page {
    void * m_page;
    struct cpe_hash_entry m_hh;
};

static uint32_t pom_validating_page_hash(const struct pom_validating_page * p) {
    return (uint32_t)(((ptr_int_t)p->m_page) & 0xFFFFFFFF);
}

static int pom_validating_page_eq(const struct pom_validating_page * l, const struct pom_validating_page * r) {
    return l->m_page == r->m_page;
}

static void pom_grp_validate_build_pages_from_buf(cpe_hash_table_t pages, pom_mgr_t mgr, error_monitor_t em) {
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
            size_t size = buf_mgr->m_buf_size;

            while(size >= buf_mgr->m_page_size) {
                char * cur_page = page_buf;
                struct pom_validating_page * validating_page;

                page_buf += buf_mgr->m_page_size;
                size -= buf_mgr->m_page_size;

                validating_page = mem_alloc(mgr->m_alloc, sizeof(struct pom_validating_page));
                if (validating_page == NULL) {
                    CPE_ERROR(em, "page %p: alloc validate info fail!", cur_page);
                    continue;
                }

                validating_page->m_page = cur_page;
                cpe_hash_entry_init(&validating_page->m_hh);

                if (cpe_hash_table_insert_unique(pages, validating_page) != 0) {
                    CPE_ERROR(em, "page %p: duplicate!", cur_page);
                    mem_free(mgr->m_alloc, validating_page);
                    continue;
                }
            }
        }
    }
}

static void pom_mgr_validate_allocked_pages(pom_mgr_t mgr, cpe_hash_table_t pages, error_monitor_t em) {
    struct pom_validating_page key;
    size_t i;

    for(i = 0; i < (sizeof(mgr->m_classMgr.m_classes) / sizeof(mgr->m_classMgr.m_classes[0])); ++i) {
        struct pom_class * pom_class = &mgr->m_classMgr.m_classes[i];
        size_t page_pos;

        for(page_pos = 0; page_pos < pom_class->m_page_array_size; ++page_pos) {
            struct pom_data_page_head * page = pom_class->m_page_array[page_pos];
            struct pom_validating_page * validating_page;

            if (page == NULL) {
                CPE_ERROR(em, "page %s.%d: page invalid!", cpe_hs_data(pom_class->m_name), (int)page_pos);
                continue;
            }

            if (page->m_magic != POM_PAGE_MAGIC) {
                CPE_ERROR(em, "page %p: page magic(%x) error!", page, page->m_magic);
            }

            if (page->m_classId != pom_class->m_id) {
                CPE_ERROR(em, "page %p: class id error, class-id(page)=%d, class-id(class)=%d!", page, page->m_classId, pom_class->m_id);
            }

            if (page->m_page_idx != page_pos) {
                CPE_ERROR(em, "page %p: page index error, index(page)=%d, index(class)=%d!", page, page->m_page_idx, (int)page_pos);
            }

            if (page->m_obj_per_page != pom_class->m_object_per_page) {
                CPE_ERROR(
                    em, "page %p: obj-per-page error, obj-per-page(page)=%d, obj-per-page(class)=%d!",
                    page, page->m_obj_per_page, (int)pom_class->m_object_per_page);
            }

            key.m_page = page; 
            validating_page = cpe_hash_table_find(pages, &key);
            if (validating_page == NULL) {
                CPE_ERROR(em, "page %p: not in validating pages!", page);
            }
            else {
                cpe_hash_table_remove_by_ins(pages, validating_page);
                mem_free(mgr->m_alloc, validating_page);
            }
        }
    }
}

static void pom_mgr_validate_free_pages(pom_mgr_t mgr, cpe_hash_table_t pages, error_monitor_t em) {
    struct pom_validating_page key;
    struct cpe_range_it range_it;
    struct cpe_range range;
    struct pom_buffer_mgr * buf_mgr;
    struct pom_validating_page * validating_page;

    buf_mgr = &mgr->m_bufMgr;

    cpe_range_mgr_ranges(&range_it, &buf_mgr->m_free_pages);

    for(range = cpe_range_it_next(&range_it);
        cpe_range_is_valid(range); 
        range = cpe_range_it_next(&range_it))
    {
        char * page_buf = (void*)range.m_start;

        while(page_buf < (char*)range.m_end) {
            struct pom_data_page_head * page = (struct pom_data_page_head * )page_buf;
            page_buf += buf_mgr->m_page_size;

            if (((char*)range.m_end - (char*)page) < buf_mgr->m_page_size) continue;

            if (page->m_magic != POM_PAGE_MAGIC) {
                CPE_ERROR(em, "page %p: free page magic(%x) error!", page, page->m_magic);
            }

            if (page->m_classId != POM_INVALID_CLASSID) {
                CPE_ERROR(em, "page %p: free page, class id error, class-id=%d!", page, page->m_classId);
            }

            if (page->m_page_idx != (uint16_t)-1) {
                CPE_ERROR(em, "page %p: free page index error, index=%d!", page, page->m_page_idx);
            }

            if (page->m_obj_per_page != 0) {
                CPE_ERROR(
                    em, "page %p: free page obj-per-page error, obj-per-page=%d!",
                    page, page->m_obj_per_page);
            }

            key.m_page = page; 
            validating_page = cpe_hash_table_find(pages, &key);
            if (validating_page == NULL) {
                CPE_ERROR(em, "page %p: free page not in validating pages!", page);
            }
            else {
                cpe_hash_table_remove_by_ins(pages, validating_page);
                mem_free(mgr->m_alloc, validating_page);
            }
        }
    }
}

static void pom_mgr_validate_bad_pages(pom_mgr_t mgr, cpe_hash_table_t pages, error_monitor_t em) {
    struct cpe_hash_it page_it;
    struct pom_validating_page * page;

    cpe_hash_it_init(&page_it, pages);

    page = cpe_hash_it_next(&page_it);
    while (page) {
        struct pom_validating_page * next = cpe_hash_it_next(&page_it);
        CPE_ERROR(em, "page %p: not allocked or free!", page->m_page);
        cpe_hash_table_remove_by_ins(pages, page);
        mem_free(mgr->m_alloc, page);
        page = next;
    }
}

static void pom_mgr_validate_i(pom_mgr_t mgr, error_monitor_t em) {
    struct cpe_hash_table pages;

    if (cpe_hash_table_init(
            &pages,
            mgr->m_alloc,
            (cpe_hash_fun_t) pom_validating_page_hash,
            (cpe_hash_cmp_t) pom_validating_page_eq,
            CPE_HASH_OBJ2ENTRY(pom_validating_page, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "create pages hash table fail!");
        return;
    }

    pom_grp_validate_build_pages_from_buf(&pages, mgr, em);

    pom_mgr_validate_allocked_pages(mgr, &pages, em);
    pom_mgr_validate_free_pages(mgr, &pages, em);
    pom_mgr_validate_bad_pages(mgr, &pages, em);

    assert(cpe_hash_table_count(&pages) == 0);
    cpe_hash_table_fini(&pages);
}

int pom_mgr_validate(pom_mgr_t mgr, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        pom_mgr_validate_i(mgr, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        pom_mgr_validate_i(mgr, &logError);
    }

    return ret;
}

