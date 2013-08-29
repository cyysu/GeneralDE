#include <assert.h> 
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_pbuf.h"
#include "center_svr_ops.h"

int center_svr_conn_send(struct center_svr_conn * conn, SVR_CENTER_PKG * pkg, size_t pkg_size) {
    center_svr_t svr = conn->m_svr;
    size_t curent_pkg_size = 2048;
    ringbuffer_block_t blk;
    char * buf;
    int32_t encode_size;

    while(curent_pkg_size < pkg_size) { 
        curent_pkg_size *= 2;
    }

RESIZE_AND_TRY_AGAIN:
    blk = ringbuffer_alloc(svr->m_ringbuf , curent_pkg_size);
    if (blk == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: send: not enouth ringbuf, curent_pkg_size=%d, data-len=%d!",
            center_svr_name(svr), conn->m_fd, (int)curent_pkg_size, (int)pkg_size);
    }

    buf = NULL;
    ringbuffer_data(svr->m_ringbuf, blk, curent_pkg_size, 0, (void*)&buf);
    assert(buf);

    encode_size =
        dr_pbuf_write(
            buf + sizeof(uint32_t),
            curent_pkg_size - sizeof(uint32_t),
            pkg, pkg_size, svr->m_pkg_meta, svr->m_em);
    if (encode_size < 0) {
        if (encode_size == dr_code_error_not_enough_output) {
            if (curent_pkg_size >= svr->m_max_pkg_size) {
                CPE_ERROR(
                    svr->m_em, "%s: conn %d: send: not enough encode buf, buf size is %d!",
                    center_svr_name(svr), conn->m_fd, (int)curent_pkg_size);
                ringbuffer_free(svr->m_ringbuf, blk);
                return -1;
            }
            else {
                curent_pkg_size *= 2;
                if (curent_pkg_size > svr->m_max_pkg_size) curent_pkg_size = svr->m_max_pkg_size;
                ringbuffer_free(svr->m_ringbuf, blk);
                goto RESIZE_AND_TRY_AGAIN;
            }
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: conn %d: send: encode fail, buf size is %d!",
                center_svr_name(svr), conn->m_fd,
                (int)curent_pkg_size);
            ringbuffer_free(svr->m_ringbuf, blk);
            return -1;
        }
    }
    else {
        encode_size += sizeof(uint32_t);
        CPE_COPY_HTON32(buf, &encode_size);
    }

    ringbuffer_shrink(svr->m_ringbuf, blk, encode_size);
    center_svr_conn_link_node_w(conn, blk);

    if (svr->m_debug) {
        struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&svr->m_dump_buffer);
        mem_buffer_clear_data(&svr->m_dump_buffer);

        dr_json_print((write_stream_t)&stream, pkg, pkg_size, svr->m_pkg_meta, 0, NULL);
        stream_putc((write_stream_t)&stream, 0);

        CPE_INFO(
            svr->m_em, "%s: conn %d: ==> send one request, (net-size=%d, data-size=%d)\n%s",
            center_svr_name(svr), conn->m_fd,
            (int)encode_size, (int)pkg_size,
            (const char *)mem_buffer_make_continuous(&svr->m_dump_buffer, 0));
    }

    return 0;
}
