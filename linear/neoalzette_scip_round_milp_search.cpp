// ============================================================================
// NeoAlzette LINEAR Round MILP Search -- SCIP C API backend
// ============================================================================
//
// This entry point is LINEAR / Walsh-correlation only.
// It runs best single-characteristic search, continuous best-trail search, or
// a round table.  Endpoint-hull modes are intentionally handled by
// neoalzette_scip_round_hull_search.cpp so the command-line boundary is obvious.
// ============================================================================

#include "model/neoalzette_scip_search_round_function.hpp"

int main( int argc, char** argv )
{
	using namespace neoalzette_linear_milp;
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
		else if ( options.continuous_best_trail )
		{
			run_continuous_best_trail( options );
		}
		else
		{
			NoGoodStore no_goods;
			ScipSolveResult result = solve_linear_model( options, no_goods );
			if ( !result.feasible )
				std::cout << "SCIP linear model infeasible or incomplete. solver_status=" << scip_status_name( result.scip_status ) << "\n";
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
