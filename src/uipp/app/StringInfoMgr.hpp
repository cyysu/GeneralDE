#ifndef UIPP_APP_STRINGINFOMGR_H
#define UIPP_APP_STRINGINFOMGR_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/MemBuffer.hpp"
#include "System.hpp"

namespace UI { namespace App {

#pragma pack(1)
struct StringTableItem {
    uint32_t m_msg_id;
    uint32_t m_msg_pos;
};

struct StringTableHead {
    char m_magic[4];
    uint32_t m_record_count;
};
#pragma pack()

class StringInfoMgr : public Cpe::Utils::Noncopyable {
public:
    StringInfoMgr(EnvExt & env);
    ~StringInfoMgr();

    void load(void);
    void unload(void);

    const char * message(uint32_t msg_id) const;
    const char * message(uint32_t msg_id, char * args) const;

private:
    EnvExt & m_env;
    void * m_data; 
    size_t m_data_size;
    uint32_t m_record_count;
    StringTableItem const * m_items; 
    const char * m_strings;
    mutable Cpe::Utils::MemBuffer m_format_buf;
};

}}

#endif
