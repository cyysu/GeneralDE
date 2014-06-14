#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_strings.h" 
#include "ops.h"

static ui_data_mgr_t load_model(const char * model, const char * model_format, int load_product, error_monitor_t em);

int main(int argc, char * argv[]) {
    /*convert*/
    struct arg_rex  * convert           =     arg_rex1(NULL, NULL, "convert", NULL, 0, NULL);
    struct arg_str  * convert_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * convert_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * convert_to        =     arg_str0(NULL, "to", NULL, "output file/dir");
    struct arg_str  * convert_to_format =     arg_str1(NULL, "to-format", "(yml|bin)", "output format");
    struct arg_end  * convert_end       =     arg_end(20);
    void* convert_argtable[] = { convert, convert_model, convert_model_format, convert_to, convert_to_format, convert_end };
    int convert_nerrors;

    /*manip*/
    struct arg_rex  * manip           =     arg_rex1(NULL, NULL, "manip", NULL, 0, NULL);
    struct arg_str  * manip_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * manip_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * manip_op      =     arg_str1(NULL, "op", NULL, "operation script");
    struct arg_end  * manip_end       =     arg_end(20);
    void* manip_argtable[] = { manip, manip_model, manip_model_format, manip_op, manip_end };
    int manip_nerrors;

    /*cocos_module_import*/
    struct arg_rex  * cocos_module           =     arg_rex1(NULL, NULL, "cocos-module-import", NULL, 0, NULL);
    struct arg_str  * cocos_module_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * cocos_module_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * cocos_module_to_module      =     arg_str1(NULL, "to-module", NULL, "import to module path");
    struct arg_str  * cocos_module_import_plist      =     arg_str1(NULL, "plist", NULL, "plist path");
    struct arg_str  * cocos_module_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_end  * cocos_module_end       =     arg_end(20);
    void* cocos_module_argtable[] = { cocos_module, cocos_module_model, cocos_module_model_format, cocos_module_to_module, cocos_module_import_plist, cocos_module_import_pic, cocos_module_end };
    int cocos_module_nerrors;

    /*cocos_effect_import*/
    struct arg_rex  * cocos_effect_import           =     arg_rex1(NULL, NULL, "cocos-effect-import", NULL, 0, NULL);
    struct arg_str  * cocos_effect_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * cocos_effect_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * cocos_effect_import_to_effect      =     arg_str1(NULL, "to-effect", NULL, "import to effect path");
    struct arg_str  * cocos_effect_import_to_module      =     arg_str1(NULL, "to-module", NULL, "import to module path");
    struct arg_str  * cocos_effect_import_plist      =     arg_str1(NULL, "plist", NULL, "plist path");
    struct arg_str  * cocos_effect_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_int  * cocos_effect_import_frame_duration      =     arg_int1(NULL, "frame-duration", NULL, "frame duration");
    struct arg_str  * cocos_effect_import_frame_position      =     arg_str1(NULL, "frame-position", "[center|center-left|center-right|bottom-center|bottom-left|bottom-right|top-center|top-left|top-right]", "frame position");
    struct arg_str  * cocos_effect_import_frame_order      =     arg_str1(NULL, "frame-order", "[native|postfix]", "frame order");
    struct arg_end  * cocos_effect_import_end       =     arg_end(20);
    void* cocos_effect_import_argtable[] = { 
        cocos_effect_import, cocos_effect_import_model, cocos_effect_import_model_format,
        cocos_effect_import_to_effect, cocos_effect_import_to_module, 
        cocos_effect_import_plist, cocos_effect_import_pic,
        cocos_effect_import_frame_duration, cocos_effect_import_frame_position, cocos_effect_import_frame_order,
        cocos_effect_import_end
    };
    int cocos_effect_import_nerrors;

    /*cocos_particle_import*/
    struct arg_rex  * cocos_particle_import           =     arg_rex1(NULL, NULL, "cocos-particle-import", NULL, 0, NULL);
    struct arg_str  * cocos_particle_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * cocos_particle_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * cocos_particle_import_to_particle      =     arg_str1(NULL, "to-particle", NULL, "import to particle path");
    struct arg_str  * cocos_particle_import_plist      =     arg_str1(NULL, "plist", NULL, "plist path");
    struct arg_str  * cocos_particle_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_end  * cocos_particle_import_end       =     arg_end(20);
    void* cocos_particle_import_argtable[] = { 
        cocos_particle_import, cocos_particle_import_model, cocos_particle_import_model_format,
        cocos_particle_import_to_particle,
        cocos_particle_import_plist, cocos_particle_import_pic, cocos_particle_import_end
    };
    int cocos_particle_import_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv = -1;

    ui_data_mgr_t data_mgr = NULL;

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    convert_nerrors = arg_parse(argc, argv, convert_argtable);
    manip_nerrors = arg_parse(argc, argv, manip_argtable);
    cocos_module_nerrors = arg_parse(argc, argv, cocos_module_argtable);
    cocos_effect_import_nerrors = arg_parse(argc, argv, cocos_effect_import_argtable);
    cocos_particle_import_nerrors = arg_parse(argc, argv, cocos_particle_import_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (convert_nerrors == 0) {
        data_mgr = load_model(convert_model->sval[0], convert_model_format->sval[0], 1, em);
        if (data_mgr == NULL) goto EXIT;

        rv = do_convert_model(
            data_mgr, 
            convert_to->count ? convert_to->sval[0] : NULL,
            convert_to_format->sval[0],
            em);
    }
    else if (manip_nerrors == 0) {
        data_mgr = load_model(manip_model->sval[0], manip_model_format->sval[0], 0, em);
        if (data_mgr == NULL) goto EXIT;

        rv = do_manip_model(data_mgr, manip_op->sval[0], em);
    }
    else if (cocos_module_nerrors == 0) {
        data_mgr = load_model(cocos_module_model->sval[0], cocos_module_model_format->sval[0], 0, em);
        if (data_mgr == NULL) goto EXIT;

        rv = do_cocos_module_import(
            data_mgr,
            cocos_module_to_module->sval[0],
            cocos_module_import_plist->sval[0],
            cocos_module_import_pic->sval[0],
            em);
    }
    else if (cocos_effect_import_nerrors == 0) {
        data_mgr = load_model(cocos_effect_import_model->sval[0], cocos_effect_import_model_format->sval[0], 0, em);
        if (data_mgr == NULL) goto EXIT;

        rv = do_cocos_effect_import(
            data_mgr,
            cocos_effect_import_to_effect->sval[0], 
            cocos_effect_import_to_module->sval[0], 
            cocos_effect_import_plist->sval[0],
            cocos_effect_import_pic->sval[0],
            cocos_effect_import_frame_duration->ival[0],
            cocos_effect_import_frame_position->sval[0],
            cocos_effect_import_frame_order->sval[0],
            em);
    }
    else if (cocos_particle_import_nerrors == 0) {
        data_mgr = load_model(cocos_particle_import_model->sval[0], cocos_particle_import_model_format->sval[0], 0, em);
        if (data_mgr == NULL) goto EXIT;

        rv = do_cocos_particle_import(
            data_mgr,
            cocos_particle_import_to_particle->sval[0], 
            cocos_particle_import_plist->sval[0],
            cocos_particle_import_pic->sval[0],
            em);
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        goto PRINT_HELP;
    }

    goto EXIT;

PRINT_HELP:
    printf("%s:\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, convert_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, cocos_module_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, cocos_effect_import_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, cocos_particle_import_argtable, "\n");
    printf("usage 5: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(convert_argtable, sizeof(convert_argtable) / sizeof(convert_argtable[0]));
    arg_freetable(cocos_module_argtable, sizeof(cocos_module_argtable) / sizeof(cocos_module_argtable[0]));
    arg_freetable(cocos_effect_import_argtable, sizeof(cocos_effect_import_argtable) / sizeof(cocos_effect_import_argtable[0]));
    arg_freetable(cocos_particle_import_argtable, sizeof(cocos_particle_import_argtable) / sizeof(cocos_particle_import_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    if (data_mgr) ui_data_mgr_free(data_mgr);

    return rv;
}

static ui_data_mgr_t load_model(const char * model, const char * model_format, int load_product, error_monitor_t em) {
    ui_data_mgr_t data_mgr;

    data_mgr = ui_data_mgr_create(NULL, model, em);
    if (data_mgr == NULL) {
        CPE_ERROR(em, "load_model: create data mgr fail!");
        return NULL;
    }

    if (strcmp(model_format, "np") == 0) {
        if (ui_data_np_load(data_mgr, model, load_product, em) != 0) {
            CPE_ERROR(em, "load_model: load %s format model model %s fail!", model_format, model);
            ui_data_mgr_free(data_mgr);
            return NULL;
        }

        ui_data_init_np_saver(data_mgr);
    }
    else {
        CPE_ERROR(em, "load_model: load %s format model model %s fail!", model_format, model);
        ui_data_mgr_free(data_mgr);
        return NULL;
    }

    return data_mgr;
}

int do_convert_model(ui_data_mgr_t data_mgr, const char * to, const char * format, error_monitor_t em) {
    if (strcmp(format, "np") == 0) {
        if (ui_data_np_save(data_mgr, to, em) != 0) {
            CPE_ERROR(em, "convert_model: np: format %s save error!", format);
            return -1;
        }
    }
    else {
        CPE_ERROR(em, "convert_model: format %s unknown!", format);
        return -1;
    }

    return 0;
}

