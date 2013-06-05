#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/share/set_repository.h"
#include "protocol/svr/set/set_share_chanel.h"

static set_chanel_t set_chanel_shm_init(int shmid, uint32_t w_capacity, uint32_t r_capacity, error_monitor_t em);

const char * set_repository_root(gd_app_context_t app, char * buf, size_t buf_capacity, error_monitor_t em) {
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, buf_capacity);
    const char * app_root = gd_app_root(app);
    const char * name_begin = NULL;
    const char * name_end = NULL;
    const char * home;
    size_t home_len;

    if (app_root) {
        const char * p;
        for(p = strchr(app_root, '/'); p; p = strchr(p + 1, '/')) {
            name_begin = name_end;
            name_end = p + 1;
        }
    }

    if (name_begin == NULL || name_end == NULL) {
        CPE_ERROR(em, "set_repository_root: root %s format error!", app_root);
        return NULL;
    }

    home = getenv("HOME");
    home_len = strlen(home);


    cpe_str_buf_append(&str_buf, home, home_len);
    if (home[home_len - 1] != '/') {
        cpe_str_buf_cat(&str_buf, "/");
    }
    cpe_str_buf_cat(&str_buf, ".");
    cpe_str_buf_append(&str_buf, name_begin, name_end - name_begin - 1);

    return cpe_str_buf_is_overflow(&str_buf) ? NULL : buf;
}

const char * set_repository_append_shm_key_file(
    gd_app_context_t app, const char * svr_type_name,
    uint16_t svr_id, char * buf, size_t capacity, error_monitor_t em)
{
    size_t data_len = strlen(buf); 

    snprintf(buf + data_len, capacity - data_len, "/svr_%s_%d.shm", svr_type_name, svr_id);

    return buf;
}

set_chanel_t
set_repository_chanel_open(gd_app_context_t app, const char * svr_type_name, uint16_t svr_id, error_monitor_t em) {
    char name_buf[256];
    set_chanel_t chanel = NULL;
    int shmid;
    cfg_t svr_cfg;
    const char * str_rq_size;
    uint64_t rq_size;
    const char * str_wq_size;
    uint64_t wq_size;

    if (set_repository_root(app, name_buf, sizeof(name_buf), em) == NULL) {
        CPE_ERROR(em, "set_repository_chanel_open: get repository_root fail!");
        return NULL;
    }

    if (!dir_exist(name_buf, em)) {
        if (dir_mk_recursion(name_buf, DIR_DEFAULT_MODE, em, gd_app_alloc(app)) != 0) {
            CPE_ERROR(em, "set_repository_chanel_open: create repository_root %s fail!", name_buf);
            return NULL;
        }
    }

    svr_cfg = cfg_find_cfg(gd_app_cfg(app), "svr_types");
    svr_cfg = cfg_find_cfg(svr_cfg, svr_type_name);
    if (svr_cfg == NULL) {
        CPE_ERROR(
            em, "set_repository_chanel_open: config of svr_type at %s not exist!",
            svr_type_name);
        return NULL;
    }

    if ((str_rq_size = cfg_get_string(svr_cfg, "rq-size", NULL)) == NULL) {
        CPE_ERROR(
            em, "set_repository_chanel_open: svr_type %s rq-size not configured!",
            svr_type_name);
        return NULL;
    }

    if (cpe_str_parse_byte_size(&rq_size, str_rq_size) != 0) {
        CPE_ERROR(
            em, "set_repository_chanel_open: svr_type %s rq-size %s format error!",
            svr_type_name, str_rq_size);
        return NULL;
    }

    if ((str_wq_size = cfg_get_string(svr_cfg, "wq-size", NULL)) == NULL) {
        CPE_ERROR(
            em, "set_repository_chanel_open: svr_type %s wq-size not configured!",
            svr_type_name);
        return NULL;
    }

    if (cpe_str_parse_byte_size(&wq_size, str_wq_size) != 0) {
        CPE_ERROR(
            em, "set_repository_chanel_open: svr_type %s wq-size %s format error!",
            svr_type_name, str_wq_size);
        return NULL;
    }

    if (set_repository_append_shm_key_file(app, svr_type_name, svr_id, name_buf, sizeof(name_buf), em) == NULL) {
        CPE_ERROR(em, "set_repository_chanel_open: get shm_key_file fail!");
        return NULL;
    }

    shmid = cpe_shm_key_gen(name_buf, 'a');
    if (shmid == -1) {
        CPE_ERROR(em, "set_repository_chanel_open: shm_key_gen at %s fail!", name_buf);
        return NULL;
    }

    chanel = set_chanel_shm_init(shmid, wq_size, rq_size, em);

    if (chanel) {
        CPE_ERROR(
            em, "set_repository_chanel_open: shm_key_file=%s, shm_id=%d, rq_size=%d, wq_size=%d",
            name_buf, shmid, (int)rq_size, (int)wq_size);
    }

    return chanel;
}

set_chanel_t set_chanel_shm_attach(int shmid, error_monitor_t em) {
    int h;
    SVR_SET_CHANEL * chanel;

    h = cpe_shm_get(shmid);
    if (h == -1) {
        CPE_ERROR(em, "set_chanel_shm_attach: get shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    chanel = shmat(h, NULL, 0);
    if (chanel == NULL) {
        CPE_ERROR(em, "set_chanel_shm_attach: attach shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    if (chanel->magic != 132523426) {
        CPE_ERROR(em, "set_chanel_shm_attach: magic mismatch!");
        shmdt(chanel);
        return NULL;
    }

    return (set_chanel_t)chanel;
}

set_chanel_t set_chanel_shm_init(int shmid, uint32_t w_capacity, uint32_t r_capacity, error_monitor_t em) {
    int h;
    int is_new;
    SVR_SET_CHANEL * chanel;
    uint32_t capacity = sizeof(SVR_SET_CHANEL) + w_capacity + r_capacity;

TRY_AGAIN:
    is_new = 0;

    h = cpe_shm_get(shmid);
    if (h == -1) {
        if (errno != ENOENT) {
            CPE_ERROR(em, "set_chanel_shm_init: get shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
            return NULL;
        }
        else {
            is_new = 1;
            h = cpe_shm_create(shmid, capacity, 0666);
            if (h == -1) {
                CPE_ERROR(em, "set_chanel_shm_init: init shm (id=%d, size=%d) fail, errno=%d (%s)\n", shmid, (int)capacity, errno, strerror(errno));
                return NULL;
            }
        }
    }

    chanel = shmat(h, NULL, 0);
    if (chanel == NULL) {
        CPE_ERROR(em, "set_chanel_shm_init: attach shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    if (is_new) {
        chanel->magic = 132523426;
        chanel->r.capacity = r_capacity;
        chanel->r.begin = sizeof(SVR_SET_CHANEL);
        chanel->r.rp = 0;
        chanel->r.wp = 0;

        chanel->w.capacity = w_capacity;
        chanel->w.begin = sizeof(SVR_SET_CHANEL) + r_capacity;
        chanel->w.rp = 0;
        chanel->w.wp = 0;
    }
    else {
        if (chanel->r.capacity != r_capacity || chanel->w.capacity != w_capacity) {
            CPE_ERROR(em, "set_chanel_shm_init: shm capacity mismatch, delete!\n");

            if (shmctl(h, IPC_RMID, NULL) == -1) {
                CPE_ERROR(em, "set_chanel_shm_init: remove shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
                return NULL;
            }

            goto TRY_AGAIN;
        }
    }

    return (set_chanel_t)chanel;
}

set_chanel_t
set_repository_chanel_attach(gd_app_context_t app, const char * svr_type_name, uint16_t svr_id, error_monitor_t em) {
    char name_buf[256];
    set_chanel_t chanel = NULL;
    int shmid;

    if (set_repository_root(app, name_buf, sizeof(name_buf), em) == NULL) {
        CPE_ERROR(em, "set_repository_chanel_attach: get repository_root fail!");
        return NULL;
    }

    if (!dir_exist(name_buf, em)) {
        CPE_ERROR(em, "set_repository_chanel_attach: create repository_root %s fail!", name_buf);
        return NULL;
    }

    if (set_repository_append_shm_key_file(app, svr_type_name, svr_id, name_buf, sizeof(name_buf), em) == NULL) {
        CPE_ERROR(em, "set_repository_chanel_attach: get shm_key_file fail!");
        return NULL;
    }

    shmid = cpe_shm_key_get(name_buf, 'a');
    if (shmid == -1) {
        CPE_ERROR(em, "set_repository_chanel_attach: shm_key_get at %s fail!", name_buf);
        return NULL;
    }

    chanel = set_chanel_shm_attach(shmid, em);
    if (chanel == NULL) {
        CPE_ERROR(em, "set_repository_chanel_attach: attach fail, shm_key_file=%s, shm_key_id=%d", name_buf, shmid);
        return NULL;
    }

    return chanel;
}

dir_visit_next_op_t set_repository_search_on_dir_enter(const char * full, const char * base, void * ctx) {
    return dir_visit_next_ignore;
}

dir_visit_next_op_t set_repository_search_on_dir_leave(const char * full, const char * base, void * ctx) {
    return dir_visit_next_ignore;
}

struct set_repository_search_ctx {
    void (*on_find_svr)(void * ctx, const char * svr_type, uint16_t svr_id);
    void * ctx;
};

dir_visit_next_op_t set_repository_search_on_file(const char * full, const char * base, void * input_ctx) {
    struct set_repository_search_ctx * ctx = input_ctx;
    char name_buf[128];
    const char * name_begin;
    const char * name_end;
    const char * buf;

    if (strstr(base, "svr_") != base) return dir_visit_next_go;

    name_begin = base + 4;
    name_end = name_begin;
    while((buf = strchr(name_end + 1, '_'))) {
        name_end = buf;
    }

    memcpy(name_buf, name_begin, name_end - name_begin);
    name_buf[name_end - name_begin] = 0;

    ctx->on_find_svr(ctx->ctx, name_buf, atoi(name_end + 1));

    return dir_visit_next_go;
}

int set_repository_search(
    gd_app_context_t app,
    void (*on_find_svr)(void * ctx, const char * svr_type, uint16_t svr_id), void * input_ctx,
    error_monitor_t em)
{
    struct dir_visitor visitor;
    struct set_repository_search_ctx ctx;
    char name_buf[256];

    if (set_repository_root(app, name_buf, sizeof(name_buf), em) == NULL) {
        CPE_ERROR(em, "set_repository_search: get repository_root fail!");
        return -1;
    }

    ctx.on_find_svr = on_find_svr;
    ctx.ctx = input_ctx;

    visitor.on_dir_enter = set_repository_search_on_dir_enter;
    visitor.on_dir_leave = set_repository_search_on_dir_leave;
    visitor.on_file = set_repository_search_on_file;

    dir_search(&visitor, &ctx, name_buf, 1, em, gd_app_alloc(app));
    
    return 0;
}

void set_repository_chanel_detach(set_chanel_t chanel, error_monitor_t em) {
    if (shmdt(chanel) != 0) {
        CPE_ERROR(em, "set_repository_chanel_detach: shmdt fail, errno=%d (%s)\n", errno, strerror(errno));
    }
}
