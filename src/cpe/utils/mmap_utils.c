#include "cpe/pal/pal_fcntl.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/utils/mmap_utils.h"
#include "cpe/utils/file.h"
#include "file_internal.h"
#ifdef _MSC_VER
#else
#include <sys/mman.h>
#endif

void * mmap_file_load(const char * file, const char * mod, size_t * size, error_monitor_t em) {
    int fd;
    int mode = O_BINARY;
    char c;
    void * r;
    struct stat state_buf;
    const char * p = mod;
#ifdef _MSC_VER
    ssize_t readed_size = 0;
#else
    int prot = 0;
#endif

    for(c = *p; c; p++, c=*p) {
        if (c == 'r') {
            mode |= O_RDONLY;
#ifndef _MSC_VER
            prot |= PROT_READ;
#endif
        }
        else if (c == 'w') {
            mode |= O_WRONLY;
#ifndef _MSC_VER
            prot |= PROT_WRITE;
#endif
        }
        else {
            CPE_ERROR(em, "mmap_file_load: mod %s format error!", mod);
            return NULL;
        }
    }

    fd = open(file, mode, FILE_DEFAULT_MODE); 
    if (fd < 0) {
        CPE_ERROR(
            em, "mmap_file_load: open file %s fail, error=%d (%s)",
            file, errno, strerror(errno));
        return NULL;
    }

    if (fstat(fd, &state_buf) != 0) {
        CPE_ERROR(
            em, "mmap_file_load: state file %s fail, error=%d (%s)",
            file, errno, strerror(errno));
        close(fd);
        return NULL;
    }

#if defined _MSC_VER
    r = malloc(state_buf.st_size);
    if (r == NULL) {
        CPE_ERROR(
            em, "mmap_file_load: malloc file buff, size=%d, error=%d (%s)",
            (int)state_buf.st_size, errno, strerror(errno));
        close(fd);
        return NULL;
    }

    while(readed_size < state_buf.st_size) {
        ssize_t s = read(fd, ((char*)r) + readed_size, state_buf.st_size - readed_size);
        if (s <= 0) {
            CPE_ERROR(
                em, "mmap_file_load: read file %s fail, error=%d (%s)",
                file, errno, strerror(errno));
            close(fd);
            return NULL;
        }

        readed_size += s;
    };

#else
    r = mmap(NULL, state_buf.st_size, prot, MAP_FILE | MAP_PRIVATE, fd, 0);
    if ((ptr_int_t)r == -1) {
        CPE_ERROR(
            em, "mmap_file_load: mmap file %s fail, size=%d, error=%d (%s)",
            file, (int)state_buf.st_size, errno, strerror(errno));
        close(fd);
        return NULL;
    }
#endif

    close(fd);

    if (size) *size = state_buf.st_size;

    return r;
}

void mmap_unload(void * p, size_t size) {
#ifdef _MSC_VER
    free(p);
#else
    munmap(p, size);
#endif
}

