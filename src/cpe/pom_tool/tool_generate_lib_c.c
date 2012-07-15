#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "tool_env.h"

static int pom_tool_do_generate_lib_c(write_stream_t stream, struct pom_tool_env * env, const char * arg_name) {
    int rv;
    unsigned char * buf;
    size_t size;
    int first_line;

    assert(stream);
    assert(env);
    assert(ctx->m_metalib);

    rv = 0;

    buf = (unsigned char *)ctx->m_metalib;
    size = dr_lib_size(ctx->m_metalib);

    first_line = 1;

    stream_printf(stream, "#include \"cpe/pal/pal_external.h\"\n EXPORT_DIRECTIVE\nchar %s[] = {", arg_name);

    while(size > 0) {
        size_t i;
        size_t line_size = size > 16 ? 16 : size;

        stream_printf(stream, "\n    ");

        if (first_line) {
            stream_printf(stream, "  ");
            first_line = 0;
        }
        else {
            stream_printf(stream, ", ");
        }

        for(i = 0; i < line_size; ++i, ++buf) {
            if (i > 0) { stream_printf(stream, ", "); }

            stream_printf(stream, "%s", "0x");
            stream_printf(stream, "%.2X", *buf);
        }

        size -= line_size;
    }

    stream_printf(stream, "\n};\n");

    return rv;
}

int pom_tool_generate_lib_c(struct pom_tool_env * env, const char * filename) {
    struct write_stream_file stream;
    FILE * fp;
    int rv;

    fp = file_stream_open(filename, "w", env->m_em);
    if (fp == NULL) {
        CPE_ERROR(env->m_em, "open %s fro generate lib c fail!", filename);
        return -1;
    }

    write_stream_file_init(&stream, fp, env->m_em);

    rv = pom_tool_do_generate_lib_c((write_stream_t)&stream, env);

    file_stream_close(fp, env->m_em);

    return rv;
}


