// ============================================================================
// NeoAlzette XOR-Differential Round MILP Search -- SCIP C API backend
// ============================================================================
//
// Scope of this source file
// ----------------------------------------------------------------------------
// This is the open-source / non-Gurobi companion backend for the NeoAlzette
// XOR-differential MILP artifact.  It builds the same mathematical model as the
// Gurobi backend, but uses SCIP through its C API.
//
//   * This is XOR-differential cryptanalysis, not linear cryptanalysis.
//   * This is a MILP/CIP backend, not a SAT/SMT bit-vector input file.
//   * This is not a division-property / integral model.
//   * The optimized object is a single characteristic unless bounded hull
//     enumeration is explicitly requested.
//
// Solver architecture
// ----------------------------------------------------------------------------
// Static XOR-differential constraints are added through SCIPcreateVarBasic and
// SCIPcreateConsBasicLinear.  Injection support validity is modeled explicitly:
// each injection creates a witness word x, bit-blasts H(x) and H(x xor delta),
// and constrains the two 32-bit output differences from their XOR.  A small
// SCIP constraint handler also enforces the same joint affine-image support
// relation as local XOR constraints/candidate rejection, and keeps the joint
// affine-derivative rank lower bound paid by the objective.
//
// Model notes
// ----------------------------------------------------------------------------
// Modular add/sub differential oracles, MILP states, and bibliographic notes
// are centralized in model/neoalzette_scip_operator_analysis_milp_constraint.hpp.  This source
// file only instantiates those rules with the SCIP C API.
//
// Injection rank bound:
//    Each core-side joint injection map H has one 32-bit source and two 32-bit
//    outputs: the cross-branch XOR term and the modular-add operand term.  The
//    support relation is enforced by witness MILP constraints.  Since H is still
//    quadratic over GF(2), its derivative
//        D_delta H(x) = H(x) xor H(x xor delta) = M_delta x xor c_delta
//    is affine, and the local rank weight is rank(M_delta).
//
//    The implementation uses a C++20 polar-form fast path for this rank oracle.
//    This fast path is mathematically valid only because the current injection
//    maps are confirmed quadratic.  If the injected XOR maps are modified to
//    contain cubic or higher-degree terms, the polar fast path and the affine
//    rank model must be disabled/rederived.  Run
//    a dedicated injection-affine validation check after any injection-layer
//    change.
//
// Build example, depending on your SCIP installation:
//   g++ -O3 -std=c++20 -march=native \
//       neoalzette_scip_round_milp_search.cpp \
//       $(pkg-config --cflags --libs scip) \
//       -o neoalzette_scip_round_milp_search
//
// If pkg-config is unavailable, use your local include/lib paths, e.g.:
//   g++ -O3 -std=c++20 -I/path/to/scip/include \
//       neoalzette_scip_round_milp_search.cpp \
//       -L/path/to/scip/lib -lscip \
//       -o neoalzette_scip_round_milp_search
//
// Run example:
//   ./neoalzette_scip_round_milp_search \
//       --constant-model fixed-public-exact \
// ============================================================================

#include "model/neoalzette_scip_search_round_function.hpp"

int main( int argc, char** argv )
{
	using namespace neoalzette_diff_milp;
	try
	{
		if ( has_arg( argc, argv, "--help" ) )
		{
			print_help( argv[ 0 ] );
			return 0;
		}
		SearchOptions options = parse_options( argc, argv );
		if ( !options.output_round_table_json.empty() )
		{
			run_round_table( options );
		}
		else
		{
			ensure_default_best_trail_json_paths( options );
			EnumerationState enumeration;
			ScipSolveResult  result = solve_in_model( options, enumeration, true, true );
			if ( !result.feasible )
				std::cout << "SCIP model infeasible or incomplete. solver_status=" << scip_status_name( result.scip_status ) << "\n";
			write_best_result_json_file( options.output_result_json, options, result );
			write_weight_trace_json_file( options.output_weight_trace_json, options, result );
		}
		return 0;
	}
	catch ( const std::exception& e )
	{
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
}
