/**
 * @file test_bignum_free.c
 * @brief Deterministic contract tests for bignum_free.
 * @details The tests use fixed patterns and a byte-for-byte oracle. They verify
 * NULL rejection, complete representation clearing, idempotence, and the fact
 * that storage is cleared without being deallocated.
 */
#include "bignum_free.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Fills every byte of a bignum object with a deterministic pattern.
 * @param[out] number Caller-owned object to initialize.
 * @param[in] value Repeated byte pattern.
 * @post Every byte in number has the requested value.
 */
static void fill_pattern(bignum_t *number, unsigned char value)
{
    memset(number, (int)value, sizeof(*number));
}

/**
 * @brief Verifies that the complete object representation is zero.
 * @param[in] number Object under test.
 * @return Nonzero when every byte is zero.
 */
static int is_zero_representation(const bignum_t *number)
{
    const unsigned char zero[sizeof(*number)] = {0};
    return memcmp(number, zero, sizeof(*number)) == 0;
}

/**
 * @brief Verifies NULL input does not access or modify memory.
 */
static void test_null_argument(void)
{
    assert(bignum_free(NULL) == BIGNUM_FREE_ERROR_NULL_ARG);
}

/**
 * @brief Verifies all words and len are cleared from a non-normalized object.
 */
static void test_complete_wipe(void)
{
    bignum_t number;

    fill_pattern(&number, 0xA5U);
    number.len = BIGNUM_CAPACITY;
    assert(bignum_free(&number) == BIGNUM_FREE_SUCCESS);
    assert(is_zero_representation(&number));
}

/**
 * @brief Verifies zero input and repeated clearing are successful and stable.
 */
static void test_idempotence(void)
{
    bignum_t number = {0};

    assert(bignum_free(&number) == BIGNUM_FREE_SUCCESS);
    assert(is_zero_representation(&number));
    assert(bignum_free(&number) == BIGNUM_FREE_SUCCESS);
    assert(is_zero_representation(&number));
}

/**
 * @brief Verifies the caller retains valid storage after the wipe.
 */
static void test_storage_remains_writable(void)
{
    bignum_t number;

    fill_pattern(&number, 0x5AU);
    assert(bignum_free(&number) == BIGNUM_FREE_SUCCESS);
    number.words[0] = UINT64_C(0x1234);
    number.len = 1U;
    assert(number.words[0] == UINT64_C(0x1234));
    assert(number.len == 1U);
}

int main(void)
{
    test_null_argument();
    test_complete_wipe();
    test_idempotence();
    test_storage_remains_writable();
    puts("--- All deterministic bignum_free tests passed ---");
    return 0;
}
