#ifndef USFPP_LOGIC_USE_UNI_RES_H
#define USFPP_LOGIC_USE_UNI_RES_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic_use/logic_uni_res.h"
#include "LogicOpDynData.hpp"

namespace Usf { namespace Logic {

class LogicUniRes : public Cpe::Utils::Noncopyable {
public:
    static bool find_at(LogicOpDynData & data, logic_require_t require) { data = logic_uni_res_data(require); return data.isValid(); }
    static LogicOpDynData get_at(logic_require_t require);
    static void init_at(logic_require_t require, LPDRMETA meta, size_t record_capacity = 0);
    static void fini_at(logic_require_t require) { logic_uni_res_fini(require); }
};

}}

#endif
