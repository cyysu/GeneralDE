#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"

int combine_cfg(const char * input_dir, const char * output_file_name, const char * format, error_monitor_t em) {
    cfg_t cfg = NULL;
    FILE * output_file = NULL;
    int rv = -1;

    cfg = cfg_create(NULL);
    if (cfg == NULL) {
        CPE_ERROR(em, "conbine cfg: create cfg fail!");
        goto COMBINE_CFG_COMPLETE;
    }
    
    if (cfg_read_dir(cfg, input_dir, cfg_replace, em, NULL) != 0) {
        CPE_ERROR(em, "conbine cfg: read from dir %s fail!", input_dir);
        goto COMBINE_CFG_COMPLETE;
    }

    output_file = file_stream_open(output_file_name, "w", em);
    if (output_file == NULL) {
        CPE_ERROR(em, "conbine cfg: create output file %s fail!", output_file_name);
        goto COMBINE_CFG_COMPLETE;
    }

    if (strcmp(format, "yml") == 0) {
        struct write_stream_file fs;

        write_stream_file_init(&fs, output_file, em);

        if (cfg_write((write_stream_t)&fs, cfg, em) != 0) {
            CPE_ERROR(em, "conbine cfg: write result(yml) file to %s fail!", output_file_name);
            goto COMBINE_CFG_COMPLETE;
        }
    }
    else if (strcmp(format, "bin") == 0) {
        struct mem_buffer buffer;

        mem_buffer_init(&buffer, NULL);

        if (cfg_write_bin_to_buffer(&buffer, cfg, em) <= 0) {
            CPE_ERROR(em, "conbine cfg: write result(bin) to buffer fail!");
            mem_buffer_clear(&buffer);
            goto COMBINE_CFG_COMPLETE;
        }

        if (file_stream_write_from_buffer(output_file, &buffer, em) <= 0) {
            CPE_ERROR(em, "conbine cfg: write result(bin) file to %s fail!", output_file_name);
            mem_buffer_clear(&buffer);
            goto COMBINE_CFG_COMPLETE;
        }

        mem_buffer_clear(&buffer);
    }
    else {
        CPE_ERROR(em, "conbine cfg: unknown format %s", format);
        goto COMBINE_CFG_COMPLETE;
    }
    
    rv = 0;

COMBINE_CFG_COMPLETE:
    if (cfg) cfg_free(cfg);
    if (output_file) file_stream_close(output_file, em);

    return rv;
}

int main(int argc, char * argv[]) {
    /*mk hpp*/
    struct arg_rex  * combine =     arg_rex1(NULL, NULL, "combine", NULL, 0, NULL);
    struct arg_file  * combine_input =     arg_file1(NULL, "input", NULL, "input cfg root");
    struct arg_file  * combine_output =     arg_file1(NULL, "output", NULL, "output cfg file");
    struct arg_str  * combine_format =     arg_str1(NULL, "format", "(yml|bin)", "output format");
    struct arg_end  * combine_end = arg_end(20);
    void* combine_argtable[] = { combine, combine_input, combine_output, combine_format, combine_end };
    int combine_nerrors;

    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv;

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    rv = -1;

    combine_nerrors = arg_parse(argc, argv, combine_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    if (combine_nerrors == 0) {
        rv = combine_cfg(combine_input->filename[0], combine_output->filename[0], combine_format->sval[0], em);
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
    printf("usage 1: %s ", argv[0]); arg_print_syntax(stdout, combine_argtable, "\n");
    printf("usage 2: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
    arg_freetable(combine_argtable, sizeof(combine_argtable) / sizeof(combine_argtable[0]));
    arg_freetable(common_argtable, sizeof(common_argtable) / sizeof(common_argtable[0]));

    return rv;
}
