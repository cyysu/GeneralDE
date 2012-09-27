#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "protocol/base/base_package.h"
#include "bpg_pkg_internal_types.h"

dr_cvt_result_t
bpg_pkg_encode(
    bpg_pkg_t pkg,
    void * output, size_t * output_capacity,
    error_monitor_t em, int debug)
{
    dr_cvt_result_t r;
    size_t pkg_size;

    pkg_size = bpg_pkg_pkg_data_size(pkg);

    r  =
        dr_cvt_encode(
            bpg_pkg_base_cvt(pkg),
            bpg_pkg_base_meta(pkg),
            output, output_capacity,
            bpg_pkg_pkg_data(pkg), &pkg_size,
            em, debug);

    return r;
}

dr_cvt_result_t
bpg_pkg_decode(
    bpg_pkg_t pkg,
    const void * input, size_t * input_capacity,
    error_monitor_t em, int debug)
{
    dr_cvt_result_t r;
    size_t output_size;

    bpg_pkg_init(pkg);

    output_size = bpg_pkg_pkg_data_capacity(pkg);

    r =  dr_cvt_decode(
        bpg_pkg_base_cvt(pkg), bpg_pkg_base_meta(pkg),
        bpg_pkg_pkg_data(pkg), &output_size,
        input, input_capacity, 
        em, debug);

    if (r == dr_cvt_result_success) {
        if (bpg_pkg_pkg_data_set_size(pkg, output_size) != 0) {
            r = dr_cvt_result_error;
        }
    }

    return r;
}
