#ifndef USFPP_MONGO_TABLE_H
#define USFPP_MONGO_TABLE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "usf/mongo_agent/mongo_table.h"
#include "System.hpp"

namespace Usf { namespace Mongo {

class Table : public Cpe::Utils::SimulateObject {
public:
    operator mongo_table_t() const { return (mongo_table_t)this; }

};

}}

#endif
