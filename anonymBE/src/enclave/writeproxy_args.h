#ifndef _WRITEPROXY_ARGS_H_
#define _WRITEPROXY_ARGS_H_

#ifdef __cplusplus
extern "C" {
#endif
struct Arguments {
    size_t port;
    char minioendpoint[100];
    char miniosecret[100];
    char minioaccesskey[100];
};
#ifdef __cplusplus
}
#endif

#endif

