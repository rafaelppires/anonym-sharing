#ifdef NATIVE
#ifndef _OCALLS_ANONUMBE_H_
#define _OCALLS_ANONUMBE_H_

int ocall_print_string(const char *str);
long ocall_sgx_clock(void);
struct tm *ocall_sgx_localtime(const time_t *timep, int t_len);
struct tm *ocall_sgx_gmtime_r(const time_t *timep, int t_len, struct tm *tmp,
                              int tmp_len);
int ocall_sgx_gettimeofday(void *tv, int tv_size);
int ocall_sgx_write(int fd, const void *buf, int n);

#endif // _OCALLS_ANONUMBE_H_
#endif // NATIVE
