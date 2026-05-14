# NeoAlzette ARX CryptoAnalysis MILP

This repository contains reproducible SCIP C API MILP models for NeoAlzette
ARX cryptanalysis.

There are two independent backends:

- `linear/`: Walsh-correlation search for best signed linear characteristics
  and fixed-endpoint linear hull aggregation.
- `differential/`: XOR-differential search for best characteristics and
  fixed-endpoint differential hull aggregation.

The C++ code is the source of truth.  The Markdown derivations explain the
current code and cite the same theory sources used by the code comments.

## Start Here

Read these files in order:

```text
README_BUILD.md
linear/MILP_LINEAR_MODEL_DERIVATION.md
differential/MILP_DIFFERENTIAL_MODEL_DERIVATION.md
linear/README_RUN_ENGINEERING.md
differential/README_RUN_ENGINEERING.md
```

The local NeoAlzette paper source entry point is:

```text
NeoAlzette_ARX_box_Specification_Version_6_5/iacrdoc.tex
```

Do not edit, clean, or regenerate:

```text
linear/runs/**
differential/runs/**
```

unless the task is explicitly to reproduce or refresh run artifacts.

## Repository Layout

```text
linear/                         linear/Walsh SCIP backend
differential/                   XOR-differential SCIP backend
NeoAlzette_ARX_box_Specification_Version_6_5/
                                local paper source and bibliography
scipoptsuite-10.0.2/            SCIPOptSuite source tree
scipoptsuite-build-static-papilo-short-fresh/
                                existing Windows static solver build
README_BUILD.md                 Windows/Linux build and reproduction notes
```

## Build Overview

Build SCIPOptSuite first, then build the repository executables.  Fresh
machines need the solver libraries; compiling only the `.cpp` entry points is
not enough.

Windows rebuilds use batch files:

```cmd
cmd /c linear\rebuild_linear.bat
cmd /c differential\rebuild_differential.bat
```

Linux build commands, including a SCIPOptSuite CMake build and manual static
link commands for all four executables, are in `README_BUILD.md`.

Expected executables:

```text
linear/neoalzette_scip_round_milp_search(.exe)
linear/neoalzette_scip_round_hull_search(.exe)
differential/neoalzette_scip_round_milp_search(.exe)
differential/neoalzette_scip_round_hull_search(.exe)
```

After building, run each program with `--help` and compare options against the
engineering READMEs before starting long runs.

## Model Summary

The linear backend models:

- two-variable modular addition/subtraction with Wallen/Fu-Wang-Guo
  constraints;
- fixed-public-addend add/sub with Miyano's exact signed transfer recurrence
  and a SCIP exact log-weight epigraph handler;
- joint quadratic injection Walsh support/rank constraints.

The differential backend models:

- two-variable modular addition with the Lipmaa-Moriai feasibility and weight
  rule encoded by the Fu-Wang-Guo-Sun 13-inequality system;
- fixed-public-constant add/sub with the exact Miyano/Machado/Azimi recurrence;
- joint injection support by explicit witness MILP plus an affine-derivative
  rank constraint handler.

## Result Discipline

Best-trail JSON is paper-ready only when `complete=true` and the SCIP status
proves optimality.  Time-limited incumbents are engineering progress data, not
final numerical claims by themselves.

Hull JSON is exact only for endpoints whose enumeration reaches infeasibility
before time, memory, or solution limits.  Bounded hull modes are partial unless
the JSON explicitly proves endpoint completeness.

When code changes, update the matching derivation and engineering README in
the same patch if the change affects model semantics, trace fields, command
options, or reproducibility.
