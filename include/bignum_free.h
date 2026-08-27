/**
 * @file bignum_free.h
 * @brief Public API for clearing a caller-owned bignum object.
 * @details The module provides an in-place, allocation-free operation that
 * overwrites every word and the normalized length field of a bignum_t. The
 * operation is idempotent, does not release the object storage, and is safe
 * for independent objects. The C11 and x86-64 implementations expose the
 * same status and memory contract.
 */
#ifndef BIGNUM_FREE_H
#define BIGNUM_FREE_H

#include <bignum.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reports the result of clearing a bignum object.
 * @details A successful result means every representation word and the length
 * field were overwritten with zero. The operation allocates no memory and
 * never takes ownership of the caller-provided object.
 */
typedef enum bignum_free_status {
    BIGNUM_FREE_SUCCESS = 0, /**< The object was cleared completely; it is a valid zero value. */
    BIGNUM_FREE_ERROR_NULL_ARG = -1 /**< The pointer was NULL; no memory was accessed and no retry state changed. */
} bignum_free_status_t;

/**
 * @brief Clears all observable state in a caller-owned bignum object.
 * @details The function first validates the pointer, then overwrites all
 * BIGNUM_CAPACITY little-endian words and sets len to zero. It does not call
 * the C library free function and does not release the storage containing
 * `num`. Calling it repeatedly is valid. The C11 implementation uses volatile
 * byte stores so the wipe remains observable to the compiler; the ASM version
 * uses a fixed-size REP STOSQ sequence.
 *
 * @param[in,out] num Caller-owned live bignum_t object to clear. The pointer
 *                    must be writable and must not be accessed concurrently
 *                    by another thread. NULL is accepted as an error input.
 * @return BIGNUM_FREE_SUCCESS when all words and len are zero;
 *         BIGNUM_FREE_ERROR_NULL_ARG when num is NULL and no memory is touched.
 * @pre For a successful call, num points to writable storage for a complete
 *      bignum_t object. The object is not concurrently accessed.
 * @post On success, num->words[0..BIGNUM_CAPACITY-1] and num->len are zero.
 *       On NULL error, no caller-visible memory is changed.
 * @warning This operation clears contents but does not deallocate storage.
 * @thread_safety Safe for independent objects; external synchronization is
 *                required when another thread accesses the same object.
 * @complexity O(BIGNUM_CAPACITY) time and O(1) auxiliary space.
 */
bignum_free_status_t bignum_free(bignum_t *num);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_FREE_H */
