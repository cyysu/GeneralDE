#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "ui/model_ed/ui_ed_mgr.h"
#include "ui/model_manip/model_manip_cocos.h"
#include "ops.h"

int do_cocos_effect_import(
    ui_data_mgr_t data_mgr,
    const char * to_effect, const char * to_module,
    const char * plist, const char * pic, uint8_t frame_duration,
    const char * str_frame_position, const char * str_frame_order,
    error_monitor_t em)
{
    ui_ed_mgr_t ed_mgr = NULL;
    ui_manip_action_import_frame_position_t frame_position;
    ui_manip_action_import_frame_order_t frame_order;
    int rv = -1;

    if (strcmp(str_frame_position, "center") == 0) {
        frame_position = ui_manip_action_import_frame_center;
    }
    else if (strcmp(str_frame_position, "center-left") == 0) {
        frame_position = ui_manip_action_import_frame_center_left;
    }
    else if (strcmp(str_frame_position, "center-right") == 0) {
        frame_position = ui_manip_action_import_frame_center_right;
    }
    else if (strcmp(str_frame_position, "bottom-center") == 0) {
        frame_position = ui_manip_action_import_frame_bottom_center;
    }
    else if (strcmp(str_frame_position, "bottom-left") == 0) {
        frame_position = ui_manip_action_import_frame_bottom_left;
    }
    else if (strcmp(str_frame_position, "bottom-right") == 0) {
        frame_position = ui_manip_action_import_frame_bottom_right;
    }
    else if (strcmp(str_frame_position, "top-center") == 0) {
        frame_position = ui_manip_action_import_frame_top_center;
    }
    else if (strcmp(str_frame_position, "top-left") == 0) {
        frame_position = ui_manip_action_import_frame_top_left;
    }
    else if (strcmp(str_frame_position, "top-right") == 0) {
        frame_position = ui_manip_action_import_frame_top_right;
    }
    else {
        CPE_ERROR(em, "unknown frame-position %s!", str_frame_position);
        goto IMPORT_COMPLETE;
    }

    if (strcmp(str_frame_order, "native") == 0) {
        frame_order = ui_manip_action_import_frame_order_native;
    }
    else if (strcmp(str_frame_order, "postfix") == 0) {
        frame_order = ui_manip_action_import_frame_order_postfix;
    }
    else {
        CPE_ERROR(em, "unknown frame-order %s!", str_frame_order);
        goto IMPORT_COMPLETE;
    }

    ed_mgr = ui_ed_mgr_create(NULL, data_mgr, em);
    if (ed_mgr == NULL) {
        CPE_ERROR(em, "create ed fail!");
        goto IMPORT_COMPLETE;
    }

    rv =  ui_model_import_cocos_action(
        ed_mgr,
        to_effect,
        to_module,
        plist,
        pic,
        frame_duration,
        frame_position,
        frame_order,
        em);
    if (rv != 0) {
        CPE_ERROR(em, "import cocos action fail!");
        goto IMPORT_COMPLETE;
    }

    if (ui_ed_mgr_save(ed_mgr, NULL, em) != 0) {
        CPE_ERROR(em, "save changed data fail!");
        rv = -1;
    }

    rv = 0;

IMPORT_COMPLETE:
    if (ed_mgr) ui_ed_mgr_free(ed_mgr);

    return rv;
}
