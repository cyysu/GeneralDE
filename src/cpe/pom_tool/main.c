#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_builder.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_meta_build.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "tool_env.h"

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

void prepare_input_meta_group(
    dr_metalib_builder_t builder,
    struct arg_file * meta_group_root,
    struct arg_file * meta_group,
    error_monitor_t em)
{
    char path_buf[256];
    size_t path_len = 0;
    FILE * group_file;

    if (meta_group->count <= 0) return;

    path_len = strlen(meta_group_root->filename[0]);
    if (path_len + 5 > sizeof(path_buf)) {
        CPE_ERROR(em, "group input %s is too long!", meta_group->filename[0]);
    }

    snprintf(path_buf, sizeof(path_buf), "%s", meta_group_root->filename[0]);
    if (path_buf[path_len - 1] != '/') {
        ++path_len;
        path_buf[path_len - 1] = '/';
        path_buf[path_len] = 0;
    }

    group_file = file_stream_open(meta_group->filename[0], "r", em);
    if (group_file == NULL) {
        CPE_ERROR(em, "group input %s not exist!", meta_group->filename[0]);
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

void prepare_input_meta_file(dr_metalib_builder_t builder, struct arg_file * meta_file, error_monitor_t em) {
    int i;
    for(i = 0; i < meta_file->count; ++i) {
        const char * filename;
        size_t filename_len;

        filename = meta_file->filename[i];
        filename_len = strlen(filename);
        if (filename[filename_len - 1] == '\\' || filename[filename_len - 1] == '/') {
            ((char *)filename)[filename_len - 1] = 0;
        }

        if (dir_exist(filename, em)) {
            dir_search(&g_input_meta_search_visitor, builder, filename, 5, em, NULL);
        }
        else if (file_exist(filename, em)) {
            if (dr_metalib_builder_add_file(builder, NULL, filename) == NULL) {
                CPE_ERROR(em, "add input meta file %s fail!", filename);
            }
        }
        else {
            CPE_ERROR(em, "input meta file %s not exist!", filename);
        }
    }
}

static int env_init_meta(
    struct pom_tool_env * env, 
    struct arg_file * pom_meta_file,
    struct arg_int * page_size,
    struct arg_file * dr_meta_file,
    struct arg_file * dr_meta_group_root,
    struct arg_file * dr_meta_group)
{
    dr_metalib_builder_t builder;
    int build_rv;

    if (dr_meta_file->count == 0 && dr_meta_group->count == 0) {
        printf("no metalib file or group input!");
        return -1;
    }

    if (pom_meta_file->count == 0) {
        printf("no pom meta file input!");
        return -1;
    }


    builder = dr_metalib_builder_create(NULL, env->m_em);
    if (builder == NULL) {
        CPE_ERROR(env->m_em, "create metalib builder fail!\n");
        return -1;
    }

    prepare_input_meta_file(builder, dr_meta_file, env->m_em);
    prepare_input_meta_group(builder, dr_meta_group_root, dr_meta_group, env->m_em);

    dr_metalib_builder_analize(builder);
    build_rv = dr_inbuild_build_lib(
        &env->m_input_meta_buffer,
        dr_metalib_bilder_lib(builder),
        env->m_em);
    dr_metalib_builder_free(builder);

    if (build_rv == 0) {
        env->m_input_metalib = (LPDRMETALIB)mem_buffer_make_continuous(&env->m_input_meta_buffer, 0);
    }
    else {
        CPE_ERROR(env->m_em, "build meta lib fail!\n");
        return -1;
    }

    env->m_pom_cfg = cfg_create(NULL);
    if (env->m_pom_cfg == NULL) {
        CPE_ERROR(env->m_em, "create cfg fail!");
        return -1;
    }

    if (cfg_read_file(env->m_pom_cfg, pom_meta_file->filename[0], cfg_replace, env->m_em) != 0) {
        CPE_ERROR(env->m_em, "read pom meta from %s fail!", pom_meta_file->filename[0]);
        return -1;
    }

    env->m_pom_grp_meta = pom_grp_meta_build_from_cfg(NULL, (page_size && page_size->count) ? page_size->ival[0] : 1024, cfg_child_only(env->m_pom_cfg), env->m_input_metalib, env->m_em);
    if (env->m_pom_grp_meta == NULL) {
        CPE_ERROR(env->m_em, "create pom meta from %s fail!", pom_meta_file->filename[0]);
        return -1;
    }

    return 0;
}

int main(int argc, char * argv[]) {
    /*mk meta bin*/
    struct arg_rex  * mk_clib =     arg_rex1(NULL, NULL, "mk-clib", NULL, 0, NULL);
    struct arg_file  * mk_clib_pom_meta =     arg_file1(NULL, "pom-meta", NULL, "input pom meta file");
    struct arg_file  * mk_clib_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * mk_clib_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * mk_clib_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * mk_clib_page_size =     arg_int1(NULL, "page-size", NULL, "object page size");
    struct arg_file  * mk_clib_o_file =     arg_file1(NULL, "output-lib-c", NULL, "output lib c file");
    struct arg_str  * mk_clib_o_argname =     arg_str1(NULL, "output-lib-c-arg", NULL, "output c value arg name");
    struct arg_end  * mk_clib_end = arg_end(20);
    void* mk_clib_argtable[] = { 
        mk_clib, mk_clib_pom_meta, mk_clib_page_size,
        mk_clib_dr_file, mk_clib_dr_group_root, mk_clib_dr_group,
        mk_clib_o_file, mk_clib_o_argname,
        mk_clib_end
    };
    int mk_clib_nerrors;

    /*mk metalib xml*/
    struct arg_rex  * metalib_xml =     arg_rex1(NULL, NULL, "metalib-xml", NULL, 0, NULL);
    struct arg_file  * metalib_xml_pom_meta =     arg_file1(NULL, "pom-meta", NULL, "input pom meta file");
    struct arg_file  * metalib_xml_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * metalib_xml_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * metalib_xml_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_file  * metalib_xml_o_file =     arg_file1(NULL, "output-metalib-xml", NULL, "output metalib xml file");
    struct arg_end  * metalib_xml_end = arg_end(20);
    void* metalib_xml_argtable[] = { 
        metalib_xml, metalib_xml_pom_meta,
        metalib_xml_dr_file, metalib_xml_dr_group_root, metalib_xml_dr_group,
        metalib_xml_o_file,
        metalib_xml_end
    };
    int metalib_xml_nerrors;

    /*mk hpp*/
    struct arg_rex  * mk_hpp =     arg_rex1(NULL, NULL, "mk-hpp", NULL, 0, NULL);
    struct arg_file  * mk_hpp_pom_meta =     arg_file1(NULL, "pom-meta", NULL, "input pom meta file");
    struct arg_file  * mk_hpp_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * mk_hpp_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * mk_hpp_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_file  * mk_hpp_o_file =     arg_file1(NULL, "output-hpp", NULL, "output hpp file");
    struct arg_str  * mk_hpp_o_classname =     arg_str1(NULL, "class-name", NULL, "output class name");
    struct arg_str  * mk_hpp_o_namespace =     arg_str0(NULL, "namespace", NULL, "output class namespace");
    struct arg_end  * mk_hpp_end = arg_end(20);
    void* mk_hpp_argtable[] = { 
        mk_hpp, mk_hpp_pom_meta, 
        mk_hpp_dr_file, mk_hpp_dr_group_root, mk_hpp_dr_group,
        mk_hpp_o_file, mk_hpp_o_classname, mk_hpp_o_namespace,
        mk_hpp_end
    };
    int mk_hpp_nerrors;

    /*init shm bin*/
    struct arg_rex  * shm_init =     arg_rex1(NULL, NULL, "shm-init", NULL, 0, NULL);
    struct arg_file  * shm_init_pom_meta =     arg_file1(NULL, "pom-meta", NULL, "input pom meta file");
    struct arg_file  * shm_init_dr_file =     arg_filen(NULL, "dr-meta", NULL, 0, 100, "input dr meta file(s)");
    struct arg_file  * shm_init_dr_group_root =     arg_file0(NULL, "dr-meta-group-root", NULL, "input dr meta group root");
    struct arg_file  * shm_init_dr_group =     arg_file0(NULL, "dr-meta-group", NULL, "input dr meta group file");
    struct arg_int  * shm_init_page_size =     arg_int1(NULL, "page-size", NULL, "object page size");
    struct arg_int  * shm_init_shm_key =     arg_int0(NULL, "shm-key", NULL, "shm key");
    struct arg_end  * shm_init_end = arg_end(20);
    void* shm_init_argtable[] = { 
        shm_init, shm_init_pom_meta, shm_init_page_size,
        shm_init_dr_file, shm_init_dr_group_root, shm_init_dr_group,
        shm_init_shm_key,
        shm_init_end
    };
    int shm_init_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv;
    struct pom_tool_env env;

    bzero(&env, sizeof(env));

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    env.m_em = em;
    mem_buffer_init(&env.m_input_meta_buffer, 0);

    rv = -1;

    assert(arg_nullcheck(mk_clib_argtable) == 0);
    assert(arg_nullcheck(shm_init_argtable) == 0);

    mk_clib_nerrors = arg_parse(argc, argv, mk_clib_argtable);
    metalib_xml_nerrors = arg_parse(argc, argv, metalib_xml_argtable);
    mk_hpp_nerrors = arg_parse(argc, argv, mk_hpp_argtable);
    shm_init_nerrors = arg_parse(argc, argv, shm_init_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (shm_init_nerrors == 0) {
        /* int shmkey = generate_shm_key(shm_init_key->count ? shm_init_key->sval[0] : NULL, em); */
        /* if (shmkey == -1) return -1; */

        rv = 0;
        /* tool_shm_init( */
            /* shmkey, */
            /* shm_init_size->count ? shm_init_size->ival[0] : 512 * 1024 * 1024, */
            /* shm_init_force->count, */
            /* em); */
    }
    else if (mk_clib_nerrors == 0) {
        if (env_init_meta(
                &env, 
                mk_clib_pom_meta,
                mk_clib_page_size,
                mk_clib_dr_file,
                mk_clib_dr_group_root,
                mk_clib_dr_group) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_lib_c(&env, mk_clib_o_file->filename[0], mk_clib_o_argname->sval[0]);
    }
    else if (metalib_xml_nerrors == 0) {
        if (env_init_meta(
                &env, 
                metalib_xml_pom_meta,
                NULL,
                metalib_xml_dr_file,
                metalib_xml_dr_group_root,
                metalib_xml_dr_group) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_metalib_xml(&env, metalib_xml_o_file->filename[0]);
    }
    else if (mk_hpp_nerrors == 0) {
        if (env_init_meta(
                &env, 
                mk_hpp_pom_meta,
                NULL,
                mk_hpp_dr_file,
                mk_hpp_dr_group_root,
                mk_hpp_dr_group) != 0)
        {
            rv = -1;
            goto EXIT;
        }

        rv = pom_tool_generate_hpp(
            &env,
            mk_hpp_o_file->filename[0],
            mk_hpp_o_classname->sval[0],
            mk_hpp_o_namespace->count ? mk_hpp_o_namespace->sval[0] : "");
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
        else if (mk_clib->count) {
            arg_print_errors(stdout, mk_clib_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, mk_clib_argtable, "\n");
        }
        else if (metalib_xml->count) {
            arg_print_errors(stdout, metalib_xml_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, metalib_xml_argtable, "\n");
        }
        else if (mk_hpp->count) {
            arg_print_errors(stdout, mk_hpp_end, argv[0]);
            printf("usage: %s ", argv[0]);
            arg_print_syntax(stdout, mk_hpp_argtable, "\n");
        }
        else {
            goto PRINT_HELP;
        }
    }

    goto EXIT;

PRINT_HELP:
    printf("%s: missing <mk-clib|mk-hpp|metalib-xml|shm-init> command.\n", argv[0]);
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, mk_clib_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, metalib_xml_argtable, "\n");
    printf("usage 3: %s ", argv[0]); arg_print_syntax(stdout, mk_hpp_argtable, "\n");
    printf("usage 4: %s ", argv[0]); arg_print_syntax(stdout, shm_init_argtable, "\n");
    printf("usage 5: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(mk_clib_argtable, sizeof(mk_clib_argtable) / sizeof(mk_clib_argtable[0]));
    arg_freetable(metalib_xml_argtable, sizeof(metalib_xml_argtable) / sizeof(metalib_xml_argtable[0]));
    arg_freetable(mk_hpp_argtable, sizeof(mk_hpp_argtable) / sizeof(mk_hpp_argtable[0]));
    arg_freetable(shm_init_argtable, sizeof(shm_init_argtable) / sizeof(shm_init_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    mem_buffer_clear(&env.m_input_meta_buffer);
    if (env.m_pom_cfg) cfg_free(env.m_pom_cfg);
    if (env.m_pom_grp_meta) pom_grp_meta_free(env.m_pom_grp_meta);

    return rv;
}
