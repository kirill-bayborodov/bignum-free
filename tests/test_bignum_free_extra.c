/**
 * @file test_bignum_free_extra.c
 * @brief Extended randomized and guard-region tests for bignum_free.
 * @details A fixed-seed generator produces varied byte patterns and len values.
 * Each case is checked against a complete zero-representation oracle and
 * adjacent guard objects detect writes outside bignum_t.
 */
#include "bignum_free.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Generates the next deterministic pseudo-random value.
 * @param[in,out] state Caller-owned generator state.
 * @return Next xorshift value.
 */
static uint64_t next_value(uint64_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 7;
    *state ^= *state << 17;
    return *state;
}

/**
 * @brief Fills a bignum representation with deterministic varied bytes.
 * @param[out] number Object to initialize.
 * @param[in,out] state Fixed-seed generator state.
 */
static void fill_random(bignum_t *number, uint64_t *state)
{
    size_t index;

    for (index = 0U; index < BIGNUM_CAPACITY; ++index) {
        number->words[index] = next_value(state);
    }
    number->len = (size_t)(next_value(state) % (BIGNUM_CAPACITY + 1U));
}

/**
 * @brief Checks whether every byte in an object representation is zero.
 * @param[in] number Object under test.
 * @return Nonzero when the complete representation is zero.
 */
static int is_zero_representation(const bignum_t *number)
{
    const unsigned char zero[sizeof(*number)] = {0};
    return memcmp(number, zero, sizeof(*number)) == 0;
}

/**
 * @brief Runs 4096 fixed-seed randomized representation wipes.
 * @details The trusted oracle is an all-zero byte representation. Any mismatch
 * aborts at the failing iteration and is reproducible from the fixed seed.
 */
static void test_randomized_wipes(void)
{
    uint64_t state = UINT64_C(0xD1B54A32D192ED03);
    size_t iteration;

    for (iteration = 0U; iteration < 4096U; ++iteration) {
        bignum_t number;

        fill_random(&number, &state);
        assert(bignum_free(&number) == BIGNUM_FREE_SUCCESS);
        assert(is_zero_representation(&number));
    }
}

/**
 * @brief Confirms that bignum_free does not write outside bignum_t.
 * @details Guard values before and after the object remain unchanged after a
 * patterned wipe, proving the fixed-size operation respects its boundary.
 */
static void test_guard_regions(void)
{
    struct guarded {
        uint64_t before;
        bignum_t number;
        uint64_t after;
    } value;

    value.before = UINT64_C(0x1111222233334444);
    value.after = UINT64_C(0xAAAABBBBCCCCDDDD);
    memset(&value.number, 0xCC, sizeof(value.number));
    assert(bignum_free(&value.number) == BIGNUM_FREE_SUCCESS);
    assert(value.before == UINT64_C(0x1111222233334444));
    assert(value.after == UINT64_C(0xAAAABBBBCCCCDDDD));
    assert(is_zero_representation(&value.number));
}

int main(void)
{
    test_randomized_wipes();
    test_guard_regions();
    puts("--- All extended bignum_free tests passed ---");
    return 0;
}
