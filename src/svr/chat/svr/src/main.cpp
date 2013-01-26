#include "argtable2.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_dlfcn.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/stream_file.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_shm.h"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Application.hpp"

static int svr_main(int argc, char * argv[], int shmkey) {
    gd_app_context_t ctx;
    int rv;

    ctx = gd_app_context_create_main(NULL, 0, argc, argv);
    if (ctx == NULL) return -1;

    gd_app_set_debug(ctx, 1);

    Gd::App::Application::_cast(ctx).cfg()["shmkey"] = shmkey;

    gd_set_default_library(dlopen(NULL, RTLD_NOW));

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

#ifdef _MSC_VER

int main(int argc, char * argv[]) {
    return svr_main(argc, argv, 0);
}

#else

#include <pwd.h>

int tool_shm_init(int shm_key, int shm_size, int force, error_monitor_t em) {
    return 0;
}

int generate_shm_key(const char * name, error_monitor_t em) {
    char buf[256];
    if (name == NULL) {
        struct passwd * pwd;
        pwd = getpwuid(getuid());
        snprintf(buf, sizeof(buf), "/tmp/%s.chat_svr.shm.key", pwd->pw_name);
        name = buf;
    }

    int key = cpe_shm_key_gen(name, 'a');
    if (key == -1) {
        CPE_ERROR(em, "generate shm key at %s fail, errno=%d(%s)", name, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    return key;
}

int main(int argc, char * argv[]) {
    int rv;

    /*init shm*/
    struct arg_rex  * shm_init =     arg_rex1(NULL, NULL, "init-shm", NULL, 0, NULL);
    struct arg_str *  shm_init_key = arg_str0(NULL, "shm-key", NULL,    "init shm key");
    struct arg_int *  shm_init_size = arg_int0(NULL, "shm-size", NULL,    "init shm size");
    struct arg_lit *  shm_init_force = arg_lit0(NULL, "force", "force create new shm");
    struct arg_end  * shm_init_end = arg_end(20);
    void* shm_init_argtable[] = { shm_init, shm_init_key, shm_init_size, shm_init_force, shm_init_end };
    int shm_init_nerrors;

    /*rm shm*/
    struct arg_rex  * shm_rm =     arg_rex1(NULL, NULL, "rm-shm", NULL, 0, NULL);
    struct arg_str *  shm_rm_key = arg_str0(NULL, "shm-key", NULL,    "rm shm key");
    struct arg_int *  shm_rm_size = arg_int0(NULL, "shm-size", NULL,    "rm shm size");
    struct arg_end  * shm_rm_end = arg_end(20);
    void* shm_rm_argtable[] = { shm_rm, shm_rm_key, shm_rm_size, shm_rm_end };
    int shm_rm_nerrors;

    /*info shm*/
    struct arg_rex  * shm_info =     arg_rex1(NULL, NULL, "shm-info", NULL, 0, NULL);
    struct arg_str *  shm_info_key = arg_str0(NULL, "shm-key", NULL,    "info shm key");
    struct arg_end  * shm_info_end = arg_end(20);
    void* shm_info_argtable[] = { shm_info, shm_info_key, shm_info_end };
    int shm_info_nerrors;

    /*run as service*/
    struct arg_rex  * run = arg_rex1(NULL, NULL, "run", NULL, 0, NULL);
    struct arg_str *  run_shm_key = arg_str0(NULL, "shm-key", NULL,    "shm key");
    struct arg_end  * run_end = arg_end(20);
    void* run_argtable[] = { run, run_shm_key, run_end };
    int run_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;

    shm_init_nerrors = arg_parse(argc, argv, shm_init_argtable);
    shm_rm_nerrors = arg_parse(argc, argv, shm_rm_argtable);
    shm_info_nerrors = arg_parse(argc, argv, shm_info_argtable);
    run_nerrors = arg_parse(argc, argv, run_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);
    (void)common_nerrors;

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = 0;
    if (shm_init_nerrors == 0) {
        int shmkey = generate_shm_key(shm_init_key->count ? shm_init_key->sval[0] : NULL, em);
        if (shmkey == -1) return -1;

        rv =  tool_shm_init(
            shmkey,
            shm_init_size->count ? shm_init_size->ival[0] : 512 * 1024 * 1024,
            shm_init_force->count,
            em);
    }
    else if (shm_rm_nerrors == 0) {
        int shmkey = generate_shm_key(shm_rm_key->count ? shm_rm_key->sval[0] : NULL, em);
        if (shmkey == -1) return -1;
        rv =  pom_grp_shm_rm(shmkey, em);
    }
    else if (shm_info_nerrors == 0) {
        struct write_stream_file stream = CPE_WRITE_STREAM_FILE_INITIALIZER(stdout, em);
        int shmkey = generate_shm_key(shm_info_key->count ? shm_info_key->sval[0] : NULL, em);
        if (shmkey == -1) return -1;
        rv =  pom_grp_shm_info(shmkey, (write_stream_t)&stream, 0, em);
    }
    else if (run_nerrors == 0) {
        int shmkey = generate_shm_key(shm_info_key->count ? shm_info_key->sval[0] : NULL, em);
        if (shmkey == -1) return -1;
        rv = svr_main(argc, argv, shmkey);
    }
    else if (common_argtable == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        rv = -1;
        if (shm_init->count) {
            arg_print_errors(stdout, shm_init_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, shm_init_argtable, "\n");
        }
        else if (shm_rm->count) {
            arg_print_errors(stdout, shm_rm_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, shm_rm_argtable, "\n");
        }
        else if (shm_info->count) {
            arg_print_errors(stdout, shm_info_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, shm_info_argtable, "\n");
        }
        else if (run->count) {
            arg_print_errors(stdout, run_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, run_argtable, "\n");
        }
        else {
            goto PRINT_HELP;
        }
    }

    goto EXIT;

PRINT_HELP:
    printf("%s: missing <run|init-shm> command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, shm_init_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, shm_rm_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, shm_info_argtable, "\n");
    printf("usage 4: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 5: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(shm_init_argtable, sizeof(shm_init_argtable) / sizeof(shm_init_argtable[0]));
    arg_freetable(shm_rm_argtable, sizeof(shm_rm_argtable) / sizeof(shm_rm_argtable[0]));
    arg_freetable(shm_info_argtable, sizeof(shm_info_argtable) / sizeof(shm_info_argtable[0]));
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}

#endif
