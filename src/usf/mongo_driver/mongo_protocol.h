#ifndef TSF4WG_TCAPLUS_PROTOCOL_TYPES_H
#define TSF4WG_TCAPLUS_PROTOCOL_TYPES_H
#include "cpe/pal/pal_types.h"

#pragma pack(1)

struct mongo_pro_header {
    int32_t m_len;
    int32_t m_id;
    int32_t m_response_to;
    int32_t m_op;
};

struct mongo_pro_reply_fields {
    int32_t m_flag; /* FIX THIS COMMENT non-zero on failure */
    int64_t m_cursor_id;
    int32_t m_start;
    int32_t m_num;
};

#pragma pack()


#endif
