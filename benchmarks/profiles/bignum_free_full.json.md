# bignum_free full benchmark profile

## Purpose

`bignum_free_full.json` is the controlled 12-profile matrix for reproducible analysis of the fixed-size in-place `bignum_free` wipe. It varies input class, source-size description, capacity boundary, and measurement scope; it never changes the operation itself.

The pinned C11 `benchmark-framework v1.0.0` `bench_matrix` tool consumes this document and launches project-owned ST/MT adapter binaries. The runner writes raw samples, while `benchmark_stats` produces robust per-profile statistics and optional regression results.

## Location and lifecycle

The committed source manifest is `benchmarks/profiles/bignum_free_full.json`, and this file is its required adjacent guide. Update both files in one change whenever a profile or schema contract changes. The adapter uses no heap-owned profile state and accepts only `operation_kind: "free"`.

## Schema and vocabulary

The root fields are `schema_version` (integer exactly `1`), `description` (string), and `profiles` (non-empty array). Every profile contains required string fields `id`, `input_kind`, `operation_kind`, `measure_mode`, `size_profile`, and `capacity_profile`.

Allowed values are `input_kind: zero|nonzero|mixed`, `operation_kind: free`, `measure_mode: end-to-end|kernel-only`, `size_profile: one|quarter|half|variable|near-capacity`, and `capacity_profile: normal|near-capacity`. `bignum_free` clears every word and `len`, so size and capacity values describe source generation and benchmark context rather than function parameters.

## Profile coverage

| Profile family | Count | Purpose |
|---|---:|---|
| Zero/nonzero small inputs | 3 | Compare zero representation and one-word source lifecycle |
| Quarter/half sources | 4 | Measure representative multi-word source preparation and wipe |
| Variable/mixed source | 2 | Exercise deterministic varied data and alternating input classes |
| Near-capacity sources | 3 | Exercise valid boundary-sized records without an error workload |
| **Total** | **12** | One ST and one MT sample stream per repetition |

The matrix produces `12 × 2 × R` samples for `R` repetitions. The operation has no intentionally timed failure path; NULL handling and invalid profiles are covered by unit tests and adapter validation tests.

## Complete example

```json
{"schema_version":1,"description":"bignum_free full matrix","profiles":[{"id":"nonzero-near-capacity-kernel","input_kind":"nonzero","operation_kind":"free","measure_mode":"kernel-only","size_profile":"near-capacity","capacity_profile":"near-capacity"}]}
```

## How to run

Build and run a controlled C11 matrix:

```bash
make build CONFIG=release USE_ASM=no
make bench_matrix CONFIG=release USE_ASM=no \
  BENCH_MATRIX_PROFILE=benchmarks/profiles/bignum_free_full.json \
  REPORT_NAME=c11_full \
  BENCH_MATRIX_REPETITIONS=7 \
  BENCH_MATRIX_ITERATIONS=200000 \
  BENCH_MATRIX_MT_TOTAL_ITERATIONS=320000 \
  MT_THREADS=2 \
  BENCH_MATRIX_WARMUP=10000 \
  BENCH_MATRIX_DATA_COUNT=4096 \
  BENCH_MATRIX_SEED=11400714819323198485
```

The raw report is written under `benchmarks/reports/` with the selected report name, alongside the summary JSON. Repeat with `USE_ASM=yes` and a different report name for the ASM comparison. Every successful benchmark process must publish one machine-readable `benchmark=...` line immediately before `Benchmark finished.`.

## How to modify

Add a profile only when it isolates a meaningful source or measurement condition. Preserve all required fields, use the allowed vocabulary, update the profile table and example, parse the JSON with the current matrix tool, and execute both C11 and ASM smoke runs. Do not encode invalid or NULL API cases as performance profiles.

## Baseline and regression comparison

A baseline and candidate must use identical profile IDs, manifest contents, schema version, seed, data count, warm-up, iteration counts, repetitions, thread count, CPU affinity, compiler/configuration, and host context. The statistics tool rejects missing or extra profiles. A regression is actionable only when the candidate median exceeds the configured percentage threshold and the robust baseline noise floor; a single smoke result is not a stable performance conclusion.

## Failure handling

Malformed JSON, unsupported schema, absent required fields, or unsupported vocabulary must fail before benchmark samples are accepted. The adapter rejects NULL workload descriptors and invalid axis values. Callback initialization or operation errors are surfaced by benchmark-core as callback failures, and their output must not be interpreted as a valid performance sample.
