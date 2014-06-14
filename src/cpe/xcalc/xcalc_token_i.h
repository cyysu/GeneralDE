#ifndef CPE_XCALC_TOKEN_I_H
#define CPE_XCALC_TOKEN_I_H
#include "cpe/xcalc/xcalc_token.h"

#ifdef __cplusplus
extern "C" {
#endif

xtoken_t xtoken_set_sub(xtoken_t token, xtoken_t sub);

/*type operations*/
#define xtoken_type_is_data(__token_type) (__token_type & XTOKEN_DATA_FLAG)
#define xtoken_type_is_sign(__token_type) (!xtoken_is_data(__token_type))
#define xtoken_type_index(__token_type) ((__token_type & 0xFF))

#define xtoken_is_data(__token) xtoken_type_is_data((__token)->m_type)
#define xtoken_is_sign(__token) xtoken_type_is_sign((__token)->m_type)

#define xtoken_set_type(__token, __type) ((__token)->m_type = (uint32_t)__type)
#define xtoken_get_type(__token) ((__token)->m_type)
const char * xtoken_type_name(uint32_t token_type);

/*int operations*/
#define xtoken_get_int(__token) ((__token)->m_data.num._int)
#define xtoken_set_int(__token, __val) ((__token)->m_data.num._int = (int64_t)__val)

/*double operations*/
#define xtoken_get_double(__token) ((__token)->m_data.num._double)
#define xtoken_set_double(__token, __val) ((__token)->m_data.num._double = (double)(__val))
double xtoken_get_double_2(xtoken_t token);

void xtoken_set_str(xtoken_t token, char * begin, char * end);

const char * xtoken_type_name(uint32_t token_type);

#define XTOKEN_DATA_FLAG 0x00008000u
#define XTOKEN_ADD 0u
#define XTOKEN_SUB 1u
#define XTOKEN_MUL 2u
#define XTOKEN_DIV 3u
#define XTOKEN_EQU 4u
#define XTOKEN_NE  5U
#define XTOKEN_BG  6U
#define XTOKEN_BE  7U
#define XTOKEN_LT  8u
#define XTOKEN_LE  9u
#define XTOKEN_AND 10u
#define XTOKEN_OR  11u
#define XTOKEN_NOT 12u
#define XTOKEN_LEFT_BRACKET 13u
#define XTOKEN_RIGHT_BRACKET 14u
#define XTOKEN_COLON 15u
#define XTOKEN_QES 16u
#define XTOKEN_COMMA 17u
#define XTOKEN_FUNC 18u
#define XTOKEN_END 19u
#define XTOKEN_VAL (XTOKEN_DATA_FLAG | 20u)
#define XTOKEN_NUM_INT  (XTOKEN_DATA_FLAG | 21u)
#define XTOKEN_NUM_FLOAT  (XTOKEN_DATA_FLAG | 22u)
#define XTOKEN_STRING  (XTOKEN_DATA_FLAG | 23u)
#define XTOKEN_VAL2  (XTOKEN_DATA_FLAG | 24u)
#define XTOKEN_ERROR 25
#define XTOKEN_NUMBER 26

#ifdef __cplusplus
}
#endif

#endif
