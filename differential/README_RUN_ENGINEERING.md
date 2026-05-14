# NeoAlzette Differential MILP Engineering Runner

This directory contains the XOR-differential SCIP C API backend.  It searches
best single differential characteristics, round tables, and endpoint hulls under
the current MILP model.

## Model Boundary

- This is XOR-differential cryptanalysis, not linear cryptanalysis.
- Two-variable modular addition uses the Lipmaa-Moriai feasibility/weight rule
  encoded through the Fu-Wang-Guo-Sun inequality system used by the code.
- Modular subtraction is modeled through the add/sub permutation
  `x - y = z` iff `z + y = x`.
- Fixed-public-constant add/sub uses the exact Miyano/Machado/Azimi recurrence.
- Experimental constant averaging is not exposed by the production CLI;
  engineering runs use the exact fixed-public-constant model.
- Joint injection support is enforced by explicit witness MILP constraints and
  a SCIP `injection_rank` handler that also enforces the affine-derivative rank
  lower bound paid by the objective.

## Files

- `model/neoalzette_scip_operator_analysis_oracle.hpp`
  - NeoAlzette injection value functions, affine-image support/rank oracle, and
    SCIP `injection_rank` handler support.
- `model/neoalzette_scip_operator_analysis_milp_constraint.hpp`
  - Arithmetic differential models, SCIP variables/constraints, and model
    builder utilities.
- `model/neoalzette_scip_search_round_function.hpp`
  - Round construction, solve flow, trace/JSON output, and best-trail runner.
- `neoalzette_scip_round_milp_search.cpp`
  - Best-trail and round-table entry point.
- `neoalzette_scip_round_hull_search.cpp`
  - Endpoint hull entry point using one global hull time budget.
- `rebuild_differential.bat`
  - Windows `cmd.exe` rebuild helper for both differential executables.

## Windows Build

The checked Windows build links against the existing static SCIPOptSuite tree:

```text
E:\_ABOUT~1\_CODEP~1\C__~1\NEOALZ~3\scipoptsuite-build-static-papilo-short-fresh
```

The repository path contains square brackets, so the Windows command line uses
8.3 short paths.  Rebuild the differential executables with:

```cmd
cmd /c differential\rebuild_differential.bat
```

The batch file compiles to `.tmp.exe`, verifies that the temporary file exists
and is nonzero, then replaces the final executable:

```text
differential\neoalzette_scip_round_milp_search.exe
differential\neoalzette_scip_round_hull_search.exe
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
differential executables from the repository root with:

```sh
g++ -O2 -std=c++20 differential/neoalzette_scip_round_milp_search.cpp $(pkg-config --cflags --libs scip) -o differential/neoalzette_scip_round_milp_search
g++ -O2 -std=c++20 differential/neoalzette_scip_round_hull_search.cpp $(pkg-config --cflags --libs scip) -o differential/neoalzette_scip_round_hull_search
```

If `pkg-config --libs scip` is unavailable, link against the local static
SCIPOptSuite build:

```sh
SRC="$PWD/scipoptsuite-10.0.2"
BUILD="$PWD/scipoptsuite-build-linux"
CXXFLAGS="-O2 -std=c++20 -I$SRC/scip/src -I$BUILD/scip -I$SRC/soplex/src -I$BUILD/soplex -I$SRC/papilo/src -I$BUILD/papilo"
LIBS="$BUILD/lib/libscip.a $BUILD/lib/libsoplex.a $BUILD/papilo/libpapilo-core.a -lboost_iostreams -lboost_program_options -lboost_serialization -lgmpxx -lgmp -lmpfr -lz -lbz2 -lreadline -ltbb -lquadmath -lpthread -ldl -lm"

g++ $CXXFLAGS differential/neoalzette_scip_round_milp_search.cpp -o differential/neoalzette_scip_round_milp_search $LIBS
g++ $CXXFLAGS differential/neoalzette_scip_round_hull_search.cpp -o differential/neoalzette_scip_round_hull_search $LIBS
```

If your build emits `lib64/` instead of `lib/`, adjust the two SCIP/SoPlex
archive paths in `LIBS`.

## Best-Trail Runs

The fixed-public exact mode used for engineering reproduction is:

```cmd
set "REPO=E:\[About Programming]\[CodeProjects]\C++\NeoAlzette_ARX_CryptoAnalysis_MILP"
set "MSYS=E:\_ABOUT~1\MSYS2"
set "STAMP=YYYYMMDD_HHMMSS"
set "LOG=%REPO%\differential\runs\diff_r1_best_cmd_43200s_%STAMP%.log"
set "EXCLOG=%REPO%\differential\runs\diff_r1_best_cmd_43200s_%STAMP%_exception.log"
set "RESULT=%REPO%\differential\runs\diff_r1_best_cmd_43200s_%STAMP%_result.json"
set "TRACE=%REPO%\differential\runs\diff_r1_best_cmd_43200s_%STAMP%_weight_trace.json"
type nul > "%LOG%"
type nul > "%EXCLOG%"
set "PATH=%MSYS%\mingw64\bin;%MSYS%\usr\bin;%PATH%"
"%REPO%\differential\neoalzette_scip_round_milp_search.exe" --rounds 1 --constant-model fixed-public-exact --time-limit 43200 --output-result-json "%RESULT%" --output-weight-trace-json "%TRACE%" 2>&1 | "%MSYS%\usr\bin\tee.exe" "%LOG%" | "%MSYS%\usr\bin\tee.exe" "%EXCLOG%"
```

Inspect these top-level JSON fields before using a result in a paper:

```text
solver_status
complete
objective_weight
weight_trace_available
weight_trace_matches_objective
weight_trace_oracles_valid
```

Injection trace entries also expose:

```text
joint_injection.support_source
joint_injection.support_audit_valid
joint_injection.joint_rank
joint_injection.rank_weight
joint_injection.affine_constant
```

Time-limited incumbents are anytime data unless `complete=true` and the solver
status proves optimality for that run.

## Round Tables

Round tables run best-trail search for `1..R` and refresh the JSON after each
finished prefix:

```cmd
"%REPO%\differential\neoalzette_scip_round_milp_search.exe" --rounds 3 --constant-model fixed-public-exact --output-round-table-json "%REPO%\differential\runs\diff_round_table_1_to_3.json"
```

## Differential Hull Modes

The hull executable uses one global `--hull-time-limit`.  All input-difference
attempts, SCIP solves, endpoint enumeration calls, and JSON aggregation consume
that same wall-clock budget.  Forest hull search requires fixed input
differences and the exact fixed-public-constant model.

Bounded endpoint hull:

```cmd
"%REPO%\differential\neoalzette_scip_round_hull_search.exe" --rounds 1 --constant-model fixed-public-exact --hull-mode bounded-endpoint --fix-input-da 0x00000001 --fix-input-db 0x00000000 --hull-time-limit 43200 --enumerate-window 8 --max-enumerate-solutions 1000 --hull-output-json "%REPO%\differential\runs\diff_hull_r1.json"
```

Complete fixed endpoint hull:

```cmd
"%REPO%\differential\neoalzette_scip_round_hull_search.exe" --rounds 1 --constant-model fixed-public-exact --hull-mode complete-endpoint --fix-input-da 0x00000001 --fix-input-db 0x00000000 --fix-output-da 0x12345678 --fix-output-db 0x9abcdef0 --hull-time-limit 43200 --max-enumerate-solutions 10000 --hull-output-json "%REPO%\differential\runs\diff_complete_endpoint_r1.json"
```

`bounded-endpoint` is a threshold-bounded exploration and is a partial lower
bound unless every endpoint enumeration proves complete.  `complete-endpoint`
is exact under the current MILP model only if enumeration reaches infeasibility
before time, memory, or solution limits.

## Multi-process HULL campaign runner

For multi-core server campaigns, `hull_multiple_thread_runner.cpp` launches many isolated differential HULL child processes and merges only coverage/candidate statistics.  It does not change the differential MILP model, CDDT logic, fixed-addend model, joint-injection model, or Forest Layer output-feedback semantics.

See the sidecar document:

```text
differential/hull_multiple_thread_runner.md
```

The runner supports two campaign modes:

```text
Scheme A: multi-seed Forest exploration.
Scheme B: fixed-input sector / fan-sliced exploration over (dA,dB).
```

Cross-worker HULL probabilities are not summed by the runner; use the merged files for coverage and candidate discovery, and use per-worker `result.json` files for local HULL evidence.
