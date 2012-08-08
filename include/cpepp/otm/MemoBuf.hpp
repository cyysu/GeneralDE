#ifndef CPEPP_OTM_MEMOBUF_H
#define CPEPP_OTM_MEMOBUF_H
#include "System.hpp"

namespace Cpe { namespace Otm {

template<size_t capacity>
class MemoBuf {
public:
    operator otm_memo_t (void) { return &m_memos[0]; }
public:
	otm_memo * findMemo( otm_timer_id_t timer_id )
	{
		for( size_t i = 0; i < capacity; ++i )
		{
			if( m_memos[i].m_id == timer_id )
			{
				return &m_memos[i];
			}
		}
		return NULL;
	}

private:
    struct otm_memo m_memos[capacity];
};

}}

#endif
