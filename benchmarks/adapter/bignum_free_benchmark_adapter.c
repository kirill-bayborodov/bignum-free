/**
 * @file bignum_free_benchmark_adapter.c
 * @brief C11 benchmark-framework adapter for bignum_free.
 * @details The adapter owns no mutable global state. It validates workload
 * vocabulary, initializes an opaque bignum_t state deterministically, invokes
 * bignum_free once per callback, and hashes the complete post-operation state.
 */
#include "bignum_free_benchmark_adapter.h"

#include "bignum_free.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define FREE_FNV_OFFSET UINT64_C(14695981039346656037)
#define FREE_FNV_PRIME UINT64_C(1099511628211)

/**
 * @brief Advances the adapter deterministic generator.
 * @param[in,out] state Generator state owned by the current callback.
 * @return Next pseudo-random value.
 */
static uint64_t next_value(uint64_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 7;
    *state ^= *state << 17;
    return *state;
}

/**
 * @brief Initializes one benchmark state according to the input profile.
 * @param[out] state Opaque buffer with at least sizeof(bignum_t) bytes.
 * @param[in] sequence_index Stable dataset index.
 * @param[in] workload Validated immutable workload descriptor.
 * @param[in] adapter_context Unused project context.
 * @return Benchmark adapter status.
 */
static benchmark_adapter_status_t initialize_state(
    void *state,
    uint64_t sequence_index,
    const benchmark_workload_t *workload,
    void *adapter_context)
{
    bignum_t *number = state;
    uint64_t generator;
    size_t index;
    int make_zero;

    (void)adapter_context;
    if (state == NULL || workload == NULL) {
        return BENCHMARK_ADAPTER_STATUS_INPUT_ERROR;
    }

    make_zero = strcmp(workload->input_kind, "zero") == 0 ||
        (strcmp(workload->input_kind, "mixed") == 0 &&
         (sequence_index & UINT64_C(1)) == 0U);
    memset(number, 0, sizeof(*number));
    if (make_zero) {
        return BENCHMARK_ADAPTER_STATUS_SUCCESS;
    }

    generator = workload->seed ^
        (sequence_index + UINT64_C(0x9E3779B97F4A7C15));
    for (index = 0U; index < BIGNUM_CAPACITY; ++index) {
        number->words[index] = next_value(&generator);
    }
    number->len = BIGNUM_CAPACITY;
    if (number->words[BIGNUM_CAPACITY - 1U] == 0U) {
        number->words[BIGNUM_CAPACITY - 1U] = UINT64_C(1);
    }
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/**
 * @brief Executes one bignum_free operation on a mutable state copy.
 * @param[in,out] state Independent mutable bignum_t copy.
 * @param[in] iteration Logical benchmark iteration.
 * @param[in] workload Validated immutable workload descriptor.
 * @param[in] adapter_context Unused project context.
 * @return Success when the complete state was cleared.
 */
static benchmark_adapter_status_t operate_state(
    void *state,
    uint64_t iteration,
    const benchmark_workload_t *workload,
    void *adapter_context)
{
    (void)iteration;
    (void)workload;
    (void)adapter_context;
    if (state == NULL) {
        return BENCHMARK_ADAPTER_STATUS_INPUT_ERROR;
    }
    return bignum_free((bignum_t *)state) == BIGNUM_FREE_SUCCESS
        ? BENCHMARK_ADAPTER_STATUS_SUCCESS
        : BENCHMARK_ADAPTER_STATUS_OPERATION_ERROR;
}

/**
 * @brief Hashes the complete post-operation state for observability.
 * @param[in] state Read-only bignum_t state.
 * @param[in] iteration Logical benchmark iteration.
 * @param[in] adapter_context Unused project context.
 * @return FNV-1a checksum over all bytes and the iteration number.
 */
static uint64_t checksum_state(
    const void *state,
    uint64_t iteration,
    void *adapter_context)
{
    const unsigned char *bytes = state;
    uint64_t checksum = FREE_FNV_OFFSET;
    size_t index;

    (void)adapter_context;
    if (state == NULL) {
        return 0U;
    }
    for (index = 0U; index < sizeof(bignum_t); ++index) {
        checksum ^= bytes[index];
        checksum *= FREE_FNV_PRIME;
    }
    checksum ^= iteration;
    checksum *= FREE_FNV_PRIME;
    return checksum;
}

/* The public contract for this definition is documented in the header. */
bignum_free_benchmark_status_t bignum_free_benchmark_validate_workload(
    const benchmark_workload_t *workload)
{
    if (workload == NULL || workload->input_kind == NULL ||
        workload->operation_kind == NULL || workload->measure_mode == NULL ||
        workload->size_profile == NULL || workload->capacity_profile == NULL) {
        return BIGNUM_FREE_BENCHMARK_STATUS_NULL_ARGUMENT;
    }
    if (strcmp(workload->operation_kind, "free") != 0 ||
        (strcmp(workload->input_kind, "zero") != 0 &&
         strcmp(workload->input_kind, "nonzero") != 0 &&
         strcmp(workload->input_kind, "mixed") != 0) ||
        (strcmp(workload->measure_mode, "end-to-end") != 0 &&
         strcmp(workload->measure_mode, "kernel-only") != 0) ||
        (strcmp(workload->size_profile, "one") != 0 &&
         strcmp(workload->size_profile, "quarter") != 0 &&
         strcmp(workload->size_profile, "half") != 0 &&
         strcmp(workload->size_profile, "variable") != 0 &&
         strcmp(workload->size_profile, "near-capacity") != 0) ||
        (strcmp(workload->capacity_profile, "normal") != 0 &&
         strcmp(workload->capacity_profile, "near-capacity") != 0)) {
        return BIGNUM_FREE_BENCHMARK_STATUS_INVALID_PROFILE;
    }
    return BIGNUM_FREE_BENCHMARK_STATUS_SUCCESS;
}

/* The public contract for this definition is documented in the header. */
bignum_free_benchmark_status_t bignum_free_benchmark_adapter_init(
    benchmark_adapter_t *adapter)
{
    if (adapter == NULL) {
        return BIGNUM_FREE_BENCHMARK_STATUS_NULL_ARGUMENT;
    }
    memset(adapter, 0, sizeof(*adapter));
    adapter->benchmark_name = "bignum_free";
    adapter->state_size = sizeof(bignum_t);
    adapter->success_code = BENCHMARK_ADAPTER_STATUS_SUCCESS;
    adapter->initialize = initialize_state;
    adapter->operation = operate_state;
    adapter->checksum = checksum_state;
    return BIGNUM_FREE_BENCHMARK_STATUS_SUCCESS;
}
