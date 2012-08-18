#ifndef USF_LOGIC_USE_UNI_RESULT_H
#define USF_LOGIC_USE_UNI_RESULT_H
#include "cpe/utils/buffer.h"
#include "usf/logic/logic_types.h"
#include "logic_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int logic_uni_res_init(logic_require_t require, LPDRMETA meta, size_t record_capacity);
void logic_uni_res_fini(logic_require_t require);

logic_data_t logic_uni_res_data(logic_require_t require);

#ifdef __cplusplus
}
#endif

#endif
