#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "ui_data_src_i.h"

ui_data_src_t
ui_data_src_create_i(ui_data_mgr_t mgr, ui_data_src_t parent, ui_data_src_type_t type, const char * data) {
    ui_data_src_t src;
    size_t data_len = strlen(data);

    if (data_len > 0 && data[data_len - 1] == '/') data_len--;

    src = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src));
    if (src == NULL) {
        CPE_ERROR(mgr->m_em, "src_create: %s: alloc fail!", data);
        return NULL;
    }

    src->m_mgr = mgr;
    src->m_parent = parent;
    src->m_type = type;
    src->m_product = NULL;
    src->m_id = 0;

    TAILQ_INIT(&src->m_childs);
    TAILQ_INIT(&src->m_using_srcs);
    TAILQ_INIT(&src->m_user_srcs);

    src->m_data = mem_alloc(mgr->m_alloc, data_len + 1);
    if (src->m_data == NULL) {
        CPE_ERROR(mgr->m_em, "src_create: %s: alloc for data fail!", data);
        mem_free(mgr->m_alloc, src);
        return NULL;
    }
    memcpy(src->m_data, data, data_len);
    src->m_data[data_len] = 0;

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_childs, src, m_next_for_parent);
    }

    cpe_hash_entry_init(&src->m_hh_for_mgr);
    if (cpe_hash_table_insert(&mgr->m_srcs, src) != 0) {
        CPE_ERROR(mgr->m_em, "src_create: %s: hash table insert fail!", data);
        if (parent) TAILQ_REMOVE(&parent->m_childs, src, m_next_for_parent);
        mem_free(mgr->m_alloc, (void*)src->m_data);
        mem_free(mgr->m_alloc, src);
        return NULL;
    }

    return src;
}

ui_data_src_t ui_data_src_create_child(ui_data_src_t parent, ui_data_src_type_t type, const char * name) {
    return ui_data_src_create_i(parent->m_mgr, parent, type, name);
}

void ui_data_src_free(ui_data_src_t src) {
    ui_data_mgr_t mgr = src->m_mgr;

    /*释放子资源*/
    while(!TAILQ_EMPTY(&src->m_childs)) {
        ui_data_src_t c = TAILQ_FIRST(&src->m_childs);
        ui_data_src_free(c);
        assert(c != TAILQ_FIRST(&src->m_childs));
    }

    /*如果已经加载了数据，则释放数据 */
    if (src->m_product) {
        ui_product_type_of(src->m_mgr, src->m_type)->product_free(src->m_product);
        src->m_product = NULL;
    }

    /*自己使用的引用关系应该已经释放干净了 !!!*/
    assert(TAILQ_EMPTY(&src->m_using_srcs));

    /*解除所有到自己的引用 */
    while(!TAILQ_EMPTY(&src->m_user_srcs)) {
        ui_data_src_ref_t user_src = TAILQ_FIRST(&src->m_user_srcs);

        assert(user_src->m_using_src == src);

        TAILQ_REMOVE(&src->m_user_srcs, user_src, m_next_for_user);
        user_src->m_using_src = NULL;
    }

    if (src->m_parent) {
        TAILQ_REMOVE(&src->m_parent->m_childs, src, m_next_for_parent);
    }

    if (src->m_id) {
        cpe_hash_table_remove_by_ins(&mgr->m_srcs_by_id, src);
        src->m_id = 0;
    }

    cpe_hash_table_remove_by_ins(&mgr->m_srcs, src);

    mem_free(mgr->m_alloc, (void*)src->m_data);
    mem_free(mgr->m_alloc, src);
}

uint32_t ui_data_src_id(ui_data_src_t src) {
    return src->m_id;
}

void * ui_data_src_product(ui_data_src_t src) {
    return src->m_product;
}

int ui_data_src_set_id(ui_data_src_t src, uint32_t id) {
    ui_data_mgr_t mgr = src->m_mgr;
    uint32_t old_id = src->m_id;

    if (old_id) {
        cpe_hash_table_remove_by_ins(&mgr->m_srcs_by_id, src);
    }

    src->m_id = id;

    if (src->m_id) {
        cpe_hash_entry_init(&src->m_hh_for_mgr_id);
        if (cpe_hash_table_insert_unique(&mgr->m_srcs_by_id, src) != 0) {
            CPE_ERROR(mgr->m_em, "set src id: id %d duplicate", src->m_id);
            src->m_id = old_id;
            if (src->m_id) {
                cpe_hash_entry_init(&src->m_hh_for_mgr_id);
                cpe_hash_table_insert_unique(&mgr->m_srcs_by_id, src);
            }
            return -1;
        }
    }

    return 0;
}

ui_data_mgr_t ui_data_src_mgr(ui_data_src_t src) {
    return src->m_mgr;
}

ui_data_src_t ui_data_src_find_by_id(ui_data_mgr_t mgr, uint32_t id) {
    struct ui_data_src key;
    key.m_id = id;

    return cpe_hash_table_find(&mgr->m_srcs_by_id, &key);
}

ui_data_src_type_t ui_data_src_type(ui_data_src_t src) {
    return src->m_type;
}

const char * ui_data_src_data(ui_data_src_t src) {
    return src->m_data;
}

ui_data_src_t ui_data_src_parent(ui_data_src_t src) {
    return src->m_parent;
}

ui_data_src_t ui_data_src_child_find(ui_data_src_t src, const char * name) {
    struct ui_data_src key;
    ui_data_src_t r;

    key.m_data = (char*)name;

    for(r = cpe_hash_table_find(&src->m_mgr->m_srcs, &key);
        r;
        r = cpe_hash_table_find_next(&src->m_mgr->m_srcs, r))
    {
        if (r->m_parent == src) return r;
    }

    return NULL;
}

ui_data_src_t ui_data_src_child_find_by_path(ui_data_src_t src, const char * path) {
    ui_data_src_t r = src;
    const char * sep;

    while(r && (sep = strchr(path, '/'))) {
        char name_buf[64];
        int name_len = sep -path;

        if (name_len + 1 >= sizeof(name_buf)) {
            CPE_ERROR(src->m_mgr->m_em, "src_child_find_by_path: name overflow!");
            return NULL;
        }
        else if (name_len == 0) {
            CPE_ERROR(src->m_mgr->m_em, "src_child_find_by_path: name empty!");
            return NULL;
        }

        memcpy(name_buf, path, name_len);
        name_buf[name_len] = 0;
        path += name_len + 1;

        r = ui_data_src_child_find(r, name_buf);
    }

    if (r && *path != 0) {
        r = ui_data_src_child_find(r, path);
    }

    return r;
}

static ui_data_src_t ui_data_src_childs_next(struct ui_data_src_it * it) {
    ui_data_src_t * data = (ui_data_src_t *)(it->m_data);
    ui_data_src_t r;

    if (*data == NULL) return NULL;

    r = *data;

    assert(r->m_parent);

    *data = TAILQ_NEXT(r, m_next_for_parent);

    return r;
}

void ui_data_src_childs(ui_data_src_it_t it, ui_data_src_t src) {
    *(ui_data_src_t *)(it->m_data) = TAILQ_FIRST(&src->m_childs);
    it->next = ui_data_src_childs_next;
}

struct ui_data_src_all_childs_it_data {
    ui_data_src_t m_root;
    ui_data_src_t m_cur;
};

static ui_data_src_t ui_data_src_all_childs_next(struct ui_data_src_it * it) {
    struct ui_data_src_all_childs_it_data * data = (struct ui_data_src_all_childs_it_data *)(it->m_data);
    ui_data_src_t r;
    ui_data_src_t n;

    if (data->m_cur == NULL) return NULL;

    r = data->m_cur;

    if (r == data->m_root) {
        n = NULL;
    }
    else {
        n = TAILQ_NEXT(r, m_next_for_parent);
        if (n) {
            while(!TAILQ_EMPTY(&n->m_childs)) {
                n = TAILQ_FIRST(&n->m_childs);
            }
        }
        else {
            n = r->m_parent;
        }
    }

    data->m_cur = n;

    return r;
}

void ui_data_src_all_childs(ui_data_src_it_t it, ui_data_src_t src) {
    struct ui_data_src_all_childs_it_data * data = (struct ui_data_src_all_childs_it_data *)(it->m_data);
    ui_data_src_t r = src;

    while(!TAILQ_EMPTY(&r->m_childs)) {
        r = TAILQ_FIRST(&r->m_childs);
    }

    data->m_root = src;
    data->m_cur = r;
    it->next = ui_data_src_all_childs_next;
}

ui_data_src_load_state_t ui_data_src_load_state(ui_data_src_t src) {
    return src->m_product == NULL
        ? ui_data_src_state_notload
        : ui_data_src_state_loaded;
}

int ui_data_src_init(ui_data_src_t src) {
    void * p;

    ui_data_src_unload(src);

    p = ui_product_type_of(src->m_mgr, src->m_type)->product_create(src->m_mgr, src);
    if (p == NULL) return -1;

    assert(p == src->m_product);

    return 0;
}

int ui_data_src_load(ui_data_src_t src, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_product_type * product_type = ui_product_type_of(mgr, src->m_type);
    int r;

    if (product_type->product_load == NULL) {
        CPE_ERROR(em, "%s not support load", ui_data_src_type_name(src->m_type));
        return -1;
    }

    if (src->m_product) {
        CPE_ERROR(em, "%s is already loaded", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }

    if ((r = product_type->product_load(product_type->product_load_ctx, mgr, src, em))) return r;

    if (src->m_product == NULL) {
        CPE_ERROR(em, "%s load complete, no product!", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }

    return 0;
}

void ui_data_src_unload(ui_data_src_t src) {
    if (src->m_product) {
        ui_product_type_of(src->m_mgr, src->m_type)->product_free(src->m_product);
        src->m_product = NULL;
    }
}

int ui_data_src_save(ui_data_src_t src, const char * root, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_product_type * product_type = ui_product_type_of(mgr, src->m_type);
    int r;

    if (product_type->product_save == NULL) {
        CPE_ERROR(em, "%s not support save", ui_data_src_type_name(src->m_type));
        return -1;
    }

    if (src->m_product == NULL) {
        CPE_ERROR(em, "%s not loaded", ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        return -1;
    }

    if (root == NULL) root = mgr->m_src_root->m_data;

    if ((r = product_type->product_save(product_type->product_save_ctx, mgr, src, root, em))) return r;

    return 0;
}

int ui_data_src_remove(ui_data_src_t src, const char * root, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    struct ui_product_type * product_type = ui_product_type_of(mgr, src->m_type);
    int r;

    if (product_type->product_remove == NULL) {
        CPE_ERROR(em, "%s not support remove", ui_data_src_type_name(src->m_type));
        return -1;
    }

    if (root == NULL) root = mgr->m_src_root->m_data;

    if ((r = product_type->product_remove(product_type->product_save_ctx, mgr, src, root, em))) return r;

    return 0;
}

ui_data_src_ref_t ui_data_src_ref_create(uint32_t using_src_id, ui_data_src_type_t using_src_type, ui_data_src_t using_src, ui_data_src_t user_src) {
    ui_data_mgr_t mgr = user_src->m_mgr;
    ui_data_src_ref_t ref;

    ref = mem_alloc(mgr->m_alloc, sizeof(struct ui_data_src_ref));
    if (ref == NULL) {
        CPE_ERROR(mgr->m_em, "alloc ui_data_src_ref fail!");
        return NULL;
    }

    ref->m_using_src_id = using_src_id;
    ref->m_using_src_type = using_src_type;
    ref->m_using_src = using_src;
    ref->m_user_src = user_src;

    TAILQ_INSERT_TAIL(&user_src->m_using_srcs, ref, m_next_for_using);

    if (using_src) {
        TAILQ_INSERT_TAIL(&using_src->m_user_srcs, ref, m_next_for_user);
    }

    return ref;
}

void ui_data_src_ref_free(ui_data_src_ref_t src_ref) {
    ui_data_mgr_t mgr;

    assert(src_ref->m_user_src);

    mgr = src_ref->m_user_src->m_mgr;

    TAILQ_REMOVE(&src_ref->m_user_src->m_using_srcs, src_ref, m_next_for_using);

    if (src_ref->m_using_src) {
        TAILQ_REMOVE(&src_ref->m_using_src->m_user_srcs, src_ref, m_next_for_user);
    }

    mem_free(mgr->m_alloc, src_ref);
}

uint32_t ui_data_src_hash(const ui_data_src_t src) {
    return cpe_hash_str(src->m_data, strlen(src->m_data));
}

int ui_data_src_eq(const ui_data_src_t l, const ui_data_src_t r) {
    return strcmp(l->m_data, r->m_data) == 0;
}

uint32_t ui_data_src_id_hash(const ui_data_src_t src) {
    return src->m_id;
}

int ui_data_src_id_eq(const ui_data_src_t l, const ui_data_src_t r) {
    return l->m_id == r->m_id;
}

void ui_data_src_path_print_to(write_stream_t s, ui_data_src_t src, ui_data_src_t to) {
    if (src == to) return;

    if (src->m_parent != to) {
        ui_data_src_path_print_to(s, src->m_parent, to);
        stream_printf(s, "/%s", src->m_data);
    }
    else {
        stream_printf(s, "%s", src->m_data);
    }
}

void ui_data_src_path_print(write_stream_t s, ui_data_src_t src) {
    ui_data_src_path_print_to(s, src, src->m_mgr->m_src_root);
}

const char * ui_data_src_path_dump(mem_buffer_t buffer, ui_data_src_t src) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    ui_data_src_path_print((write_stream_t)&stream, src);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

uint32_t ui_data_src_ref_using_src_id(ui_data_src_ref_t src_ref) {
    return src_ref->m_using_src_id;
}

ui_data_src_type_t ui_data_src_ref_using_src_type(ui_data_src_ref_t src_ref) {
    return src_ref->m_using_src_type;
}

ui_data_src_t ui_data_src_ref_using_src(ui_data_src_ref_t src_ref) {
    return src_ref->m_using_src;
}

ui_data_src_t ui_data_src_ref_user(ui_data_src_ref_t src_ref) {
    return src_ref->m_user_src;
}

int ui_data_src_ref_bind(ui_data_src_ref_t src_ref) {
    if (src_ref->m_using_src && src_ref->m_using_src->m_id == src_ref->m_using_src_id) return 0;

    if (src_ref->m_using_src) {
        TAILQ_REMOVE(&src_ref->m_using_src->m_user_srcs, src_ref, m_next_for_user);
        src_ref->m_user_src = NULL;
    }

    if (src_ref->m_using_src_id == 0) return -1;

    src_ref->m_user_src = ui_data_src_find_by_id(src_ref->m_user_src->m_mgr, src_ref->m_using_src_id);
    if (src_ref->m_user_src == NULL) return -1;

    TAILQ_INSERT_TAIL(&src_ref->m_using_src->m_user_srcs, src_ref, m_next_for_user);
    return 0;
}

const char * ui_data_src_type_name(ui_data_src_type_t type) {
    switch(type) {
    case ui_data_src_type_dir:
        return "dir";
    case ui_data_src_type_module:
        return "module";
    case ui_data_src_type_sprite:
        return "sprite";
    case ui_data_src_type_action:
        return "action";
    case ui_data_src_type_layout:
        return "layout";
    case ui_data_src_type_texture_png:
        return "png";
    default:
        return "unknown";
    }
}
