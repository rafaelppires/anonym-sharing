#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdio.h>

#if defined (__cplusplus)
extern "C" {
#endif

void random_stream(unsigned char b[], int n);

/* ------- AES OPERATIONS ---------- */
void aes_encrypt(unsigned char* plaintext,
    int plaintext_size,
    unsigned char* key, unsigned char* iv,
    unsigned char* ciphertext);

void aes_decrypt(unsigned char* ciphertext,
    int ciphertext_len,
    unsigned char* key, unsigned char* iv,
    unsigned char* plaintext);

#if defined (__cplusplus)
}
#endif

// CRYPTO_H
#endif
