#ifndef USFPP_LOGIC_USE_DYNDATA_T_H
#define USFPP_LOGIC_USE_DYNDATA_T_H
#include "LogicOpDynData.hpp"

namespace Usf { namespace Logic {

template<typename DataT>
class LogicOpDynDataT : public LogicOpDynData {
public:
    typedef typename Cpe::Dr::MetaTraits<DataT>::dyn_element_type RecordType;
    typedef DataT DataType;

    LogicOpDynDataT(LogicOpDynDataT & o) : LogicOpDynData(o) {
    }

    LogicOpDynDataT(logic_data_t data = NULL) : LogicOpDynData(data) {
    }

    template<typename OT>
    LogicOpDynDataT(OT & owner, size_t record_capacity = 1)
        : LogicOpDynData(owner, Cpe::Dr::MetaTraits<DataT>::META, record_capacity) 
    {
    }

    using LogicOpDynData::recordAppend;
    using LogicOpDynData::record;
    using LogicOpDynData::as;

    DataType const & as(void) { return LogicOpDynData::as<DataType>(); }

    RecordType & recordAppend(void) { return LogicOpDynData::recordAppend<RecordType>(); }

    RecordType & recordAppend(RecordType const & o) { 
        RecordType & r = recordAppend();
        memcpy(&r, &o, sizeof(o));
        return r;
    }
};

}}

#endif
