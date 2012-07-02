#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "usf/logic/logic_data.h"
#include "logic_internal_ops.h"

static logic_data_list_t * logic_data_list(logic_data_t data) {
    switch(data->m_owner_type) {
    case logic_data_owner_context:
        return &data->m_owner_data.m_context->m_datas;
    case logic_data_owner_stack:
        return &data->m_owner_data.m_stack->m_datas;
    case logic_data_owner_require:
        return &data->m_owner_data.m_require->m_datas;
    }

    return NULL;
}

static logic_manage_t logic_data_dequeue(logic_manage_t mgr, logic_data_t data) {
    logic_data_list_t * data_list;

    data_list = logic_data_list(data);
    assert(data_list);

    TAILQ_REMOVE(data_list, data, m_next);

    return mgr;
}

void logic_data_init_data(logic_data_t data, logic_data_t old_data) {
    if (old_data) {
        assert(data->m_capacity >= old_data->m_capacity);
        memcpy(data + 1, old_data + 1, old_data->m_capacity);
        bzero(((char *)(data + 1)) + old_data->m_capacity, data->m_capacity - old_data->m_capacity);
    }
    else {
        bzero(data + 1, data->m_capacity);
    }
}

logic_data_t 
logic_data_get_or_create_i(logic_data_t key, LPDRMETA meta, size_t capacity, logic_data_t src_data) {
    logic_data_t old_data;
    logic_data_t new_data;
    logic_manage_t mgr;
    logic_data_list_t * data_list;

    key->m_name = dr_meta_name(meta);

    mgr = logic_data_mgr(key);
    assert(mgr);

    data_list = logic_data_list(key);
    assert(data_list);

    if (capacity == 0) capacity = dr_meta_size(meta);

    old_data = (logic_data_t)cpe_hash_table_find(&mgr->m_datas, key);
    if (old_data && old_data->m_capacity >= capacity) {

        if (src_data) logic_data_init_data(old_data, src_data);
        return old_data;
    }

    new_data = (logic_data_t)mem_alloc(mgr->m_alloc, sizeof(struct logic_data) + capacity);
    if (new_data == NULL) return NULL;

    new_data->m_owner_type = key->m_owner_type;
    new_data->m_owner_data = key->m_owner_data;
    new_data->m_name = key->m_name;
    new_data->m_meta = meta;
    new_data->m_capacity = capacity;
    cpe_hash_entry_init(&new_data->m_hh);

    if (src_data) {
        logic_data_init_data(new_data, src_data);
    }
    else if (old_data) {
        logic_data_init_data(new_data, old_data);
    }
    else {
        logic_data_init_data(new_data, NULL);
    }

    if (old_data) logic_data_free(old_data);

    if (cpe_hash_table_insert_unique(&mgr->m_datas, new_data) != 0) {
        mem_free(mgr->m_alloc, new_data);
        return NULL;
    }

    TAILQ_INSERT_TAIL(data_list, new_data, m_next);

    return new_data;
}

logic_context_t logic_data_context(logic_data_t data) {
    switch(data->m_owner_type) {
    case logic_data_owner_context:
        return data->m_owner_data.m_context;
    case logic_data_owner_stack:
        return data->m_owner_data.m_stack->m_context;
    case logic_data_owner_require:
        return data->m_owner_data.m_require->m_context;
    }

    return NULL;
}

logic_manage_t logic_data_mgr(logic_data_t data) {
    logic_context_t context = logic_data_context(data);

    return context ? context->m_mgr : NULL;
}

logic_data_t logic_context_data_find(logic_context_t context, const char * name) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_context;
    key.m_owner_data.m_context = context;
    key.m_name = name;

    return (logic_data_t)cpe_hash_table_find(&context->m_mgr->m_datas, &key);    
}

logic_data_t logic_context_data_get_or_create(logic_context_t context, LPDRMETA meta, size_t capacity) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_context;
    key.m_owner_data.m_context = context;

    return logic_data_get_or_create_i(&key, meta, capacity, NULL);
}

logic_data_t logic_context_data_copy(logic_context_t context, logic_data_t data) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_context;
    key.m_owner_data.m_context = context;

    return logic_data_get_or_create_i(&key, data->m_meta, data->m_capacity, data);
}

logic_data_t logic_stack_data_find(logic_stack_node_t stack_node, const char * name) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_stack;
    key.m_owner_data.m_stack = stack_node;
    key.m_name = name;

    return (logic_data_t)cpe_hash_table_find(&stack_node->m_context->m_mgr->m_datas, &key);    
}

logic_data_t logic_stack_data_get_or_create(logic_stack_node_t stack_node, LPDRMETA meta, size_t capacity) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_stack;
    key.m_owner_data.m_stack = stack_node;

    return logic_data_get_or_create_i(&key, meta, capacity, NULL);
}

logic_data_t logic_stack_data_copy(logic_stack_node_t stack_node, logic_data_t data) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_stack;
    key.m_owner_data.m_stack = stack_node;

    return logic_data_get_or_create_i(&key, data->m_meta, data->m_capacity, data);
}

logic_data_t logic_require_data_find(logic_require_t require, const char * name) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_require;
    key.m_owner_data.m_require = require;
    key.m_name = name;

    return (logic_data_t)cpe_hash_table_find(&require->m_context->m_mgr->m_datas, &key);    
}

logic_data_t logic_require_data_get_or_create(logic_require_t require, LPDRMETA meta, size_t capacity) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_require;
    key.m_owner_data.m_require = require;

    return logic_data_get_or_create_i(&key, meta, capacity, NULL);
}

logic_data_t logic_require_data_copy(logic_require_t require, logic_data_t data) {
    struct logic_data key;

    key.m_owner_type = logic_data_owner_require;
    key.m_owner_data.m_require = require;

    return logic_data_get_or_create_i(&key, data->m_meta, data->m_capacity, data);
}

void logic_data_free(logic_data_t data) {
    logic_manage_t mgr;

    assert(data);

    mgr = logic_data_mgr(data);
    assert(mgr);

    logic_data_dequeue(mgr, data);

    cpe_hash_table_remove_by_ins(&mgr->m_datas, data);

    mem_free(mgr->m_alloc, data);
}

void logic_data_free_all(logic_manage_t mgr) {
    struct cpe_hash_it data_it;
    logic_data_t data;

    cpe_hash_it_init(&data_it, &mgr->m_datas);

    data = cpe_hash_it_next(&data_it);
    while (data) {
        logic_data_t next = cpe_hash_it_next(&data_it);
        logic_data_free(data);
        data = next;
    }
}

LPDRMETA logic_data_meta(logic_data_t data) {
    return data->m_meta;
}

void * logic_data_data(logic_data_t data) {
    return data + 1;
}

size_t logic_data_capacity(logic_data_t data) {
    return data->m_capacity;
}

const char * logic_data_name(logic_data_t data) {
    return data->m_name;
}

uint32_t logic_data_hash(const struct logic_data * data) {
    switch(data->m_owner_type) {
    case logic_data_owner_context:
        return (cpe_hash_str(data->m_name, strlen(data->m_name)) << 4)
            | (data->m_owner_data.m_context->m_id & 0xFF);
    case logic_data_owner_stack:
        return cpe_hash_str(data->m_name, strlen(data->m_name));
    case logic_data_owner_require:
        return (cpe_hash_str(data->m_name, strlen(data->m_name)) << 4)
            | (data->m_owner_data.m_require->m_id & 0xFF);
    }

    assert(0);
    return 1;
}

int logic_data_cmp(const struct logic_data * l, const struct logic_data * r) {
    if (l->m_owner_type != r->m_owner_type || strcmp(l->m_name, r->m_name) != 0) return -1;

    switch(l->m_owner_type) {
    case logic_data_owner_context:
        return l->m_owner_data.m_context->m_id == r->m_owner_data.m_context->m_id;
    case logic_data_owner_stack:
        return l->m_owner_data.m_stack == r->m_owner_data.m_stack;
    case logic_data_owner_require:
        return l->m_owner_data.m_require->m_id == r->m_owner_data.m_require->m_id;
    }

	assert(1 && "unknown owner type");
	return 0;
}

struct logic_data_next_data {
    logic_data_t m_next;
};

static logic_data_t logic_data_it_next(struct logic_data_it * it) {
    struct logic_data_next_data * data;
    logic_data_t r;

    assert(sizeof(struct logic_data_next_data) <= sizeof(it->m_data));

    data = (struct logic_data_next_data *)it->m_data;

    if (data->m_next == NULL) return NULL;

    r = data->m_next;
    data->m_next = TAILQ_NEXT(data->m_next, m_next);
    return r;
}

void logic_context_datas(logic_context_t context, logic_data_it_t it) {
    struct logic_data_next_data * data;
    
    assert(sizeof(struct logic_data_next_data) <= sizeof(it->m_data));

    data = (struct logic_data_next_data *)it->m_data;

    data->m_next = TAILQ_FIRST(&context->m_datas);

    it->next = logic_data_it_next;
}

void logic_stack_node_datas(logic_stack_node_t stack, logic_data_it_t it) {
    struct logic_data_next_data * data;
    
    assert(sizeof(struct logic_data_next_data) <= sizeof(it->m_data));

    data = (struct logic_data_next_data *)it->m_data;

    data->m_next = TAILQ_FIRST(&stack->m_datas);

    it->next = logic_data_it_next;
}

void logic_require_datas(logic_require_t require, logic_data_it_t it) {
    struct logic_data_next_data * data;
    
    assert(sizeof(struct logic_data_next_data) <= sizeof(it->m_data));

    data = (struct logic_data_next_data *)it->m_data;

    data->m_next = TAILQ_FIRST(&require->m_datas);

    it->next = logic_data_it_next;
}

