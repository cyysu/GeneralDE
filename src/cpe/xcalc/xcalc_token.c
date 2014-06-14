#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "xcalc_token_i.h"

int32_t xtoken_to_int32(xtoken_t token) {
    int ret = 0;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        ret = token->m_data.num._int;
        break;
    case XTOKEN_NUM_FLOAT:
        ret = (int32_t)token->m_data.num._double;
        break;
    case XTOKEN_STRING:
        if (token->m_data.str._string > token->m_data.str._end) {
        }
        else if (token->m_data.str._string) {
            int len = token->m_data.str._end - token->m_data.str._string;
            char buf[256];
            memcpy(buf, token->m_data.str._string, len);
            //TODO: SET_STR
            ret = atoi(buf);
        }
        break;
    default:
        break;
    }

    return ret;
}

int64_t xtoken_to_int64(xtoken_t token);

xtoken_t xtoken_set_sub(xtoken_t token, xtoken_t sub) {
    xtoken_t old_sub = token->m_sub;
    token->m_sub = sub;
    return old_sub;
}

double xtoken_get_double_2(xtoken_t token) {
    if (token->m_type == XTOKEN_NUM_INT) {
        return (double)token->m_data.num._int;
    }
    else if (token->m_type == XTOKEN_STRING) {
        return 0;
    }
    else {
        return token->m_data.num._double;
    }
}

void xtoken_dump(write_stream_t s, xtoken_t token) {
    stream_printf(s, "type: %u", token->m_type);

    if (xtoken_is_data(token) || token->m_type == XTOKEN_NUM_INT) {
    }
}

static const char * g_token_names[XTOKEN_NUMBER] = {
    "+",
    "-",
    "*",
    "/",
    "==",
    "!-",
    ">",
    ">=",
    "<",
    "<=",
    "&&",
    "||",
    "!",
    "(",
    ")",
    ":",
    "?",
    ",",
    "fun",
    "end",
    "val",
    "int",
    "dou",
    "str",
    "val2",
    "err"
};

const char * xtoken_type_name(uint32_t token_type) {
    assert(token_type >= 0 && token_type < CPE_ARRAY_SIZE(g_token_names));
    return g_token_names[token_type];
}

void xtoken_set_str(xtoken_t token, char * begin, char * end) {
}
