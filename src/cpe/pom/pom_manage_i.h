#ifndef CPE_POM_MAMAGE_H
#define CPE_POM_MAMAGE_H
#include "cpe/pom/pom_types.h"
#include "pom_class_i.h"
#include "pom_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_mgr {
    mem_allocrator_t m_alloc;
    struct pom_class_mgr m_classMgr;
    struct pom_buffer_mgr m_bufMgr;
};

#ifdef __cplusplus
}
#endif

#endif


