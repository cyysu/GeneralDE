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
#include "cpe/aom/aom_shm.h"
#include "gd/app/app_context.h"

extern char g_metalib_svr_room_pro[];

static int svr_main(int argc, char * argv[], int shmkey) {
    struct mem_buffer buffer;
    gd_app_context_t ctx;
    int rv;

    ctx = gd_app_context_create_main(NULL, 0, argc, argv);
    if (ctx == NULL) return -1;

    mem_buffer_init(&buffer, NULL);
    gd_app_set_root(ctx, dir_name_ex(argv[0], 2, &buffer));
    mem_buffer_clear(&buffer);

    gd_app_set_debug(ctx, 1);

    cfg_struct_add_int32(gd_app_cfg(ctx), "shmkey", shmkey, cfg_replace);

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

int tool_shm_init(int shm_key, int shm_size, int force, error_monitor_t em) {
    LPDRMETA meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_room_pro, "svr_room_cli_record");
    if (meta == NULL) {
        CPE_ERROR(em, "shm init: can`t find meta svr_room_cli_record!");
        return -1;
    }

    return aom_shm_init(meta, shm_key, shm_size, force, em);
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

int generate_shm_key(const char * progname, error_monitor_t em) {
    struct mem_buffer buffer;
    char buf[256];
    int key;
    struct passwd * pwd;

    pwd = getpwuid(getuid());
    mem_buffer_init(&buffer, NULL);
    snprintf(buf, sizeof(buf), "/tmp/%s.%s.shm.key", pwd->pw_name, file_name_base(progname, &buffer));
    mem_buffer_clear(&buffer);

    key = cpe_shm_key_gen(buf, 'a');
    if (key == -1) {
        CPE_ERROR(em, "generate shm key at %s fail, errno=%d(%s)", buf, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
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

    /*dump shm*/
    struct arg_rex  * shm_dump =     arg_rex1(NULL, NULL, "shm-dump", NULL, 0, NULL);
    struct arg_file *  shm_dump_output = arg_file0(NULL, "output", NULL,    "dump to file");
    struct arg_end  * shm_dump_end = arg_end(20);
    void* shm_dump_argtable[] = { shm_dump, shm_dump_output, shm_dump_end };
    int shm_dump_nerrors;

    /*run*/
    struct arg_rex  * run = arg_rex1(NULL, NULL, "run", NULL, 0, NULL);
    struct arg_file * run_pidfile = arg_file1(NULL, "pidfile", NULL, "pid file path");
    struct arg_file * run_root = arg_file1(NULL, "root", NULL, "root dir");
    struct arg_int *  run_app_id = arg_int0(NULL, "app-id", NULL,    "app id");
    struct arg_end  * run_end = arg_end(20);
    void* run_argtable[] = { run, run_pidfile, run_root, run_app_id, run_end };
    int run_nerrors;

    /*start service*/
    struct arg_rex  * start = arg_rex1(NULL, NULL, "start", NULL, 0, NULL);
    struct arg_str *  start_shm_key = arg_str0(NULL, "shm-key", NULL,    "shm key");
    struct arg_end  * start_end = arg_end(20);
    void* start_argtable[] = { start, start_shm_key, start_end };
    int start_nerrors;

    /*stop service*/
    struct arg_rex  * stop = arg_rex1(NULL, NULL, "stop", NULL, 0, NULL);
    struct arg_end  * stop_end = arg_end(20);
    void* stop_argtable[] = { stop, stop_end };
    int stop_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;

    shm_init_nerrors = arg_parse(argc, argv, shm_init_argtable);
    shm_rm_nerrors = arg_parse(argc, argv, shm_rm_argtable);
    shm_dump_nerrors = arg_parse(argc, argv, shm_dump_argtable);
    shm_info_nerrors = arg_parse(argc, argv, shm_info_argtable);
    run_nerrors = arg_parse(argc, argv, run_argtable);
    start_nerrors = arg_parse(argc, argv, start_argtable);
    stop_nerrors = arg_parse(argc, argv, stop_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = 0;
    if (shm_init_nerrors == 0) {
        int shmkey = generate_shm_key(argv[0], em);
        if (shmkey == -1) return -1;

        rv =  tool_shm_init(
            shmkey,
            shm_init_size->count ? shm_init_size->ival[0] : 512 * 1024 * 1024,
            shm_init_force->count,
            em);
    }
    else if (shm_rm_nerrors == 0) {
        int shmkey = generate_shm_key(argv[0], em);
        if (shmkey == -1) return -1;
        rv =  aom_shm_rm(shmkey, em);
    }
    else if (shm_info_nerrors == 0) {
        struct write_stream_file stream = CPE_WRITE_STREAM_FILE_INITIALIZER(stdout, em);
        int shmkey = generate_shm_key(argv[0], em);
        if (shmkey == -1) return -1;
        rv =  aom_shm_info(shmkey, (write_stream_t)&stream, 0, em);
    }
    else if (shm_dump_nerrors == 0) {
        FILE * file = stdout;
        if (shm_dump_output->count) {
            file = fopen(shm_dump_output->filename[0], "w");
        }

        if (file == NULL) {
            printf("open %s fail!", shm_dump_output->filename[0]);
            rv = -1;
        }
        else {
            struct write_stream_file stream = CPE_WRITE_STREAM_FILE_INITIALIZER(file, em);
            int shmkey = generate_shm_key(argv[0], em);
            if (shmkey == -1) {
                if (shm_dump_output->count) fclose(file);
                return -1;
            }
            rv =  aom_shm_dump(shmkey, (write_stream_t)&stream, 0, em);
            if (shm_dump_output->count) fclose(file);
        }
    }
    else if (run_nerrors == 0) {
        if (cpe_check_and_write_pid(generate_pkd_file(argv[0]), em) != 0) {
            printf("%s is already runing!\n", file_name_no_dir(argv[0]));
            rv = -1;
        }
        else {
            int shmkey = generate_shm_key(argv[0], em);
            if (shmkey == -1) return -1;

            gd_stop_on_signal(SIGHUP);

            rv = svr_main(argc, argv, shmkey);
        }
    }
    else if (start_nerrors == 0) {
        cpe_daemonize(em);

        if (cpe_check_and_write_pid(generate_pkd_file(argv[0]), em) != 0) {
            printf("%s is already runing!\n", file_name_no_dir(argv[0]));
            rv = -1;
        }
        else {
            int shmkey = generate_shm_key(argv[0], em);
            if (shmkey == -1) return -1;

            gd_stop_on_signal(SIGHUP);

            rv = svr_main(argc, argv, shmkey);
        }
    }
    else if (stop_nerrors == 0) {
        rv = cpe_kill_by_pidfile(generate_pkd_file(argv[0]), SIGHUP, em);
    }
    else if (common_nerrors == 0) {
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
        else if (shm_dump->count) {
            arg_print_errors(stdout, shm_dump_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, shm_dump_argtable, "\n");
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
    printf("%s: missing <run|init-shm|rm-shm|shm-info|shm-dump> command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, shm_init_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, shm_rm_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, shm_info_argtable, "\n");
    printf("usage 4: %s ", argv[0]); arg_print_syntax(stdout, shm_dump_argtable, "\n");
    printf("usage 5: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 6: %s ", argv[0]); arg_print_syntax(stdout, run_argtable, "\n");
    printf("usage 7: %s ", argv[0]); arg_print_syntax(stdout, start_argtable, "\n");
    printf("usage 8: %s ", argv[0]); arg_print_syntax(stdout, stop_argtable, "\n");

EXIT:
    arg_freetable(shm_init_argtable, sizeof(shm_init_argtable) / sizeof(shm_init_argtable[0]));
    arg_freetable(shm_rm_argtable, sizeof(shm_rm_argtable) / sizeof(shm_rm_argtable[0]));
    arg_freetable(shm_info_argtable, sizeof(shm_info_argtable) / sizeof(shm_info_argtable[0]));
    arg_freetable(shm_dump_argtable, sizeof(shm_dump_argtable) / sizeof(shm_dump_argtable[0]));
    arg_freetable(run_argtable, sizeof(run_argtable) / sizeof(run_argtable[0]));
    arg_freetable(start_argtable, sizeof(start_argtable) / sizeof(start_argtable[0]));
    arg_freetable(stop_argtable, sizeof(stop_argtable) / sizeof(stop_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));
    return rv;
}
