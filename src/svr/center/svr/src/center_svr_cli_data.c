#include <assert.h> 
#include "center_svr_ops.h"

center_cli_data_t
center_cli_data_create(center_svr_t svr, SVR_CENTER_CLI_RECORD * record) {
    center_cli_data_t data;
    data = mem_alloc(svr->m_alloc, sizeof(struct center_cli_data));
    if (data == NULL) {
        CPE_ERROR(svr->m_em, "%s: create data: malloc fail!", center_svr_name(svr));
        return NULL;
    }

    data->m_svr = svr;
    data->m_conn = NULL;
    data->m_data = record;

    cpe_hash_entry_init(&data->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_datas, data) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create data: insert fail, %d.%d already exist!",
            center_svr_name(svr), record->svr_type, record->svr_id);
        mem_free(svr->m_alloc, data);
        return NULL;
    }
    
    data->m_group = center_cli_group_get_or_create(svr, record->svr_type);
    if (data->m_group == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: create data: get_or_create group of %d fail!",
            center_svr_name(svr), record->svr_type);
        cpe_hash_table_remove_by_ins(&svr->m_datas, data);
        mem_free(svr->m_alloc, data);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&data->m_group->m_datas, data, m_next);
    ++data->m_group->m_svr_count;

    return data;
}

void center_cli_data_free(center_cli_data_t cli) {
    center_svr_t svr = cli->m_svr;

    assert(svr);

    /*clear data*/
    assert(cli->m_data);
    cli->m_data = NULL;

    /*disconnect with connection */
    if (cli->m_conn) {
        cli->m_conn->m_data = NULL;
        cli->m_conn = NULL;
    }

    /*remove from group*/
    assert(cli->m_group);
    --cli->m_group->m_svr_count;
    TAILQ_REMOVE(&cli->m_group->m_datas, cli, m_next);
    if (TAILQ_EMPTY(&cli->m_group->m_datas)) {
        center_cli_group_free(cli->m_group);
    }
    cli->m_group = NULL;

    /*remove from svr*/
    cpe_hash_table_remove_by_ins(&svr->m_datas, cli);

    mem_free(svr->m_alloc, cli);
}

void center_cli_data_free_all(center_svr_t svr) {
    struct cpe_hash_it data_it;
    center_cli_data_t data;

    cpe_hash_it_init(&data_it, &svr->m_datas);

    data = cpe_hash_it_next(&data_it);
    while(data) {
        center_cli_data_t next = cpe_hash_it_next(&data_it);
        center_cli_data_free(data);
        data = next;
    }
}

center_cli_data_t
center_cli_data_find(center_svr_t svr, uint16_t svr_type, uint16_t svr_id) {
    SVR_CENTER_CLI_RECORD key_data;
    struct center_cli_data key;

    key.m_data = &key_data;
    key_data.svr_type = svr_type;
    key_data.svr_id = svr_id;

    return cpe_hash_table_find(&svr->m_datas, &key);
}

uint32_t center_cli_data_hash(center_cli_data_t cli) {
    return ((uint32_t)cli->m_data->svr_type) << 16
        | cli->m_data->svr_id;
}

int center_cli_data_eq(center_cli_data_t l, center_cli_data_t r) {
    return l->m_data->svr_type == r->m_data->svr_type
        && l->m_data->svr_id == r->m_data->svr_id;
}
