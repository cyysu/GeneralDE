#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "protocol/base/base_package.h"
#include "bpg_pkg_internal_types.h"

int bpg_pkg_set_main_data(bpg_pkg_t pkg, void const * buf, size_t size, error_monitor_t em) {
    BASEPKG_HEAD * head;
    size_t cur_size;
    size_t remain_size;

    assert(pkg);
    assert(buf);

    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);

    if (head->appendInfoCount > 0) {
        CPE_ERROR(em, "bpg_pkg_set_data: already have append info!");
        return -1;
    }

    cur_size = sizeof(BASEPKG_HEAD);
    remain_size = bpg_pkg_pkg_data_capacity(pkg) - cur_size;
    if (remain_size < size) {
        CPE_ERROR(
            em, "bpg_pkg_set_data: not enough buf! buf-size=%d, input-size=%d",
            (int)remain_size, (int)size);
        return -1;
    }

    memcpy(bpg_pkg_body_data(pkg), buf, size);

    bpg_pkg_pkg_data_set_size(pkg, cur_size + size);

    head->bodylen = size;
    head->bodytotallen = size;

    return 0;
}

void * bpg_pkg_main_data(bpg_pkg_t pkg) {
    return bpg_pkg_body_data(pkg);
}

size_t bpg_pkg_main_data_len(bpg_pkg_t pkg) {
    BASEPKG_HEAD * head;

    head = (BASEPKG_HEAD *)bpg_pkg_pkg_data(pkg);

    return head->bodylen;
}

int bpg_pkg_add_append_data(bpg_pkg_t pkg, LPDRMETA meta, const void * buf, size_t size, error_monitor_t em) {
    BASEPKG * basepkg;
    APPENDINFO * appendInfo;
    size_t cur_size;
    size_t remain_size;

    assert(pkg);

    basepkg = (BASEPKG *)bpg_pkg_pkg_data(pkg);

    if (basepkg->head.appendInfoCount >= APPEND_INFO_MAX_COUNT) {
        CPE_ERROR(em, "bpg_pkg_add_append_data: max append info reached!");
        return -1;
    }

    cur_size = bpg_pkg_pkg_data_size(pkg);
    remain_size = bpg_pkg_pkg_data_capacity(pkg) - cur_size;
    if (remain_size < size) {
        CPE_ERROR(
            em, "bpg_pkg_add_append_data: not enough buf! buf-size=%d, input-size=%d",
            (int)remain_size, (int)size);
        return -1;
    }

    memcpy(((char *)bpg_pkg_pkg_data(pkg)) + cur_size, buf, size);

    appendInfo = &basepkg->head.appendInfos[basepkg->head.appendInfoCount++];
    appendInfo->id = dr_meta_id(meta);
    appendInfo->size = size;

    bpg_pkg_pkg_data_set_size(pkg, cur_size + size);

    basepkg->head.bodytotallen += size;

    return 0;
}

void * bpg_pkg_append_data(bpg_pkg_t pkg, bpg_pkg_append_info_t append_info) {
    int pos;
    BASEPKG * basepkg;
    char * buf;
    int i;

    basepkg = (BASEPKG *)bpg_pkg_pkg_data(pkg);

    pos = (((APPENDINFO *)append_info) - basepkg->head.appendInfos);

    if (pos < 0 || pos > basepkg->head.appendInfoCount) return NULL;

    buf = (char*)basepkg->body;
    buf += basepkg->head.bodylen;

    for(i = 0; i < pos; ++i) {
        buf += basepkg->head.appendInfos[i].size;
    }

    return buf;
}
