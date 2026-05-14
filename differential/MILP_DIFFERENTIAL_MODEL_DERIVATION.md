# NeoAlzette XOR-Differential MILP Model Derivation

This note documents the current `differential/` SCIP backend and keeps the
math tied to the code that is actually compiled today.  The engineering code is
moving quickly, so this file is intentionally written as a source map as well
as a derivation.

Checked against:

```text
differential/model/neoalzette_scip_operator_analysis_oracle.hpp
differential/model/neoalzette_scip_operator_analysis_milp_constraint.hpp
differential/neoalzette_scip_round_milp_search.cpp
differential/neoalzette_scip_round_hull_search.cpp
neoalzette_core.cpp
```

Current backend summary:

- Backend: SCIP C API.
- Analysis: XOR-differential single-characteristic search, with optional
  endpoint hull enumeration.
- Word size: 32 bits, little-endian bit order.
- Default constant model: `fixed-public-exact`.
- Injection support model: explicit MILP witness circuit for each joint 32-to-64
  injection transition.
- Injection rank/support callback model: SCIP constraint handler named
  `injection_rank`, enforcing `rank_weight >= rank(din)`, adding local affine
  image XOR constraints when `din` is locally fixed, and rejecting impossible
  integral joint transitions.
- The JSON weight trace rechecks the incumbent and verifies that traced weight
  matches the SCIP objective.
- Windows rebuild entry point: `differential/rebuild_differential.bat`.
- Linux builds are documented as direct `g++ ... $(pkg-config --cflags --libs
  scip)` commands in `differential/README_RUN_ENGINEERING.md`.

## Source Map

`model/neoalzette_scip_operator_analysis_oracle.hpp` and
`model/neoalzette_scip_operator_analysis_milp_constraint.hpp` are the arithmetic
modeling boundary:

- two-input modular addition XOR-differential oracle;
- two-input modular subtraction by reduction to addition;
- fixed-public-constant addition/subtraction oracle;
- fixed-public-constant MILP selector model;
- optional zero-difference average comparison model.

`neoalzette_scip_round_milp_search.cpp` and
`neoalzette_scip_round_hull_search.cpp` are the SCIP backend boundaries:

- SCIP variable and linear/XOR constraint construction;
- bit-vector XOR and rotation glue;
- current NeoAlzette round schedule;
- explicit injection support witness circuit and custom local support/rank
  handler;
- branching priorities for injection input/output bits;
- best-trail, round-table, endpoint hull, and JSON weight-trace code.

`neoalzette_core.cpp` is the value-domain implementation that the differential
round schedule must match.

## Arithmetic Differential Models

The arithmetic models are implemented in
`model/neoalzette_scip_operator_analysis_oracle.hpp` and
`model/neoalzette_scip_operator_analysis_milp_constraint.hpp`.

### Two-Input Modular Addition

Local value operation:

```text
z = x + y mod 2^n
```

XOR differences:

```text
alpha = x xor x'
beta  = y xor y'
gamma = z xor z'
```

The local probability is:

```text
DP+(alpha,beta -> gamma)
  = Pr[(x + y) xor ((x xor alpha) + (y xor beta)) = gamma]
```

Define:

```text
t_i = alpha_i xor beta_i xor gamma_i
alpha[-1] = beta[-1] = gamma[-1] = 0
```

The Lipmaa-Moriai feasibility rule used by the code is:

```text
for each bit i:
    if alpha[i-1] = beta[i-1] = gamma[i-1],
    then t_i must equal alpha[i-1].
```

Equivalently, the transition is impossible if some bit has:

```text
alpha[i-1] = beta[i-1] = gamma[i-1]
and
alpha[i-1] != alpha_i xor beta_i xor gamma_i
```

If feasible, the weight is:

```text
W = sum_{i=0}^{n-2} [(alpha_i,beta_i,gamma_i) not in {000,111}]
DP+ = 2^-W
```

Code entry points:

```text
differential_oracle::oracle_add2
arithmetic_model::add_two_input_add_diff
```

The current MILP implementation uses the Fu-Wang-Guo-Sun 13-inequality local
transition system that appears in Speck-style ARX MILP searches.  It does not
use the older compact triple-indicator/quotient formulation.

The least significant bit has zero incoming carry difference, so the code first
emits the XOR equality:

```text
alpha_0 xor beta_0 = gamma_0
```

For each adjacent bit transition `i -> i+1`, `0 <= i <= n-2`, the model creates
one objective bit:

```text
p_i = transition_weight_i
```

The first five inequalities force `p_i` to be the local Lipmaa-Moriai weight
indicator:

```text
p_i = 0  iff  (alpha_i,beta_i,gamma_i) in {000,111}
p_i = 1  otherwise
```

In code these rows are named:

```text
fu_weight_beta_minus_gamma_i
fu_weight_alpha_minus_beta_i
fu_weight_gamma_minus_alpha_i
fu_weight_sum_ge_weight_i
fu_weight_sum_le_three_minus_weight_i
```

The remaining eight rows, named `fu_transition_0_i` through
`fu_transition_7_i`, only bite when `p_i = 0`.  In that case the first five rows
have already forced:

```text
alpha_i = beta_i = gamma_i
```

and the eight transition rows enforce the Lipmaa-Moriai next-bit condition:

```text
alpha_{i+1} xor beta_{i+1} xor gamma_{i+1} = alpha_i
```

The SCIP objective is exactly:

```text
minimize sum_{i=0}^{n-2} transition_weight_i
```

The code also assigns high branch priorities to the transition weights and then
to the carry-chain source/output bits so that SCIP fixes low-level add boxes in
a useful order.

### Two-Input Modular Subtraction

Local value operation:

```text
z = x - y mod 2^n
```

The code uses:

```text
x - y = z  iff  z + y = x
```

Therefore:

```text
DP-(alpha,beta -> gamma) = DP+(gamma,beta -> alpha)
W-(alpha,beta -> gamma)  = W+(gamma,beta -> alpha)
```

Code entry points:

```text
differential_oracle::oracle_sub2
arithmetic_model::add_two_input_sub_diff
```

### Fixed-Public-Constant Addition

Local value operation:

```text
y = x + a mod 2^n
```

Here `a` is a public constant, not a second random variable.  The concrete bits
of `a` affect the probability.

XOR differences:

```text
u = x xor x'
v = y xor y'
```

Probability:

```text
DP_a(u -> v)
  = Pr[(x + a) xor ((x xor u) + a) = v]
```

The code follows the fixed-addend recurrence documented in the header.  It
uses states:

```text
u[-1] = v[-1] = a[-1] = 0
e_i = u_i xor v_i
S_i = (u[i-1], v[i-1], e_i)
```

The state `001` is impossible.  The base objective contributes one unit when:

```text
u[i-1] xor v[i-1] = 1
```

For each chain state `S_i = 11*`, the code computes an exact chain length
`lambda`, a public-constant block value `B`, a numerator `p`, and contributes:

```text
(lambda - 1) - log2(p)
```

If `p = 0`, that selector is forbidden.

Code entry points:

```text
differential_oracle::oracle_add_const
arithmetic_model::add_fixed_public_constant_exact
```

The MILP uses:

```text
e_i = u_i xor v_i
s_{state,i} for state S_i
z_{i,lambda,q} for the exact zero-chain selector
```

The objective has real coefficients of the form `-log2(p)`.  This is why the
fixed-public-constant model is not the same as the compact one-constant-input
average model.

### Fixed-Public-Constant Subtraction

Local value operation:

```text
y = x - a mod 2^n
```

The code reduces it to fixed-public-constant addition with the two's-complement
negative constant:

```text
x - a = x + (-a mod 2^n) mod 2^n
```

Code entry points:

```text
differential_oracle::oracle_sub_const
arithmetic_model::add_fixed_public_constant_sub_exact
```

### Removed Experimental Zero-Difference Average CLI Mode

`arithmetic_model::add_zero_diff_operand_average` remains only as internal
legacy comparison code.  It is not exposed by the production CLI because it
ignores the concrete public constant bits and is not the exact model for
`x + a` or `x - a` with a known public constant.  The rigorous production model
for NeoAlzette engineering runs is:

```text
--constant-model fixed-public-exact
```

## Injection Layer Model

The injection layer is not modeled as independent AND/OR gate probabilities.
The gates share the same 32-bit source word, so multiplying independent local
gate probabilities would be the wrong object.

Each injection is modeled as a joint 32-to-64 differential transition.  The
joint map `H` returns two 32-bit outputs:

```text
low 32 bits:  XOR-output delta target
high 32 bits: modular-add operand delta
```

Support validity is enforced by an explicit witness MILP:

```text
x_prime = x xor din
dout_pair = H(x) xor H(x_prime)
```

The code bit-blasts the exact core-side Boolean expressions for both `H(x)` and
`H(x_prime)` using binary XOR, AND, OR, NOT, zero-filled shifts, and rotations.
The selected output differences are then linearly bound to the witness-derived
differences.  This is the primary support constraint.  In addition, the
`injection_rank` constraint handler derives the same joint affine image and uses
it as a redundant local SCIP constraint: fixed `din` produces local XOR image
constraints, and integral unsupported `(din,dout_pair)` candidates are rejected.

Because the current joint maps are quadratic, the derivative is affine:

```text
D_delta H(x) = H(x) xor H(x xor delta) = M_delta x xor c_delta
```

The local differential rank weight is:

```text
rank(M_delta)
```

This rank is paid once per joint injection via `rank_weight >= rank(M_delta)`.
It is an affine-derivative rank bound, not a substitute for support validity.

Code entry points:

```text
joint_injection_outputs_from_witness_bits
create_joint_injection_differences
InjectionRankOracle::rank
add_injection_rank_constraint
injection_rank_constraint_materialize_local_affine_image
injection_rank_constraint_separate_one
```

### Public XOR Constants

The value-domain implementation xors these public constants into one branch:

```text
B ^= RC[4]
A ^= RC[9]
A ^= RC[10]
B ^= RC[11]
```

In XOR-differential state propagation, public XOR constants do not change the
state difference.  They are therefore represented as zero-difference trace
checkpoints:

```text
after_XOR_RC4
after_XOR_RC9
after_XOR_RC10
after_XOR_RC11
after_FINAL_XOR_RC10_RC11
```

The constants are not part of the joint injection support witness argument.
They also do not change the XOR differences entering the modular additions,
because the affected operand differences are unchanged by public XOR whitening.
The rank audit evaluates:

```text
InjectionRankOracle::function
```

with:

```text
B_TO_A_AFTER_RC4: H_B(x)
A_TO_B_AFTER_RC9: H_A(x)
```

So `RC[4]`, `RC[9]`, `RC[10]`, and `RC[11]` are value-domain constants and trace
checkpoints, but they do not change `M_delta`, `c_delta`, support, or rank for
the current joint injection derivative model.

### Polar Fast Path

For the current quadratic maps, the derivative matrix columns can be computed
from the polar bilinear form:

```text
col_i(M_delta) = B_J(e_i, delta)
               = xor over j with delta_j = 1 of B_J(e_i, e_j)
```

The code precomputes byte tables for this.  A cache miss for a new `delta`
requires one exact `J(delta)` evaluation plus table lookups.

If the injection maps become cubic or higher degree, this fast path and the
affine derivative model must be rederived or disabled.  After any injection
change, add or run a dedicated exhaustive validation tool that compares
`InjectionRankOracle::transition()` against direct evaluation of
`H(x) xor H(x xor delta)` for the affected map.  The current repository does
not expose a standalone command for that validation step, so do not claim a
changed injection layer is supported until such a check exists and passes.

## SCIP Injection Constraint Handler

`create_joint_injection_differences()` creates:

```text
xor_output_delta[32]       binary XOR-output difference variables
add_operand_delta[32]      binary modular-add operand difference variables
support_witness_x[32]      binary support witness
support_witness_x_prime    x xor din
rank_weight               continuous variable in [0,32], objective coefficient 1
```

It bit-blasts `H(x)` and `H(x_prime)`, forms their XOR differences, and binds
those differences to the two modeled 32-bit outputs.  The two outputs are also
packed into one 64-bit vector:

```text
joint_output_bits = xor_output_delta || add_operand_delta
```

Then the code calls:

```text
add_injection_rank_constraint(...)
```

The explicit witness circuit is the primary support model.  The
`injection_rank` handler also receives the packed joint output so it can enforce
the same affine-image support relation locally after `din` is fixed and can
reject unsupported integral candidates during SCIP enforcement/checking.

The handler owns:

```text
input_bits[32]
output_bits[64]
rank_weight
InjectionRankOracle
```

It enforces two things:

```text
joint_output_bits in c_delta + image(M_delta)
rank_weight >= rank(M_delta)
```

The support part is not a post-solve audit.  It is a SCIP constraint handler:
local affine-image XOR constraints are added with `SCIPaddConsLocal`, and
unsupported integral joint transitions are returned as infeasible.

### Integral Support And Rank Enforcement

For integral injection bits, the handler evaluates the joint transition for the
fixed input/output difference:

```text
transition = InjectionRankOracle::transition(kind, din, joint_dout)
```

If `transition.valid == false`, the candidate is infeasible.  Otherwise it
applies:

```text
rank = transition.rank
rank_weight >= rank
```

If this condition fails during enforcement or final candidate checking, the
candidate is infeasible.  No dynamic rank epigraph row is emitted for fractional
LP solutions, because the current engineering goal is strict mathematical prune
without long-run cut/row accumulation.

For separation callbacks on candidate solutions, unsupported pairs are cut by a
local no-good for that exact joint pattern.  For enforcement/checking callbacks,
unsupported pairs are reported infeasible.  No global support big-M rows or
permanent auxiliary variables are emitted.

Code entry points:

```text
injection_rank_constraint_separate_one
injectionRankConsCheck
injection_rank_constraint_tighten_weight_for_fixed_input
injection_rank_constraint_add_local_no_good_pair
```

### Input-Fixed Guard

The value read into `fixed_delta` is meaningful only when:

```text
injection_rank_cons_input_is_locally_fixed(...) == true
```

Current code stores that boolean in:

```text
const bool input_fixed = ...
```

and guards fixed-input weight tightening with it.  This prevents accidentally
treating the default initialized value `fixed_delta = 0` as a real local node
assignment.

### SCIP Callback Behavior

Current handler callbacks:

```text
CONSENFOLP / CONSENFOPS:
    reject unsupported integral joint transitions and enforce the rank epigraph
    for integer LP/pseudo solutions.

CONSSEPALP:
    when input is locally fixed, tighten the local rank lower bound and add
    local XOR affine-image constraints.  It does not add nearest-rounded
    fractional LP rank cuts.

CONSSEPASOL:
    separate a given SCIP solution; unsupported integral joint patterns receive
    local no-goods only.

CONSCHECK:
    for integral joint bits, verify support validity and rank_weight >= rank.

CONSPROP:
    when input is locally fixed, tighten rank_weight lower bound.
```

The handler gives the source difference bits high branching priority and the
injection output bits a lower but still elevated priority:

```text
source/input bits: 200000
output bits:       100000
rank_weight:       no branching priority
```

This helps SCIP fix `din` early enough for local affine image constraints and
rank lower bounds to become active.

## Current Round Schedule

The differential schedule in `build_one_round()` follows the current engineering
construction used by this MILP backend.  It keeps two fixed-public-constant
subtractions and two modular additions, but each injection is a joint transition
that supplies both a cross-branch XOR difference and the corresponding
modular-add operand difference.

Constants that are mixed into the value state are either:

- represented inside the fixed-public-constant add/sub model, for arithmetic
  constants; or
- represented as zero-difference checkpoints for target-branch public XOR
  constants `RC[4]`, `RC[9]`, `RC[10]`, and `RC[11]`.

One round is currently:

```text
start: A0, B0

B1 = B0 - RC[1]                      // fixed-public-constant subtraction

J0_xor, J0_add = D H_B(B1)           // explicit witness MILP support
A1 = A0 xor J0_xor

A2 = A1 + J0_add                     // two-input add differential, RC[0] no diff

B2 = B1 xor rotl(A2,22)              // bridge line 0, zero weight

trace checkpoint after_XOR_RC4       // B ^= RC[4], no XOR-difference update

A3 = A2 xor rotl(B2,13)              // bridge line 1, zero weight

A4 = A3 - RC[6]                      // fixed-public-constant subtraction

J1_xor, J1_add = D H_A(A4)           // explicit witness MILP support
B3 = B2 xor J1_xor

B4 = B3 + J1_add                     // two-input add differential, RC[5] no diff

A5 = A4 xor rotl(B4,5)               // second bridge family, zero weight

trace checkpoint after_XOR_RC9       // A ^= RC[9], no XOR-difference update

B5 = B4 xor rotl(A5,25)              // final feedback bridge, zero weight

trace checkpoints after_XOR_RC10/11  // final whitening, no XOR-difference update

end: A5, B5
```

The final value-domain whitening:

```text
A ^= RC[10]
B ^= RC[11]
```

does not change XOR differences, so the current differential model ends at
`A5,B5`.

Trace stages emitted by the code include:

```text
start
after_CONST_SUB_RC1
after_JOINT_INJECTION_B_to_A
after_ADD0_A_plus_joint_operand
after_BRIDGE0_A_to_B
after_XOR_RC4
after_BRIDGE1_B_to_A
after_CONST_SUB_RC6
after_JOINT_INJECTION_A_to_B
after_ADD1_B_plus_joint_operand
after_LINEAR_B_to_A_ROT5
after_XOR_RC9
after_FINAL_BRIDGE_A_to_B_ROT25
after_XOR_RC10
after_XOR_RC11
after_FINAL_XOR_RC10_RC11
end
```

The weight steps include:

```text
CONST_SUB_RC1
JOINT_INJECTION_B_to_A
ADD0_A_plus_joint_operand
BRIDGE0_A_to_B_linear_xor
XOR_RC4_B
BRIDGE1_B_to_A_linear_xor
CONST_SUB_RC6
JOINT_INJECTION_A_to_B
ADD1_B_plus_joint_operand
LINEAR_B_to_A_ROT5
XOR_RC9_A
FINAL_BRIDGE_A_to_B_ROT25
FINAL_XOR_RC10_A
FINAL_XOR_RC11_B
```

Both bridge steps are zero-weight linear XOR constraints.  Both injection steps
have `rank_weight` variables in the SCIP objective.

## Search Model Boundary

The best-trail model always requires a nonzero external input difference:

```text
sum bits(dA_in,dB_in) >= 1
```

Optional endpoint fixing is available with:

```text
--fix-input-da
--fix-input-db
--fix-output-da
--fix-output-db
```

The optimized quantity is a single characteristic weight under the current
MILP model.  Endpoint hull modes enumerate multiple characteristics for
one endpoint or for selected endpoints, but a time-limited or solution-limited
hull is a partial lower-bound aggregation, not a complete endpoint hull.

## Weight Trace Contract

Every operation that contributes to the objective records a `WeightStepSpec`.
`collect_weight_trace()` reads the incumbent solution and reconstructs:

```text
local_weight
cumulative_weight
selected_objective_terms
```

For injection steps it additionally records:

```text
joint_injection.joint_input_delta
joint_injection.xor_output_delta
joint_injection.add_operand_delta
joint_injection.joint_output_delta
joint_injection.support_source
joint_injection.support_audit_valid
joint_injection.joint_rank
joint_injection.rank_weight
joint_injection.affine_constant
```

The code explicitly throws if an injection step is missing its rank objective
term from the recorded objective-term slice.  This is the guard that prevents
the injection rank weight from silently disappearing from the trace.

Top-level JSON fields to inspect before using a result:

```text
solver_status
complete
objective_weight
weight_trace_available
weight_trace_total
weight_trace_objective_delta
weight_trace_matches_objective
```

For time-limited best-trail runs, an incumbent can still be written.  It should
be treated as an anytime incumbent unless `complete=true` and SCIP status proves
optimality for that run.

## Smoke Command

A one-round SCIP smoke run should write both result JSON and weight-trace JSON:

```cmd
differential\neoalzette_scip_round_milp_search.exe ^
    --rounds 1 ^
    --constant-model fixed-public-exact ^
    --time-limit 120 ^
    --output-result-json _smoke_5min\diff_r1_smoke_result.json ^
    --output-weight-trace-json _smoke_5min\diff_r1_smoke_weight_trace.json
```

Endpoint hull enumeration uses
`differential\neoalzette_scip_round_hull_search.exe` and requires a finite
`--hull-time-limit` plus fixed input differences.  The Windows batch build and
Linux compile commands are in:

```text
differential/README_RUN_ENGINEERING.md
```

## Bibliographic Anchors

The detailed citations live in the header comments of
`model/neoalzette_scip_operator_analysis_oracle.hpp` and
`model/neoalzette_scip_operator_analysis_milp_constraint.hpp`.  The code
currently relies on:

- Lipmaa and Moriai for two-variable modular-addition XOR-differential
  feasibility and weight.
- Wallen-style add/sub variable permutation for modular subtraction.
- Miyano, Machado, and Azimi et al. for fixed-addend modular addition.
- Bagherzadeh and Ahmadian only for the optional compact one-constant-input
  comparison model, not for the exact fixed-public-constant model.

## Maintenance Rule

When the engineering code changes, update this document in the same patch if
any of these move:

- the `build_one_round()` operation order;
- the fixed-public-constant arithmetic model;
- the injection value functions or their quadratic precondition;
- the SCIP `injection_rank` callbacks;
- the weight-trace accounting contract;
- command-line options that affect model semantics.
