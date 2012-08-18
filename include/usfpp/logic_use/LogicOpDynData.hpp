#ifndef USFPP_LOGIC_USE_DYNDATA_H
#define USFPP_LOGIC_USE_DYNDATA_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dr/Meta.hpp"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpDynData : public Cpe::Utils::SimulateObject {
public:
    operator logic_data_t() const { return (logic_data_t)this; }

    Cpe::Dr::Meta const & dataMeta(void) const { return *(Cpe::Dr::Meta const *)logic_data_meta(*this); }

    Cpe::Dr::Meta const & recordMeta(void) const { return *(Cpe::Dr::Meta const *)logic_data_record_meta(*this); }
    size_t recordSize(void) const { return logic_data_record_size(*this); }

    void setRecordCount(size_t count);
    size_t recordCount(void) const { return logic_data_record_count(*this); }

    void recordReserve(size_t capacity);
    size_t recordCapacity(void) const { return logic_data_record_capacity(*this); }

    void * record(size_t i);
    void const * record(size_t i) const;

    void * recordAppend(void);

    template<typename T>
    T & record(size_t i) { return *(T*)this->record(i); }

    template<typename T>
    T const & record(size_t i) const { return *(const T*)this->record(i); }
    
    template<typename T>
    T & recordAppend(void) { return *(T*)this->recordAppend(); }
    
    const char * dump(mem_buffer_t buffer) const { return logic_data_dump(*this, buffer); }
};

}}

#endif
