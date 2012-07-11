#ifndef GD_OM_GRP_INTERNAL_DATA_H
#define GD_OM_GRP_INTERNAL_DATA_H_H
#include "om_grp_internal_types.h"

#define OM_GRP_OBJ_CONTROL_MAGIC (38438u)

struct om_grp_obj_control_data {
    uint16_t m_magic;
    uint16_t m_head_version;
    uint16_t m_page_size;
    uint16_t m_buffer_size;
    uint16_t m_objmeta_start;
    uint16_t m_objmeta_size;
    uint16_t m_metalib_start;
    uint16_t m_metalib_size;
    uint32_t m_data_start;
    uint32_t m_data_size;
};

#endif
