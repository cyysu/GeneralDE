#ifndef USFPP_MONGO_REQUEST_H
#define USFPP_MONGO_REQUEST_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpepp/dr/System.hpp"
#include "cpepp/dr/Meta.hpp"
#include "usf/mongo_agent/mongo_request.h"
#include "System.hpp"

namespace Usf { namespace Mongo {

class Request : public Cpe::Utils::SimulateObject {
public:
    operator mongo_request_t() const { return (mongo_request_t)this; }

    Table const & table(void) const { return *(Table const *)mongo_request_table(*this); }

    void addRecord(LPDRMETA meta, const void * data, size_t size);

    template<typename T>
    void addRecord(T const & data) {
        addRecord(Cpe::Dr::MetaTraits<T>::META, &data, sizeof(data));
    }

    static Request & get(mongo_agent_t agent, const char * name);
};

}}

#endif
