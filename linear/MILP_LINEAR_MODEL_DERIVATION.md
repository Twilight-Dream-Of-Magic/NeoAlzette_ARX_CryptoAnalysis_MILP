# NeoAlzette Linear MILP Model Derivation

This document is the audit map for the `linear/` SCIP backend.  The C++ code
is the source of truth; this file explains the mathematics implemented by that
code and names the functions that emit each MILP/CIP component.

The linear backend is Walsh-correlation only.  Do not import XOR-differential
witness variables or derivative wording from `differential/`.

## Directory Contract

```text
linear/
|-- MILP_LINEAR_MODEL_DERIVATION.md        # this derivation and source map
|-- README_RUN_ENGINEERING.md              # build/run notes
|-- rebuild_linear.bat                     # Windows cmd.exe rebuild helper
|-- neoalzette_scip_round_milp_search.cpp  # best trail and round tables
|-- neoalzette_scip_round_hull_search.cpp  # endpoint hull enumeration
`-- model/
    |-- neoalzette_scip_operator_analysis_oracle.hpp
    |-- neoalzette_scip_operator_analysis_milp_constraint.hpp
    `-- neoalzette_scip_search_round_function.hpp
```

The paper entry point for the local NeoAlzette specification is:

```text
NeoAlzette_ARX_box_Specification_Version_6_5/iacrdoc.tex
```

The references used below are the references cited in the code comments and in
that paper source tree.

## Model Boundary

- Masks are little-endian: bit 0 is the least significant bit.
- The objective is one signed linear characteristic weight.
- Linear bijections and XOR bridges are propagated by transpose relations.
- Two-variable modular addition/subtraction uses the Wallen condition encoded
  by Fu-Wang-Guo's eight-inequality MILP transition model.
- Fixed-public-addend add/sub uses Miyano's exact signed two-state transfer
  recurrence, an exact numerator MILP, and a SCIP epigraph handler for the
  exact log weight.
- Injection constraints use quadratic Walsh support/rank for
  `alpha.x xor beta.J(x)`.
- No differential witness input `x` is introduced here.
- Hull modes enumerate characteristics for fixed endpoints and aggregate signed
  contributions.  Supported executable modes are `bounded-endpoint` and
  `complete-endpoint`; `strong-hull` is parsed as a legacy value but rejected
  before solving in the current Q1 forest entry point.

## Two-Variable Modular Addition

For the value operation

```text
Z = X + Y mod 2^n
```

the local masks are:

```text
u : output mask on Z
v : input mask on X
w : input mask on Y
```

The signed correlation is

```text
C_+(u,v,w) =
  2^(-2n) sum_{X,Y} (-1)^(u.(X+Y) xor v.X xor w.Y)
```

The oracle follows Wallen's common-prefix/carry-mask condition.  In the local
notation used by the implementation, the nonzero-correlation weight mask `z`
can be written as:

```text
z[n-1] = 0
z[n-2] = u[n-1] xor v[n-1] xor w[n-1]
z[j]   = z[j+1] xor u[j+1] xor v[j+1] xor w[j+1], 0 <= j <= n-3
```

with nonzero conditions:

```text
z[i] >= u[i] xor v[i]
z[i] >= u[i] xor w[i]
```

If the transition is nonzero, the characteristic magnitude is `2^-sum(z)`;
the sign is computed by the oracle and recorded in the trace.

Code entry points:

```text
linear_oracle::oracle_add2
arithmetic_model::add_two_input_modular_addition_linear_characteristic_constraints
arithmetic_model::add_fu_wang_guo_linear_addition_transition_constraints
```

The SCIP model does not enumerate carry paths.  It creates one binary weight
bit per mask bit, fixes the most significant weight bit to zero, and uses a
free lower-boundary state.  For every bit it emits Fu-Wang-Guo's eight
transition inequalities over:

```text
(next_state, output_mask_bit, first_input_mask_bit, second_input_mask_bit, current_state)
```

The objective contribution is:

```text
minimize sum_i weight_bits[i]
```

The free lower-boundary state is intentional.  Fixing it to zero would remove
valid perfect approximations, including one-bit all-one addition masks.

## Two-Variable Modular Subtraction

For

```text
Z = X - Y mod 2^n
```

the code uses:

```text
X - Y = Z  iff  Z + Y = X
```

Therefore:

```text
C_-(u,v,w) = C_+(v,u,w)
```

Code entry points:

```text
linear_oracle::oracle_sub2
arithmetic_model::add_two_input_modular_subtraction_linear_characteristic_constraints
```

The MILP is the same addition box with the output and first-input mask roles
permuted.

## Fixed-Public-Constant Addition And Subtraction

For

```text
Y = X + K mod 2^n
```

the masks are:

```text
a : input mask on X
b : output mask on Y
K : public fixed addend
```

Miyano's fixed-addend LAP is a signed two-state transfer recurrence.  In the
notation used by the C++ comments:

```text
S_0 = (1,0)
S_{i+1} = S_i M_{K_i,a_i,b_i}
N = S_n[0] + S_n[1]
C_K(a,b) = 2^-n N
W = -log2 |C_K(a,b)| = n - log2 |N|
```

The active strict model is not the older `min sum q_i` carry-state proxy.
The function

```text
arithmetic_model::add_fixed_public_constant_linear_exact_numerator_threshold_constraints
```

compiles the signed transfer recurrence into ordinary SCIP constraints:

- four binary selectors per bit for `(a_i,b_i) in {00,01,10,11}`;
- two continuous scaled state components for every bit boundary;
- selector-state product linearizations for the signed transfer matrix;
- highest-active-bit equality constraints, including the all-zero case;
- a final signed numerator variable `N`;
- a sign selector and exact threshold constraint `|N| >= threshold_numerator`.

The round builder passes `threshold_numerator = 1`, so every chosen transition
has nonzero correlation.  The function

```text
add_fixed_addend_exact_log_weight_milp_objective
```

then creates:

```text
A = |N|                         integer, 1 <= A <= 2^n
W = n - log2(A)                 continuous objective variable
```

and installs the SCIP constraint handler:

```text
fixed_addend_exact_log_weight
```

That handler enforces the convex epigraph of `f(A)=n-log2(A)` lazily with
tangent cuts:

```text
W >= f(a) + f'(a)(A-a)
```

At an integral numerator value, the tangent at that value is exact.  Since SCIP
minimizes `W`, the selected fixed-addend transition is charged the exact
Miyano log weight in one solve.  The exact oracle remains an offline validator
and trace metadata source; it is not a pricing oracle during solving.

Fixed subtraction is reduced to fixed addition by two's complement:

```text
X - C = X + (-C mod 2^n)
```

Code entry points:

```text
linear_oracle::oracle_add_const
linear_oracle::oracle_sub_const
arithmetic_model::add_fixed_public_constant_linear_exact_numerator_threshold_constraints
arithmetic_model::add_fixed_public_constant_subtraction_linear_exact_numerator_threshold_constraints
add_fixed_addend_exact_log_weight_milp_objective
```

## Linear Layers And Round Schedule

For linear bijections and XOR layers, masks propagate through the transpose of
the value-domain linear map.  Rotations reverse direction on masks:

```text
mask(rotl(x,r)) = rotr(mask,r)
mask(rotr(x,r)) = rotl(mask,r)
```

The value-domain source of truth is `NeoAlzetteCore::forward()` in
`neoalzette_core.cpp`.  The linear round builder in
`model/neoalzette_scip_search_round_function.hpp` follows that order.

One current core round is:

```text
# First subround
B -= RC[1]

# B-to-A nonlinear injection block, evaluated from current B
(C0,D0) = cd_injection_from_B(B)
CD0 = (C0 << 2) xor (D0 >> 2)
CD1 = (C0 >> 5) xor (D0 << 5)
A ^= rotl(B,24) xor rotl(C0,16) xor rotl(B,8)
A += rotl(CD0,31) xor rotl(CD1,17) xor RC[0]

# first cross-branch bridge
B ^= rotl(A,22) xor RC[4]
A ^= rotl(B,13)

# Second subround
A -= RC[6]

# A-to-B nonlinear injection block, evaluated from current A
(C1,D1) = cd_injection_from_A(A)
CD2 = (C1 >> 3) xor (D1 << 3)
CD3 = (C1 << 1) xor (D1 >> 1)
B ^= rotr(A,24) xor rotr(D1,16) xor rotr(A,8)
B += CD2 xor CD3 xor RC[5]

# second cross-branch bridge
A ^= rotl(B,5) xor RC[9]
B ^= rotl(A,25)

# final whitening constants
A ^= RC[10]
B ^= RC[11]
```

The round builder emits trace stages in this order:

```text
CONST_SUB_B_RC1
INJECTION_B_TO_A_JOINT
ADD_A_WITH_B_TO_A_ADDEND_RC0
XOR_B_WITH_ROTL_A_22_RC4
XOR_A_WITH_ROTL_B_13
CONST_SUB_A_RC6
INJECTION_A_TO_B_JOINT
ADD_B_WITH_A_TO_B_ADDEND_RC5
XOR_A_WITH_ROTL_B_5_RC9
XOR_B_WITH_ROTL_A_25
FINAL_XOR_CONSTANTS_RC10_RC11
```

The fixed subtract checkpoints are modeled as fixed-addend additions:

```text
B_after_sub = B_before_sub + (-RC[1] mod 2^32)
A_after_sub = A_before_sub + (-RC[6] mod 2^32)
```

Public XOR constants have zero linear weight and only contribute sign phases.

## Injection Layer

The injected maps are treated as quadratic vectorial Boolean functions.  For a
linear transition

```text
alpha on input x
beta  on joint output J(x)
```

the oracle evaluates the Walsh transform of:

```text
alpha.x xor beta.J(x)
```

The packed `beta` is 64 bits:

```text
low  32 bits : mask on the XOR-output side
high 32 bits : mask on the injected modular-add operand
```

For a quadratic Boolean scalar, nonzero Walsh magnitude is determined by the
rank of the quadratic part.  The SCIP model installs one custom
`LinearInjectionWalshConstraint(alpha,beta,weight)` per injection:

- `alpha` must lie in the Walsh support of `beta.J`;
- `weight >= rank(beta.J) / 2`;
- unsupported fixed `beta` values are excluded;
- unsupported `alpha` values for a fixed `beta` are cut by support parity rows;
- the rank lower bound is added through conditional epigraph cuts.

Code entry points:

```text
LinearInjectionWalshOracle::transition
add_linear_injection_walsh_constraint
linearInjectionWalshConsCheck
linear_injection_walsh_constraint_separate_one
```

The oracle in the JSON trace verifies support, rank, sign, and local weight.
It does not replace the SCIP constraint handler.

## Trace Contract

Every solved incumbent records:

- operation name and round;
- before/after A/B masks;
- local input/output masks;
- public constants and effective constants;
- selected objective-term slice;
- local and cumulative weight;
- local sign metadata;
- fixed-addend numerator/log-weight metadata when applicable;
- injection support/rank/weight metadata when applicable.

Important JSON fields before using a result as a paper claim:

```text
solver_status
complete
paper_usable_characteristic
objective_weight
weight_trace_available
weight_trace_matches_objective
weight_trace_oracles_valid
```

Time-limited incumbents can be written for debugging and anytime progress.
They are final claims only when the solver status and completeness fields prove
the optimum for that run.

## Forest Layer And Endpoint Hull Aggregation

The linear hull executable is a Forest Layer driver, not a one-shot endpoint-only
script.  The current input-mask source is solved through the NeoAlzette Walsh
MILP, the resulting output endpoint is recorded, and that output endpoint can be
fed back as the next input source for continued weak-path growth until the
single global `--time-limit` expires.

The external input-mask options are optional:

```text
--fix-input-ma default 0x00000001
--fix-input-mb default 0x00000001
```

Omitting them leaves the remaining masks to the MILP constraints and Forest
continuation logic.

For each selected endpoint/window, the executable enumerates semantic linear
characteristics and accumulates:

```text
sum_char sign(char) * 2^(-weight(char))
```

The implementation class `LinearHullReoptimizationSession` owns one persistent
SCIP model for that selected endpoint/window.  After a characteristic is accepted
and verified by Q1/oracle trace checks, the session adds exactly one semantic
no-good inequality and calls SCIP reoptimization.  The feasible set changes only
by removing the already-counted characteristic:

```text
S_{k+1} = S_k \ { characteristic_k }
```

No local arithmetic model is changed by this optimization.  Two-variable add/sub
still uses the Wallen/Fu-Wang-Guo model and CLAT/Q1 checks; fixed-constant
add/sub still uses the exact Miyano signed transfer model.

The JSON records both floating summaries and polynomial terms:

```text
signed_correlation_sum
abs_correlation_sum
signed_polynomial_terms = [{ coefficient: c_W, basis: 2^-W }]
```

`bounded-endpoint` restricts enumeration to a weight window and is a partial
signed sum unless the endpoint enumeration proves complete.  `complete-endpoint`
removes the weight window and is exact under the current MILP model only if
enumeration reaches infeasibility before time, memory, or solution limits.

`strong-hull` remains a legacy parser value in the linear hull executable, but
the current forest Q1 entry point rejects it before solving because Q2 endpoint
candidate generation is not enabled here.

## Build And Run Pointers

Windows rebuild entry point:

```cmd
cmd /c linear\rebuild_linear.bat
```

Linux compile commands and SCIPOptSuite library build notes are in:

```text
README_BUILD.md
linear/README_RUN_ENGINEERING.md
```

The executable help text is the source of truth for CLI options:

```cmd
linear\neoalzette_scip_round_milp_search.exe --help
linear\neoalzette_scip_round_hull_search.exe --help
```

## References

- Johan Wallen, "Linear Approximations of Addition Modulo 2^n", FSE 2003,
  LNCS 2887, pp. 261-273, DOI 10.1007/978-3-540-39887-5_20.
- Kai Fu, Meiqin Wang, Yinghua Guo, Siwei Sun, and Lei Hu, "MILP-Based
  Automatic Search Algorithms for Differential and Linear Trails for Speck",
  FSE 2016, LNCS 9783, pp. 268-288.
- Hiroshi Miyano, "Addend Dependency of Differential/Linear Probability of
  Addition", IEICE Transactions on Fundamentals E81-A(1), pp. 106-109,
  January 1998.

The joint injection Walsh support/rank logic is implemented directly from the
quadratic Boolean representation of the current NeoAlzette injection maps.  If
those maps stop being quadratic, both the oracle and the SCIP constraint
handler must be rederived before this document remains valid.
