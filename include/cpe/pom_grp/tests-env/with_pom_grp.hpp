#ifndef CPE_POM_GRP_TESTENV_OMGRP_H
#define CPE_POM_GRP_TESTENV_OMGRP_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../pom_grp_meta.h"
#include "../pom_grp_obj_mgr.h"

namespace cpe { namespace pom_grp { namespace testenv {

class with_pom_grp : public ::testenv::env<> {
public:
    with_pom_grp();

    const char * t_pom_grp_meta_dump(pom_grp_meta_t meta);

    pom_grp_meta_t t_pom_grp_meta_create(
        const char * om_meta,
        const char * metalib, 
        uint16_t page_size = 256);

    pom_grp_meta_t t_pom_grp_meta_create(
        const char * om_meta,
        LPDRMETALIB metalib,
        uint16_t page_size = 256);

    pom_grp_obj_mgr_t t_pom_grp_obj_mgr_create(
        const char * om_meta,
        const char * metalib, 
        size_t capacity = 2048, uint16_t page_size = 256);

    pom_grp_obj_mgr_t t_pom_grp_obj_mgr_create(
        const char * om_meta,
        LPDRMETALIB metalib, 
        size_t capacity = 2048, uint16_t page_size = 256);
};

}}}

#endif
