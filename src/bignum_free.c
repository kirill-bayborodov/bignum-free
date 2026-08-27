/**
 * @file bignum_free.c
 * @brief C11 reference implementation of bignum_free.
 * @details The implementation clears the complete fixed-size representation
 * through volatile byte stores. This reference is used for correctness tests,
 * coverage, and baseline benchmarks; it intentionally does not mirror the
 * instruction scheduling of the assembly implementation.
 */
#include "bignum_free.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Overwrites one byte through a volatile lvalue.
 * @details Volatile access prevents the compiler from removing the wipe as a
 * dead store. The helper performs no allocation and has no observable status.
 * @param[out] destination Writable byte in caller-owned bignum storage.
 * @param[in] value Zero byte to publish.
 * @pre destination points to writable storage.
 * @post The addressed byte contains value.
 */
static void bignum_free_store_byte(volatile unsigned char *destination,
                                   unsigned char value)
{
    *destination = value;
}

/* The public contract for this definition is documented in the header. */
bignum_free_status_t bignum_free(bignum_t *num)
{
    size_t byte_index;
    volatile unsigned char *bytes;

    if (num == NULL) {
        return BIGNUM_FREE_ERROR_NULL_ARG;
    }

    bytes = (volatile unsigned char *)num;
    for (byte_index = 0U; byte_index < sizeof(*num); ++byte_index) {
        bignum_free_store_byte(bytes + byte_index, 0U);
    }

    return BIGNUM_FREE_SUCCESS;
}
