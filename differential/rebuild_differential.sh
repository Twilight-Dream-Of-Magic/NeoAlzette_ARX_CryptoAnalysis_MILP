#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO="${REPO:-$(cd -- "$SCRIPT_DIR/.." && pwd)}"
SRC="${SCIPOPTSUITE_SRC:-$REPO/scipoptsuite-10.0.2}"
BUILD="${SCIPOPTSUITE_BUILD:-$REPO/scipoptsuite-build-linux}"
CXX="${CXX:-g++}"
CXXFLAGS_BASE="${CXXFLAGS_BASE:--O2 -std=c++20 -pthread}"

if [[ ! -d "$SRC" ]]; then
    echo "ERROR: SCIPOptSuite source directory not found: $SRC" >&2
    echo "Extract scipoptsuite-10.0.2.tgz to scipoptsuite-10.0.2, or set SCIPOPTSUITE_SRC." >&2
    exit 1
fi
if [[ ! -d "$BUILD" ]]; then
    echo "ERROR: SCIPOptSuite build directory not found: $BUILD" >&2
    echo "Build SCIPOptSuite first, or set SCIPOPTSUITE_BUILD." >&2
    exit 1
fi

SCIP_LIB="$BUILD/lib/libscip.a"
SOPLEX_LIB="$BUILD/lib/libsoplex.a"
PAPILO_LIB="$BUILD/papilo/libpapilo-core.a"
if [[ ! -f "$SCIP_LIB" && -f "$BUILD/lib64/libscip.a" ]]; then SCIP_LIB="$BUILD/lib64/libscip.a"; fi
if [[ ! -f "$SOPLEX_LIB" && -f "$BUILD/lib64/libsoplex.a" ]]; then SOPLEX_LIB="$BUILD/lib64/libsoplex.a"; fi
if [[ ! -f "$PAPILO_LIB" && -f "$BUILD/lib64/papilo/libpapilo-core.a" ]]; then PAPILO_LIB="$BUILD/lib64/papilo/libpapilo-core.a"; fi

for lib in "$SCIP_LIB" "$SOPLEX_LIB" "$PAPILO_LIB"; do
    if [[ ! -f "$lib" ]]; then
        echo "ERROR: required static library not found: $lib" >&2
        exit 1
    fi
done

CXXFLAGS=( $CXXFLAGS_BASE
    -I"$SRC/scip/src" -I"$BUILD/scip"
    -I"$SRC/soplex/src" -I"$BUILD/soplex"
    -I"$SRC/papilo/src" -I"$BUILD/papilo"
)
LIBS=(
    "$SCIP_LIB" "$SOPLEX_LIB" "$PAPILO_LIB"
    -lboost_iostreams -lboost_program_options -lboost_serialization
    -lgmpxx -lgmp -lmpfr -lz -lbz2 -lreadline -ltbb -lquadmath -lpthread -ldl -lm
)

build_one() {
    local step="$1" total="$2" name="$3" source="$4" output="$5"
    local tmp="${output}.tmp"
    echo
    echo "[$step/$total] ===== Building: $name ====="
    echo "[$step/$total] Source: $source"
    echo "[$step/$total] Output: $output"
    [[ -f "$source" ]] || { echo "ERROR: source file not found: $source" >&2; exit 1; }
    rm -f "$tmp"
    "$CXX" "${CXXFLAGS[@]}" "$source" -o "$tmp" "${LIBS[@]}"
    [[ -s "$tmp" ]] || { echo "ERROR: temporary executable missing or empty: $tmp" >&2; exit 1; }
    mv -f "$tmp" "$output"
    echo "[$step/$total] OK: $name"
}

build_one 1 3 "Differential MILP search" "$REPO/differential/neoalzette_scip_round_milp_search.cpp" "$REPO/differential/neoalzette_scip_round_milp_search"
build_one 2 3 "Differential HULL search" "$REPO/differential/neoalzette_scip_round_hull_search.cpp" "$REPO/differential/neoalzette_scip_round_hull_search"
build_one 3 3 "Differential HULL multi-core runner" "$REPO/differential/hull_multiple_thread_runner.cpp" "$REPO/differential/hull_multiple_thread_runner"

echo
echo "Differential Linux build OK."
