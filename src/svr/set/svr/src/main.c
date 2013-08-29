#include <pwd.h>
#include <signal.h>
#include "argtable2.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/service.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"

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

const char * generate_pkd_file(const char * progname) {
    struct mem_buffer buffer;
    static char buf[128];
    struct passwd * pwd;
    pwd = getpwuid(getuid());

    mem_buffer_init(&buffer, NULL);
    snprintf(buf, sizeof(buf), "/tmp/%s.%s.pid", pwd->pw_name, file_name_base(progname, &buffer));
    mem_buffer_clear(&buffer);

    return buf;
}

int main(int argc, char * argv[]) {
    int rv;

    /*run*/
    struct arg_rex  * run = arg_rex1(NULL, NULL, "run", NULL, 0, NULL);
    struct arg_str  * run_set_type = arg_str1(NULL, "set-type", NULL, "set type name");
    struct arg_int  * run_set_id = arg_int1(NULL, "set-id", NULL, "set id");
    struct arg_end  * run_end = arg_end(20);
    void* run_argtable[] = { run, run_set_type, run_set_id, run_end };
    int run_nerrors;

    /*start service*/
    struct arg_rex  * start = arg_rex1(NULL, NULL, "start", NULL, 0, NULL);
    struct arg_str  * start_set_type = arg_str1(NULL, "set-type", NULL, "set type name");
    struct arg_int  * start_set_id = arg_int1(NULL, "set-id", NULL, "set id");
    struct arg_end  * start_end = arg_end(20);
    void* start_argtable[] = { start, start_set_type, start_set_id, start_end };
    int start_nerrors;

    /*stop service*/
    struct arg_rex  * stop = arg_rex1(NULL, NULL, "stop", NULL, 0, NULL);
    struct arg_str  * stop_set_type = arg_str1(NULL, "set-type", NULL, "set type name");
    struct arg_int  * stop_set_id = arg_int1(NULL, "set-id", NULL, "set id");
    struct arg_end  * stop_end = arg_end(20);
    void* stop_argtable[] = { stop, stop_set_type, stop_set_id, stop_end };
    int stop_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;

    run_nerrors = arg_parse(argc, argv, run_argtable);
    start_nerrors = arg_parse(argc, argv, start_argtable);
    stop_nerrors = arg_parse(argc, argv, stop_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = 0;
    if (run->count) {
        if (cpe_check_and_write_pid(generate_pkd_file(argv[0]), em) != 0) {
            printf("%s is already runing!\n", file_name_no_dir(argv[0]));
            rv = -1;
        }
        else {
            gd_stop_on_signal(SIGHUP);

            rv = svr_main(argc, argv);
        }
    }
    else if (start->count) {
        cpe_daemonize(em);

        if (cpe_check_and_write_pid(generate_pkd_file(argv[0]), em) != 0) {
            printf("%s is already runing!\n", file_name_no_dir(argv[0]));
            rv = -1;
        }
        else {
            gd_stop_on_signal(SIGHUP);

            rv = svr_main(argc, argv);
        }
    }
    else if (stop->count) {
        rv = cpe_kill_by_pidfile(generate_pkd_file(argv[0]), SIGHUP, em);
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
    printf("%s: missing <run|init-shm|rm-shm|shm-info|shm-dump> command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, start_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, stop_argtable, "\n");
    printf("usage 4: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(start_argtable, sizeof(start_argtable) / sizeof(start_argtable[0]));
    arg_freetable(stop_argtable, sizeof(stop_argtable) / sizeof(stop_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}
