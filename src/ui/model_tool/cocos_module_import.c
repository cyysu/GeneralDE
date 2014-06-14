#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "ui/model_ed/ui_ed_mgr.h"
#include "ui/model_manip/model_manip_cocos.h"
#include "ops.h"

int do_cocos_module_import(
    ui_data_mgr_t data_mgr,
    const char * to_module, const char * plist, const char * pic, error_monitor_t em)
{
    ui_ed_mgr_t ed_mgr = NULL;
    int rv = -1;

    ed_mgr = ui_ed_mgr_create(NULL, data_mgr, em);
    if (ed_mgr == NULL) {
        CPE_ERROR(em, "create ed fail!");
        goto IMPORT_COMPLETE;
    }

    rv =  ui_model_import_cocos_module(ed_mgr, to_module, plist, pic, em);
    if (rv != 0) {
        CPE_ERROR(em, "import cocos module fail!");
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
