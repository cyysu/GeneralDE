#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/mmap_utils.h"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "StringInfoMgr.hpp"
#include "EnvExt.hpp"

namespace UI { namespace App {

StringInfoMgr::StringInfoMgr(EnvExt & env)
    : m_env(env)
    , m_data(NULL)
    , m_data_size(0)
    , m_record_count(0)
    , m_items(NULL)
    , m_strings(NULL)
    , m_format_buf(NULL)
{
}

StringInfoMgr::~StringInfoMgr() {
    unload();
}

static const char * languagePostfix(Language lan)  {
    switch(lan) {
    case LANGUAGE_EN:
        return "en";
    case LANGUAGE_CN:
        return "cn";
    case LANGUAGE_TW:
        return "tw";
    default:
        return "";
    }
}

void StringInfoMgr::load(void) {
    unload();

    char file_buf[256];
    snprintf(file_buf, sizeof(file_buf), "%s/etc/meta/strings_%s.stb", m_env.app().root(), languagePostfix(m_env.language()));
    m_data = mmap_file_load(file_buf, "r", &m_data_size, m_env.app().em());
    if (m_data == NULL) {
        APP_CTX_ERROR(m_env.app(), "StringInfoMgr: load from %s fail!", file_buf);
        return;
    }

    StringTableHead const * head = (StringTableHead const *)m_data;
    if (head->m_magic[0] != 'S' || head->m_magic[1] != 'T' || head->m_magic[2] != 'B' || head->m_magic[3] != 01) {
        APP_CTX_ERROR(m_env.app(), "StringInfoMgr: load from %s: head magic check fail!", file_buf);
        unload();
        return;
    }

    CPE_COPY_NTOH32(&m_record_count, &head->m_record_count);

    uint32_t head_sizde = sizeof(StringTableHead) + sizeof(StringTableItem) * m_record_count * 2;
    if (head_sizde > m_data_size) {
        APP_CTX_ERROR(
            m_env.app(), "StringInfoMgr: load from %s: recount count %d, head size %d, total size %d!",
            file_buf, m_record_count, head_sizde, (int)m_data_size);
        unload();
        return;
    }

#ifdef CPE_BIG_ENDIAN
    m_items = (StringTableItem const *)(head + 1);
#else
    m_items = ((StringTableItem const *)(head + 1)) + m_record_count;
#endif

    m_strings = ((const char *)(m_data)) + head_sizde;
}

void StringInfoMgr::unload(void) {
    if (m_data) {
        mmap_unload(m_data, m_data_size);
    }

    m_data = NULL;
    m_data_size = 0;
    m_record_count = 0;
    m_items = NULL;
    m_strings = NULL;
}

static int cmp_item(void const * l, void const * r) {
    return ((int)((StringTableItem const *)l)->m_msg_id) - ((int)((StringTableItem const *)r)->m_msg_id);
}

const char * StringInfoMgr::message(uint32_t msg_id) const {
    if (m_data == NULL) {
        const_cast<StringInfoMgr&>(*this).load();
        if (m_data == NULL) {
            return "load language fail";
        }
    }

    StringTableItem key;
    key.m_msg_id = msg_id;
    StringTableItem const * p = (StringTableItem const *)bsearch(&key, m_items, m_record_count, sizeof(key), cmp_item);
    if (p == NULL) {
        static char s_buf[64];
        snprintf(s_buf, sizeof(s_buf), "%d", msg_id);
        return s_buf;
    }

    return m_strings + p->m_msg_pos;
}

const char * StringInfoMgr::message(uint32_t msg_id, char * args) const {
    const char * msg = message(msg_id);

    bool have_arg = false;
    while(const char * arg_begin = strstr(msg, "$(")) {
        if (!have_arg) {
            m_format_buf.clear();
            have_arg = true;
        }

        m_format_buf.append(msg, arg_begin - msg);

        /*读取参数名字 */
        char arg_buf[64];
        const char * arg_end = strchr(arg_begin, ')');
        if (arg_end == NULL) return msg;
        uint32_t arg_len = arg_end - arg_begin - 2;
        if (arg_len + 1 > CPE_ARRAY_SIZE(arg_buf)) return msg;

        memcpy(arg_buf, arg_begin + 2, arg_len);
        arg_buf[arg_len] = 0;

        /*填写参数 */
        bool found = false;
        char * find_at = args;
        while(find_at) {
            char * sep = strchr(find_at, ',');
            if (sep) *sep = 0;

            if (char * eq = strchr(find_at, '=')) {
                *eq = 0;

                char * h = cpe_str_trim_head(find_at);
                char * e = cpe_str_trim_tail(eq, h);

                char s = *e;

                *e = 0;

                if (strcmp(h, arg_buf) == 0) {
                    char * v = cpe_str_trim_head(eq + 1);
                    char * ve = cpe_str_trim_tail(v + strlen(v), v);
                    m_format_buf.append(v, ve - v);
                    found = true;
                }
                
                *e = s;

                *eq = '=';

                if (found) break;
            }
            
            if (sep) *sep = ',';

            if (found) break;
        }

        if (!found) return msg;

        msg = arg_end + 1;
    }

    if (have_arg) {
        m_format_buf.append(msg, strlen(msg));
        m_format_buf.append(0);
        return (const char *)m_format_buf.make_continuous();
    }
    else {
        return msg;
    }
}

}}
