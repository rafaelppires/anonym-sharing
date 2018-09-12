#include <enclave_anonymbe_u.h>
#include <stdio.h>
#include <sys/time.h>

//------------------------------------------------------------------------------
int ocall_print_string(const char *str) {
    return printf("\033[96m%s\033[0m", str);
}

//------------------------------------------------------------------------------
long ocall_sgx_clock(void) {
    struct timespec tstart = {0, 0}, tend = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    return tstart.tv_sec * 1000000 +
           tstart.tv_nsec / 1000;  // Return micro seconds
}

//------------------------------------------------------------------------------
struct tm *ocall_sgx_localtime(const time_t *timep, int t_len) {
    return localtime(timep);
}

//------------------------------------------------------------------------------
struct tm *ocall_sgx_gmtime_r(const time_t *timep, int t_len, struct tm *tmp,
                              int tmp_len) {
    return gmtime_r(timep, tmp);
}

//------------------------------------------------------------------------------
int ocall_sgx_gettimeofday(void *tv, int tv_size) {
    return gettimeofday((struct timeval *)tv, NULL);
}

//------------------------------------------------------------------------------
int ocall_sgx_write(int fd, const void *buf, int n) {
    return write(fd, buf, n);
}

//------------------------------------------------------------------------------
