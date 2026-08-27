/**
 * @file test_bignum_free_mt.c
 * @brief Multithreaded reentrancy tests for bignum_free.
 * @details Eight workers repeatedly clear independent caller-owned objects.
 * The test verifies no shared mutable state is required and every object is
 * fully zero after its worker completes.
 */
#include "bignum_free.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS 8U
#define NUM_ITERATIONS 10000U

typedef struct free_thread_data {
    bignum_t number;
    int failed;
} free_thread_data_t;

/**
 * @brief Executes repeated wipes for one independent object.
 * @param[in,out] argument Thread-owned free_thread_data_t.
 * @return NULL after all iterations complete.
 */
static void *worker(void *argument)
{
    free_thread_data_t *data = argument;
    size_t iteration;

    for (iteration = 0U; iteration < NUM_ITERATIONS; ++iteration) {
        if (bignum_free(&data->number) != BIGNUM_FREE_SUCCESS) {
            data->failed = 1;
            return NULL;
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    free_thread_data_t data[NUM_THREADS];
    size_t index;

    for (index = 0U; index < NUM_THREADS; ++index) {
        memset(&data[index].number, (int)(0x20U + index), sizeof(bignum_t));
        data[index].failed = 0;
        assert(pthread_create(&threads[index], NULL, worker, &data[index]) == 0);
    }
    for (index = 0U; index < NUM_THREADS; ++index) {
        assert(pthread_join(threads[index], NULL) == 0);
        assert(data[index].failed == 0);
        assert(data[index].number.len == 0U);
        for (size_t word = 0U; word < BIGNUM_CAPACITY; ++word) {
            assert(data[index].number.words[word] == 0U);
        }
    }
    puts("--- All multithreaded bignum_free tests passed ---");
    return 0;
}
