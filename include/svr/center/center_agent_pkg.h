#ifndef SVR_CENTER_AGENT_PKG_H
#define SVR_CENTER_AGENT_PKG_H
#include "cpe/utils/hash_string.h"
#include "center_agent_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cpe_hash_string_t center_agent_pkg_type_name;

center_agent_pkg_t
center_agent_pkg_create(center_agent_t agent, size_t capacity);
void center_agent_pkg_free(center_agent_pkg_t pkg);

size_t center_agent_pkg_capacity(center_agent_pkg_t pkg);
void * center_agent_pkg_data(center_agent_pkg_t pkg);
size_t center_agent_pkg_size(center_agent_pkg_t pkg);
int center_agent_pkg_set_size(center_agent_pkg_t pkg, size_t size);

dp_req_t center_agent_pkg_to_dp_req(center_agent_pkg_t pkg);
center_agent_pkg_t center_agent_pkg_from_dp_req(dp_req_t pkg);

#ifdef __cplusplus
}
#endif

#endif
