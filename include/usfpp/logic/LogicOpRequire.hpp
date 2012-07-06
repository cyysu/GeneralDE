#ifndef USFPP_LOGIC_OPREQUIRE_H
#define USFPP_LOGIC_OPREQUIRE_H
#include "cpe/utils/hash_string.h"
#include "cpepp/utils/System.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "LogicOpData.hpp"

namespace Usf { namespace Logic {

class LogicOpRequire : public Cpe::Utils::SimulateObject  {
public:
    operator logic_require_t () const { return (logic_require_t)this; }

    logic_require_id_t id(void) const { return logic_require_id(*this); }

    logic_require_state_t state(void) const { return logic_require_state(*this); }

    LogicOpContext & context(void) { return *(LogicOpContext*)logic_require_context(*this); }
    LogicOpContext const & context(void) const { return *(LogicOpContext*)logic_require_context(*this); }

    void cancel(void) { logic_require_cancel(*this); }
    void setError(void) { logic_require_set_error(*this); }
    void setDone(void) { logic_require_set_done(*this); }

    LogicOpData & data(const char * name);
    LogicOpData const & data(const char * name) const;

    LogicOpData * findData(const char * name) { return (LogicOpData *)logic_require_data_find(*this, name); }
    LogicOpData const * findData(const char * name) const { return (LogicOpData *)logic_require_data_find(*this, name); }
    LogicOpData & checkCreateData(LPDRMETA meta, size_t capacity = 0);
    LogicOpData & copy(logic_data_t input);

    template<typename T>
    T & data(const char * name = Cpe::Dr::MetaTraits<T>::NAME) { return data(name).as<T>(); }

    template<typename T>
    T const & data(const char * name = Cpe::Dr::MetaTraits<T>::NAME) const { return data(name).as<T>(); }

    template<typename T>
    T & checkCreateData(size_t capacity = 0, LPDRMETA meta = Cpe::Dr::MetaTraits<T>::META) {
        return checkCreateData(meta, capacity).as<T>();
    }

    void dump_data(cfg_t cfg) const { logic_require_data_dump_to_cfg(*this, cfg); }

    void destory(void) { logic_require_free(*this); }
};

}}

#endif
