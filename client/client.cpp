#include<stdlib.h>
#include<string.h>
#include <pthread.h>
#include "crypto.h"
#include <time.h>

#define THREADS_COUNT 4

unsigned char* iv = (unsigned char*) "wWw456789012345678901234567890IV";
unsigned char* positive_hint = (unsigned char*) "11111111111111111111111111111111";
unsigned char* negative_hint = (unsigned char*) "00000000000000000000000000000000";

static inline void print_hex(unsigned char *h, int l)
{
    for (int i=0; i<l; i++)
        printf("%02X", h[i]);
    printf("\n");
}

int user_get_file_key(
    unsigned char* user_key, size_t k_size,
    unsigned char* ciphertext, size_t c_size,
    unsigned char* p_file_key, size_t* p_fk_size)
{
    int trials = c_size / 64;
    for(int i=0; i<trials; i++)
    {
        unsigned char dec_hint[32];
        aes_decrypt(ciphertext, 32, user_key, iv, dec_hint);
        if (memcmp(dec_hint, positive_hint, 32) == 0)
        {
            unsigned char dec_fk[32];
            aes_decrypt(ciphertext + 32, 32, user_key, iv, p_file_key);
            *p_fk_size = 32;
            //printf("SUCCESS !\n");
            return 0;
        }
        ciphertext += 64;
    }

    // error - no hint decryption worked
    return -1;
}

int FOUND_POSITIVE = 0;

struct ThreadArg {
    unsigned char* user_key;
    unsigned char* ciphertext;
    size_t c_size;
};

// The function to be executed by all threads
void *myThreadFun(void* p_arg)
{
    struct ThreadArg *a = (struct ThreadArg*) p_arg;
    unsigned char dec_fk[32];
    size_t s;

    int trials = a->c_size / 64;
    printf("%d trials to do\n", trials);
    for(int i=0; i<trials && !FOUND_POSITIVE; i++)
    {
        unsigned char dec_hint[32];
        aes_decrypt(a->ciphertext, 32, a->user_key, iv, dec_hint);
        if (memcmp(dec_hint, positive_hint, 32) == 0)
        {
            unsigned char dec_fk[32];
            aes_decrypt(a->ciphertext + 32, 32, a->user_key, iv, dec_fk);
            FOUND_POSITIVE = 1;
            printf("SUCCESS !\n");
            return 0;
        }
        a->ciphertext += 64;
    }
}


int _mt_user_get_file_key(
    unsigned char* user_key, size_t k_size,
    unsigned char* ciphertext, size_t c_size,
    unsigned char* p_file_key, size_t* p_fk_size)
{
    int i;
    int trials = c_size / 64;

    // Let us create three threads
    pthread_t tid[THREADS_COUNT];
    for (i = 0; i < THREADS_COUNT; i++)
    {
        int start_trials = (i * trials) / THREADS_COUNT;
        int end_trials = ((i + 1) * trials) / THREADS_COUNT;
        if (end_trials > trials)
        {
            end_trials = trials;
        }
        printf("Feeding thread with trials %d to %d\n", start_trials, end_trials);
        struct ThreadArg *a = (struct ThreadArg*) malloc(sizeof(struct ThreadArg));
        a->user_key = user_key;
        a->c_size = (end_trials - start_trials) * 64;
        a->ciphertext = ciphertext + (64 * start_trials);
        pthread_create(&tid[i], NULL, myThreadFun, (void *)a);
    }

    for (i = 0; i < THREADS_COUNT; i++)
    {
         pthread_join(tid[i], NULL);
    }
    return 0;
}

int basic_user_validation_test(int users_in_group)
{
    unsigned char* file_key = (unsigned char*) "12345678901234567890123456789012";
    unsigned char* user_key = (unsigned char*) "ux000000000000000000000000000000";

    int size_of_ciphertext = users_in_group * 32 * 2;
    unsigned char* ciphertext = (unsigned char*) malloc(size_of_ciphertext);

    // simulate a reference monitor encryption
    unsigned char* it = ciphertext;
    for(int i=0; i<users_in_group; i++)
    {
        if (i == 2 * (users_in_group/3))
        {
            // assume a hordcoded member in the group
            aes_encrypt(positive_hint, 32, user_key, iv, it);
            it += 32;
            aes_encrypt(file_key, 32, user_key, iv, it);
        }
        else
        {
            unsigned char random_key[32];
            random_stream(random_key, 32);
            aes_encrypt(negative_hint, 32, random_key, iv, it);
            it += 32;
            aes_encrypt(file_key, 32, random_key, iv, it);
        }
        it += 32;
    }
    printf("[test] : simmulated a reference monitor key envelope for %d users...\n", users_in_group);
    printf("[test] : ciphertext size is %d bytes...\n", size_of_ciphertext);

    // perform a decryption
    unsigned char dec_fk[32];
    size_t dec_fk_size;
    clock_t begin = clock();

    int ret_status = user_get_file_key(user_key, 32, ciphertext, size_of_ciphertext,
        dec_fk, &dec_fk_size);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("[test] : decrypted file_key by using user_key in %f seconds ...\n", time_spent);
    // TODO : remove me after multithreading is in place
    return 0;

    // assert that the two keys are the same
    if (ret_status == 0 && memcmp(dec_fk, file_key, 32) == 0)
    {
        printf("[test] : SUCCESS.\n");
    }
    else
    {
        printf("[test] : FAILED.\n");
    }
}

int main()
{
    basic_user_validation_test(100);
    basic_user_validation_test(1000);
    basic_user_validation_test(10000);
    basic_user_validation_test(100000);
}
