#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/plist/plist_cfg.h"
#include "ui/model/ui_data_module.h"
#include "ui/model/ui_data_action.h"
#include "ui/model_manip/model_manip_cocos.h"
#include "model_manip_cocos_utils.h"

struct ui_mode_import_frame_node {
    UI_IMG_BLOCK * img_block;
    cfg_t cfg;
    int order;
};

static int ui_model_import_frame_node_cmp_by_order(const void * l, const void * r) {
    return ((struct ui_mode_import_frame_node const *)l)->order
        - ((struct ui_mode_import_frame_node const *)r)->order;
}

static int32_t ui_model_import_frame_gen_order_by_postfix(UI_IMG_BLOCK * img_block);

int ui_model_import_cocos_action(
    ui_ed_mgr_t ed_mgr, const char * action_path, const char * module_path,
    const char * plist, const char * pic,
    uint32_t frame_duration,
    ui_manip_action_import_frame_position_t frame_position,
    ui_manip_action_import_frame_order_t frame_order,
    error_monitor_t em)
{
    cfg_t root_cfg;
    ui_ed_src_t module_src;
    ui_ed_obj_t module_obj;
    UI_MODULE * module_data;
    ui_ed_src_t action_src;
    ui_ed_obj_t actor_obj;
    ui_ed_obj_t actor_layer_obj;
    struct cfg_it frame_it;
    cfg_t frame_cfg;
    uint32_t id = 0;
    struct ui_mode_import_frame_node * frame_nodes = NULL;
    uint32_t frame_count;
    uint32_t i;

    /*读取配置文件 */
    root_cfg = plist_cfg_load_dict_from_file(plist, em);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "check create module at %s: read plist %s fail!", module_path, plist);
        return -1;
    }

    module_src = ui_ed_src_check_create(ed_mgr, module_path, ui_data_src_type_module);
    if (module_src == NULL) {
        CPE_ERROR(em, "check create module at %s fail!", module_path);
        cfg_free(root_cfg);
        return -1;
    }

    /*创建module */
    module_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(module_src));
    assert(module_obj);
    ui_ed_obj_remove_childs(module_obj);

    /*设置来源的图像文件 */
    module_data = ui_ed_obj_data(module_obj);
    strncpy(module_data->use_img, pic, sizeof(module_data->use_img));

    /*创建action*/
    action_src = ui_ed_src_check_create(ed_mgr, action_path, ui_data_src_type_action);
    if (action_src == NULL) {
        CPE_ERROR(em, "build_action_from_frame: check create action at %s fail!", action_path);
        return -1;
    }

    ui_ed_obj_remove_childs(ui_ed_src_root_obj(action_src));
    actor_obj = ui_ed_obj_new(ui_ed_src_root_obj(action_src), ui_ed_obj_actor);
    if (actor_obj == NULL) {
        CPE_ERROR(em, "build_action_from_frame: create root actor fail!");
        ui_ed_src_delete(action_src);
        return -1;
    }

    actor_layer_obj = ui_ed_obj_new(actor_obj, ui_ed_obj_actor_layer);
    if (actor_layer_obj == NULL) {
        CPE_ERROR(em, "build_action_from_frame: create actor layer fail!");
        ui_ed_src_delete(action_src);
        return -1;
    }

    /*创建frame和actor_frame */
    frame_count = cfg_child_count(cfg_find_cfg(root_cfg, "frames"));
    frame_nodes = mem_alloc(NULL, sizeof(struct ui_mode_import_frame_node) * frame_count);
    if (frame_nodes == NULL) {
        CPE_ERROR(em, "build_action_from_frame: alloc frame nodes fail!");
        ui_ed_src_delete(action_src);
        return -1;
    }

    cfg_it_init(&frame_it, cfg_find_cfg(root_cfg, "frames"));
    while((frame_cfg = cfg_it_next(&frame_it))) {
        frame_nodes[id].cfg = frame_cfg;

        frame_nodes[id].img_block = cocos_build_img_block(frame_cfg, module_obj, id, em);
        if (frame_nodes[id].img_block == NULL) {
            cfg_free(root_cfg);
            ui_ed_src_delete(module_src);
            mem_free(NULL, frame_nodes);
            return -1;
        }
        ++id;
    }

    /*frame排序*/
    switch(frame_order) {
    case ui_manip_action_import_frame_order_native:
        break;
    case ui_manip_action_import_frame_order_postfix:
        for(i = 0; i < frame_count; ++i) {
            struct ui_mode_import_frame_node * node = frame_nodes + i;
            node->order = ui_model_import_frame_gen_order_by_postfix(node->img_block);
        }
        qsort(frame_nodes, frame_count, sizeof(frame_nodes[0]), ui_model_import_frame_node_cmp_by_order);
        break;
    default:
        CPE_ERROR(em, "build_action_from_frame: unknown order %d!", frame_order);
        cfg_free(root_cfg);
        ui_ed_src_delete(module_src);
        mem_free(NULL, frame_nodes);
        return -1;
    }

    for(i = 0; i < frame_count; ++i) {
        struct ui_mode_import_frame_node * node = frame_nodes + i;
        UI_ACTOR_FRAME * frame = 
            cocos_build_actor_frame(node->cfg, actor_layer_obj, ui_ed_src_id(module_src), node->img_block, i, frame_duration, frame_position, em);
        const char * offset;
        if (frame == NULL) {
            cfg_free(root_cfg);
            ui_ed_src_delete(module_src);
            mem_free(NULL, frame_nodes);
            return -1;
        }

        if ((offset = cfg_get_string(node->cfg, "offset", NULL))) {
            int offset_x;
            int offset_y;
            sscanf(offset, "{%d,%d}", &offset_x, &offset_y);
            frame->texture.data.img.trans.world_trans.trans.value[0] = (float)offset_x;
            frame->texture.data.img.trans.world_trans.trans.value[1] = (float)-offset_y;
        }
    }

    mem_free(NULL, frame_nodes);
    cfg_free(root_cfg);
    return 0;
}

static int32_t ui_model_import_frame_gen_order_by_postfix(UI_IMG_BLOCK * img_block) {
    const char * scan_p;

    scan_p = strrchr(img_block->name, '.');
    if (scan_p == NULL) {
        scan_p = img_block->name + strlen(img_block->name);
    }

    --scan_p;
    while(scan_p > img_block->name && *(scan_p - 1) >= '0' && *(scan_p - 1) <= '9') {
        --scan_p;
    }

    return atoi(scan_p);
}
