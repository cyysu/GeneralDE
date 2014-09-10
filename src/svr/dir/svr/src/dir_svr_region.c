#include "dir_svr_ops.h"

dir_svr_region_t
dir_svr_region_create(dir_svr_t svr, uint16_t region_id, const char * region_name, uint8_t region_state) {
    dir_svr_region_t region;

    region = mem_alloc(svr->m_alloc, sizeof(struct dir_svr_region));
    if (region == NULL) return NULL;

    region->m_svr = svr;
    region->m_region_id = region_id;
    strncpy(region->m_region_name, region_name, sizeof(region->m_region_name));
    region->m_region_state = region_state;
    TAILQ_INIT(&region->m_servers);

    svr->m_region_count++;
    TAILQ_INSERT_TAIL(&svr->m_regions, region, m_next);

    return region;
}

void dir_svr_region_free(dir_svr_region_t region) {
    dir_svr_t svr = region->m_svr;

    while(!TAILQ_EMPTY(&region->m_servers)) {
        dir_svr_server_free(TAILQ_FIRST(&region->m_servers));
    }

    svr->m_region_count--;
    TAILQ_REMOVE(&svr->m_regions, region, m_next);
    mem_free(svr->m_alloc, region);
}

