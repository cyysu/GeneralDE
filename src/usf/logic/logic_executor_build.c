#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "usf/logic/logic_executor_build.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic/logic_executor.h"
#include "logic_internal_ops.h"

static logic_executor_t
logic_executor_create_composite(
    logic_manage_t mgr,
    cfg_t cfg,
    logic_executor_composite_type_t composite_type,
    logic_executor_type_group_t type_group,
    error_monitor_t em);

static logic_executor_t
logic_executor_create_decorator(
    logic_manage_t mgr,
    cfg_t cfg,
    logic_executor_decorator_type_t decorator_type,
    logic_executor_type_group_t type_group,
    error_monitor_t em);

logic_executor_t
logic_executor_build(
    logic_manage_t mgr,
    cfg_t cfg,
    logic_executor_type_group_t type_group,
    error_monitor_t em)
{
    logic_executor_type_t type;

    assert(mgr);
    assert(type_group);

    if (cfg == NULL) {
        CPE_ERROR(em, "not support create logic_executor from null cfg node");
        return NULL;
    }

    if (cfg_type(cfg) == CPE_CFG_TYPE_STRUCT) {
        cfg_t child;
        const char * name;

        child = cfg_child_only(cfg);
        if (child == 0) {
            CPE_ERROR(em, "not support create logic_executor from struct node");
            return NULL;
        }

        name = cfg_name(child);
        if (strcmp(name, "protect") == 0) {
            return logic_executor_create_decorator(
                mgr,
                child,
                logic_executor_decorator_protect,
                type_group,
                em);
        }
        else if (strcmp(name, "not") == 0) {
            return logic_executor_create_decorator(
                mgr,
                child,
                logic_executor_decorator_not,
                type_group,
                em);
        }
        else {
            type = logic_executor_type_find(type_group, name);
            if (type == 0) {
                CPE_ERROR(em, "not support logic_executor type %s", name);
                return NULL;
            }

            return logic_executor_action_create(mgr, type, child);
        }
    }
    else if (cfg_type(cfg) == CPE_CFG_TYPE_SEQUENCE) {
        return logic_executor_create_composite(
            mgr,
            cfg,
            logic_executor_composite_sequence,
            type_group,
            em);
    }
    else if (cfg_type(cfg) == CPE_CFG_TYPE_STRING) {
        const char * name = cfg_as_string(cfg, "unknown-string-type-name");
        type = logic_executor_type_find(type_group, name);
        if (type == 0) {
            CPE_ERROR(em, "not support logic_executor type %s", name);
            return NULL;
        }

        return logic_executor_action_create(mgr, type, NULL);
    }
    else {
        CPE_ERROR(em, "not support create logic_executor from cfg type %d!", cfg_type(cfg));
    }

    return NULL;
}

logic_executor_t
logic_executor_create_decorator(
    logic_manage_t mgr,
    cfg_t cfg,
    logic_executor_decorator_type_t decorator_type,
    logic_executor_type_group_t type_group,
    error_monitor_t em)
{
    logic_executor_t inner;
    logic_executor_t protect;
    inner = logic_executor_build(mgr, cfg, type_group, em);
    if (inner == NULL) return NULL;

    protect = logic_executor_decorator_create(mgr, decorator_type, inner);
    if (protect == NULL) logic_executor_free(inner);
    return protect;
}

logic_executor_t
logic_executor_create_composite(
    logic_manage_t mgr,
    cfg_t cfg,
    logic_executor_composite_type_t composite_type,
    logic_executor_type_group_t type_group,
    error_monitor_t em)
{
    struct cfg_it childIt;
    cfg_t child;
    logic_executor_t composite = logic_executor_composite_create(mgr, composite_type);
    if (composite == NULL) {
        CPE_ERROR(em, "create logic_executor_composite fail!");
        return NULL;
    }

    cfg_it_init(&childIt, cfg);
    while((child = cfg_it_next(&childIt))) {
        logic_executor_t member = logic_executor_build(mgr, child, type_group, em);
        if (member == NULL) {
            logic_executor_free(composite);
            return NULL;
        }

        logic_executor_composite_add(composite, member);
    }

    return composite;
}
