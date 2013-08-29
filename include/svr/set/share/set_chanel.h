#ifndef SVR_SET_CHANEL_H
#define SVR_SET_CHANEL_H
#include "cpe/utils/buffer.h"
#include "set_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int set_chanel_r_write(set_chanel_t chanel, dp_req_t body, size_t * size);
int set_chanel_r_peak(set_chanel_t chanel, dp_req_t body);
int set_chanel_r_erase(set_chanel_t chanel);

int set_chanel_w_write(set_chanel_t chanel, dp_req_t body, size_t * size);
int set_chanel_w_peak(set_chanel_t chanel, dp_req_t body);
int set_chanel_w_erase(set_chanel_t chanel);

const char * set_chanel_dump(set_chanel_t chanel, mem_buffer_t buffer);
const char * set_chanel_str_error(int err);

#ifdef __cplusplus
}
#endif

#endif
