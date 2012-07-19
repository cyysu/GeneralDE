#include "cpe/pal/pal_shm.h"

#if defined _MSC_VER /*windows*/

#else /*else windows*/
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int cpe_shm_key_gen(const char * name, char app_id) {
    struct stat state_buf;
    int f_status;

    f_status = stat(name, &state_buf);
    if (f_status == -1) {
        if (errno == ENOENT) {
            int fd = creat(name, 0666);
            if (fd == -1) return -1;
            close(fd);
        }
        else {
            return -1;
        }
    }
    
    return ftok(name, app_id);
}

#endif
