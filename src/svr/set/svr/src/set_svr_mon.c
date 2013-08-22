#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "cpe/pal/pal_external.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "set_svr_mon_ops.h"

static void set_svr_mon_sig_child_handler(int sig);
set_svr_mon_t g_set_svr_mon = NULL;

set_svr_mon_t set_svr_mon_create(set_svr_t svr) {
    set_svr_mon_t mon;

    assert(g_set_svr_mon == NULL);

    mon = mem_alloc(svr->m_alloc, sizeof(struct set_svr_mon));
    if (mon == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon: alloc fail!", set_svr_name(svr));
        return NULL;
    }

    mon->m_svr = svr;
    mon->m_restart_wait_ms = 0;

    mon->m_fsm_def = set_svr_mon_app_create_fsm_def(set_svr_name(svr), svr->m_alloc, svr->m_em);
    if (mon->m_fsm_def == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon: create fsm def fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, mon);
        return NULL;
    }

    TAILQ_INIT(&mon->m_mon_apps);

    g_set_svr_mon = mon;
    signal(SIGCHLD, set_svr_mon_sig_child_handler);

    return mon;
}

void set_svr_mon_free(set_svr_mon_t mon) {
    set_svr_t svr = mon->m_svr;

    assert(g_set_svr_mon == mon);
    g_set_svr_mon = NULL;

    signal(SIGCHLD, NULL);

    while(!TAILQ_EMPTY(&mon->m_mon_apps)) {
        set_svr_mon_app_free(TAILQ_FIRST(&mon->m_mon_apps));
    }

    fsm_def_machine_free(mon->m_fsm_def);
    mon->m_fsm_def = NULL;
    
    mem_free(svr->m_alloc, mon);
}

#ifndef WCONTINUED
# define WCONTINUED 0
#endif

static void set_svr_mon_sig_child_handler(int sig) {
    int pid, status;
    set_svr_t svr;
    set_svr_mon_app_t mon_app;

    assert(g_set_svr_mon);
    svr = g_set_svr_mon->m_svr;

    /* some systems define WCONTINUED but then fail to support it (linux 2.4) */
    if (0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED | WCONTINUED))) {
        if (!WCONTINUED
            || errno != EINVAL
            || 0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED)))
        {
            CPE_ERROR(svr->m_em, "%s: sig child: waitpid fail, error=%d (%s)", set_svr_name(svr), errno, strerror(errno));
            return;
        }
    }

    mon_app = set_svr_mon_app_find_by_pid(g_set_svr_mon, pid);
    if (mon_app == NULL) {
        CPE_ERROR(svr->m_em, "%s: sig child: no mon app with id %d", set_svr_name(svr), pid);
        return;
    }

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: sig child: mon app %s: stop, pid=%d, status=%d", set_svr_name(svr), mon_app->m_bin, pid, status);
    }

    set_svr_mon_app_apply_evt(mon_app, set_svr_mon_app_fsm_evt_stoped);
}

