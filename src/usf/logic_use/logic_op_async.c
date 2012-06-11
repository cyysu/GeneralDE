#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_stack.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic_use/logic_op_async.h"
#include "protocol/logic_op_async_data.h"

struct logic_op_async_ctx {
    mem_allocrator_t m_alloc;
    logic_op_fun_t m_send_fun;
    logic_op_fun_t m_recv_fun;
    void * m_user_data;
    logic_op_ctx_fini_fun_t m_fini_fun;
};

extern char g_metalib_logic_op_async_package[];

logic_op_exec_result_t
logic_op_asnyc_exec(
    logic_context_t context,
    logic_stack_node_t stack_node,
    logic_op_fun_t send_fun,
    logic_op_fun_t recv_fun,
    void * user_data,
    cfg_t args)
{
    logic_data_t async_op_data;
    LPDRMETA meta;
    LOGIC_OP_ASNYC_STATE * async_op_state;
    logic_executor_t executor;

    executor = 

    meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_logic_op_async_package, "logic_op_asnyc_state");
    if (meta == NULL) {
        APP_CTX_ERROR(
            logic_context_app(context), "logic_op_asnyc_exec: meta logic_op_asnyc_state not exist!");
        return logic_op_exec_result_null;
    }

    async_op_data = logic_stack_node_data_check_or_create(stack_node, meta, 0);
    if (async_op_data == NULL) {
        APP_CTX_ERROR(
            logic_context_app(context), "logic_op_asnyc_exec: create context fail!");
        return logic_op_exec_result_null;
    }

    async_op_state = (LOGIC_OP_ASNYC_STATE *)logic_data_data(async_op_data);
    if (async_op_state->state == 0) {
        async_op_state->state = 1;
        return send_fun(context, stack_node, user_data, args);
    }
    else {
        return recv_fun(context, stack_node, user_data, args);
    }
}

static logic_op_exec_result_t
logic_op_asnyc(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    struct logic_op_async_ctx * ctx = (struct logic_op_async_ctx *)user_data;
    return logic_op_asnyc_exec(context, stack_node, ctx->m_send_fun, ctx->m_recv_fun, ctx->m_user_data, args);
};

static void logic_op_async_ctx_free(void * ctx) {
    struct logic_op_async_ctx * pkg_ctx = (struct logic_op_async_ctx *)ctx;
    if (pkg_ctx->m_fini_fun) pkg_ctx->m_fini_fun(pkg_ctx->m_user_data);
    mem_free(pkg_ctx->m_alloc, pkg_ctx);
}

logic_executor_type_t
logic_op_async_type_create(
    gd_app_context_t app,
    const char * group_name,
    const char * name,
    logic_op_fun_t send_fun,
    logic_op_fun_t recv_fun,
    void * user_data,
    logic_op_ctx_fini_fun_t fini_fun,
    error_monitor_t em)
{
    logic_executor_type_t type;
    struct logic_op_async_ctx * ctx;


    ctx = mem_alloc(gd_app_alloc(app), sizeof(struct logic_op_async_ctx));
    if (ctx == NULL) {
        CPE_ERROR(em, "bpg_use_op_send_pkg_create: malloc ctx fail!");
        return NULL;
    }

    ctx->m_alloc = gd_app_alloc(app);
    ctx->m_send_fun = send_fun;
    ctx->m_recv_fun = recv_fun;
    ctx->m_user_data = user_data;
    ctx->m_fini_fun = fini_fun;

    type = logic_executor_type_create_global(
        app,
        group_name,
        name,
        logic_op_asnyc,
        ctx,
        logic_op_async_ctx_free,
        gd_app_em(app));

    if (type == NULL) {
        CPE_ERROR(em, "bpg_use_op_send_pkg_create: create type fail!");
        logic_op_async_ctx_free(ctx);
        return NULL;
    }

    return type;
}

