#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_ops.h"

static int cfg_apply_modify_set(cfg_t cfg, cfg_t modify_info, error_monitor_t em) {
    const char * path = cfg_name(modify_info);

    switch(cfg_type(modify_info)) {
    case CPE_CFG_TYPE_SEQUENCE: {
        cfg_t new_node = cfg_add_seq(cfg, path, em);
        if (new_node == NULL) return -1;
        return cfg_merge(new_node, cfg, cfg_replace, em);
    }
    case CPE_CFG_TYPE_STRUCT: {
        cfg_t new_node = cfg_add_struct(cfg, path, em);
        if (new_node == NULL) return -1;
        return cfg_merge(new_node, cfg, cfg_replace, em);
    }
    case CPE_CFG_TYPE_INT8:
        if (cfg_add_int8(cfg, path, cfg_as_int8(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT8:
        if (cfg_add_uint8(cfg, path, cfg_as_uint8(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_INT16:
        if (cfg_add_int16(cfg, path, cfg_as_int16(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT16:
        if (cfg_add_uint16(cfg, path, cfg_as_uint16(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_INT32:
        if (cfg_add_int32(cfg, path, cfg_as_int32(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT32:
        if (cfg_add_uint32(cfg, path, cfg_as_uint32(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_INT64:
        if (cfg_add_int64(cfg, path, cfg_as_int64(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT64:
        if (cfg_add_uint64(cfg, path, cfg_as_uint64(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_STRING:
        if (cfg_add_string(cfg, path, cfg_as_string(modify_info, ""), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_FLOAT:
        if (cfg_add_float(cfg, path, cfg_as_float(modify_info, 0.0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_DOUBLE:
        if (cfg_add_double(cfg, path, cfg_as_double(modify_info, 0.0), em) == NULL) {
            return -1;
        }
        break;
    default:
        CPE_ERROR(em, "cfg_apply_modify_set: unknown type %d!", cfg_type(modify_info));
        return -1;
    }

    return 0;
}

int cfg_apply_modify(cfg_t cfg, cfg_t modify_info, error_monitor_t em) {
    const char * op_name;

    modify_info = cfg_child_only(modify_info);
    if (modify_info == NULL) {
        CPE_ERROR(em, "cfg_apply_modify: input format error, should only one child!");
        return -1;
    }

    op_name = cfg_name(modify_info);
    if (strcmp(op_name, "set") == 0) {
        modify_info = cfg_child_only(modify_info);
        if (modify_info == NULL) {
            CPE_ERROR(em, "cfg_apply_modify: argument of set format error, should only one child!");
            return -1;
        }

        return cfg_apply_modify_set(cfg, modify_info, em);
    }
    else {
        CPE_ERROR(em, "cfg_apply_modify: unknown op name %s!", op_name);
        return -1;
    }

    return 0;
}


int cfg_apply_modify_seq(cfg_t cfg, cfg_t modify_info, error_monitor_t em) {
    int r = 0;
    struct cfg_it it;
    cfg_t child;

    cfg_it_init(&it, modify_info);

    while((child = cfg_it_next(&it))) {
        if (cfg_apply_modify(cfg, child, em) != 0) {
            r = -1;
        }
    }

    return r;
}


