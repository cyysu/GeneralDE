#include <assert.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/tsort.h"
#include "cpe/utils/hash.h"

uint32_t tsorter_str_element_hash(struct tsorter_str_element const * e);
int tsorter_str_element_cmp(struct tsorter_str_element const * l, struct tsorter_str_element const * r);

struct tsorter_str_depend {
    struct tsorter_str_element * m_depend_from;
    TAILQ_ENTRY(tsorter_str_depend) m_next_for_depend_from;

    struct tsorter_str_element * m_depend_to;
    TAILQ_ENTRY(tsorter_str_depend) m_next_for_depend_to;
};

TAILQ_HEAD(tsorter_str_depend_list, tsorter_str_depend);

struct tsorter_str_element {
    const char * m_name;
    tsorter_str_t m_sorter;
    struct tsorter_str_depend_list  m_depend_tos;
    struct tsorter_str_depend_list m_depend_froms;
    struct cpe_hash_entry m_hh;

    TAILQ_ENTRY(tsorter_str_element) m_next;
};

TAILQ_HEAD(tsorter_str_element_list, tsorter_str_element);

struct tsorter_str {
    mem_allocrator_t m_alloc;
    struct cpe_hash_table m_elements;
    struct tsorter_str_element_list m_sorted_elements;
    int m_dup_str;
};

tsorter_str_t tsorter_str_create(mem_allocrator_t alloc, int dup_str) {
    tsorter_str_t sorter;

    sorter = mem_alloc(alloc, sizeof(struct tsorter_str));
    if (sorter == NULL) return NULL;

    sorter->m_alloc = alloc;
    sorter->m_dup_str = dup_str;

    if (cpe_hash_table_init(
            &sorter->m_elements,
            alloc,
            (cpe_hash_fun_t) tsorter_str_element_hash,
            (cpe_hash_cmp_t) tsorter_str_element_cmp,
            CPE_HASH_OBJ2ENTRY(tsorter_str_element, m_hh),
            -1) != 0)
    {
        mem_free(alloc, sorter);
        return NULL;
    }

    TAILQ_INIT(&sorter->m_sorted_elements);
    
    return sorter;
}

void tsorter_str_free(tsorter_str_t sorter) {
    struct cpe_hash_it element_it;
    tsorter_str_element_t element;

    TAILQ_INIT(&sorter->m_sorted_elements);
    
    cpe_hash_it_init(&element_it, &sorter->m_elements);
    element = cpe_hash_it_next(&element_it);
    while (element) {
        tsorter_str_element_t next = cpe_hash_it_next(&element_it);
        tsorter_str_element_free(element);
        element = next;
    }

    cpe_hash_table_fini(&sorter->m_elements);

    mem_free(sorter->m_alloc, sorter);
}

tsorter_str_element_t tsorter_str_element_check_create(tsorter_str_t sorter, const char * name) {
    tsorter_str_element_t r;
    struct tsorter_str_element key;
    key.m_name = name;

    r =  (tsorter_str_element_t)cpe_hash_table_find(&sorter->m_elements, &key);
    if (r == NULL) {
        size_t name_len = sorter->m_dup_str ? (strlen(name) + 1) : 0;

        r = (tsorter_str_element_t)mem_alloc(sorter->m_alloc, sizeof(struct tsorter_str_element) + name_len);
        if (r == NULL) return NULL;

        if (sorter->m_dup_str) {
            memcpy(r + 1, name, name_len);
            r->m_name = (const char *)(r + 1);
        }
        else {
            r->m_name = name;
        }

        r->m_sorter = sorter;
        TAILQ_INIT(&r->m_depend_tos);
        TAILQ_INIT(&r->m_depend_froms);

        cpe_hash_entry_init(&r->m_hh);
        if (cpe_hash_table_insert_unique(&sorter->m_elements, r) != 0) {
            mem_free(sorter->m_alloc, r);
            return NULL;
        }
    }

    assert(r);
    return r;
}

tsorter_str_element_t tsorter_str_element_find(tsorter_str_t sorter, const char * name) {
    struct tsorter_str_element key;
    key.m_name = name;

    return (tsorter_str_element_t)cpe_hash_table_find(&sorter->m_elements, &key);
}

void tsorter_str_element_free(tsorter_str_element_t element) {
    while(!TAILQ_EMPTY(&element->m_depend_tos)) {
        tsorter_str_depend_free(TAILQ_FIRST(&element->m_depend_tos));
    }
    
    while(!TAILQ_EMPTY(&element->m_depend_froms)) {
        tsorter_str_depend_free(TAILQ_FIRST(&element->m_depend_froms));
    }

    cpe_hash_table_remove_by_ins(&element->m_sorter->m_elements, element);

    mem_free(element->m_sorter->m_alloc, element);
}

int tsorter_str_add_dep(tsorter_str_element_t element, const char * dep_by) {
    tsorter_str_element_t element_depend_from;

    element_depend_from = tsorter_str_element_check_create(element->m_sorter, dep_by);
    if (element_depend_from == NULL) return -1;

    if (tsorter_str_depend_create(element, element_depend_from) == NULL) return -1;
    
    return 0;
}

tsorter_str_depend_t tsorter_str_depend_create(tsorter_str_element_t depend_to, tsorter_str_element_t depend_from) {
    tsorter_str_depend_t r;

    r = (tsorter_str_depend_t)mem_alloc(depend_to->m_sorter->m_alloc, sizeof(struct tsorter_str_depend));
    if (r == NULL) return NULL;

    r->m_depend_to = depend_to;
    r->m_depend_from = depend_from;

    TAILQ_INSERT_TAIL(&depend_to->m_depend_froms, r, m_next_for_depend_from);
    TAILQ_INSERT_TAIL(&depend_from->m_depend_tos, r, m_next_for_depend_to);

    return r;
}

void tsorter_str_depend_free(tsorter_str_depend_t dep) {
    TAILQ_REMOVE(&dep->m_depend_to->m_depend_froms, dep, m_next_for_depend_from);
    TAILQ_REMOVE(&dep->m_depend_from->m_depend_tos, dep, m_next_for_depend_to);
    mem_free(dep->m_depend_to->m_sorter->m_alloc, dep);
}

int tsorter_str_sort(tsorter_str_t sorter) {
    TAILQ_INIT(&sorter->m_sorted_elements);

    return 0;
}

const char * tsorter_str_next(tsorter_str_t sorter);

uint32_t tsorter_str_element_hash(struct tsorter_str_element const * e) {
    return cpe_hash_str(e->m_name, strlen(e->m_name));
}

int tsorter_str_element_cmp(struct tsorter_str_element const * l, struct tsorter_str_element const * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
