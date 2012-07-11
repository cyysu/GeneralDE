#ifndef GD_OM_GRP_TESTENV_OMGRP_H
#define GD_OM_GRP_TESTENV_OMGRP_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../om_grp_meta.h"
#include "../om_grp_obj_mgr.h"

namespace gd { namespace om_grp { namespace testenv {

class with_om_grp : public ::testenv::env<> {
public:
    with_om_grp();

    om_grp_meta_t t_om_grp_meta_create(
        const char * om_meta,
        const char * metalib, 
        uint16_t page_size = 256, size_t buffer_size = 512);

    om_grp_meta_t t_om_grp_meta_create(
        const char * om_meta,
        LPDRMETALIB metalib,
        uint16_t page_size = 256, size_t buffer_size = 512);

    om_grp_obj_mgr_t t_om_grp_obj_mgr_create(
        const char * om_meta,
        const char * metalib, 
        size_t capacity = 2048, uint16_t page_size = 256, size_t buffer_size = 512);

    om_grp_obj_mgr_t t_om_grp_obj_mgr_create(
        const char * om_meta,
        LPDRMETALIB metalib, 
        size_t capacity = 2048, uint16_t page_size = 256, size_t buffer_size = 512);
};

}}}

#endif
