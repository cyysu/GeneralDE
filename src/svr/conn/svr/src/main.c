#include <pwd.h>
#include <signal.h>
#include "argtable2.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/service.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"

extern char g_metalib_svr_conn_pro[];

static int svr_main(int argc, char * argv[]) {
    struct mem_buffer buffer;
    gd_app_context_t ctx;
    int rv;

    ctx = gd_app_context_create_main(NULL, 0, argc, argv);
    if (ctx == NULL) return -1;

    mem_buffer_init(&buffer, NULL);
    gd_app_set_root(ctx, dir_name_ex(argv[0], 2, &buffer));
    mem_buffer_clear(&buffer);

    gd_app_set_debug(ctx, 1);

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

int main(int argc, char * argv[]) {
    int rv;

    /*run*/
    struct arg_rex  * run = arg_rex1(NULL, NULL, "run", NULL, 0, NULL);
    struct arg_str *  run_pidfile = arg_str1(NULL, "pidfile", NULL,    "pidfile");
    struct arg_str *  run_root = arg_str1(NULL, "root", NULL,    "root dir");
    struct arg_str *  run_appid = arg_str1(NULL, "app-id", NULL,    "app-id");
    struct arg_end  * run_end = arg_end(20);
    void* run_argtable[] = { run, run_pidfile, run_root, run_appid, run_end };
    int run_nerrors;

    /*stop service*/
    struct arg_rex  * stop = arg_rex1(NULL, NULL, "stop", NULL, 0, NULL);
    struct arg_str *  stop_pidfile = arg_str1(NULL, "pidfile", NULL, "pidfile");
    struct arg_end  * stop_end = arg_end(20);
    void* stop_argtable[] = { stop, stop_pidfile, stop_end };
    int stop_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;

    run_nerrors = arg_parse(argc, argv, run_argtable);
    stop_nerrors = arg_parse(argc, argv, stop_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = 0;
    if (run->count) {
        rv = svr_main(argc, argv);
    }
    else if (stop->count) {
        rv = cpe_kill_by_pidfile(stop_pidfile->sval[0], SIGUSR1, em);
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        rv = -1;
        if (run->count) {
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
    printf("%s: missing <run|stop|help> command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, stop_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(stop_argtable, sizeof(stop_argtable) / sizeof(stop_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}