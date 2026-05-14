# Differential HULL multi-process / multi-thread runner

`differential/hull_multiple_thread_runner.cpp` is the server campaign runner for the XOR-differential Forest/HULL search.

It implements the requested three-level execution model:

```text
one parent runner process
└─ a std::thread scheduler pool
   └─ one OS child process per active job
      └─ direct call to the differential Forest/HULL C++ API
         └─ one independent SCIP instance
```

On POSIX systems, scheduler threads create children with `posix_spawn`; on Windows they use `CreateProcess`. The runner does not call `system()`, does not invoke a shell, and does not launch `neoalzette_scip_round_hull_search` as a separate executable. Each child is the same runner binary in an internal worker mode and calls the shared HULL API entry compiled from `neoalzette_scip_round_hull_search.cpp`.

## Cryptanalytic boundary

The runner does **not** alter:

- the two-variable modular-addition/subtraction differential MILP;
- the fixed-public-constant addition/subtraction differential model;
- CDDT/Q1 verification;
- the joint-injection support/rank model;
- the Forest Layer rule that feeds a Q1-valid output back as a later input;
- semantic no-good exclusion or SCIP reoptimization inside an individual HULL job.

It only partitions a campaign into isolated jobs and schedules them concurrently.

## Build

The runner embeds the HULL implementation, so it must be linked with the same SCIP 10.0.2 include paths, libraries, and platform libraries used by the differential HULL executable.

Example on a system exposing SCIP through `pkg-config`:

```bash
g++ -O3 -std=c++20 -pthread \
  hull_multiple_thread_runner.cpp \
  $(pkg-config --cflags --libs scip) \
  -o hull_multiple_thread_runner
```

A plain C++ compilation without SCIP libraries is not sufficient.

## Scheme A: independent Forest seeds

Scheme A starts many Forest jobs from different deterministic seeds. Every job retains the complete output-to-input Forest growth performed by the existing HULL implementation.

```bash
./hull_multiple_thread_runner \
  --mode A \
  --workers 16 \
  --jobs 64 \
  --job-time-limit 43200 \
  --output runs/differential_seed_campaign \
  -- --rounds 1
```

Job `i` uses `--forest-seed (seed_start+i)`. Optional fixed input differences may be passed after `--`; when omitted, the differential HULL defaults remain:

```text
--fix-input-da 0x00000001
--fix-input-db 0x00000001
```

## Scheme B: fixed-input sector slicing

Scheme B fixes an externally selected input pair `(dA,dB)` for each job. Within that job, the existing Forest Layer still feeds valid outputs back as later inputs until its time budget ends. After that child exits, another sector representative is scheduled.

```bash
./hull_multiple_thread_runner \
  --mode B \
  --workers 16 \
  --jobs 4096 \
  --job-time-limit 300 \
  --output runs/differential_sector_campaign \
  --prefix-bits 8 \
  --sector-start 0 \
  --samples-per-sector 1 \
  --sample-seed 0x1234 \
  -- --rounds 1
```

With `--prefix-bits 8`, `(high8(dA),high8(dB))` defines `2^16` sectors. Sample zero uses zero low bits, except that the all-zero pair is repaired to a nonzero input. Additional samples use deterministic SplitMix64-derived low bits. These are representatives of a sector, not exhaustive enumeration of every pair in it.

## Runner-owned and forwarded options

Arguments after `--` are passed to the existing differential HULL parser inside the child API worker. The runner itself owns:

```text
--hull-time-limit
--forest-seed
--hull-output-json
```

Scheme B additionally owns:

```text
--fix-input-da
--fix-input-db
```

Repeating a runner-owned option after `--` is rejected rather than silently overridden.

## Process isolation, logs, affinity, and interruption

The parent uses `--workers N` scheduler threads and permits at most `N` concurrent child processes. On Linux and Windows, each child can be pinned to an allowed logical CPU; `--no-affinity` disables this.

The OS redirects all child standard streams, including C-level SCIP output and C++ trace output, before the child starts:

```text
job_000000/
  worker_invocation.txt
  stdout.log
  stderr.log
  result.json
  api_error.txt        only on launch/API/worker failure
```

Thus default trajectory printing remains enabled without mixing all SCIP and trace output on the parent terminal.

On `SIGINT`/`SIGTERM` or a Windows console stop event, the parent stops scheduling new jobs and terminates active children. POSIX children first receive `SIGTERM` and are escalated to `SIGKILL` after a grace period. The interrupted campaign returns code `130`; jobs never started are recorded as `not_started_due_to_stop_request`.

## Campaign artifacts and coverage semantics

The parent writes:

```text
campaign_summary.json
forest_candidates.jsonl
job_manifest.jsonl
```

The summary reports observed unique `dA`, `dB`, input pairs, output pairs, prefix-cell coverage, Hamming-region coverage, maximum Forest layer, cycles, and best observed weights. Coverage means states actually present in worker Forest attempt records; it is not a claim of exhaustive coverage of the `2^64` pair space.

## HULL exactness boundary

Within each child, HULL correctness remains governed by the original MILP, Q1/CDDT verification, semantic no-good constraints, and reoptimization.

The parent deliberately does **not** add HULL probabilities across jobs. Different workers can rediscover the same semantic characteristic, so a cross-worker numerical HULL sum would require a separate characteristic-level deduplication proof. Campaign-level aggregation is therefore limited to coverage and candidate statistics.
