#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include "argtable2.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/service.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"

static int svr_main(char * progname, int argc, char * argv[]) {
    struct mem_buffer buffer;
    gd_app_context_t ctx;
    int rv;
    
    ctx = gd_app_context_create_main(NULL, 0, argc, argv);

    if (ctx == NULL) return -1;

    mem_buffer_init(&buffer, NULL);
    gd_app_set_root(ctx, dir_name_ex(progname, 2, &buffer));
    mem_buffer_clear(&buffer);

    gd_app_set_debug(ctx, 1);

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

const char * generate_pid_file(const char * progname, const char * set_type, int set_id, error_monitor_t em) {
    static char buf[256];
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, sizeof(buf));
    const char * dir_end_1 = NULL;
    const char * dir_end_2 = NULL;
    const char * dir_end_3 = NULL;
    const char * dir_end_4 = NULL;
    const char * p;
    const char * home;
    int home_len;

    for(p = strchr(progname, '/'); p; p = strchr(p + 1, '/')) {
        dir_end_1 = dir_end_2;
        dir_end_2 = dir_end_3;
        dir_end_3 = dir_end_4;
        dir_end_4 = p;
    }

    home = getenv("HOME");
    home_len = strlen(home);

    cpe_str_buf_append(&str_buf, home, home_len);
    if (home[home_len - 1] != '/') {
        cpe_str_buf_cat(&str_buf, "/");
    }
    cpe_str_buf_cat(&str_buf, ".");
    cpe_str_buf_append(&str_buf, dir_end_1 + 1, dir_end_2 - dir_end_1 - 1);
    cpe_str_buf_cat_printf(&str_buf, "/%s-%d", set_type, set_id);

    if (dir_mk_recursion(buf, DIR_DEFAULT_MODE, em, NULL) != 0) {
        printf("create pid dir %s fail, error=%d (%s)\n", buf, errno, strerror(errno));
        return NULL;
    }

    
    cpe_str_buf_cat(&str_buf, "/");
    cpe_str_buf_cat(&str_buf, dir_end_4 + 1);
    cpe_str_buf_cat(&str_buf, ".pid");

    return cpe_str_buf_is_overflow(&str_buf) ? NULL : buf;
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
    char progname[256];

    if (argv[0][0] == '/') {
        strncpy(progname, argv[0], sizeof(progname));
    }
    else {
        snprintf(progname, sizeof(progname), "%s/%s", getcwd(NULL, 0), argv[0]);
    }
    file_name_normalize(progname);
    argv[0] = progname;

    run_nerrors = arg_parse(argc, argv, run_argtable);
    start_nerrors = arg_parse(argc, argv, start_argtable);
    stop_nerrors = arg_parse(argc, argv, stop_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = 0;
    if (run->count) {
        const char * pid_file = generate_pid_file(progname, run_set_type->sval[0], run_set_id->ival[0], em);
        if (pid_file == NULL) {
            rv = -1;
        }
        else {
            if (cpe_check_and_write_pid(pid_file, em) != 0) {
                printf("%s is already runing!\n", file_name_no_dir(progname));
                rv = -1;
            }
            else {
                gd_stop_on_signal(SIGHUP);

                rv = svr_main(progname, argc, argv);
            }
        }
    }
    else if (start->count) {
        const char * pid_file;

        cpe_daemonize(em);

        pid_file = generate_pid_file(progname, stop_set_type->sval[0], stop_set_id->ival[0], em);
        if (pid_file == NULL) {
            rv = -1;
        }
        else {
            if (cpe_check_and_write_pid(pid_file, em) != 0) {
                printf("%s is already runing!\n", file_name_no_dir(progname));
                rv = -1;
            }
            else {
                gd_stop_on_signal(SIGHUP);

                rv = svr_main(progname, argc, argv);
            }
        }
    }
    else if (stop->count) {
        const char * pid_file = generate_pid_file(progname, stop_set_type->sval[0], stop_set_id->ival[0], em);
        if (pid_file == NULL) {
            rv = -1;
        }
        else {
            rv = cpe_kill_by_pidfile(pid_file, SIGHUP, em);
        }
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        rv = -1;
        if (run->count) {
            arg_print_errors(stdout, run_end, progname);
            printf("usage: %s ", progname);
            arg_print_syntax(stdout, run_argtable, "\n");
        }
        else {
            goto PRINT_HELP;
        }
    }

    goto EXIT;

PRINT_HELP:
    printf("%s: missing <run|init-shm|rm-shm|shm-info|shm-dump> command.\n", progname);
    printf("usage 1: %s ", progname); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 2: %s ", progname); arg_print_syntax(stdout, start_argtable, "\n");
    printf("usage 3: %s ", progname); arg_print_syntax(stdout, stop_argtable, "\n");
    printf("usage 4: %s ", progname); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(start_argtable, sizeof(start_argtable) / sizeof(start_argtable[0]));
    arg_freetable(stop_argtable, sizeof(stop_argtable) / sizeof(stop_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}
