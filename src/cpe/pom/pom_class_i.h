#ifndef CPE_POM_CLASS_H
#define CPE_POM_CLASS_H
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/utils/range.h"
#include "cpe/pom/pom_class.h"
#include "cpe/pom/pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POM_CLASS_BUF_LEN (POM_MAX_TYPE_COUNT + 1)

struct pom_page;

struct pom_class {
    pom_class_id_t m_id;
    char m_name_buf[cpe_hs_len_to_binary_len(POM_MAX_TYPENAME_LEN)];
    cpe_hash_string_t m_name;
    struct cpe_hash_entry m_hh;
    struct cpe_range_mgr m_range_alloc;
    mem_allocrator_t m_alloc;
    size_t m_object_size;

    size_t m_page_size;
    size_t m_object_per_page;
    size_t m_alloc_buf_capacity;
    size_t m_object_buf_begin_in_page;

    size_t m_page_array_capacity;
    size_t m_page_array_size;
    void * * m_page_array;
};

struct pom_class_mgr {
    struct pom_class m_classes[POM_CLASS_BUF_LEN];
    struct cpe_hash_table m_classNameIdx;
};

int pom_class_mgr_init(struct pom_class_mgr * classMgr, mem_allocrator_t alloc);
void pom_class_mgr_fini(struct pom_class_mgr * classMgr);

pom_class_id_t
pom_class_add(
    struct pom_class_mgr * classMgr,
    const char * className,
    size_t object_size,
    size_t page_size,
    size_t align,
    error_monitor_t em);

int pom_class_add_with_id(
    struct pom_class_mgr * classMgr,
    pom_class_id_t classId,
    const char * className,
    size_t object_size,
    size_t page_size,
    size_t align,
    error_monitor_t em);

struct pom_class *
pom_class_get(struct pom_class_mgr * classMgr, pom_class_id_t classId);

struct pom_class *
pom_class_find(struct pom_class_mgr * classMgr, cpe_hash_string_t className);

int pom_class_add_new_page(
    struct pom_class * theClass,
    void * page,
    error_monitor_t em);

int pom_class_add_old_page(
    struct pom_class * theClass,
    void * page,
    error_monitor_t em);

int32_t pom_class_alloc_object(struct pom_class *cls);
int32_t pom_class_addr_2_object(struct pom_class *cls, void * page, void * addr);

void pom_class_free_object(struct pom_class *cls, int32_t value, error_monitor_t em);
void * pom_class_get_object(struct pom_class *cls, int32_t value, error_monitor_t em);

#define pom_class_ba_of_page(page) (cpe_ba_t)(((char*)(page)) + sizeof(struct pom_data_page_head))
#define pom_class_page_buf_len(page_count) (sizeof(void*) * (page_count))

#ifdef __cplusplus
}
#endif

#endif
