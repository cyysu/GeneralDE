#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "usf/bpg_rsp/bpg_rsp_addition.h"
#include "protocol/bpg_rsp/bpg_rsp_addition.h"

int16_t bpg_rsp_addition_data_count(logic_context_t ctx) {
    logic_data_t data;
    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    return data == NULL
        ? 0
        : ((BPG_RSP_ADDITION_DATA *)logic_data_data(data))->count;
}

uint32_t bpg_rsp_addition_data_at(logic_context_t ctx, int16_t pos) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == NULL || addition_data->count <= pos) return 0;

    return addition_data->pieces[pos];
}

#define BPG_RSP_ADDITION_DATA_ONCE_SIZE (16)
extern char g_metalib_carry_package[];

static BPG_RSP_ADDITION_DATA *
bpg_rsp_addition_data_create(logic_context_t ctx, size_t capacity) {
    LPDRMETA meta;
    BPG_RSP_ADDITION_DATA * addition_data;
    logic_data_t data;

    meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_carry_package, "bpg_rsp_addition_data");
    if (meta == NULL) return NULL;

    data = logic_context_data_get_or_create(ctx, meta, sizeof(BPG_RSP_ADDITION_DATA) + (capacity - 1) * sizeof(uint32_t));

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data) addition_data->capacity = capacity;
    return addition_data;
}

int bpg_rsp_addition_data_add(logic_context_t ctx, uint32_t meta_id) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;
    int i;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == NULL) {
        addition_data = 
            bpg_rsp_addition_data_create(ctx, BPG_RSP_ADDITION_DATA_ONCE_SIZE - 1);
        if (addition_data == NULL) return -1;
    }

    for(i = 0; i < addition_data->count; ++i) {
        if (addition_data->pieces[i] == meta_id) return 0;

        if (addition_data->pieces[i] > meta_id) {
            if (addition_data->count + 1 >= addition_data->capacity) {
                addition_data = 
                    bpg_rsp_addition_data_create(ctx, addition_data->capacity + BPG_RSP_ADDITION_DATA_ONCE_SIZE);
                if (addition_data == NULL) return -1;
            }

            addition_data->count++;

            memmove(
                addition_data->pieces + i + 1,
                addition_data->pieces + i,
                sizeof(addition_data->pieces[0]) * (addition_data->count - i));

            addition_data->pieces[i] = meta_id;

            return 0;
        }
    }

    if (addition_data->count + 1 >= addition_data->capacity) {
        addition_data = 
            bpg_rsp_addition_data_create(ctx, addition_data->capacity + BPG_RSP_ADDITION_DATA_ONCE_SIZE);
        if (addition_data == NULL) return -1;
    }

    addition_data->pieces[addition_data->count++] = meta_id;
    return 0;
}


static int bpg_rsp_addition_data_cmp(const void * l, const void * r) {
    const uint32_t * l_v = (const uint32_t *)l;
    const uint32_t * r_v = (const uint32_t *)r;

    return *l_v == *r_v ? 0
        : *l_v < *r_v ? -1
        : 1;
}

int bpg_rsp_addition_data_exist(logic_context_t ctx, uint32_t meta_id) {
    logic_data_t data;
    BPG_RSP_ADDITION_DATA * addition_data;

    data = logic_context_data_find(ctx, "bpg_rsp_addition_data");

    addition_data = data == NULL
        ? NULL
        : (BPG_RSP_ADDITION_DATA *)logic_data_data(data);

    if (addition_data == 0) return 0;

    return bsearch(
        &meta_id,
        addition_data->pieces,
        addition_data->count,
        sizeof(addition_data->pieces[0]),
        bpg_rsp_addition_data_cmp)
        ? 1
        : 0;
}
