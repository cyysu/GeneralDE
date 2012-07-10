#include "argtable2.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "tool_env.h"

struct arg_file * i_om_file;
struct arg_file * i_meta_file;
struct arg_file * i_meta_group_root;
struct arg_file * i_meta_group;
struct arg_lit * help;
struct arg_lit * op_init;
struct arg_int * arg_shmkey;
struct arg_end *end;

dir_visit_next_op_t
accept_input_meta_file(const char * full, const char * base, void * ctx) {
    if (strcmp(file_name_suffix(base), "xml") == 0) {
        dr_metalib_builder_add_file((dr_metalib_builder_t)ctx, NULL, full);
    }
    return dir_visit_next_go;
}

struct dir_visitor g_input_meta_search_visitor = {
    NULL, NULL, accept_input_meta_file
};

void prepare_input_meta_group(dr_metalib_builder_t builder, error_monitor_t em) {
    char path_buf[256];
    size_t path_len = 0;
    FILE * group_file;

    if (i_meta_group->count <= 0) return;

    path_len = strlen(i_meta_group_root->filename[0]);
    if (path_len + 5 > sizeof(path_buf)) {
        CPE_ERROR(em, "group input %s is too long!", i_meta_group->filename[0]);
    }

    snprintf(path_buf, sizeof(path_buf), "%s", i_meta_group_root->filename[0]);
    if (path_buf[path_len - 1] != '/') {
        ++path_len;
        path_buf[path_len - 1] = '/';
        path_buf[path_len] = 0;
    }

    group_file = file_stream_open(i_meta_group->filename[0], "r", em);
    if (group_file == NULL) {
        CPE_ERROR(em, "group input %s not exist!", i_meta_group->filename[0]);
    }

    while(fgets(path_buf + path_len, sizeof(path_buf) - path_len, group_file)) {
        size_t total_len;
        for(total_len = strlen(path_buf);
            total_len > 0
                && (path_buf[total_len - 1] == '\n'
                    || path_buf[total_len - 1] == '\r');
            --total_len)
        {
            path_buf[total_len - 1] = 0;
        }

        if (file_exist(path_buf, em)) {
            dr_metalib_builder_add_file(builder, NULL, path_buf);
        }
        else {
            CPE_ERROR(em, "input %s not exist!", path_buf);
        }
    }
}

void prepare_input_meta_file(dr_metalib_builder_t builder, error_monitor_t em) {
    int i;
    for(i = 0; i < i_meta_file->count; ++i) {
        const char * filename;
        size_t filename_len;

        filename = i_meta_file->filename[i];
        filename_len = strlen(filename);
        if (filename[filename_len - 1] == '\\' || filename[filename_len - 1] == '/') {
            ((char *)filename)[filename_len - 1] = 0;
        }

        if (dir_exist(filename, em)) {
            dir_search(&g_input_meta_search_visitor, builder, filename, 5, em, NULL);
        }
        else if (file_exist(i_meta_file->filename[i], em)) {
            dr_metalib_builder_add_file(builder, NULL, filename);
        }
        else {
            CPE_ERROR(em, "input meta file %s not exist!", filename);
        }
    }
}

int tools_main(error_monitor_t em) {
    struct gd_om_tool_env env;
    struct mem_buffer input_meta_buffer;
    int rv;

    mem_buffer_init(&input_meta_buffer, 0);

    env.m_em = em;
    env.m_om_mgr = NULL;
    env.m_om_grp_meta = NULL;
    env.m_input_metalib = NULL;
    env.m_shm_id = 0;

    if (arg_shmkey->count) {
        env.m_shm_id = arg_shmkey->ival[0];
    }

    if (i_meta_file->count > 0 || i_meta_group->count > 0) {
        dr_metalib_builder_t builder;

        builder = dr_metalib_builder_create(NULL, em);
        if (builder == NULL) {
            CPE_ERROR(em, "create metalib builder fail!\n");
            goto ERROR;
        }

        prepare_input_meta_file(builder, em);
        prepare_input_meta_group(builder, em);

        dr_metalib_builder_free(builder);

        if (dr_inbuild_build_lib(
                &input_meta_buffer,
                dr_metalib_bilder_lib(builder),
                em) == 0)
        {
            env.m_input_metalib = (LPDRMETALIB)mem_buffer_size(&input_meta_buffer);
        }
        else {
            printf("build meta lib fail!\n");
            goto ERROR;
        }
    }

    rv = 0;

    if (op_init->count) {
        rv =  gd_om_tool_shm_init(&env);
    }
    else {
        printf("no operation specify!\n");
        rv = -1;
    }

    mem_buffer_clear(&input_meta_buffer);

    return rv;

ERROR:
    mem_buffer_clear(&input_meta_buffer);
    return -1;
}

int main(int argc, char * argv[]) {
    void* argtable[] = {
        i_om_file              = arg_file0(   NULL,   "om-def",              "<string>",    "input om def file")
        , i_meta_file          = arg_filen(   NULL,   "meta-file",              "<string>", 0, 100,    "input meta file")
        , i_meta_group_root    = arg_file0(   NULL,  "meta-group-root",     "<string>",            "root of input meta files listed in group file")
        , i_meta_group         = arg_file0(   NULL,  "meta-group",     "<string>",            "a file defined a list of input fild")
        , help                 = arg_lit0(   NULL,  "help",                                   "print this help and exit")
        , op_init              = arg_lit0(   NULL,  "init",                                "init shm")
        , arg_shmkey           = arg_int0(   NULL,  "shm-key",    "<integer>",                 "shm-key")
        , end                  = arg_end(20)
    };

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv;
    int nerrors;

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = -1;

    if (arg_nullcheck(argtable) != 0) {
        CPE_ERROR(em, "init arg table fail!");
        goto exit;
    }

    nerrors = arg_parse(argc,argv,argtable);

    if (help->count > 0) {
        printf("Usage: %s", argv[0]);
        arg_print_syntax(stdout,argtable,"\n");
        rv = 0;
        goto exit;
    }

    if (nerrors > 0) {
        arg_print_errors(stdout, end, argv[0]);
        printf("Try '%s --help' for more information.\n", argv[0]);
        goto exit;
    }

    rv = tools_main(em);

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    return rv;
}
