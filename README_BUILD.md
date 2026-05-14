# NeoAlzette MILP Build And Reproduction Notes

This repository contains two SCIP C API MILP backends for NeoAlzette:

- `linear/`: Walsh-correlation single-characteristic search and endpoint hulls.
- `differential/`: XOR-differential single-characteristic search and endpoint
  hulls.

The model derivations are in:

```text
linear/MILP_LINEAR_MODEL_DERIVATION.md
differential/MILP_DIFFERENTIAL_MODEL_DERIVATION.md
```

The engineering run notes are in:

```text
linear/README_RUN_ENGINEERING.md
differential/README_RUN_ENGINEERING.md
```

Do not edit, delete, clean, or regenerate anything under:

```text
linear/runs/**
differential/runs/**
```

while doing documentation or code-comment work.

## Paper Source

The current paper entry point in this checkout is:

```text
NeoAlzette_ARX_box_Specification_Version_6_5/iacrdoc.tex
```

Build from that directory so `iacrtrans.cls` and the bibliography are in scope:

```powershell
Set-Location -LiteralPath 'E:\[About Programming]\[CodeProjects]\C++\NeoAlzette_ARX_CryptoAnalysis_MILP\NeoAlzette_ARX_box_Specification_Version_6_5'
latexmk -pdf -interaction=nonstopmode iacrdoc.tex
```

If `latexmk` is unavailable:

```powershell
pdflatex -interaction=nonstopmode iacrdoc.tex
pdflatex -interaction=nonstopmode iacrdoc.tex
```

## Build SCIPOptSuite First

The MILP programs link to SCIP, SoPlex, and PaPILO.  Building only the repository executables is not enough on a fresh machine;
the solver libraries must exist first.

The local source tree expected by the build commands is:

```text
scipoptsuite-10.0.2/
```

If the checkout only contains `scipoptsuite-10.0.2.tgz`, extract it first from
the repository root:

```sh
tar -xzf scipoptsuite-10.0.2.tgz
```

The local SCIPOptSuite README documents the same CMake entry point and the
`AUTOBUILD=ON` option.

### Windows Solver Library

The checked Windows setup uses MSYS2 MinGW64 and a static SCIPOptSuite build
under the repository.  The batch rebuild scripts assume these short 8.3 paths:

```text
REPO=E:\_ABOUT~1\_CODEP~1\C__~1\NEOALZ~3
MSYS=E:\_ABOUT~1\MSYS2
SRC=%REPO%\SCIPOP~1.2
BUILD=%REPO%\scipoptsuite-build-static-papilo-short-fresh
```

Install the MSYS2 MinGW64 toolchain and libraries if they are missing.  From an
MSYS2 shell this is the expected package shape:

```sh
pacman -S --needed \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-boost \
  mingw-w64-x86_64-gmp \
  mingw-w64-x86_64-mpfr \
  mingw-w64-x86_64-zlib \
  mingw-w64-x86_64-bzip2 \
  mingw-w64-x86_64-readline \
  mingw-w64-x86_64-tbb
```

Configure and build the static SCIPOptSuite library from `cmd.exe`:

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

### Linux Solver Library

On Debian or Ubuntu, install the build tools and the dependency shape used by
the static link commands below:

```sh
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  flex \
  bison \
  libboost-iostreams-dev \
  libboost-program-options-dev \
  libboost-serialization-dev \
  libgmp-dev \
  libmpfr-dev \
  zlib1g-dev \
  libbz2-dev \
  libreadline-dev \
  libtbb-dev
```

Build SCIPOptSuite from the repository root:

```sh
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

`AUTOBUILD=ON` lets SCIPOptSuite disable optional packages it cannot find.  The
commands above keep SoPlex and PaPILO enabled because the repository link lines
expect them.

## Build Repository Executables

### Windows Executables

Windows rebuild entry points are plain `cmd.exe` batch scripts.  They compile
to `.tmp.exe`, verify the temporary executable exists and is nonzero, then move
it to the final path.

```cmd
cmd /c linear\rebuild_linear.bat
cmd /c differential\rebuild_differential.bat
```

Each batch script now builds the multi-core runner next to the two SCIP search
executables.  The runner is not a shell wrapper; it is a C++ parent process that
uses worker threads to create OS child processes of itself, and each child calls
the HULL API internally.

Expected outputs:

```text
linear\neoalzette_scip_round_milp_search.exe
linear\neoalzette_scip_round_hull_search.exe
linear\hull_multiple_thread_runner.exe
differential\neoalzette_scip_round_milp_search.exe
differential\neoalzette_scip_round_hull_search.exe
differential\hull_multiple_thread_runner.exe
```

### Linux Executables With pkg-config

If your system SCIP installation provides `pkg-config` metadata, the shortest
build is:

```sh
g++ -O2 -std=c++20 linear/neoalzette_scip_round_milp_search.cpp $(pkg-config --cflags --libs scip) -o linear/neoalzette_scip_round_milp_search
g++ -O2 -std=c++20 linear/neoalzette_scip_round_hull_search.cpp $(pkg-config --cflags --libs scip) -o linear/neoalzette_scip_round_hull_search
g++ -O2 -std=c++20 -pthread linear/hull_multiple_thread_runner.cpp $(pkg-config --cflags --libs scip) -o linear/hull_multiple_thread_runner
g++ -O2 -std=c++20 differential/neoalzette_scip_round_milp_search.cpp $(pkg-config --cflags --libs scip) -o differential/neoalzette_scip_round_milp_search
g++ -O2 -std=c++20 differential/neoalzette_scip_round_hull_search.cpp $(pkg-config --cflags --libs scip) -o differential/neoalzette_scip_round_hull_search
g++ -O2 -std=c++20 -pthread differential/hull_multiple_thread_runner.cpp $(pkg-config --cflags --libs scip) -o differential/hull_multiple_thread_runner
```

The local SCIPOptSuite source build does not need to provide `scip.pc`; use the
manual static link below when `pkg-config --libs scip` is unavailable.

### Linux Executables With The Local Static Build

From the repository root:

```sh
SRC="$PWD/scipoptsuite-10.0.2"
BUILD="$PWD/scipoptsuite-build-linux"
CXXFLAGS="-O2 -std=c++20 -I$SRC/scip/src -I$BUILD/scip -I$SRC/soplex/src -I$BUILD/soplex -I$SRC/papilo/src -I$BUILD/papilo"
LIBS="$BUILD/lib/libscip.a $BUILD/lib/libsoplex.a $BUILD/papilo/libpapilo-core.a -lboost_iostreams -lboost_program_options -lboost_serialization -lgmpxx -lgmp -lmpfr -lz -lbz2 -lreadline -ltbb -lquadmath -lpthread -ldl -lm"

g++ $CXXFLAGS linear/neoalzette_scip_round_milp_search.cpp -o linear/neoalzette_scip_round_milp_search $LIBS
g++ $CXXFLAGS linear/neoalzette_scip_round_hull_search.cpp -o linear/neoalzette_scip_round_hull_search $LIBS
g++ $CXXFLAGS -pthread linear/hull_multiple_thread_runner.cpp -o linear/hull_multiple_thread_runner $LIBS
g++ $CXXFLAGS differential/neoalzette_scip_round_milp_search.cpp -o differential/neoalzette_scip_round_milp_search $LIBS
g++ $CXXFLAGS differential/neoalzette_scip_round_hull_search.cpp -o differential/neoalzette_scip_round_hull_search $LIBS
g++ $CXXFLAGS -pthread differential/hull_multiple_thread_runner.cpp -o differential/hull_multiple_thread_runner $LIBS
```

If your distro or CMake version emits `lib64/` instead of `lib/`, adjust the
two archive paths in `LIBS`.

### Linux Executables With Repository Scripts

The Linux rebuild scripts mirror the Windows batch scripts and build three
executables per analysis family: single-trail MILP, Forest/HULL, and the
multi-core server runner.  They assume SCIPOptSuite has already been extracted
and built as described above.

```sh
./linear/rebuild_linear.sh
./differential/rebuild_differential.sh
```

Optional environment variables:

```sh
SCIPOPTSUITE_SRC=/path/to/scipoptsuite-10.0.2 \
SCIPOPTSUITE_BUILD=/path/to/scipoptsuite-build-linux \
CXX=clang++ \
./linear/rebuild_linear.sh
```

Expected Linux outputs:

```text
linear/neoalzette_scip_round_milp_search
linear/neoalzette_scip_round_hull_search
linear/hull_multiple_thread_runner
differential/neoalzette_scip_round_milp_search
differential/neoalzette_scip_round_hull_search
differential/hull_multiple_thread_runner
```

## CLI Smoke Checks

After building, check the option surface:

```cmd
linear\neoalzette_scip_round_milp_search.exe --help
linear\neoalzette_scip_round_hull_search.exe --help
linear\hull_multiple_thread_runner.exe --help
differential\neoalzette_scip_round_milp_search.exe --help
differential\neoalzette_scip_round_hull_search.exe --help
differential\hull_multiple_thread_runner.exe --help
```

Linux paths are the same without `.exe`.

## 43200-Second Result Reproduction

These commands intentionally write under `runs/`.  Do not run them while only
editing documentation or comments.

### Differential

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

### Linear

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

## Result Use

- Best-trail JSON is paper-ready only when `complete=true` and the solver
  status proves optimality for that run.
- Linear hull JSON is exact only for endpoints whose characteristic enumeration
  reaches infeasibility before the time, memory, or solution cap.
- Differential hull JSON follows the same endpoint-completeness rule.
- Time-limited incumbents are useful engineering data, not final numerical
  claims by themselves.
