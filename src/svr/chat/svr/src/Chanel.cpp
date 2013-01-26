#include "Chanel.hpp"

namespace Svr { namespace Chat {

uint32_t Chanel::chanel_hash(Chanel const * chanel) {
    return (((uint32_t)chanel->m_data->chanel_type) << 29
            | ((uint32_t)chanel->m_data->chanel_id));
}

int Chanel::chanel_eq(Chanel const * l, Chanel const * r) {
    return l->m_data->chanel_type == r->m_data->chanel_type
        && l->m_data->chanel_id == r->m_data->chanel_id;
}

SVR_CHAT_MSG & Chanel::appendMsg(void) {
    SVR_CHAT_MSG * msgs = (SVR_CHAT_MSG *)(m_data + 1);

    SVR_CHAT_MSG & r = msgs[m_data->chanel_msg_w];
    r.sn = m_data->chanel_sn++;

    m_data->chanel_msg_w++;
    if (m_data->chanel_msg_w >= m_data->chanel_msg_capacity) {
        m_data->chanel_msg_w = 0;
    }

    if (m_data->chanel_msg_r == m_data->chanel_msg_r) {
        m_data->chanel_msg_r++;
        if (m_data->chanel_msg_r >= m_data->chanel_msg_capacity) {
            m_data->chanel_msg_r = 0;
        }
    }

    return r;
}

}}
