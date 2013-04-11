#include <assert.h>
#include "log4c/appender_type_rollingfile.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "log_internal_types.h"

static void log4c_on_error(struct error_info * info, void * context, const char * fmt, va_list args);
static int log4c_em_set_appender_rollingfile(struct log_context * context, struct log4c_em * log4c_em, log4c_appender_t * appender, cfg_t cfg);
static int log4c_em_set_appender(struct log_context * context, struct log4c_em * log4c_em, cfg_t cfg);
static void log4c_em_clear_appender(struct log4c_em * log4c_em);

int log4c_em_create(struct log_context * context, const char * em_name, cfg_t cfg) {
    struct log4c_em * log4c_em;
    size_t name_len;
    log4c_category_t * log4c_category;

    assert(em_name);

    name_len = strlen(em_name) + 1;

    log4c_em = mem_alloc(context->m_alloc, sizeof(struct log4c_em) + name_len);
    if (log4c_em == NULL) {
        CPE_ERROR(context->m_em, "log4c_em: %s: malloc fail!", em_name);
        return -1;
    }

    log4c_category = log4c_category_new(em_name);
    if (log4c_category == NULL) {
        CPE_ERROR(context->m_em, "log4c_em: %s: create category fail!", em_name);
        mem_free(context->m_alloc, log4c_em);
        return -1;
    }

    memcpy(log4c_em + 1, em_name, name_len);
    log4c_em->m_name = (const char *)(log4c_em + 1);
    log4c_em->m_log4c_category = log4c_category;

    log4c_category_set_additivity(log4c_category, cfg_get_int32(cfg, "additivity", 0));

    if (log4c_em_set_appender(context, log4c_em, cfg) != 0) {
        log4c_category_delete(log4c_em->m_log4c_category);
        mem_free(context->m_alloc, log4c_em);
        return -1;
    }

    cpe_error_monitor_init(&log4c_em->m_em, log4c_on_error, log4c_em);

    if (strcmp(em_name, "default") == 0) {
        gd_app_set_em(context->m_app, &log4c_em->m_em);
    }
    else {
        gd_app_set_named_em(context->m_app, em_name, &log4c_em->m_em);
    }

    return 0;
}

static void log4c_em_free(struct log_context * context, struct log4c_em * log4c_em) {
    gd_app_context_t app;

    app = context->m_app;

    TAILQ_REMOVE(&context->m_log4c_ems, log4c_em, m_next);

    if (log4c_em->m_name) {
        if (gd_app_named_em(app, log4c_em->m_name) == &log4c_em->m_em) {
            gd_app_remove_named_em(app, log4c_em->m_name);
        }
    }
    else {
        if (gd_app_em(app) == &log4c_em->m_em) {
            gd_app_set_em(app, gd_app_print_em(app));
        }
    }

    log4c_em_clear_appender(log4c_em);

    log4c_category_delete(log4c_em->m_log4c_category);

    mem_free(context->m_alloc, log4c_em);
}

void log4c_em_free_all(struct log_context * context) {
    while(!TAILQ_EMPTY(&context->m_log4c_ems)) {
        log4c_em_free(context, TAILQ_FIRST(&context->m_log4c_ems));
    }
}

static int log4c_em_set_appender(struct log_context * context, struct log4c_em * log4c_em, cfg_t cfg) {
    const char * appender_type = cfg_get_string(cfg, "appender", NULL);
    log4c_appender_t * appender;

    if (appender_type == NULL) {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender not configured!", log4c_em->m_name);
        return -1;
    }

    appender = log4c_appender_new(log4c_em->m_name);
    if (appender == NULL) {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender new fail!", log4c_em->m_name);
        return -1;
    }

    if (strcmp(appender_type, "stderr") == 0) {
        log4c_appender_set_type(appender, log4c_appender_type_get("stream"));
        log4c_appender_set_udata(appender, stderr);
    }
    else if (strcmp(appender_type, "stdout") == 0) {
        log4c_appender_set_type(appender, log4c_appender_type_get("stream"));
        log4c_appender_set_udata(appender, stdout);
    }
    else if (strcmp(appender_type, "syslog") == 0) {
        log4c_appender_set_type(appender, log4c_appender_type_get("syslog"));
    }
    else if (strcmp(appender_type, "rollingfile") == 0) {
        if (log4c_em_set_appender_rollingfile(context, log4c_em, appender, cfg) != 0) {
            log4c_appender_delete(appender);
            return -1;
        }
    }
    else {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender %s unknown!", log4c_em->m_name, appender_type);
        log4c_appender_delete(appender);
        return -1;
    }

    log4c_category_set_appender(log4c_em->m_log4c_category, appender);

    return 0;
}

static void log4c_em_clear_appender(struct log4c_em * log4c_em) {
    log4c_appender_t * appender;

    appender = (log4c_appender_t * )log4c_category_set_appender(log4c_em->m_log4c_category, NULL);
    if (appender == NULL) return;
    log4c_appender_delete(appender);
}

static int log4c_em_set_appender_rollingfile(struct log_context * context, struct log4c_em * log4c_em, log4c_appender_t * appender, cfg_t cfg) {
    const char * log_dir = cfg_get_string(cfg, "dir", NULL);
    const char * log_prefix = cfg_get_string(cfg, "prefix", NULL);
    struct mem_buffer buffer;
    rollingfile_udata_t * udata = rollingfile_make_udata();
    if (udata == NULL) {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender rollingfile: create udata fail!", log4c_em->m_name);
        return -1;
    }

    log4c_appender_set_type(appender, log4c_appender_type_get("rollingfile"));
    log4c_appender_set_udata(appender, udata);

    mem_buffer_init(&buffer, NULL);

    /*set log dir*/
    if (log_dir == NULL || log_dir[0] != '/') {
        const char * root_dir = gd_app_root(context->m_app);
        if (root_dir) {
            mem_buffer_strcat(&buffer, root_dir);
            mem_buffer_strcat(&buffer, "/");
        }
        else {
            mem_buffer_strcat(&buffer, "/tmp/");
        }

        mem_buffer_strcat(&buffer, log_dir ? log_dir : "log");
        log_dir = (const char *)mem_buffer_make_continuous(&buffer, 0);
    }

    if (rollingfile_udata_set_logdir(udata, (char *)log_dir) != 0) {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender rollingfile: set log dir to %s fail!", log4c_em->m_name, log_dir);
        mem_buffer_clear(&buffer);
        return -1;
    }

    if (dir_mk_recursion(log_dir, DIR_DEFAULT_MODE, context->m_em, NULL) != 0) {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender rollingfile: set log dir to %s: create dir fail!", log4c_em->m_name, log_dir);
        mem_buffer_clear(&buffer);
        return -1;
    }

    /*set files prefix*/
    mem_buffer_clear_data(&buffer);
    if (log_prefix == NULL) {
        if (gd_app_argc(context->m_app) > 0) {
            const char * base_name = file_name_base(gd_app_argv(context->m_app)[0], &buffer);
            if (mem_buffer_size(&buffer) == 0) {
                mem_buffer_strcat(&buffer, base_name);
            }
            mem_buffer_strcat(&buffer, ".log");
            log_prefix = mem_buffer_make_continuous(&buffer, 0);
        }
        else {
            log_prefix = "app.log";
        }
    }

    if (rollingfile_udata_set_files_prefix(udata, (char *)log_prefix) != 0) {
        CPE_ERROR(context->m_em, "log4c_em: %s: appender rollingfile: set prefix to %s fail!", log4c_em->m_name, log_prefix);
        mem_buffer_clear(&buffer);
        return -1;
    }

    /*setup complete*/
    if (context->m_debug) {
        CPE_INFO(
            context->m_em, "log4c_em: %s: appender rollingfile: write log to %s/%s!",
            log4c_em->m_name, rollingfile_udata_get_logdir(udata), rollingfile_udata_get_files_prefix(udata));
    }

    mem_buffer_clear(&buffer);
    return 0;
}

static void log4c_on_error(struct error_info * info, void * context, const char * fmt, va_list args) {
    struct log4c_em * log4c_em;
    const log4c_location_info_t locinfo = { info->m_file, info->m_line, "(nil)", context };
    int priority = LOG4C_PRIORITY_NOTSET;

    log4c_em = (struct log4c_em *)context;

    switch (info->m_level) {
    case CPE_EL_INFO:
        priority = LOG4C_PRIORITY_INFO;
        break;
    case CPE_EL_WARNING:
        priority = LOG4C_PRIORITY_WARN;
        break;
    case CPE_EL_ERROR:
    default:
        priority = LOG4C_PRIORITY_ERROR;
        break;
    }

    __log4c_category_vlog(log4c_em->m_log4c_category, &locinfo, priority, fmt, args);
}
