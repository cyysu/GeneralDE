#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/plist/plist_cfg.h"
#include "model_manip_cocos_utils.h"

int ui_model_import_cocos_module(ui_ed_mgr_t ed_mgr, const char * module_path, const char * plist, const char * pic, error_monitor_t em) {
    cfg_t root_cfg;
    ui_ed_src_t module_src;
    ui_ed_obj_t module_obj;
    UI_MODULE * module_data;
    struct cfg_it frame_it;
    cfg_t frame_cfg;
    uint32_t id = 0;

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

    /*创建图片 */
    cfg_it_init(&frame_it, cfg_find_cfg(root_cfg, "frames"));
    while((frame_cfg = cfg_it_next(&frame_it))) {
        const char * name = cfg_name(frame_cfg);
        char * end_p;
        int img_id = strtol(name, &end_p, 10);

        if (end_p <= name) {
            img_id = id;
        }

        if (cocos_build_img_block(frame_cfg, module_obj, img_id, em) == 0) {
            cfg_free(root_cfg);
            ui_ed_src_delete(module_src);
            return -1;
        }

        ++id;
    }

    cfg_free(root_cfg);
    return 0;
}
