#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"

static void bpg_use_op_dump_all(logic_context_t context, cfg_t root) {
    struct mem_buffer buffer;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    gd_app_context_t app = logic_context_app(context);

    mem_buffer_init(&buffer, gd_app_alloc(app));

    cfg_dump(root, (write_stream_t)&stream, 4, 4);

    stream_putc((write_stream_t)&stream, 0);

    APP_CTX_INFO(app, "dump context\n%s", (const char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);
}

static void bpg_use_op_dump_part(logic_context_t context, cfg_t root, const char * path) {
    gd_app_context_t app = logic_context_app(context);
    cfg_t child = cfg_find_cfg(root, path);
    if (child) {
        struct mem_buffer buffer;
        struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);

        mem_buffer_init(&buffer, gd_app_alloc(app));

        cfg_dump_inline(child, (write_stream_t)&stream);
        stream_putc((write_stream_t)&stream, 0);

        mem_buffer_clear(&buffer);

        APP_CTX_INFO(app, "    %s: %s", path, (const char *)mem_buffer_make_continuous(&buffer, 0));
    }
    else {
        APP_CTX_INFO(app, "    %s: NULL", path);
    }
}

logic_op_exec_result_t logic_use_op_dump(logic_context_t context, logic_executor_t executor, void * user_data, cfg_t args) {
    gd_app_context_t app = logic_context_app(context);
    cfg_t dump_data;
    cfg_t parts_cfg;

    if (cfg_get_int32(args, "dump-in-debug", 1)) {
        if (!logic_context_flag_is_enable(context, logic_context_flag_debug)) {
            return logic_op_exec_result_true;
        }
    }

    dump_data = cfg_create(gd_app_alloc(app));
    if (dump_data == NULL) {
        APP_CTX_ERROR(app, "logicuse_op_dump: create dump data fail!");
        return logic_op_exec_result_null;
    }

    logic_context_data_dump_to_cfg(context, dump_data);

    parts_cfg = cfg_find_cfg(args, "parts");
    if (parts_cfg == NULL) {
        bpg_use_op_dump_all(context, dump_data);
    }
    else {
        struct cfg_it it;
        cfg_t child;
        cfg_it_init(&it, parts_cfg);

        while((child = cfg_it_next(&it))) {
            const char * path = cfg_as_string(child, NULL);
            if (path) {
                bpg_use_op_dump_part(context, cfg_child_only(dump_data), path);
            }
        }
    }

    cfg_free(dump_data);

    return logic_op_exec_result_true;

};

EXPORT_DIRECTIVE
int logic_use_op_dump_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    logic_executor_type_t type;

    type = logic_executor_type_create_global(
        app,
        cfg_get_string(cfg, "type-group", NULL),
        gd_app_module_name(module),
        logic_use_op_dump,
        NULL,
        gd_app_em(app));

    return type ? 0 : -1;
}

EXPORT_DIRECTIVE
void logic_use_op_dump_app_fini(gd_app_context_t app, gd_app_module_t module) {
}
