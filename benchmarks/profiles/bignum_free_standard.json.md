# bignum_free standard benchmark profile

## Purpose

`bignum_free_standard.json` is a six-profile smoke and regression matrix consumed by the pinned C11 `benchmark-framework` `bench_matrix` tool. It measures the same fixed-size representation wipe over zero, nonzero, mixed, size, capacity, and measurement-boundary scenarios.

## Location and lifecycle

The manifest is a committed source artifact at `benchmarks/profiles/bignum_free_standard.json`. It is edited together with this companion document when a profile, vocabulary value, or schema field changes. The adapter accepts only `operation_kind: "free"` because `bignum_free` has no operation parameter.

## Schema and vocabulary

The root object contains `schema_version` (integer, exactly `1`), `description` (string), and `profiles` (non-empty array). Each profile requires string fields `id`, `input_kind`, `operation_kind`, `measure_mode`, `size_profile`, and `capacity_profile`.

Allowed values are `input_kind: zero|nonzero|mixed`, `operation_kind: free`, `measure_mode: end-to-end|kernel-only`, `size_profile: one|quarter|half|variable|near-capacity`, and `capacity_profile: normal|near-capacity`. The free operation clears all 32 words and `len`; size profiles describe deterministic source-state generation and do not change the API operation.

## Profiles

| Identifier | Input | Operation | Measurement | Size | Capacity | Scenario |
|---|---|---|---|---|---|---|
| `zero-one-end-to-end` | zero | free | end-to-end | one | normal | Zero source and complete lifecycle |
| `nonzero-one-kernel` | nonzero | free | kernel-only | one | normal | Small nonzero source wipe |
| `nonzero-quarter-kernel` | nonzero | free | kernel-only | quarter | normal | Quarter-capacity source wipe |
| `nonzero-half-kernel` | nonzero | free | kernel-only | half | normal | Half-capacity source wipe |
| `mixed-variable-end-to-end` | mixed | free | end-to-end | variable | normal | Alternating zero/nonzero lifecycle |
| `nonzero-near-capacity-kernel` | nonzero | free | kernel-only | near-capacity | near-capacity | Boundary-sized source wipe |

## Complete example

```json
{"schema_version":1,"description":"bignum_free smoke matrix","profiles":[{"id":"nonzero-one-kernel","input_kind":"nonzero","operation_kind":"free","measure_mode":"kernel-only","size_profile":"one","capacity_profile":"normal"}]}
```

## How to run

Build the selected implementation and run a short reproducible matrix:

```bash
make build CONFIG=release USE_ASM=no
make bench_matrix CONFIG=release USE_ASM=no \
  BENCH_MATRIX_PROFILE=benchmarks/profiles/bignum_free_standard.json \
  REPORT_NAME=c11_standard_smoke \
  BENCH_MATRIX_REPETITIONS=3 \
  BENCH_MATRIX_ITERATIONS=100000 \
  BENCH_MATRIX_MT_TOTAL_ITERATIONS=100000 \
  MT_THREADS=2
```

The raw report is written to `benchmarks/reports/c11_standard_smoke_matrix.json`, with a matching summary JSON. For ASM, repeat with `USE_ASM=yes` and a distinct report name. A successful callback returns the framework success status; malformed JSON, unsupported schema, invalid vocabulary, or callback failure makes the matrix command fail.

## How to modify

Add or change a profile in the JSON and this guide's profile table together. Keep the six required root/profile fields, use only the documented vocabulary, validate the JSON with the matrix tool, and run at least one C11 and one ASM smoke matrix before committing.

## Baseline and comparison

A baseline is valid only when candidate and baseline use the same profile IDs, schema, implementation configuration, seed, data count, warm-up, iteration counts, repetitions, thread count, CPU affinity, and host context. The framework rejects missing or extra profile IDs and applies its configured median/MAD regression gate. A smoke run is evidence of functionality, not a stable performance conclusion.

## Failure handling

The adapter rejects NULL workload descriptors and unsupported axis values before state generation. The framework reports malformed JSON or unsupported schema as an argument error. Callback initialization or operation failure is surfaced as a callback error; no benchmark result should be treated as valid after such a failure.
