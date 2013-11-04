#include <assert.h>
#include "dblog_svr_ops.h"

void dblog_svr_net_cb(EV_P_ ev_io *w, int revents) {
    dblog_svr_t svr = w->data;

    assert(svr);

    /* puts("udp socket has become readable"); */
    /* socklen_t bytes = recvfrom(sd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*) &addr, (socklen_t *) &addr_len); */

    /* // add a null to terminate the input, as we're going to use it as a string */
    /* buffer[bytes] = '\0'; */

    /* printf("udp client said: %s", buffer); */

    /* // Echo it back. */
    /* // WARNING: this is probably not the right way to do it with libev. */
    /* // Question: should we be setting a callback on sd becomming writable here instead? */
    /* sendto(sd, buffer, bytes, 0, (struct sockaddr*) &addr, sizeof(addr)); */
}
