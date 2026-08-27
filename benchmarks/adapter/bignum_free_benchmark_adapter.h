/**
 * @file bignum_free_benchmark_adapter.h
 * @brief Benchmark-framework binding for bignum_free.
 * @details The adapter maps generic benchmark-core workloads to a fixed-size
 * bignum_t wipe. Initialization creates deterministic nonzero representations;
 * operation clears the state in place; checksum observes all post-operation
 * bytes so the measured call cannot be removed as dead work.
 */
#ifndef BIGNUM_FREE_BENCHMARK_ADAPTER_H
#define BIGNUM_FREE_BENCHMARK_ADAPTER_H

#include <benchmark_framework.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reports adapter validation and initialization outcomes.
 */
typedef enum bignum_free_benchmark_status {
    BIGNUM_FREE_BENCHMARK_STATUS_SUCCESS = 0, /**< Adapter action completed successfully. */
    BIGNUM_FREE_BENCHMARK_STATUS_NULL_ARGUMENT = 1, /**< A required pointer was NULL; no output was published. */
    BIGNUM_FREE_BENCHMARK_STATUS_INVALID_PROFILE = 2 /**< A workload value is outside the free adapter vocabulary. */
} bignum_free_benchmark_status_t;

/**
 * @brief Initializes the benchmark-core binding for bignum_free.
 * @param[out] adapter Receives the complete callback binding.
 * @return Named bignum_free_benchmark_status_t result.
 * @post On success all callbacks, state size, name, and success code are valid.
 */
bignum_free_benchmark_status_t bignum_free_benchmark_adapter_init(
    benchmark_adapter_t *adapter);

/**
 * @brief Validates workload values accepted by the bignum_free adapter.
 * @param[in] workload Immutable benchmark-core workload descriptor.
 * @return Success when all fields use the documented free vocabulary; otherwise
 *         a named validation error.
 */
bignum_free_benchmark_status_t bignum_free_benchmark_validate_workload(
    const benchmark_workload_t *workload);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_FREE_BENCHMARK_ADAPTER_H */
