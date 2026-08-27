/**
 * @file test_bignum_free_runner.c
 * @brief Distribution integration smoke test for bignum_free.
 * @details The runner includes the public header, links the selected C11 or
 * YASM implementation, invokes the one-function API, and verifies the named
 * success status plus the complete zero representation.
 */
#include "bignum_free.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    bignum_t number;

    memset(&number, 0xA7, sizeof(number));
    assert(bignum_free(&number) == BIGNUM_FREE_SUCCESS);
    assert(number.len == 0U);
    for (size_t index = 0U; index < BIGNUM_CAPACITY; ++index) {
        assert(number.words[index] == 0U);
    }
    puts("--- bignum_free distribution runner passed ---");
    return 0;
}
