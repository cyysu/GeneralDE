#ifndef UI_MODEL_DATA_SRC_H
#define UI_MODEL_DATA_SRC_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_it {
    ui_data_src_t (*next)(struct ui_data_src_it * it);
    char m_data[64];
};

struct ui_data_src_ref_it {
    ui_data_src_ref_t (*next)(struct ui_data_src_ref_it * it);
    char m_data[64];
};

/*src create and free*/
ui_data_src_t ui_data_src_create_file(ui_data_mgr_t mgr, ui_data_src_type_t type, const char * full_file);
ui_data_src_t ui_data_src_create_relative(ui_data_mgr_t mgr, ui_data_src_type_t type, const char * path);
ui_data_src_t ui_data_src_create_child(ui_data_src_t parent, ui_data_src_type_t type, const char * name);
void ui_data_src_free(ui_data_src_t src);

/*src mgr*/
uint32_t ui_data_src_id(ui_data_src_t src);
int ui_data_src_set_id(ui_data_src_t src, uint32_t id);
ui_data_mgr_t ui_data_src_mgr(ui_data_src_t src);

ui_data_src_type_t ui_data_src_type(ui_data_src_t src);
const char * ui_data_src_data(ui_data_src_t src);
ui_data_src_t ui_data_src_find_by_id(ui_data_mgr_t mgr, uint32_t id);

/*src and product*/
#define ui_data_src_is_loaded(src) (ui_data_src_load_state(src) == ui_data_src_state_loaded)

ui_data_src_load_state_t ui_data_src_load_state(ui_data_src_t src);
void * ui_data_src_product(ui_data_src_t src);
int ui_data_src_load(ui_data_src_t src, error_monitor_t em);
int ui_data_src_init(ui_data_src_t src);
void ui_data_src_unload(ui_data_src_t src);
int ui_data_src_save(ui_data_src_t src, const char * root, error_monitor_t em);
int ui_data_src_remove(ui_data_src_t src, const char * root, error_monitor_t em);

/*src tree*/
ui_data_src_t ui_data_src_parent(ui_data_src_t src);
void ui_data_src_childs(ui_data_src_it_t it, ui_data_src_t src);
void ui_data_src_all_childs(ui_data_src_it_t it, ui_data_src_t src);
ui_data_src_t ui_data_src_child_find(ui_data_src_t src, const char * name);
ui_data_src_t ui_data_src_child_find_by_path(ui_data_src_t src, const char * path);
void ui_data_src_path_print_to(write_stream_t s, ui_data_src_t src, ui_data_src_t stop);
void ui_data_src_path_print(write_stream_t s, ui_data_src_t src);
const char * ui_data_src_path_dump(mem_buffer_t buff, ui_data_src_t src);

/*src using relation*/
void ui_data_src_using_refs(ui_data_src_ref_it_t it, ui_data_src_t src);
void ui_data_src_using_srcs(ui_data_src_it_t it, ui_data_src_t src);
void ui_data_src_users(ui_data_src_it_t it, ui_data_src_t src);
#define ui_data_src_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*src_ref*/
uint32_t ui_data_src_ref_using_src_id(ui_data_src_ref_t src_ref);
ui_data_src_type_t ui_data_src_ref_using_src_type(ui_data_src_ref_t src_ref);
ui_data_src_t ui_data_src_ref_using_src(ui_data_src_ref_t src_ref);
ui_data_src_t ui_data_src_ref_user(ui_data_src_ref_t src_ref);
int ui_data_src_ref_bind(ui_data_src_ref_t src_ref);

#define ui_data_src_ref_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*static*/
const char * ui_data_src_type_name(ui_data_src_type_t type);

#ifdef __cplusplus
}
#endif

#endif
