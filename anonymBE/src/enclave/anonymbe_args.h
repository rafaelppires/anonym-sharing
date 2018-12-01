#ifndef _ANONYMBE_ARGS_H_
#define _ANONYMBE_ARGS_H_

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif
struct Arguments {
    size_t port;
    char mongo[1024];
};
#ifdef __cplusplus
}
#endif

#endif

