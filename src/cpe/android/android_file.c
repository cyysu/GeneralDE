#include "cpe/android/android_file.h"

ssize_t android_file_load_to_buf(char * buf, size_t size, FILE * fp, error_monitor_t em);
ssize_t android_fild_load_to_buffer(mem_buffer_t buffer, FILE * fp, error_monitor_t em);
