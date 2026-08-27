# bignum-free

[![C/ASM CI](https://github.com/kirill-bayborodov/bignum-free/actions/workflows/ci.yml/badge.svg)](https://github.com/kirill-bayborodov/bignum-free/actions/workflows/ci.yml)
[![GitHub release](https://img.shields.io/github/v/release/kirill-bayborodov/bignum-free?label=release)](https://github.com/kirill-bayborodov/bignum-free/releases/latest)

`bignum-free` is a standalone C11/x86-64 YASM module that securely clears a caller-owned `bignum_t` representation in place. The production path is an x86-64 YASM implementation conforming to the System V AMD64 ABI. It validates the pointer and overwrites all `BIGNUM_CAPACITY` words plus `len` without releasing the object storage.

The module is a production component of the `bignum-lib` family and preserves the build, test, distribution, and benchmark conventions used by the family.

## Distribution

`bignum-free` has two project-local dependencies. The first is `bignum-core`, which is required to compile the public API because it defines `bignum_t` and `BIGNUM_CAPACITY`. The second is the pinned `benchmark-framework` distribution, which is required only by the benchmark adapters, ST/MT runners, JSON matrix commands, and benchmark statistics workflow. The free operation itself requires no additional arithmetic module.

| Component | Required for | Expected location | Purpose |
|---|---|---|---|
| `bignum-core` | Library and tests | `libs/bignum-core` | Git submodule defining `bignum_t` and `BIGNUM_CAPACITY` |
| `benchmark-framework v1.0.0` | Benchmarks only | `libs/benchmark-framework/dist` | Public header, static framework library, matrix/statistics tools, and benchmark documentation |

The generated product distribution is created by `make dist CONFIG=release` and contains the public `bignum_free.h`, `libbignum_free.a`, `LICENSE`, `README.md`, and the distribution runner source. It does not bundle the `bignum-core` submodule or the benchmark framework; those remain build-time project dependencies.

Clone the repository with the required submodule:

```bash
git clone --recurse-submodules https://github.com/kirill-bayborodov/bignum-free.git
cd bignum-free
```

For an existing clone, initialize the submodule with:

```bash
git submodule update --init --recursive
```

Before running benchmark targets, ensure that `libs/benchmark-framework/dist` contains `benchmark_framework.h`, `libbenchmark_framework.a`, and the matrix/statistics tools. System build tools such as `gcc`, `yasm`, `make`, and `pthread` are documented separately in the Dependencies section.

## Features

- **Production ASM path:** x86-64 YASM implementation for the System V AMD64 ABI.
- **Explicit API status:** the public API exposes `bignum_free_status_t` rather than reusing a generic core status type.
- **Complete representation wipe:** all words and the `len` field are overwritten with zero.
- **Allocation-free API:** the operation never releases caller storage or transfers ownership.
- **Idempotent behavior:** clearing an already-zero object is successful and safe.
- **Deterministic verification:** unit, boundary, extended, multithreaded, and integration-runner tests are included.
- **Reproducible benchmarks:** ST and MT runners accept deterministic seeds, report data fingerprints and checksums, and support legacy and parameterized workloads.
- **Pinned C11 benchmark framework:** CI installs the latest successful public `v1.0.0` distribution under `libs/benchmark-framework/dist`; the project consumes its public header, static library, matrix/statistics tools, profiles, and documentation as a library artifact.
- **Bignum domain adapter:** `benchmarks/adapter/` maps generic transport fields to the single bignum-specific `free` operation and deterministic source-state profiles.
- **Stable benchmark protocol:** successful runners print a machine-readable `benchmark=...` line immediately before `Benchmark finished.`.
- **Perf workflow:** Makefile targets provide sampling, repeated counter measurements, cloud-compatible software-event measurements, and report retention.

## Dependencies

| Dependency | Purpose |
|---|---|
| `make` | Build, test, lint, benchmark, and distribution targets |
| `gcc` | C compilation and linking |
| `yasm` | x86-64 assembly compilation |
| `cppcheck` | Static analysis |
| `perf` | Performance counters and sampling profiles |
| `taskset` | CPU-affinity control for benchmarks |
| `valgrind` | Helgrind race-detection target |
| `pthread` | Multithreaded tests and benchmarks |

The cloud benchmark target expects a `perf` binary compatible with the running kernel. In the current container workflow, this binary is configured by the `PERF` Makefile variable and is typically installed at `/usr/local/bin/perf`.

## API

The public API is declared in `include/bignum_free.h`:

```c
typedef enum {
    BIGNUM_FREE_SUCCESS = 0,
    BIGNUM_FREE_ERROR_NULL_ARG = -1
} bignum_free_status_t;

bignum_free_status_t bignum_free(bignum_t *num);
```

### Contract

| Condition | Return value | Result |
|---|---|---|
| `num == NULL` | `BIGNUM_FREE_ERROR_NULL_ARG` | No object is dereferenced or modified |
| Writable live `bignum_t` | `BIGNUM_FREE_SUCCESS` | Every word and `len` are overwritten with zero |
| Already-zero object | `BIGNUM_FREE_SUCCESS` | The operation is idempotent and storage remains valid |

The operation clears contents but does not call the C library `free` function and does not deallocate the object. It is thread-safe for independent objects; concurrent access to one object requires external synchronization.

For example:

```c
#include <stdint.h>
#include "bignum_free.h"

typedef enum {
    APPLICATION_STATUS_SUCCESS = 0,
    APPLICATION_STATUS_FREE_ERROR = 1
} application_status_t;

application_status_t clear_value(bignum_t *value)
{
    bignum_free_status_t status = bignum_free(value);

    return status == BIGNUM_FREE_SUCCESS
        ? APPLICATION_STATUS_SUCCESS
        : APPLICATION_STATUS_FREE_ERROR;
}
```

## Build and test

Build the release object and source submodules:

```bash
make build CONFIG=release
```

The production object is generated at:

```text
build/bignum_free.o
```

Run the deterministic, extended, multithreaded, and integration-runner suite:

```bash
make test CONFIG=release
```

The expected summary is:

```text
=== Summary: 0 / 5 failed ===
```

Run static analysis and the documentation quality gate:

```bash
make lint
doxygen docs/Doxyfile
```

The versioned `docs/Doxyfile` generates HTML under `build/docs/html/`, enables Graphviz dependency graphs, treats warnings as errors, and filters YASM comments without attempting to parse assembly instructions as C.

Run the sanitizer and race-detection targets:

```bash
make clean
make test_sanitize SAN=address CONFIG=debug

make clean
make test_sanitize SAN=undefined CONFIG=debug

make clean
make test_helgrind CONFIG=debug
```

The test files are organized as follows:

| File | Scope |
|---|---|
| `tests/test_bignum_free.c` | Deterministic API, contract, and boundary tests |
| `tests/test_bignum_free_extra.c` | Extended state, preservation, and boundary checks |
| `tests/test_bignum_free_mt.c` | Concurrent independent-object checks |
| `tests/test_bignum_free_runner.c` | Distribution integration smoke test |
| `tests/benchmark_adapter/test_bignum_free_benchmark_adapter.c` | C11 transport mapping, validation, deterministic initialization, operation, and checksum tests |

## Benchmarks

The active benchmark sources are:

```text
benchmarks/bench_bignum_free.c
benchmarks/bench_bignum_free_mt.c
```

Each successful run reports the selected mode, seed, input fingerprint, checksum, successful-call count, elapsed time, and nanoseconds per call. Its final two lines follow this stable protocol:

```text
benchmark=bignum_free_st ... elapsed_seconds=<seconds> ns_per_call=<nanoseconds>
Benchmark finished.
```

The MT runner uses `benchmark=bignum_free_mt`. The trailing marker is the success condition checked by the Makefile; it must remain after the machine-readable line.

| Mode | Input pattern | Purpose |
|---|---|---|
| `all_zero` | Every input `bignum_t` representation is zero | Measures the zero-value fast path |
| `all_nonzero` | Inputs are populated through `BIGNUM_CAPACITY` words with nonzero top words | Measures the normal nonzero clear path |
| `mixed` | Alternating zero and nonzero input rows | Measures a mixed workload and branch behavior |

### Single-thread CLI

```text
bin/bench_bignum_free \
  [--data-mode all_zero|all_nonzero|mixed] \
  [--input-kind zero|nonzero|mixed] \
  [--operation-kind free] \
  [--measure-mode end-to-end|kernel-only] \
  [--size-profile one|quarter|half|variable|near-capacity] \
  [--capacity-profile normal|near-capacity] \
  [--iterations N] [--warmup N] [--data-count N] [--seed N]
```

`--data-mode` preserves the three legacy scenarios. The independent `--input-kind`, `--operation-kind`, and `--size-profile` parameters select a custom profile and report `data_mode=custom`. `operation_kind` is a generic transport name, but the bignum adapter accepts only the documented `free` value and maps it to the complete representation wipe.

| Variable | Default | Meaning |
|---|---:|---|
| `BENCH_ITERATIONS` | `2000000000` | Number of ST calls; must be positive |
| `BENCH_WARMUP` | `10000` | Calls completed before the timed ST interval |
| `BENCH_DATA_COUNT` | `4096` | Size of the pre-generated immutable data pool |
| `BENCH_SEED` | `0x9E3779B97F4A7C15` | Seed for deterministic pre-generated data |
| `BENCH_INPUT_KIND` | `nonzero` | `zero`, `nonzero`, or `mixed` input profile |
| `BENCH_OPERATION_KIND` | `free` | The only accepted bignum_free operation transport value |
| `BENCH_MEASURE_MODE` | `end-to-end` | `end-to-end` includes per-call preparation; `kernel-only` excludes workspace restoration from the accumulated interval |
| `BENCH_SIZE_PROFILE` | `variable` | `one`, `quarter`, `half`, `variable`, or `near-capacity` bignum operand-length profile |
| `BENCH_CAPACITY_PROFILE` | `normal` | `normal` or `near-capacity`; the latter creates a valid boundary operand without intentionally measuring overflow handling |

CLI options override the corresponding environment variables. Example controlled ST comparison:

```bash
./bin/bench_bignum_free \
  --input-kind nonzero --operation-kind free --size-profile half \
  --measure-mode end-to-end \
  --iterations 1000000 --warmup 10000 --data-count 4096 \
  --seed 123456789

./bin/bench_bignum_free \
  --input-kind nonzero --operation-kind free --size-profile half \
  --measure-mode kernel-only \
  --iterations 1000000 --warmup 10000 --data-count 4096 \
  --seed 123456789
```

### Multithread CLI

```text
bin/bench_bignum_free_mt \
  [--threads N] [--total-iterations N] \
  [--data-mode all_zero|all_nonzero|mixed] \
  [--input-kind zero|nonzero|mixed] \
  [--operation-kind free] \
  [--measure-mode end-to-end|kernel-only] \
  [--size-profile one|quarter|half|variable|near-capacity] \
  [--capacity-profile normal|near-capacity] \
  [--warmup N] [--data-count N] [--seed N]
```

MT workers are created once, complete warm-up before the timed interval, then synchronize through barriers. `kernel-only` reports the longest per-worker accumulated operation interval, excluding the restoration copy before each batch; `end-to-end` reports wall-clock time from the synchronized release through all workers' completion.

| Variable | Default | Meaning |
|---|---:|---|
| `BENCH_MT_TOTAL_ITERATIONS` | `3200000000` | Total work across all threads; must be positive and divisible by the thread count |
| `BENCH_MT_THREADS` | `2` | Number of benchmark worker threads; must be positive |
| `BENCH_WARMUP` | `10000` | Warm-up calls per worker before the measured interval |
| `BENCH_DATA_COUNT` | `4096` | Size of the shared immutable data pool |
| `BENCH_SEED` | `0x9E3779B97F4A7C15` | Seed for deterministic pre-generated data |
| `BENCH_INPUT_KIND` | `nonzero` | `zero`, `nonzero`, or `mixed` input profile |
| `BENCH_OPERATION_KIND` | `free` | The only accepted bignum_free operation transport value |
| `BENCH_MEASURE_MODE` | `end-to-end` | `end-to-end` or `kernel-only` measurement mode |
| `BENCH_SIZE_PROFILE` | `variable` | `one`, `quarter`, `half`, `variable`, or `near-capacity` bignum operand-length profile |
| `BENCH_CAPACITY_PROFILE` | `normal` | `normal` or `near-capacity` boundary profile |

For a fair one-thread/two-thread comparison, keep the total work and seed constant:

```bash
./bin/bench_bignum_free_mt \
  --threads 1 \
  --total-iterations 3200000000 \
  --data-mode mixed

./bin/bench_bignum_free_mt \
  --threads 2 \
  --total-iterations 3200000000 \
  --data-mode mixed
```

The reusable benchmark implementation is the public `v1.0.0` flat distribution under `libs/benchmark-framework/dist`. The project-local ST and MT sources include `benchmark_framework.h` and link `libbenchmark_framework.a`; matrix/statistics tools are consumed from `dist/tools/`. The adapter validates the `free` vocabulary, constructs deterministic `bignum_t` records, performs the complete wipe, and maps `bignum_free_status_t` to the named framework callback status.

## Perf workflow

Use the cloud-compatible target when hardware PMU events are unavailable:

```bash
make bench_cl CONFIG=release \
  REPORT_NAME=baseline \
  PERF_RUNS=7
```

`bench_cl` uses `task-clock`, `context-switches`, `cpu-migrations`, and `page-faults`. It does not create raw `perf.data` profiles and requires the kernel-compatible binary configured by `PERF`.

On a host that supports the default hardware events, run the full ST/MT workflow for all three modes:

```bash
make bench_full CONFIG=release \
  REPORT_NAME=baseline \
  PERF_RUNS=7 \
  KEEP_PERF=1
```

For targeted repeated measurements:

```bash
make bench_stat_st CONFIG=release \
  REPORT_NAME=baseline_st_mixed \
  DATA_MODE=mixed \
  PERF_RUNS=7

make bench_stat_mt CONFIG=release \
  REPORT_NAME=baseline_mt_mixed \
  DATA_MODE=mixed \
  MT_THREADS=2 \
  MT_CPU_LIST=0-1 \
  MT_TOTAL_ITERATIONS=3200000000 \
  PERF_RUNS=7
```

Reports are written to `benchmarks/reports/`. With `KEEP_PERF=1`, record-mode raw profiles are retained as `.perf.data` files. Keep `CONFIG`, `PERF_RUNS`, `DATA_MODE`, seed, thread count, CPU affinity, and total iterations constant when comparing implementations.

### Parameterized JSON matrix and regression gate

`bench_matrix` invokes the pinned C11 `bench_matrix` and `benchmark_stats` tools directly, without Python or hardware PMU events. The default `benchmarks/profiles/bignum_free_full.json` covers zero/nonzero/mixed source states, one/quarter/half/variable lengths, and safe near-capacity cases for the complete representation wipe. `benchmarks/profiles/bignum_free_standard.json` is the shorter bignum-specific smoke manifest and can be selected through `BENCH_MATRIX_PROFILE`. Each JSON manifest has a companion how-to document with its exact vocabulary and baseline workflow.

```bash
make bench_matrix CONFIG=release \
  REPORT_NAME=baseline \
  BENCH_MATRIX_REPETITIONS=7 \
  BENCH_MATRIX_ITERATIONS=200000000 \
  BENCH_MATRIX_MT_TOTAL_ITERATIONS=320000000 \
  MT_THREADS=2
```

The target writes `benchmarks/reports/<report>_matrix.json` and `benchmarks/reports/<report>_matrix_summary.json`. The raw artifact preserves host metadata, the manifest hash, commands, stdout/stderr, and parsed protocol values. The summary stores robust per-profile ST/MT statistics: median, mean, sample standard deviation, and MAD.

To compare a candidate with a reviewed baseline, pass the reference JSON explicitly. The target rejects absent/extra profile IDs and fails only when candidate median `ns_per_call` is both above the configured percentage threshold and above the baseline MAD noise floor.

```bash
make bench_matrix CONFIG=release \
  REPORT_NAME=candidate \
  BENCH_BASELINE=benchmarks/reports/baseline_matrix.json \
  BENCH_REGRESSION_THRESHOLD_PCT=5
```

Do not replace a baseline automatically. Create it from a reviewed reproducible run with stable host topology, CPU affinity, compiler version, workload settings, and source revision.

For a short cloud smoke test, combine the documented environment variables with the Makefile target:

```bash
BENCH_ITERATIONS=100000 \
BENCH_MT_TOTAL_ITERATIONS=100000 \
BENCH_SEED=123456789 \
make bench_cl CONFIG=release REPORT_NAME=smoke PERF_RUNS=1 \
  MT_TOTAL_ITERATIONS=100000
```

## Installation and distribution

Build the object-file distribution:

```bash
make install CONFIG=release
```

Build the single-header and static-library distribution:

```bash
make dist CONFIG=release
```

Remove generated artifacts:

```bash
make clean
```

## Linking the object file

For development builds, first compile the module and its source dependencies:

```bash
make build CONFIG=release
```

Then link your application with the component object and the required include paths:

```bash
gcc your_app.c \
  build/bignum_free.o \
  -I./include \
  -I./libs/bignum-core/include \
  -o your_app \
  -no-pie
```

If the application requires symbols from the module dependency graph, prefer the distribution created by `make dist CONFIG=release` and link the resulting static library with the corresponding component libraries.

## Contributing

Contributions should preserve the public C/ASM contract, update deterministic and multithreaded tests when behavior changes, and run at least:

```bash
make test CONFIG=release
make lint
```

Performance changes should include reproducible benchmark parameters, matching ST/MT evidence, and a comparison that uses the same mode, seed, total work, thread count, CPU affinity, and counter configuration.

## Documentation Quality Gates

This README and the adjacent documentation artifacts follow `docs/QUALITY_GATES_DOCUMENTATION_C11_JSON.md`. A change is documentation-complete only when the reader can identify the purpose, public contract, ownership, NULL policy, aliasing and concurrency rules, algorithmic rationale, ABI boundary, error behavior, build/test commands, benchmark protocol, and reproducibility requirements without reading the implementation.

Before merge, the following artifacts must be reviewed together: `include/bignum_free.h`, C11 and YASM sources, deterministic/extended/MT/integration tests, benchmark adapter, both JSON manifests with their `.json.md` guides, `docs/Doxyfile`, and this README. Public functions require complete Doxygen `@brief`, `@details`, parameter, return, precondition, postcondition, thread-safety, and complexity documentation. Examples must be copyable, use the actual one-argument `bignum_free` API, and describe that the operation clears contents without deallocating storage.

The documentation gate is blocking for stale template/operation names, contradictory status or ownership semantics, undocumented ABI registers, invalid JSON examples, missing companion guides, broken links, undocumented benchmark variables, claims unsupported by tests or measurements, non-English production comments, and `git diff --check` failures. Run `make lint`, `doxygen docs/Doxyfile`, both C11/ASM test modes, JSON validation, and the relevant benchmark matrix before accepting a documentation change.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
