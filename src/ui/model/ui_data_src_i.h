#ifndef UI_DATA_SRC_INTERNAL_H
#define UI_DATA_SRC_INTERNAL_H
#include "ui/model/ui_data_src.h"
#include "ui_model_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_src_ref_list, ui_data_src_ref) ui_data_src_ref_list_t;
typedef TAILQ_HEAD(ui_data_src_list, ui_data_src) ui_data_src_list_t;

struct ui_data_src {
    /*全局管理相关数据 */
    ui_data_mgr_t m_mgr;
    struct cpe_hash_entry m_hh_for_mgr;
    struct cpe_hash_entry m_hh_for_mgr_id;

    /*树形关系数据 */
    ui_data_src_t m_parent;
    ui_data_src_list_t m_childs;
    TAILQ_ENTRY(ui_data_src) m_next_for_parent;

    /*使用关系数据 */
    ui_data_src_ref_list_t m_using_srcs;
    ui_data_src_ref_list_t m_user_srcs;

    /*业务数据 */
    ui_data_src_type_t m_type;
    ui_data_src_load_state_t m_state;
    char * m_data; 
    void * m_product;
    uint32_t m_id;
};

struct ui_data_src_ref {
    uint32_t m_using_src_id;
    ui_data_src_type_t m_using_src_type;
    ui_data_src_t m_using_src;
    TAILQ_ENTRY(ui_data_src_ref) m_next_for_using;
    ui_data_src_t m_user_src;
    TAILQ_ENTRY(ui_data_src_ref) m_next_for_user;
};

ui_data_src_t ui_data_src_create_i(ui_data_mgr_t mgr, ui_data_src_t parent, ui_data_src_type_t type, const char * data);

uint32_t ui_data_src_hash(const ui_data_src_t src);
int ui_data_src_eq(const ui_data_src_t l, const ui_data_src_t r);

uint32_t ui_data_src_id_hash(const ui_data_src_t src);
int ui_data_src_id_eq(const ui_data_src_t l, const ui_data_src_t r);

ui_data_src_ref_t ui_data_src_ref_create(uint32_t using_src_id, ui_data_src_type_t using_src_type, ui_data_src_t using_src, ui_data_src_t user_src);
void ui_data_src_ref_free(ui_data_src_ref_t src_ref);

#ifdef __cplusplus
}
#endif

#endif
