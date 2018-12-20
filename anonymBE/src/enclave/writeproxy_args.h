#ifndef _WRITEPROXY_ARGS_H_
#define _WRITEPROXY_ARGS_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
struct Arguments {
    size_t port;
    char minioendpoint[100],
         miniosecret[100],
         minioaccesskey[100],
         tokenuser[100],
         tokenpass[100],
         tokenendpoint[100],
         amendpoint[100];
};
#ifdef __cplusplus
}
#endif

#endif

