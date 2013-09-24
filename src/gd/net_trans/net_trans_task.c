#include <assert.h>
#include "cpe/pal/pal_stdio.h" 
#include "gd/net_trans/net_trans_manage.h"
#include "gd/net_trans/net_trans_task.h"
#include "gd/net_trans/net_trans_detail.h"
#include "net_trans_internal_ops.h"

static size_t net_trans_task_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata);

net_trans_task_t net_trans_task_create(net_trans_group_t group, size_t capacity) {
    net_trans_manage_t mgr = group->m_mgr;
    net_trans_task_t task;

    task = mem_alloc(mgr->m_alloc, sizeof(struct net_trans_task) + capacity);
    if (task == NULL) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create task: alloc fail!", net_trans_manage_name(mgr), group->m_name);
        return NULL;
    }

    task->m_id = ++mgr->m_max_id;
    task->m_group = group;
    task->m_capacity = capacity;
    task->m_is_free = 0;
    task->m_in_callback = 0;
    task->m_sockfd = -1;
    task->m_evset = 0;
    task->m_state = net_trans_task_init;
    task->m_result = net_trans_result_unknown;
    task->m_watch.data = task;
    task->m_commit_op = NULL;
    task->m_commit_ctx = NULL;

    task->m_handler = curl_easy_init();
    if (task->m_handler == NULL) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create task: curl_easy_init fail!", net_trans_manage_name(mgr), group->m_name);
        mem_free(mgr->m_alloc, task);
        return NULL;
    }
    curl_easy_setopt(task->m_handler, CURLOPT_PRIVATE, task);

	curl_easy_setopt(task->m_handler, CURLOPT_DNS_CACHE_TIMEOUT, mgr->m_cfg_dns_cache_timeout);
	curl_easy_setopt(task->m_handler, CURLOPT_CONNECTTIMEOUT_MS, mgr->m_cfg_connect_timeout_ms);
    curl_easy_setopt(task->m_handler, CURLOPT_TIMEOUT_MS, mgr->m_cfg_transfer_timeout_ms);
	curl_easy_setopt(task->m_handler, CURLOPT_FORBID_REUSE, mgr->m_cfg_forbid_reuse);

    if (mgr->m_debug >= 2) {
        curl_easy_setopt(task->m_handler, CURLOPT_STDERR, stderr );
        curl_easy_setopt(task->m_handler, CURLOPT_VERBOSE, 1L);
    }
    else {
        curl_easy_setopt(task->m_handler, CURLOPT_VERBOSE, 0L);
    }
	curl_easy_setopt(task->m_handler, CURLOPT_WRITEFUNCTION, net_trans_task_write_cb);
	curl_easy_setopt(task->m_handler, CURLOPT_WRITEDATA, task);

    cpe_hash_entry_init(&task->m_hh_for_mgr);
    if (cpe_hash_table_insert_unique(&mgr->m_tasks, task) == 0) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create task: task id duplicate!", net_trans_manage_name(mgr), group->m_name);
        curl_easy_cleanup(task->m_handler);
        mem_free(mgr->m_alloc, task);
        return NULL;
    }

    mem_buffer_init(&task->m_buffer, mgr->m_alloc);

    bzero(task + 1, capacity);

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: task %d (%s): create!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
    }

    return task;
}

void net_trans_task_free(net_trans_task_t task) {
    net_trans_group_t group = task->m_group;
    net_trans_manage_t mgr = group->m_mgr;

    if (task->m_state != net_trans_task_done) {
        task->m_is_free = 1;
        net_trans_task_set_done(task, net_trans_result_cancel);
        return;
    }

    if (task->m_in_callback) {
        task->m_is_free = 1;
        return;
    }

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: task %d (%s): free!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
    }

    TAILQ_REMOVE(&group->m_tasks, task, m_next_for_group);
    cpe_hash_table_remove_by_ins(&mgr->m_tasks, task);

    if (task->m_evset) {
        ev_io_stop(mgr->m_loop, &task->m_watch);
        task->m_evset = 0;
    }

    task->m_sockfd = -1;

    mem_buffer_clear(&task->m_buffer);

    mem_free(mgr->m_alloc, task);
}

int net_trans_task_start(net_trans_task_t task) {
    net_trans_manage_t mgr = task->m_group->m_mgr;
    int rc;

    if (task->m_state != net_trans_task_init) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t start in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_state_str(task->m_state));
        return -1;
    }

    rc = curl_multi_add_handle(mgr->m_multi_handle, task->m_handler);
    if (rc != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): curl_multi_add_handle error, rc=%d",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, rc);
        return -1;
    }
    task->m_state = net_trans_task_working;

    if (mgr->m_debug) {
        CPE_INFO(mgr->m_em, "%s: task %d (%s): start!", net_trans_manage_name(mgr), task->m_id, task->m_group->m_name);
    }

    return 0;
}

net_trans_task_state_t net_trans_task_state(net_trans_task_t task) {
    return task->m_state;
}

net_trans_task_result_t net_trans_task_result(net_trans_task_t task) {
    if (task->m_state != net_trans_task_done) return net_trans_result_unknown;

    return task->m_result;
}

void * net_trans_task_data(net_trans_task_t task) {
    return task + 1;
}

size_t net_trans_task_data_capacity(net_trans_task_t task) {
    return task->m_capacity;
}

mem_buffer_t net_trans_task_buffer(net_trans_task_t task) {
    return &task->m_buffer;
}

void net_trans_task_set_commit_op(net_trans_task_t task, net_trans_task_commit_op_t op, void * ctx) {
    task->m_commit_op = op;
    task->m_commit_ctx = ctx;
}

int net_trans_task_set_post_to(net_trans_task_t task, const char * uri, const char * data, int data_len) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (task->m_state != net_trans_task_init) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t set post %d data to %s in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            data_len, uri, net_trans_task_state_str(task->m_state));
        return -1;
    }

    if (curl_easy_setopt(task->m_handler, CURLOPT_POST, 1L) != CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_POSTFIELDSIZE, data_len) != CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_COPYPOSTFIELDS, data) != CURLM_OK
        || curl_easy_setopt(task->m_handler, CURLOPT_URL, uri) != CURLM_OK)
    {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): set post %d data to %s fail!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, data_len, uri);
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): set post %d data to %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, data_len, uri);
    }

    return 0;
}

int net_trans_task_set_done(net_trans_task_t task, net_trans_task_result_t result) {
    net_trans_manage_t mgr = task->m_group->m_mgr;

    if (task->m_state != net_trans_task_working) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): can`t done in state %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_state_str(task->m_state));
        return -1;
    }

    if (task->m_state == net_trans_task_working) {
        curl_multi_remove_handle(mgr->m_multi_handle, task->m_handler);
    }

    task->m_result = result;
    task->m_state = net_trans_task_done;

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: task %d (%s): done, result is %s!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name,
            net_trans_task_result_str(task->m_result));
    }

    if (task->m_commit_op) {
        task->m_in_callback = 1;
        task->m_commit_op(task, task->m_commit_ctx);
        task->m_in_callback = 0;

        if (task->m_is_free || task->m_state != net_trans_task_done) {
            task->m_is_free = 0;
            net_trans_task_free(task);
        }
    }
    else {
        net_trans_task_free(task);
    }

    return 0;
}

const char * net_trans_task_state_str(net_trans_task_state_t state) {
    switch(state) {
    case net_trans_task_init:
        return "init";
    case net_trans_task_working:
        return "working";
    case net_trans_task_done:
        return "done";
    default:
        return "unknown";
    }
}

const char * net_trans_task_result_str(net_trans_task_result_t result) {
    switch(result) {
    case net_trans_result_unknown:
        return "unknown";
    case net_trans_result_ok:
        return "ok";
    case net_trans_result_timeout:
        return "timeout";
    case net_trans_result_cancel:
        return "cancel";
    default:
        return "!!bad task result!!";
    }
}

static size_t net_trans_task_write_cb(char *ptr, size_t size, size_t nmemb, void * userdata) {
	net_trans_task_t task = userdata;
    net_trans_manage_t mgr = task->m_group->m_mgr;
	int total_length = size * nmemb;
    ssize_t write_size;

    write_size = mem_buffer_append(&task->m_buffer, ptr, total_length);
    if (write_size != (ssize_t)total_length) {
        CPE_ERROR(
            mgr->m_em, "%s: task %d (%s): append %d data fail, return %d!",
            net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)total_length, (int)write_size);
    }
    else {
        if (mgr->m_debug) {
            CPE_INFO(
                mgr->m_em, "%s: task %d (%s): receive %d data!",
                net_trans_manage_name(mgr), task->m_id, task->m_group->m_name, (int)total_length);
        }
    }

    return total_length;
}

uint32_t net_trans_task_hash(net_trans_task_t task) {
    return task->m_id;
}

int net_trans_task_eq(net_trans_task_t l, net_trans_task_t r) {
    return l->m_id == r->m_id;
}

