#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "set_svr_mon_ops.h"

set_svr_mon_app_t
set_svr_mon_app_create(set_svr_mon_t mon, set_svr_svr_type_t svr_type, const char * bin, const char * pidfile) {
    set_svr_t svr = mon->m_svr;
    set_svr_mon_app_t mon_app;

    mon_app = mem_alloc(svr->m_alloc, sizeof(struct set_svr_mon_app));
    if (mon_app == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon_app: alloc fail!", set_svr_name(svr));
        return NULL;
    }

    mon_app->m_svr_type = svr_type;

    mon_app->m_bin = cpe_str_mem_dup(svr->m_alloc, bin);
    if (mon_app->m_bin == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon_app: alloc bin fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, mon_app);
        return NULL;
    }

    mon_app->m_pidfile = cpe_str_mem_dup(svr->m_alloc, pidfile);
    if (mon_app->m_pidfile == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon_app: alloc bin fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, mon_app->m_bin);
        mem_free(svr->m_alloc, mon_app);
        return NULL;
    }

    mon_app->m_pid = 0;
    mon_app->m_fsm_timer_id = GD_TIMER_ID_INVALID;
    mon_app->m_mon = mon;
    mon_app->m_args = NULL;
    mon_app->m_arg_count = 0;
    mon_app->m_arg_capacity = 0;

    if (fsm_machine_init(&mon_app->m_fsm, mon->m_fsm_def, "disable", mon_app, svr->m_debug) != 0) {
        CPE_ERROR(svr->m_em, "%s: mon_app: init fsm fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, mon_app->m_bin);
        mem_free(svr->m_alloc, mon_app->m_pidfile);
        mem_free(svr->m_alloc, mon_app);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&mon->m_mon_apps, mon_app, m_next);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: mon app %s: create", set_svr_name(svr), mon_app->m_bin);
    }

    return mon_app;
}

void set_svr_mon_app_free(set_svr_mon_app_t mon_app) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: mon app %s: destory", set_svr_name(svr), mon_app->m_bin);
    }

    fsm_machine_fini(&mon_app->m_fsm);
    assert(mon_app->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    TAILQ_REMOVE(&mon->m_mon_apps, mon_app, m_next);

    if (mon_app->m_args) {
        size_t i;
        for(i = 0; i < mon_app->m_arg_count; ++i) {
            mem_free(svr->m_alloc, mon_app->m_args[i]);
        }
        mem_free(svr->m_alloc, mon_app->m_args);
        mon_app->m_args = NULL;
    }

    mem_free(svr->m_alloc, mon_app->m_bin);
    mem_free(svr->m_alloc, mon_app->m_pidfile);
    mem_free(svr->m_alloc, mon_app);
}

void set_svr_mon_app_start_all(set_svr_mon_t mon) {
    set_svr_mon_app_t mon_app;

    TAILQ_FOREACH(mon_app, &mon->m_mon_apps, m_next) {
        set_svr_mon_app_apply_evt(mon_app, set_svr_mon_app_fsm_evt_enable);
    }
}

set_svr_mon_app_t set_svr_mon_app_find_by_pid(set_svr_mon_t mon, int pid) {
    set_svr_mon_app_t mon_app;

    TAILQ_FOREACH(mon_app, &mon->m_mon_apps, m_next) {
        if (mon_app->m_pid == pid) return mon_app;
    }

    return NULL;
}

int set_svr_mon_app_add_arg(set_svr_mon_app_t mon_app, const char * arg) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;

    if ((mon_app->m_arg_count + 1) > mon_app->m_arg_capacity) {
        char ** new_args;
        size_t new_arg_capacity;

        if (mon_app->m_arg_capacity == 0) {
            new_arg_capacity = 16;
        }
        else {
            new_arg_capacity = mon_app->m_arg_capacity * 2;
        }

        new_args = mem_alloc(svr->m_alloc, sizeof(char*) * new_arg_capacity);
        if (new_args == NULL) {
            CPE_ERROR(svr->m_em, "%s: mon app %s: alloc fail", set_svr_name(svr), mon_app->m_bin);
            return -1;
        }

        memcpy(new_args, mon_app->m_args, sizeof(char*) * mon_app->m_arg_count);

        mem_free(svr->m_alloc, mon_app->m_args);

        mon_app->m_args = new_args;
        mon_app->m_arg_capacity = new_arg_capacity;
    }

    mon_app->m_args[mon_app->m_arg_count] = cpe_str_mem_dup(svr->m_alloc, arg);
    mon_app->m_arg_count++;
    mon_app->m_args[mon_app->m_arg_count] = NULL;

    return 0;
}

void set_svr_mon_app_apply_evt(struct set_svr_mon_app * mon_app, enum set_svr_mon_app_fsm_evt_type type) {
    struct set_svr_mon_app_fsm_evt evt;
    evt.m_type = type;
    fsm_machine_apply_event(&mon_app->m_fsm, &evt);
}

void set_svr_mon_app_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    struct set_svr_mon_app * mon_app = ctx;
    assert(mon_app->m_fsm_timer_id == timer_id);
    set_svr_mon_app_apply_evt(mon_app, set_svr_mon_app_fsm_evt_timeout);
}

int set_svr_mon_app_start_state_timer(struct set_svr_mon_app * mon_app, tl_time_span_t span) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            svr->m_em,
            "%s: mon app %s: start state timer: get default timer manager fail",
            set_svr_name(svr), mon_app->m_bin);
        return -1;
    }

    assert(mon_app->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &mon_app->m_fsm_timer_id, set_svr_mon_app_state_timeout, mon_app, NULL, NULL, span, span, -1) != 0) {
        assert(mon_app->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(
            svr->m_em,
            "%s: mon app %s: start state timer: regist timer fail",
            set_svr_name(svr), mon_app->m_bin);
        return -1;
    }

    assert(mon_app->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void set_svr_mon_app_stop_state_timer(struct set_svr_mon_app * mon_app) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            svr->m_em,
            "%s: mon app %s: stop state timer: get default timer manager fail",
            set_svr_name(svr), mon_app->m_bin);
        return;
    }

    assert(mon_app->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, mon_app->m_fsm_timer_id);
    mon_app->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}


static void set_svr_mon_app_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct set_svr_mon_app_fsm_evt * evt = input_event;
    switch(evt->m_type) {
    case set_svr_mon_app_fsm_evt_enable:
        stream_printf(s, "enable");
        break;
    case set_svr_mon_app_fsm_evt_disable:
        stream_printf(s, "disable");
        break;
    case set_svr_mon_app_fsm_evt_start:
        stream_printf(s, "start");
        break;
    case set_svr_mon_app_fsm_evt_stoped:
        stream_printf(s, "stoped");
        break;
    case set_svr_mon_app_fsm_evt_timeout:
        stream_printf(s, "timeout");
        break;
    }
}

fsm_def_machine_t
set_svr_mon_app_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    fsm_def_machine_t fsm_def = fsm_def_machine_create(name, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "set_svr_create_fsm_def: create fsm def fail!");
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, set_svr_mon_app_dump_event);

    if (set_svr_mon_app_fsm_create_disable(fsm_def, em) != 0
        || set_svr_mon_app_fsm_create_checking(fsm_def, em) != 0
        || set_svr_mon_app_fsm_create_runing(fsm_def, em) != 0
        || set_svr_mon_app_fsm_create_waiting(fsm_def, em) != 0)
    {
        CPE_ERROR(em, "set_svr_mon_app_create_fsm_def: init fsm fail!");
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

enum set_svr_mon_app_get_pid_result set_svr_mon_app_get_pid(set_svr_mon_app_t mon_app, int * pid) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    int fd;  
    struct flock fk;  

    /* 打开放置记录锁的文件 */
    fd = open(mon_app->m_pidfile, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  
    if (fd < 0) {  
        if (errno != ENOENT) {
            CPE_ERROR(
                svr->m_em, "%s: mon app %s: open pid file %s fail, error=%d (%s)",
                set_svr_name(svr), mon_app->m_bin,
                mon_app->m_pidfile, errno, strerror(errno));
            return set_svr_mon_app_get_pid_error;
        }
        else {
            return set_svr_mon_app_get_pid_not_runing;
        }
    }  

    /*试图对文件fd加锁，*/
    fk.l_type = F_WRLCK;  
    fk.l_start = 0;  
    fk.l_whence = SEEK_SET;  
    fk.l_len = 0;

    if (fcntl(fd, F_SETLK, &fk) < 0) {      /*如果加锁失败的话 */
        if (EACCES == errno || EAGAIN == errno) { /*正在运行*/
            char buf[64];
            size_t pos = 0;

            while(pos < sizeof(buf)) {
                int r = read(fd, buf + pos, sizeof(buf) - pos);
                pos += r;

                if (r == 0) {
                    if (sizeof(buf) == pos) {
                        CPE_ERROR(
                            svr->m_em, "%s: mon app %s: read pid fail, full",
                            set_svr_name(svr), mon_app->m_bin);
                        close(fd);
                        return set_svr_mon_app_get_pid_error;
                    }
                    else {
                        buf[pos] = 0;
                        break;
                    }
                }
            }

            sscanf(buf, "%d", pid);

            close(fd);
            return set_svr_mon_app_get_pid_ok;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: mon app %s: read pid fcntl fail, error=%d (%s)",
                set_svr_name(svr), mon_app->m_bin, errno, strerror(errno));

            close(fd);
            return set_svr_mon_app_get_pid_error;
        }
    }
    else {
        close(fd);
        return set_svr_mon_app_get_pid_not_runing;
    }
}

int set_svr_mon_app_kill(set_svr_mon_app_t mon_app, int sig) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;

    assert(mon_app->m_pid != 0);
    if (kill(mon_app->m_pid, SIGKILL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: mon app %s: kill %d to %d fail, error=%d (%s)",
            set_svr_name(svr), mon_app->m_bin, mon_app->m_pid,
            sig, errno, strerror(errno));
        return -1;
    }
    else {
        CPE_INFO(
            svr->m_em, "%s: mon app %s: kill %d to %d",
            set_svr_name(svr), mon_app->m_bin, mon_app->m_pid, sig);
        return 0;
    }
}