#ifndef USFPP_MONGO_PACKAGE_H
#define USFPP_MONGO_PACKAGE_H
#include "cpepp/utils/ClassCategory.hpp"
<<<<<<< HEAD
=======
#include "cpepp/dr/System.hpp"
>>>>>>> 5aebc81cb0ca2f0d0a569701c102fa4cf9abd362
#include "usf/mongo_driver/mongo_pkg.h"
#include "System.hpp"

namespace Usf { namespace Mongo {

class Package : public Cpe::Utils::SimulateObject {
public:
    operator mongo_pkg_t() const { return (mongo_pkg_t)this; }

    mongo_db_op_t op(void) const { return mongo_pkg_op(*this); }
    void setOp(mongo_db_op_t op) { mongo_pkg_set_op(*this, op); }

    uint32_t id(void) const { return mongo_pkg_id(*this); }
    void setId(uint32_t id) { mongo_pkg_set_id(*this, id); }

    const char * ns(void) const { return mongo_pkg_ns(*this); }
    void setNs(const char * ns) { mongo_pkg_set_ns(*this, ns); }

<<<<<<< HEAD
=======
    /*doc op*/
    void docAppend(LPDRMETA meta, void const * data, size_t size);

    template<typename T>
    void docAppend(T const & data) {
        docAppend(Cpe::Dr::MetaTraits<T>::META, &data, sizeof(data));
    }

    void docOpen(void);
    void docClose(void);
    bool docIsClosed(void) const { return mongo_pkg_doc_is_closed(*this); }
    int docCount(void) const { return mongo_pkg_doc_count(*this); }

    /*basic data op*/
    void appendInt32(const char * name, const int32_t i);
    void appendInt64(const char * name, int64_t i);
    void appendDouble(const char * name, const double d);
    void appendString(const char * name, const char *str);
    void appendString(const char * name, const char *str, int len);
    void appendSymbol(const char * name, const char *str);
    void appendSymbol(const char * name, const char *str, int len);
    void appendCode(const char * name, const char *str);
    void appendCode(const char * name, const char *str, int len);
    void appendBinary(const char * name, char type, const char *str, int len);
    void appendBool(const char *name, const bool v);
    void appendNull(const char *name);
    void appendUndefined(const char *name);
    void appendRegex(const char *name, const char *pattern, const char *opts);
    void appendTimestamp(const char *name, int time, int increment);
    void appendData(const char *name, int64_t millis);
    void appendTimeS(const char *name, time_t secs);

>>>>>>> 5aebc81cb0ca2f0d0a569701c102fa4cf9abd362
    /*other op*/
    const char * dump_data(mem_buffer_t buffer) const { return mongo_pkg_dump(*this, buffer, 0); }

    static Package & _cast(mongo_pkg_t pkg);
    static Package & _cast(dp_req_t req);
};

}}

#endif
