# NeoAlzette Linear MILP Engineering Runner

This directory contains the LINEAR / Walsh-correlation SCIP C API backend.  It
searches best signed single characteristics, round tables, fixed-endpoint
linear hulls, and engineering endpoint sweeps.

## Model Boundary

- This is the linear backend only; XOR-differential witness variables are not
  introduced here.
- Two-variable modular addition/subtraction uses the Wallen/Fu-Wang-Guo linear
  characteristic model.
- Fixed-public-addend add/sub uses the exact Miyano two-state signed transfer
  model plus the exact log-weight MILP epigraph.
- Joint injection constraints use the quadratic Walsh support/rank model for
  `alpha.x xor beta.J(x)`.
- Best-trail search optimizes one signed characteristic.  Hull modes enumerate
  characteristics for a fixed endpoint and aggregate signed contributions.

## Files

- `model/neoalzette_scip_operator_analysis_oracle.hpp`
  - Exact local linear oracles and the joint injection Walsh oracle.
- `model/neoalzette_scip_operator_analysis_milp_constraint.hpp`
  - SCIP variables, arithmetic constraints, custom Walsh and log-weight
    constraint handlers, and model-builder utilities.
- `model/neoalzette_scip_search_round_function.hpp`
  - Round construction, solve flow, trace/JSON output, and best-trail runner.
- `neoalzette_scip_round_milp_search.cpp`
  - Best-trail, continuous best-trail, and round-table entry point.
- `neoalzette_scip_round_hull_search.cpp`
  - Fixed-endpoint bounded and complete hull entry point.
- `rebuild_linear.bat`
  - Windows `cmd.exe` rebuild helper for both linear executables.

## Windows Build

The checked Windows build links against the existing static SCIPOptSuite tree:

```text
E:\_ABOUT~1\_CODEP~1\C__~1\NEOALZ~3\scipoptsuite-build-static-papilo-short-fresh
```

The repository path contains square brackets, so the Windows command line uses
8.3 short paths.  Rebuild the linear executables with:

```cmd
cmd /c linear\rebuild_linear.bat
```

The batch file compiles to `.tmp.exe`, verifies that the temporary file exists
and is nonzero, then replaces the final executable:

```text
linear\neoalzette_scip_round_milp_search.exe
linear\neoalzette_scip_round_hull_search.exe
```

Only rebuild SCIPOptSuite if the existing tree is missing or broken.  The
Windows configuration shape is:

```cmd
set "REPO=E:\_ABOUT~1\_CODEP~1\C__~1\NEOALZ~3"
set "MSYS=E:\_ABOUT~1\MSYS2"
set "SRC=%REPO%\SCIPOP~1.2"
set "BUILD=%REPO%\scipoptsuite-build-static-papilo-short-fresh"
set "MINGW=%MSYS%\mingw64"
set "PATH=%MINGW%\bin;%MSYS%\usr\bin;%PATH%"

"%MINGW%\bin\cmake.exe" -S "%SRC%" -B "%BUILD%" -G Ninja ^
  "-DCMAKE_MAKE_PROGRAM=%MINGW%\bin\ninja.exe" ^
  "-DCMAKE_PREFIX_PATH=%MINGW%" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_SHARED_LIBS=OFF ^
  -DSHARED=OFF ^
  -DAUTOBUILD=ON ^
  -DLPS=spx ^
  -DSOPLEX=ON ^
  -DPAPILO=ON ^
  -DGMP=ON ^
  -DSTATIC_GMP=ON ^
  -DMPFR=ON ^
  -DTBB=ON ^
  -DZIMPL=ON ^
  -DLTO=ON ^
  -DTPI=tny ^
  -DSYM=snauty ^
  "-DSOPLEX_DIR=%BUILD%" ^
  "-DPAPILO_DIR=%BUILD%"

"%MINGW%\bin\cmake.exe" --build "%BUILD%" --config Release
```

## Linux Build

Build SCIPOptSuite before compiling the repository executable.  The full
cross-platform build notes are in `../README_BUILD.md`; the Linux source build
shape is:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build pkg-config flex bison \
  libboost-iostreams-dev libboost-program-options-dev libboost-serialization-dev \
  libgmp-dev libmpfr-dev zlib1g-dev libbz2-dev libreadline-dev libtbb-dev

cmake -S scipoptsuite-10.0.2 -B scipoptsuite-build-linux -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DSHARED=OFF \
  -DAUTOBUILD=ON \
  -DLPS=spx \
  -DSOPLEX=ON \
  -DPAPILO=ON \
  -DGMP=ON \
  -DMPFR=ON \
  -DTBB=ON \
  -DZIMPL=ON \
  -DGCG=OFF \
  -DUG=OFF \
  -DLTO=OFF \
  -DTPI=tny \
  -DSYM=snauty \
  -DSOPLEX_DIR="$PWD/scipoptsuite-build-linux" \
  -DPAPILO_DIR="$PWD/scipoptsuite-build-linux"

cmake --build scipoptsuite-build-linux --parallel
```

If a system SCIP installation provides `pkg-config` metadata, compile the
linear executables from the repository root with:

```sh
g++ -O2 -std=c++20 linear/neoalzette_scip_round_milp_search.cpp $(pkg-config --cflags --libs scip) -o linear/neoalzette_scip_round_milp_search
g++ -O2 -std=c++20 linear/neoalzette_scip_round_hull_search.cpp $(pkg-config --cflags --libs scip) -o linear/neoalzette_scip_round_hull_search
```

If `pkg-config --libs scip` is unavailable, link against the local static
SCIPOptSuite build:

```sh
SRC="$PWD/scipoptsuite-10.0.2"
BUILD="$PWD/scipoptsuite-build-linux"
CXXFLAGS="-O2 -std=c++20 -I$SRC/scip/src -I$BUILD/scip -I$SRC/soplex/src -I$BUILD/soplex -I$SRC/papilo/src -I$BUILD/papilo"
LIBS="$BUILD/lib/libscip.a $BUILD/lib/libsoplex.a $BUILD/papilo/libpapilo-core.a -lboost_iostreams -lboost_program_options -lboost_serialization -lgmpxx -lgmp -lmpfr -lz -lbz2 -lreadline -ltbb -lquadmath -lpthread -ldl -lm"

g++ $CXXFLAGS linear/neoalzette_scip_round_milp_search.cpp -o linear/neoalzette_scip_round_milp_search $LIBS
g++ $CXXFLAGS linear/neoalzette_scip_round_hull_search.cpp -o linear/neoalzette_scip_round_hull_search $LIBS
```

If your build emits `lib64/` instead of `lib/`, adjust the two SCIP/SoPlex
archive paths in `LIBS`.

## Best-Trail Runs

The fixed-addend solve mode used for engineering reproduction is:

```cmd
set "REPO=E:\[About Programming]\[CodeProjects]\C++\NeoAlzette_ARX_CryptoAnalysis_MILP"
set "MSYS=E:\_ABOUT~1\MSYS2"
set "STAMP=YYYYMMDD_HHMMSS"
set "LOG=%REPO%\linear\runs\linear_r1_best_cmd_43200s_%STAMP%.log"
set "EXCLOG=%REPO%\linear\runs\linear_r1_best_cmd_43200s_%STAMP%_exception.log"
set "RESULT=%REPO%\linear\runs\linear_r1_best_cmd_43200s_%STAMP%_result.json"
set "TRACE=%REPO%\linear\runs\linear_r1_best_cmd_43200s_%STAMP%_weight_trace.json"
type nul > "%LOG%"
type nul > "%EXCLOG%"
set "PATH=%MSYS%\mingw64\bin;%MSYS%\usr\bin;%PATH%"
"%REPO%\linear\neoalzette_scip_round_milp_search.exe" --rounds 1 --constant-model fixed-addend-exact-log-weight-milp --time-limit 43200 --output-result-json "%RESULT%" --output-weight-trace-json "%TRACE%" 2>&1 | "%MSYS%\usr\bin\tee.exe" "%LOG%" | "%MSYS%\usr\bin\tee.exe" "%EXCLOG%"
```

Inspect these top-level JSON fields before using a result in a paper:

```text
solver_status
complete
paper_usable_characteristic
objective_weight
weight_trace_available
weight_trace_matches_objective
weight_trace_oracles_valid
```

If `paper_usable_characteristic=false`, the JSON is useful for debugging or
anytime progress only.

## Round Tables

Round tables run best-trail search for `1..R` and refresh the JSON after each
finished prefix:

```cmd
"%REPO%\linear\neoalzette_scip_round_milp_search.exe" --rounds 3 --constant-model fixed-addend-exact-log-weight-milp --output-round-table-json "%REPO%\linear\runs\linear_round_table_1_to_3.json"
```

## Linear Forest Layer / Hull Modes

The hull executable is the linear Forest Layer entry point.  It starts from an
external input-mask source, lets SCIP derive the current output endpoint, records
the signed characteristic, and feeds that output endpoint back as the next input
source for continued Forest growth.  `--time-limit` is the single total
wall-clock budget for discovery, endpoint enumeration, reoptimization, JSON
aggregation, and trace printing.

`--fix-input-ma` and `--fix-input-mb` are optional.  If omitted, both default to
`0x00000001`; the remaining masks are left for the MILP model to evolve.  Use
explicit values only when you want a custom starting source.

Bounded Forest hull with default input masks:

```cmd
"%REPO%\linear\neoalzette_scip_round_hull_search.exe" --rounds 1 --constant-model fixed-addend-exact-log-weight-milp --hull-mode bounded-endpoint --time-limit 43200 --enumerate-window 4 --max-enumerate-solutions 1000 --hull-output-json "%REPO%\linear\runs\linear_hull_r1.json"
```

Bounded Forest hull with a custom input-mask source:

```cmd
"%REPO%\linear\neoalzette_scip_round_hull_search.exe" --rounds 1 --constant-model fixed-addend-exact-log-weight-milp --hull-mode bounded-endpoint --fix-input-ma 0x00000001 --fix-input-mb 0x00000000 --time-limit 43200 --enumerate-window 4 --max-enumerate-solutions 1000 --hull-output-json "%REPO%\linear\runs\linear_hull_r1.json"
```

Complete endpoint mode removes the weight window for the selected endpoint.  It
is exact under the current MILP model only if the endpoint enumeration reaches
infeasibility before time, memory, or solution limits:

```cmd
"%REPO%\linear\neoalzette_scip_round_hull_search.exe" --rounds 1 --constant-model fixed-addend-exact-log-weight-milp --hull-mode complete-endpoint --time-limit 43200 --max-enumerate-solutions 10000 --hull-output-json "%REPO%\linear\runs\linear_complete_endpoint_r1.json"
```

During endpoint enumeration the implementation uses
`LinearHullReoptimizationSession`: one SCIP model is built for the same endpoint
and window, then each accepted semantic characteristic is excluded by one exact
no-good inequality and SCIP is reoptimized.  This is a performance optimization
only; it does not change the four local arithmetic models, the Walsh objective,
the endpoint, or the HULL set being enumerated.

`strong-hull` is parsed as a legacy name but is disabled in this forest Q1 entry
point; the supported run modes are `bounded-endpoint` and `complete-endpoint`.
The hull JSON records signed polynomial terms of the form `coefficient * 2^-W`.
Bounded hulls are partial unless explicitly complete for the endpoint.  Runtime
checkpoint and weight traces are printed by default so long server runs remain
observable from the console log.

## Multi-process HULL campaign runner

For multi-core server campaigns, `hull_multiple_thread_runner.cpp` launches many isolated linear HULL child processes and merges only coverage/candidate statistics.  It does not change the linear MILP model, CLAT logic, fixed-addend model, joint-injection model, signed local-correlation semantics, or Forest Layer output-feedback semantics.

See the sidecar document:

```text
linear/hull_multiple_thread_runner.md
```

The runner supports two campaign modes:

```text
Scheme A: multi-seed Forest exploration.
Scheme B: fixed-input sector / fan-sliced exploration over (mA,mB).
```

Cross-worker HULL correlations are not summed by the runner; use the merged files for coverage and candidate discovery, and use per-worker `result.json` / `characteristics.jsonl` files for local signed HULL evidence.
