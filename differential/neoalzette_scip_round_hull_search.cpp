// ============================================================================
// NeoAlzette XOR-Differential Round Hull Search -- SCIP C API backend
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
//   * Forest hull mode uses one total wall-clock budget: --hull-time-limit.
//     All input-difference attempts, SCIP calls, and endpoint enumeration consume
//     that one budget while searching the N-round round function MILP.
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
// Audit map:
//   1. forest options, endpoint keys, and run logs;
//   2. Q1/CDDT verification and branch-order scoring helpers;
//   3. endpoint enumeration and probability aggregation;
//   4. JSON artifact writers;
//   5. run_forest_hull_search() driver.
//
// Q2/Q1 literature anchor used by this HULL driver:
//   Zhengbin Liu, Yongqiang Li, Lin Jiao, and Mingsheng Wang,
//   "A New Method for Searching Optimal Differential and Linear Trails in ARX Ciphers",
//   IEEE Transactions on Information Theory 67(2), pp. 1054-1068,
//   2021, DOI 10.1109/TIT.2020.3040543; earlier IACR ePrint 2019/1438.
//   Their ARX method uses carry-bit-dependent DDT/LAT tables and an adapted
//   Matsui/Q2 layer that calls Q1 local transition-weight computations.  This
//   file follows that split for NeoAlzette HULL search: the forest layer orders
//   input-difference attempts, applies branch-and-bound cutoffs, and enumerates
//   fixed endpoints, while two-variable modular add/sub steps are scored and
//   verified through local Q1 CDDT/oracle calls.
//
// Build example, depending on your SCIP installation:
//   g++ -O3 -std=c++20 -march=native \
//       neoalzette_scip_round_hull_search.cpp \
//       $(pkg-config --cflags --libs scip) \
//       -o neoalzette_scip_round_hull_search
//
// If pkg-config is unavailable, use your local include/lib paths, e.g.:
//   g++ -O3 -std=c++20 -I/path/to/scip/include \
//       neoalzette_scip_round_hull_search.cpp \
//       -L/path/to/scip/lib -lscip \
//       -o neoalzette_scip_round_hull_search
//
// Run example:
//   ./neoalzette_scip_round_hull_search \
//       --constant-model fixed-public-exact \
// ============================================================================

#include "model/neoalzette_scip_search_round_function.hpp"

#include <cmath>
#include <queue>

namespace neoalzette_diff_milp
{
	enum class HullMode
	{
		BEST_TRAIL,
		BOUNDED_ENDPOINT,
		COMPLETE_ENDPOINT,
		ENDPOINT_CANDIDATE_SWEEP
	};

	struct Endpoint
	{
		std::uint32_t dA_in = 0;
		std::uint32_t dB_in = 0;
		std::uint32_t dA_out = 0;
		std::uint32_t dB_out = 0;
	};

	struct WeightDistributionEntry
	{
		double weight = 0.0;
		int	   count = 0;
	};

	struct TwoInputCddtLookupResult
	{
		bool		  possible = false;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
		bool		  cache_hit = false;
	};

	using TwoInputCddtLookupKey = std::tuple<bool, std::uint32_t, std::uint32_t, std::uint32_t, int>;

	struct CddtFreeSlotResult
	{
		bool		  possible = false;
		std::uint32_t value = 0;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
		std::uint64_t best_value_count = 0;
		std::uint64_t sorted_branch_candidates = 0;
		std::uint64_t sorted_branch_pruned_by_cutoff = 0;
	};

	struct CddtBranchScore
	{
		bool			   possible = false;
		CddtFreeSlotResult add_output;
		CddtFreeSlotResult sub_output;
		double			   score = std::numeric_limits<double>::infinity();
		std::string		   prune_reason;
	};

	struct OperatorStepProfile
	{
		int verified_trails = 0;
		int trace_entries = 0;
		int modular_addition = 0;
		int modular_subtraction = 0;
		int fixed_public_constant_subtraction = 0;
		int fixed_public_constant_addition = 0;
		int joint_injection_derivative = 0;
		int linear_xor_bridge = 0;
		int public_xor_constant = 0;
		int other = 0;
	};

	struct CddtOperatorProfile
	{
		int	   two_variable_add_steps = 0;
		int	   two_variable_sub_steps = 0;
		int	   local_bound_checks = 0;
		int	   branch_order_hints = 0;
		int	   sorted_branch_checks = 0;
		int	   sorted_branch_kept = 0;
		int	   early_prune_checks = 0;
		int	   early_pruned_by_objective_cutoff = 0;
		int	   solver_cutoff_constraints = 0;
		int	   cache_hits = 0;
		int	   cache_misses = 0;
		int	   impossible_transitions = 0;
		int	   weight_mismatches = 0;
		double local_bound_weight_sum = 0.0;
		double model_weight_sum = 0.0;
	};

	struct ForestInputDifference
	{
		std::uint32_t dA = 0;
		std::uint32_t dB = 0;
	};

	struct HullGrowthPoint
	{
		double window = 0.0;
		int count = 0;
		long double probability_sum = 0.0L;
	};

	struct StageWeightAggregate
	{
		int count = 0;
		double sum_weight = 0.0;
		double min_weight = std::numeric_limits<double>::infinity();
		double max_weight = -std::numeric_limits<double>::infinity();
	};

	struct LayerGrowthSummary
	{
		int layer = 0;
		int attempts = 0;
		int feasible = 0;
		int continuations_enqueued = 0;
		int cycles_detected = 0;
		double min_local_weight = std::numeric_limits<double>::infinity();
		double min_cumulative_weight = std::numeric_limits<double>::infinity();
		double min_average_weight = std::numeric_limits<double>::infinity();
	};

	struct ForestStateFrontierEntry
	{
		int layer = 0;
		double cumulative_weight = 0.0;
	};

	struct ForestCandidate
	{
		ForestInputDifference input;
		bool has_parent_input = false;
		ForestInputDifference parent_input;
		int parent_attempt_id = -1;
		int tree_id = 0;
		int layer = 0;
		bool derived_root = false;
		std::uint64_t derive_seed = 0;
		double cumulative_weight = 0.0;
		double priority_score = std::numeric_limits<double>::infinity();
		std::uint64_t serial = 0;
		CddtBranchScore cddt_branch_score;
		std::vector<std::uint64_t> path_inputs;
		std::vector<double> path_cumulative_weights;
	};

	struct ForestCandidateCompare
	{
		bool operator()( const ForestCandidate& left, const ForestCandidate& right ) const
		{
			if ( std::fabs( left.priority_score - right.priority_score ) > 1e-12 )
				return left.priority_score > right.priority_score;
			if ( left.layer != right.layer )
				return left.layer < right.layer;
			return left.serial > right.serial;
		}
	};

	using ForestCandidateQueue = std::priority_queue<ForestCandidate, std::vector<ForestCandidate>, ForestCandidateCompare>;

	struct ForestOptions
	{
		int			  attempts = 0;
		std::uint64_t seed = 0;
		HullMode	  hull_mode = HullMode::BOUNDED_ENDPOINT;
		bool		  hull_mode_explicit = false;
		int			  max_enumerate_solutions = 10000;
		double		  enumerate_from = std::numeric_limits<double>::quiet_NaN();
		double		  enumerate_to = std::numeric_limits<double>::quiet_NaN();
		double		  enumerate_window = 8.0;
		double		  hull_time_limit_seconds = std::numeric_limits<double>::quiet_NaN();
		std::string	  hull_output_json;
	};

	struct ForestQ1Verification
	{
		bool					 valid = true;
		int						 q1_calls = 0;
		int						 q1_failed = 0;
		int						 impossible_transitions = 0;
		int						 weight_mismatches = 0;
		CddtOperatorProfile		 cddt_operator_steps;
		std::vector<std::string> errors;
	};

	struct ForestAttemptLog
	{
		int								  attempt_id = 0;
		int								  parent_attempt_id = -1;
		int								  tree_id = 0;
		int								  layer = 0;
		bool								  derived_root = false;
		std::string						  source_kind = "initial_root";
		int								  input_frequency = 0;
		int								  output_frequency = 0;
		double							  candidate_priority = std::numeric_limits<double>::infinity();
		double							  cumulative_weight_before = 0.0;
		double							  cumulative_weight_after = std::numeric_limits<double>::quiet_NaN();
		double							  average_weight_per_layer = std::numeric_limits<double>::quiet_NaN();
		bool								  continuation_enqueued = false;
		bool								  continuation_dominated = false;
		bool								  continuation_zero_state = false;
		bool								  cycle_detected = false;
		int								  cycle_start_layer = -1;
		int								  cycle_length = 0;
		double							  cycle_weight = std::numeric_limits<double>::quiet_NaN();
		double							  cycle_average_weight = std::numeric_limits<double>::quiet_NaN();
		int								  candidate_bank_size_before = 0;
		int								  candidate_bank_size_after = 0;
		std::uint64_t					  derive_seed = 0;
		bool							  has_derive_input_source = false;
		ForestInputDifference			  derive_input_source;
		bool							  has_old_input = false;
		ForestInputDifference			  old_input;
		ForestInputDifference			  input;
		std::string						  input_source_change_message;
		std::string						  propagation_trunk_message;
		int								  cddt_branch_rank = 0;
		CddtBranchScore					  cddt_branch_score;
		bool							  cddt_branch_ordered = false;
		bool							  cddt_early_pruned = false;
		std::string						  cddt_branch_order_message;
		bool							  duplicate_input = false;
		bool							  pruned = false;
		bool							  completed = false;
		bool							  updated_global_best = false;
		bool							  hit_time_limit = false;
		bool							  hit_memory_limit = false;
		bool							  hit_solution_limit = false;
		std::string						  stop_reason;
		std::string						  solver_status = "not_run";
		int								  solver_calls = 0;
		int								  found_trails = 0;
		int								  blocking_cuts = 0;
		int								  q1_calls = 0;
		int								  q1_failed = 0;
		int								  impossible_transitions = 0;
		int								  weight_mismatches = 0;
		OperatorStepProfile				  operator_steps;
		bool							  bnb_prune_checked = false;
		bool							  bnb_pruned = false;
		bool							  bnb_prune_proven = false;
		double							  bnb_previous_global_best = std::numeric_limits<double>::infinity();
		double							  bnb_candidate_weight = std::numeric_limits<double>::infinity();
		double							  bnb_cutoff_weight = std::numeric_limits<double>::infinity();
		double							  bnb_prune_gap = std::numeric_limits<double>::quiet_NaN();
		std::string						  bnb_prune_rule;
		std::string						  bnb_bound_expression;
		std::string						  bnb_prune_message;
		double							  solver_best_weight = std::numeric_limits<double>::quiet_NaN();
		bool							  solver_best_complete = false;
		bool							  has_endpoint = false;
		Endpoint						  endpoint;
		double							  local_best_weight = std::numeric_limits<double>::quiet_NaN();
		int								  milp_operator_step_total = 0;
		int								  milp_operator_step_modular_addition = 0;
		int								  milp_operator_step_modular_subtraction = 0;
		int								  milp_operator_step_fixed_public_constant_subtraction = 0;
		int								  milp_operator_step_fixed_public_constant_addition = 0;
		int								  milp_operator_step_public_xor_constant = 0;
		int								  milp_operator_step_linear_xor_bridge = 0;
		int								  milp_operator_step_joint_injection_derivative = 0;
		int								  milp_operator_step_other = 0;
		CddtOperatorProfile				  cddt_operator_steps;
		std::vector<WeightDistributionEntry> weight_distribution;
		long double						  probability_sum = 0.0L;
		long double						  effective_weight = std::numeric_limits<long double>::infinity();
		std::vector<HullGrowthPoint>		  hull_growth;
		std::map<std::string, StageWeightAggregate> stage_weight_profile;
		std::vector<std::string>			  q1_errors;
	};

	struct ForestRunLog
	{
		ForestOptions					  forest_options;
		SearchOptions					  search_options;
		std::string						  analysis_algorithm = "MILP Solver Operator Steps + Multi-Layer Forest Growth";
		std::string						  solve_target = "N_round_search_round_function_with_milp";
		std::string						  time_budget_policy = "single_total_wall_clock_hull_time_limit";
		std::string						  time_budget_scope = "all_attempts_solver_calls_and_endpoint_enumeration";
		std::string						  result_type = "best_found_partial_forest";
		bool							  hit_global_time_limit = false;
		bool							  has_global_best = false;
		// Forest-level best total weight: prefix cumulative weight plus the local
		// SCIP objective of the audited trunk. This is intentionally not just
		// the local one-box objective; CDDT/CLAT cutoff pruning subtracts the
		// candidate prefix weight from this value.
		double							  global_best_weight = std::numeric_limits<double>::infinity();
		Endpoint						  global_best_endpoint;
		int								  global_best_trail_count = 0;
		int								  total_attempts = 0;
		int								  completed_attempts = 0;
		int								  pruned_branches = 0;
		int								  solver_calls = 0;
		int								  found_trails = 0;
		int								  q1_calls = 0;
		int								  q1_failed = 0;
		int								  impossible_transitions = 0;
		int								  weight_mismatches = 0;
		OperatorStepProfile				  operator_steps;
		int								  bnb_pruned_attempts = 0;
		int								  bnb_prune_checks = 0;
		int								  bnb_prune_applied = 0;
		int								  bnb_prune_deferred = 0;
		int								  bnb_objective_cutoff_constraints = 0;
		int								  cddt_branch_ordered_attempts = 0;
		int								  cddt_branch_score_checks = 0;
		int								  cddt_early_prune_checks = 0;
		int								  cddt_early_pruned_attempts = 0;
		int								  milp_operator_step_total = 0;
		int								  milp_operator_step_modular_addition = 0;
		int								  milp_operator_step_modular_subtraction = 0;
		int								  milp_operator_step_fixed_public_constant_subtraction = 0;
		int								  milp_operator_step_fixed_public_constant_addition = 0;
		int								  milp_operator_step_public_xor_constant = 0;
		int								  milp_operator_step_linear_xor_bridge = 0;
		int								  milp_operator_step_joint_injection_derivative = 0;
		int								  milp_operator_step_other = 0;
		CddtOperatorProfile				  cddt_operator_steps;
		int								  candidate_bank_peak = 0;
		int								  derived_roots_enqueued = 0;
		int								  continuations_enqueued = 0;
		int								  continuations_dominated = 0;
		int								  cycles_detected = 0;
		int								  max_layer = 0;
		int								  unique_input_states = 0;
		int								  unique_output_states = 0;
		bool								  saw_partial_limit = false;
		std::map<int, LayerGrowthSummary> layer_growth;
		std::map<std::string, StageWeightAggregate> operator_heatmap;
		std::vector<ForestAttemptLog>	  attempts;
	};

	// ------------------------------------------------------------------------
	// Audit section 1: forest options, endpoints, and run-log records
	// ------------------------------------------------------------------------
	static bool raw_has_arg( int argc, char** argv, const std::string& name )
	{
		for ( int i = 1; i < argc; ++i )
			if ( argv[ i ] == name )
				return true;
		return false;
	}

	static std::uint64_t parse_u64_hex_or_dec( const std::string& text )
	{
		std::size_t			   pos = 0;
		unsigned long long value = 0;
		if ( text.size() > 2 && text[ 0 ] == '0' && ( text[ 1 ] == 'x' || text[ 1 ] == 'X' ) )
			value = std::stoull( text, &pos, 16 );
		else
			value = std::stoull( text, &pos, 0 );
		if ( pos != text.size() )
			throw std::runtime_error( "invalid integer: " + text );
		return static_cast<std::uint64_t>( value );
	}

	static const char* hull_mode_name( HullMode mode )
	{
		switch ( mode )
		{
		case HullMode::BEST_TRAIL:
			return "best-trail";
		case HullMode::BOUNDED_ENDPOINT:
			return "bounded-endpoint";
		case HullMode::COMPLETE_ENDPOINT:
			return "complete-endpoint";
		case HullMode::ENDPOINT_CANDIDATE_SWEEP:
			return "endpoint-candidate-sweep";
		}
		return "unknown";
	}

	static HullMode parse_forest_hull_mode( const std::string& text )
	{
		if ( text == "best-trail" )
			return HullMode::BEST_TRAIL;
		if ( text == "bounded-endpoint" )
			return HullMode::BOUNDED_ENDPOINT;
		if ( text == "complete-endpoint" )
			return HullMode::COMPLETE_ENDPOINT;
		if ( text == "endpoint-candidate-sweep" )
			return HullMode::ENDPOINT_CANDIDATE_SWEEP;
		throw std::runtime_error( "unknown --hull-mode: " + text );
	}

	static Endpoint endpoint_from_snapshot( const SolutionSnapshot& snapshot )
	{
		return Endpoint { snapshot.dA_in, snapshot.dB_in, snapshot.dA_out, snapshot.dB_out };
	}

	static void set_fixed_endpoint( SearchOptions& options, const Endpoint& endpoint )
	{
		options.fix_input_da = endpoint.dA_in;
		options.fix_input_db = endpoint.dB_in;
		options.fix_output_da = endpoint.dA_out;
		options.fix_output_db = endpoint.dB_out;
	}

	static std::vector<WeightDistributionEntry> make_weight_distribution( const std::map<std::string, int>& grouped_counts )
	{
		std::vector<WeightDistributionEntry> distribution;
		distribution.reserve( grouped_counts.size() );
		for ( const auto& weight_count_pair : grouped_counts )
			distribution.push_back( { std::stod( weight_count_pair.first ), weight_count_pair.second } );
		std::sort( distribution.begin(), distribution.end(), []( const auto& left_entry, const auto& right_entry ) { return left_entry.weight < right_entry.weight; } );
		return distribution;
	}

	static std::string probability_polynomial_string( const std::vector<WeightDistributionEntry>& distribution )
	{
		if ( distribution.empty() )
			return "0";
		std::ostringstream output_stream;
		for ( std::size_t index = 0; index < distribution.size(); ++index )
		{
			const auto& distribution_entry = distribution[ index ];
			if ( index != 0 )
				output_stream << " + ";
			output_stream << distribution_entry.count << "*x^" << weight_key( distribution_entry.weight );
		}
		return output_stream.str();
	}

	static long double effective_weight_from_probability( long double probability )
	{
		return ( probability > 0.0L ) ? -std::log2( probability ) : std::numeric_limits<long double>::infinity();
	}

	static void write_endpoint_json( std::ostream& output_stream, const Endpoint& endpoint, const std::string& indent )
	{
		output_stream << indent << "{\n"
					  << indent << "  \"dA_in\": " << json_string( hex32_json( endpoint.dA_in ) ) << ",\n"
					  << indent << "  \"dB_in\": " << json_string( hex32_json( endpoint.dB_in ) ) << ",\n"
					  << indent << "  \"dA_out\": " << json_string( hex32_json( endpoint.dA_out ) ) << ",\n"
					  << indent << "  \"dB_out\": " << json_string( hex32_json( endpoint.dB_out ) ) << "\n"
					  << indent << "}";
	}

	static void write_weight_distribution_json( std::ostream& output_stream, const std::vector<WeightDistributionEntry>& distribution, const std::string& indent )
	{
		output_stream << "[";
		if ( !distribution.empty() )
			output_stream << "\n";
		for ( std::size_t distribution_index = 0; distribution_index < distribution.size(); ++distribution_index )
		{
			const auto& distribution_entry = distribution[ distribution_index ];
			output_stream << indent << "  {\"weight\": " << json_number( distribution_entry.weight ) << ", \"count\": " << distribution_entry.count << "}";
			if ( distribution_index + 1 != distribution.size() )
				output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static std::string hex64_forest( std::uint64_t x )
	{
		std::ostringstream output_stream;
		output_stream << std::hex << std::setw( 16 ) << std::setfill( '0' ) << x << std::dec;
		return output_stream.str();
	}

	static bool forest_value_option( const std::string& arg )
	{
		return arg == "--forest-attempts" || arg == "--forest-seed" || arg == "--hull-mode" ||
			   arg == "--enumerate-window" || arg == "--enumerate-weight-from" || arg == "--enumerate-weight-to" ||
			   arg == "--max-enumerate-solutions" || arg == "--hull-time-limit" || arg == "--hull-output-json";
	}

	static std::pair<ForestOptions, std::vector<std::string>> parse_forest_options_and_strip_argv( int argc, char** argv )
	{
		ForestOptions forest_options;
		std::vector<std::string> stripped_args;
		stripped_args.push_back( argv[ 0 ] );
		for ( int i = 1; i < argc; ++i )
		{
			const std::string arg = argv[ i ];
			if ( forest_value_option( arg ) )
			{
				if ( i + 1 >= argc )
					throw std::runtime_error( "missing value for option: " + arg );
				const std::string value = argv[ ++i ];
				if ( arg == "--forest-attempts" )
					forest_options.attempts = std::stoi( value );
				else if ( arg == "--forest-seed" )
					forest_options.seed = parse_u64_hex_or_dec( value );
				else if ( arg == "--hull-mode" )
				{
					forest_options.hull_mode = parse_forest_hull_mode( value );
					forest_options.hull_mode_explicit = true;
				}
				else if ( arg == "--enumerate-window" )
					forest_options.enumerate_window = std::stod( value );
				else if ( arg == "--enumerate-weight-from" )
					forest_options.enumerate_from = std::stod( value );
				else if ( arg == "--enumerate-weight-to" )
					forest_options.enumerate_to = std::stod( value );
				else if ( arg == "--max-enumerate-solutions" )
					forest_options.max_enumerate_solutions = std::stoi( value );
				else if ( arg == "--hull-time-limit" )
					forest_options.hull_time_limit_seconds = std::stod( value );
				else if ( arg == "--hull-output-json" )
					forest_options.hull_output_json = value;
				continue;
			}
			if ( arg == "--hull-endpoint-mode" || arg == "--endpoint-candidate-limit" )
				throw std::runtime_error( arg + " was removed from the shared round function and is not used by forest Q1 mode" );
			stripped_args.push_back( arg );
		}
		return { forest_options, stripped_args };
	}

	static SearchOptions parse_base_options_from_strings( const std::vector<std::string>& args )
	{
		std::vector<char*> argv_copy;
		argv_copy.reserve( args.size() );
		for ( const auto& arg : args )
			argv_copy.push_back( const_cast<char*>( arg.c_str() ) );
		return parse_options( static_cast<int>( argv_copy.size() ), argv_copy.data() );
	}

	static void print_forest_help()
	{
		std::cout << "\nForest hull search additions in this executable:\n"
				  << "  --forest-attempts N             optional processed-node cap; 0 means run until the global time limit (default)\n"
				  << "  --forest-seed X                 deterministic SplitMix64 derivation seed, default 0\n"
				  << "  --hull-mode MODE                bounded-endpoint | complete-endpoint\n"
				  << "  --enumerate-window W            default upper window from best weight, default 8\n"
				  << "  --enumerate-weight-from W       explicit lower weight bound\n"
				  << "  --enumerate-weight-to W         explicit upper weight bound\n"
				  << "  --max-enumerate-solutions N     per-input enumeration cap, default 10000\n"
				  << "  --hull-time-limit S             total wall-clock budget for the N-round MILP forest search\n"
				  << "  --hull-output-json FILE         write forest hull JSON\n"
				  << "\nForest mode requires --hull-time-limit. If --fix-input-da/--fix-input-db are omitted, both default to 0x00000001.\n"
				  << "Each Q1-valid best output is reinserted as the next Forest Layer input.\n"
				  << "All attempts, SCIP calls, and endpoint enumeration consume the single --hull-time-limit budget.\n"
				  << "Each attempt prints an input-source arrow before solving.\n";
	}

	static std::uint64_t splitmix64_next( std::uint64_t& state )
	{
		std::uint64_t z = ( state += 0x9E3779B97F4A7C15ull );
		z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ull;
		z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBull;
		return z ^ ( z >> 31 );
	}

	static std::uint64_t pack_input_difference( const ForestInputDifference& input )
	{
		return ( std::uint64_t( input.dA ) << 32 ) | std::uint64_t( input.dB );
	}

	static ForestInputDifference derive_next_input_difference( const ForestInputDifference& previous_input, std::uint64_t forest_seed, int attempt_id, std::uint64_t& derive_seed )
	{
		derive_seed = forest_seed ^ pack_input_difference( previous_input ) ^ ( 0xD1B54A32D192ED03ull * static_cast<std::uint64_t>( attempt_id + 1 ) );
		std::uint64_t state = derive_seed;
		ForestInputDifference next;
		next.dA = static_cast<std::uint32_t>( splitmix64_next( state ) >> 32 );
		next.dB = static_cast<std::uint32_t>( splitmix64_next( state ) >> 32 );
		if ( ( next.dA | next.dB ) == 0 )
			next.dA = 1;
		return next;
	}

	static std::string forest_input_string( const ForestInputDifference& input )
	{
		return "ΔA_in=0x" + hex32( input.dA ) + " ΔB_in=0x" + hex32( input.dB );
	}

	static std::string forest_arrow_message( int attempt_id, const ForestAttemptLog& attempt )
	{
		if ( attempt.has_old_input )
			return "ATTEMPT " + std::to_string( attempt_id ) + " INPUT_SOURCE_CHANGED: " + forest_input_string( attempt.old_input ) + " -> " + forest_input_string( attempt.input );
		return "ATTEMPT " + std::to_string( attempt_id ) + " INPUT_SOURCE_SELECTED: external_seed -> " + forest_input_string( attempt.input );
	}

	static void refresh_runtime_trunk_change_message( ForestAttemptLog& attempt, bool has_runtime_input_source, const ForestInputDifference& runtime_input_source )
	{
		attempt.has_old_input = has_runtime_input_source;
		if ( has_runtime_input_source )
			attempt.old_input = runtime_input_source;
		attempt.input_source_change_message = forest_arrow_message( attempt.attempt_id, attempt );
		attempt.propagation_trunk_message = "ATTEMPT " + std::to_string( attempt.attempt_id ) + " PROPAGATION_TRUNK_CHANGED: " +
											 ( attempt.has_old_input ? forest_input_string( attempt.old_input ) : std::string( "external_seed" ) ) + " -> " + forest_input_string( attempt.input );
	}

	// ------------------------------------------------------------------------
	// Audit section 2: Q1/CDDT verification and branch-order helpers
	// ------------------------------------------------------------------------
	// CDDT is used as a local two-variable add/sub hint for branch ordering and
	// early pruning; every accepted characteristic is still checked by exact Q1
	// trace verification before aggregation.
	static CddtBranchScore score_input_source_by_cddt( const ForestInputDifference& input, std::uint32_t cutoff_weight = std::numeric_limits<std::uint32_t>::max() );
	static std::string cddt_branch_order_message( const ForestAttemptLog& attempt );

	static std::vector<HullGrowthPoint> make_hull_growth_points()
	{
		return { { 0.0, 0, 0.0L }, { 2.0, 0, 0.0L }, { 4.0, 0, 0.0L }, { 6.0, 0, 0.0L }, { 8.0, 0, 0.0L } };
	}

	static double forest_candidate_priority( int layer, double cumulative_weight, const CddtBranchScore& score )
	{
		if ( !score.possible || !std::isfinite( score.score ) )
			return std::numeric_limits<double>::infinity();
		return ( cumulative_weight + score.score ) / static_cast<double>( std::max( 1, layer + 1 ) );
	}

	static ForestCandidate make_forest_candidate(
		const ForestInputDifference& input,
		int parent_attempt_id,
		int tree_id,
		int layer,
		bool derived_root,
		std::uint64_t derive_seed,
		double cumulative_weight,
		std::uint64_t serial,
		bool has_parent_input,
		const ForestInputDifference& parent_input,
		std::vector<std::uint64_t> path_inputs,
		std::vector<double> path_cumulative_weights )
	{
		ForestCandidate candidate;
		candidate.input = input;
		candidate.parent_attempt_id = parent_attempt_id;
		candidate.tree_id = tree_id;
		candidate.layer = layer;
		candidate.derived_root = derived_root;
		candidate.derive_seed = derive_seed;
		candidate.cumulative_weight = cumulative_weight;
		candidate.serial = serial;
		candidate.has_parent_input = has_parent_input;
		candidate.parent_input = parent_input;
		candidate.path_inputs = std::move( path_inputs );
		candidate.path_cumulative_weights = std::move( path_cumulative_weights );
		candidate.cddt_branch_score = score_input_source_by_cddt( input );
		candidate.priority_score = forest_candidate_priority( layer, cumulative_weight, candidate.cddt_branch_score );
		return candidate;
	}

	static bool register_forest_frontier(
		std::map<std::uint64_t, std::vector<ForestStateFrontierEntry>>& frontier,
		const ForestCandidate& candidate )
	{
		auto& states = frontier[ pack_input_difference( candidate.input ) ];
		for ( const auto& state : states )
		{
			if ( state.layer >= candidate.layer && state.cumulative_weight <= candidate.cumulative_weight + 1e-8 )
				return false;
		}
		states.erase( std::remove_if( states.begin(), states.end(), [&]( const ForestStateFrontierEntry& state ) {
			return candidate.layer >= state.layer && candidate.cumulative_weight <= state.cumulative_weight + 1e-8;
		} ), states.end() );
		states.push_back( { candidate.layer, candidate.cumulative_weight } );
		return true;
	}

	static int find_cycle_start_index( const ForestCandidate& candidate, std::uint64_t child_key )
	{
		for ( int index = static_cast<int>( candidate.path_inputs.size() ) - 1; index >= 0; --index )
			if ( candidate.path_inputs[ static_cast<std::size_t>( index ) ] == child_key )
				return index;
		return -1;
	}

	static ForestAttemptLog make_forest_attempt( const ForestCandidate& candidate, int attempt_id, int candidate_bank_size_before )
	{
		ForestAttemptLog attempt;
		attempt.attempt_id = attempt_id;
		attempt.parent_attempt_id = candidate.parent_attempt_id;
		attempt.tree_id = candidate.tree_id;
		attempt.layer = candidate.layer;
		attempt.derived_root = candidate.derived_root;
		attempt.source_kind = candidate.parent_attempt_id >= 0 ? "continuation" : ( candidate.derived_root ? "derived_root" : "initial_root" );
		attempt.candidate_priority = candidate.priority_score;
		attempt.cumulative_weight_before = candidate.cumulative_weight;
		attempt.candidate_bank_size_before = candidate_bank_size_before;
		attempt.derive_seed = candidate.derive_seed;
		attempt.input = candidate.input;
		attempt.has_old_input = candidate.has_parent_input;
		attempt.old_input = candidate.parent_input;
		attempt.has_derive_input_source = candidate.has_parent_input;
		attempt.derive_input_source = candidate.parent_input;
		attempt.input_source_change_message = candidate.parent_attempt_id >= 0
			? "ATTEMPT " + std::to_string( attempt_id ) + " FOREST_LAYER_CONTINUATION: output_of_attempt=" + std::to_string( candidate.parent_attempt_id ) + " -> " + forest_input_string( candidate.input )
			: forest_arrow_message( attempt_id, attempt );
		attempt.propagation_trunk_message = "ATTEMPT " + std::to_string( attempt_id ) + " FOREST_LAYER: tree=" + std::to_string( candidate.tree_id ) + " layer=" + std::to_string( candidate.layer ) + " input=" + forest_input_string( candidate.input );
		attempt.cddt_branch_score = candidate.cddt_branch_score;
		attempt.cddt_branch_ordered = true;
		attempt.cddt_branch_rank = attempt_id;
		attempt.cddt_branch_order_message = cddt_branch_order_message( attempt );
		attempt.cddt_operator_steps.sorted_branch_checks = 1;
		attempt.cddt_operator_steps.local_bound_checks = 2;
		attempt.cddt_operator_steps.branch_order_hints = 2;
		attempt.cddt_operator_steps.two_variable_add_steps = 1;
		attempt.cddt_operator_steps.two_variable_sub_steps = 1;
		if ( attempt.cddt_branch_score.possible )
			attempt.cddt_operator_steps.sorted_branch_kept = 1;
		else
		{
			attempt.cddt_early_pruned = true;
			attempt.pruned = true;
			attempt.stop_reason = attempt.cddt_branch_score.prune_reason;
			++attempt.cddt_operator_steps.impossible_transitions;
		}
		attempt.hull_growth = make_hull_growth_points();
		return attempt;
	}


	static std::uint32_t cddt_cutoff_from_double( double cutoff_weight )
	{
		if ( !std::isfinite( cutoff_weight ) )
			return std::numeric_limits<std::uint32_t>::max();
		if ( cutoff_weight < 0.0 )
			return 0;
		const double floored = std::floor( cutoff_weight + 1e-9 );
		if ( floored >= static_cast<double>( std::numeric_limits<std::uint32_t>::max() ) )
			return std::numeric_limits<std::uint32_t>::max();
		return static_cast<std::uint32_t>( floored );
	}

	static void apply_cddt_cutoff_rescore_for_attempt( ForestAttemptLog& attempt, double cutoff_weight )
	{
		if ( !std::isfinite( cutoff_weight ) )
			return;
		const std::uint32_t local_cutoff = cddt_cutoff_from_double( cutoff_weight );
		attempt.cddt_branch_score = score_input_source_by_cddt( attempt.input, local_cutoff );
		attempt.cddt_branch_order_message = cddt_branch_order_message( attempt );
		attempt.cddt_operator_steps.sorted_branch_checks += 1;
		attempt.cddt_operator_steps.local_bound_checks += 2;
		attempt.cddt_operator_steps.branch_order_hints += 2;
		attempt.cddt_operator_steps.early_prune_checks += 1;
	}

	static bool cddt_score_exceeds_cutoff( const CddtBranchScore& score, double cutoff_weight )
	{
		return std::isfinite( cutoff_weight ) && std::isfinite( score.score ) && score.score > cutoff_weight + 1e-8;
	}

	static void update_hull_growth( ForestAttemptLog& attempt, double weight, long double probability )
	{
		if ( !std::isfinite( attempt.local_best_weight ) )
			return;
		for ( auto& point : attempt.hull_growth )
		{
			if ( weight <= attempt.local_best_weight + point.window + 1e-8 )
			{
				++point.count;
				point.probability_sum += probability;
			}
		}
	}

	static void update_stage_weight_profile( std::map<std::string, StageWeightAggregate>& profile, const std::vector<WeightTraceEntry>& trace )
	{
		for ( const auto& entry : trace )
		{
			const std::string key = "r" + std::to_string( entry.round ) + "." + entry.stage;
			auto& aggregate = profile[ key ];
			++aggregate.count;
			aggregate.sum_weight += entry.local_weight;
			aggregate.min_weight = std::min( aggregate.min_weight, entry.local_weight );
			aggregate.max_weight = std::max( aggregate.max_weight, entry.local_weight );
		}
	}

	static void merge_stage_weight_profile( std::map<std::string, StageWeightAggregate>& target, const std::map<std::string, StageWeightAggregate>& source )
	{
		for ( const auto& [ key, value ] : source )
		{
			auto& aggregate = target[ key ];
			aggregate.count += value.count;
			aggregate.sum_weight += value.sum_weight;
			aggregate.min_weight = std::min( aggregate.min_weight, value.min_weight );
			aggregate.max_weight = std::max( aggregate.max_weight, value.max_weight );
		}
	}


	static void print_input_source_change( const ForestAttemptLog& attempt )
	{
		std::cout << attempt.input_source_change_message << "\n";
		std::cout << attempt.propagation_trunk_message << "\n";
	}

	static void print_cddt_branch_order( const ForestAttemptLog& attempt )
	{
		std::cout << attempt.cddt_branch_order_message << "\n";
		if ( attempt.cddt_early_pruned )
			std::cout << "ATTEMPT " << attempt.attempt_id << " CDDT_EARLY_PRUNE: reason=" << attempt.cddt_branch_score.prune_reason << "\n";
	}

	static void print_bnb_prune_audit( const ForestAttemptLog& attempt )
	{
		std::cout << "BNB_PRUNE_AUDIT: solver_complete=" << ( attempt.solver_best_complete ? "yes" : "no" )
				  << " proven_by_solver=" << ( attempt.bnb_prune_proven ? "yes" : "no" )
				  << " previous_global_best=" << ( std::isfinite( attempt.bnb_previous_global_best ) ? std::to_string( attempt.bnb_previous_global_best ) : "none" )
				  << " candidate_best=" << ( std::isfinite( attempt.bnb_candidate_weight ) ? std::to_string( attempt.bnb_candidate_weight ) : "none" )
				  << " cutoff=" << ( std::isfinite( attempt.bnb_cutoff_weight ) ? std::to_string( attempt.bnb_cutoff_weight ) : "none" )
				  << " gap=" << ( std::isfinite( attempt.bnb_candidate_weight ) && std::isfinite( attempt.bnb_previous_global_best ) ? std::to_string( attempt.bnb_candidate_weight - attempt.bnb_previous_global_best ) : "none" )
				  << " rule=" << ( attempt.bnb_bound_expression.empty() ? "solver_complete && candidate_best > previous_global_best" : attempt.bnb_bound_expression )
				  << " -> prune=" << ( attempt.bnb_pruned ? "yes" : "no" );
		if ( !attempt.bnb_prune_message.empty() )
			std::cout << " (" << attempt.bnb_prune_message << ")";
		std::cout << "\n";
	}

	static void print_cddt_audit( const ForestAttemptLog& attempt )
	{
		std::cout << "CDDT_AUDIT: attempt=" << attempt.attempt_id
				  << " two_var_add=" << attempt.cddt_operator_steps.two_variable_add_steps
				  << " two_var_sub=" << attempt.cddt_operator_steps.two_variable_sub_steps
				  << " local_bound_checks=" << attempt.cddt_operator_steps.local_bound_checks
				  << " branch_order_hints=" << attempt.cddt_operator_steps.branch_order_hints
				  << " sorted_branch_checks=" << attempt.cddt_operator_steps.sorted_branch_checks
				  << " sorted_branch_kept=" << attempt.cddt_operator_steps.sorted_branch_kept
				  << " early_prune_checks=" << attempt.cddt_operator_steps.early_prune_checks
				  << " early_pruned_by_objective_cutoff=" << attempt.cddt_operator_steps.early_pruned_by_objective_cutoff
				  << " solver_cutoff_constraints=" << attempt.cddt_operator_steps.solver_cutoff_constraints
				  << " cache_hits=" << attempt.cddt_operator_steps.cache_hits
				  << " cache_misses=" << attempt.cddt_operator_steps.cache_misses
				  << " impossible_transitions=" << attempt.cddt_operator_steps.impossible_transitions
				  << " weight_mismatches=" << attempt.cddt_operator_steps.weight_mismatches
				  << " local_bound_weight_sum=" << scientific_string( attempt.cddt_operator_steps.local_bound_weight_sum )
				  << " model_weight_sum=" << scientific_string( attempt.cddt_operator_steps.model_weight_sum ) << "\n";
	}

	static void print_time_limit_incumbent_note( int attempt_id, const char* phase, const ScipSolveResult& result )
	{
		if ( !result.hit_time_limit )
			return;
		std::cout << "FOREST_SOLVER_TIME_LIMIT: attempt=" << attempt_id
				  << " phase=" << phase
				  << " incumbent=" << ( result.feasible ? "printed_above" : "none" ) << "\n";
	}

	static double forest_elapsed_seconds( std::chrono::steady_clock::time_point start )
	{
		return std::chrono::duration<double>( std::chrono::steady_clock::now() - start ).count();
	}

	static bool forest_deadline_expired( const ForestOptions& options, std::chrono::steady_clock::time_point start )
	{
		return std::isfinite( options.hull_time_limit_seconds ) && options.hull_time_limit_seconds > 0.0 && forest_elapsed_seconds( start ) >= options.hull_time_limit_seconds;
	}

	static double forest_remaining_seconds( const ForestOptions& options, std::chrono::steady_clock::time_point start )
	{
		if ( !std::isfinite( options.hull_time_limit_seconds ) || options.hull_time_limit_seconds <= 0.0 )
			return std::numeric_limits<double>::quiet_NaN();
		return options.hull_time_limit_seconds - forest_elapsed_seconds( start );
	}

	static double forest_remaining_budget_for_solver( const ForestOptions& forest_options, std::chrono::steady_clock::time_point start )
	{
		const double remaining = forest_remaining_seconds( forest_options, start );
		if ( std::isfinite( remaining ) )
			return std::max( 0.001, remaining );
		return std::numeric_limits<double>::quiet_NaN();
	}

	static void add_q1_result_to_attempt( ForestAttemptLog& attempt, const ForestQ1Verification& verification )
	{
		attempt.q1_calls += verification.q1_calls;
		attempt.q1_failed += verification.q1_failed;
		attempt.impossible_transitions += verification.impossible_transitions;
		attempt.weight_mismatches += verification.weight_mismatches;
		attempt.cddt_operator_steps.two_variable_add_steps += verification.cddt_operator_steps.two_variable_add_steps;
		attempt.cddt_operator_steps.two_variable_sub_steps += verification.cddt_operator_steps.two_variable_sub_steps;
		attempt.cddt_operator_steps.local_bound_checks += verification.cddt_operator_steps.local_bound_checks;
		attempt.cddt_operator_steps.branch_order_hints += verification.cddt_operator_steps.branch_order_hints;
		attempt.cddt_operator_steps.sorted_branch_checks += verification.cddt_operator_steps.sorted_branch_checks;
		attempt.cddt_operator_steps.sorted_branch_kept += verification.cddt_operator_steps.sorted_branch_kept;
		attempt.cddt_operator_steps.early_prune_checks += verification.cddt_operator_steps.early_prune_checks;
		attempt.cddt_operator_steps.early_pruned_by_objective_cutoff += verification.cddt_operator_steps.early_pruned_by_objective_cutoff;
		attempt.cddt_operator_steps.solver_cutoff_constraints += verification.cddt_operator_steps.solver_cutoff_constraints;
		attempt.cddt_operator_steps.cache_hits += verification.cddt_operator_steps.cache_hits;
		attempt.cddt_operator_steps.cache_misses += verification.cddt_operator_steps.cache_misses;
		attempt.cddt_operator_steps.impossible_transitions += verification.cddt_operator_steps.impossible_transitions;
		attempt.cddt_operator_steps.weight_mismatches += verification.cddt_operator_steps.weight_mismatches;
		attempt.cddt_operator_steps.local_bound_weight_sum += verification.cddt_operator_steps.local_bound_weight_sum;
		attempt.cddt_operator_steps.model_weight_sum += verification.cddt_operator_steps.model_weight_sum;
		for ( const auto& error : verification.errors )
			attempt.q1_errors.push_back( error );
	}

	static TwoInputCddtLookupResult lookup_two_input_cddt( bool is_subtraction, std::uint32_t input0, std::uint32_t input1, std::uint32_t output, int bits )
	{
		static std::map<TwoInputCddtLookupKey, TwoInputCddtLookupResult> cache;
		const TwoInputCddtLookupKey key { is_subtraction, input0, input1, output, bits };
		auto						it = cache.find( key );
		if ( it != cache.end() )
		{
			TwoInputCddtLookupResult cached = it->second;
			cached.cache_hit = true;
			return cached;
		}

		TwoInputCddtLookupResult result;
		if ( is_subtraction )
		{
			const auto oracle_result = differential_oracle::oracle_sub2( input0, input1, output, bits );
			result.possible = oracle_result.possible;
			result.weight = oracle_result.weight;
		}
		else
		{
			const auto oracle_result = differential_oracle::oracle_add2( input0, input1, output, bits );
			result.possible = oracle_result.possible;
			result.weight = oracle_result.weight;
		}
		cache.emplace( key, result );
		return result;
	}

	static std::uint64_t saturating_add_u64( std::uint64_t left, std::uint64_t right )
	{
		if ( std::numeric_limits<std::uint64_t>::max() - left < right )
			return std::numeric_limits<std::uint64_t>::max();
		return left + right;
	}

	struct CddtDpState
	{
		bool		  reachable = false;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
		std::uint32_t value = 0;
		std::uint64_t count = 0;
	};

	struct CddtChunkCandidate
	{
		std::uint32_t value = 0;
		std::uint32_t weight = 0;
	};

	static void update_cddt_dp_state( CddtDpState& state, std::uint32_t weight, std::uint32_t value, std::uint64_t count )
	{
		if ( !state.reachable || weight < state.weight )
		{
			state.reachable = true;
			state.weight = weight;
			state.value = value;
			state.count = count;
			return;
		}
		if ( weight == state.weight )
		{
			state.value = std::min( state.value, value );
			state.count = saturating_add_u64( state.count, count );
		}
	}

	static bool cddt_lipmaa_moriai_step_possible( const std::array<int, 3>& previous_bits, const std::array<int, 3>& current_bits )
	{
		const int current_xor = current_bits[ 0 ] ^ current_bits[ 1 ] ^ current_bits[ 2 ];
		return !( previous_bits[ 0 ] == previous_bits[ 1 ] && previous_bits[ 1 ] == previous_bits[ 2 ] && current_xor != previous_bits[ 0 ] );
	}

	static std::vector<CddtChunkCandidate> cddt_chunk_free_slot_candidates( int free_slot,
																		   const std::array<std::uint32_t, 3>& fixed,
																		   const std::array<int, 3>& previous_msb,
																		   int chunk_start,
																		   int chunk_width,
																		   int bits )
	{
		const std::uint32_t chunk_mask = differential_oracle::mask_for_bits( chunk_width );
		std::array<std::uint32_t, 3> fixed_chunk {};
		for ( int slot = 0; slot < 3; ++slot )
			fixed_chunk[ slot ] = ( fixed[ slot ] >> chunk_start ) & chunk_mask;

		std::vector<CddtChunkCandidate> candidates;
		candidates.reserve( static_cast<std::size_t>( chunk_mask ) + 1 );
		for ( std::uint32_t free_chunk = 0; free_chunk <= chunk_mask; ++free_chunk )
		{
			bool		  possible = true;
			std::uint32_t weight = 0;
			for ( int bit = 0; bit < chunk_width; ++bit )
			{
				std::array<int, 3> previous_bits {};
				std::array<int, 3> current_bits {};
				for ( int slot = 0; slot < 3; ++slot )
				{
					const std::uint32_t slot_chunk = ( slot == free_slot ) ? free_chunk : fixed_chunk[ slot ];
					previous_bits[ slot ] = ( bit == 0 ) ? previous_msb[ slot ] : differential_oracle::word_bit( slot_chunk, bit - 1 );
					current_bits[ slot ] = differential_oracle::word_bit( slot_chunk, bit );
				}
				if ( !cddt_lipmaa_moriai_step_possible( previous_bits, current_bits ) )
				{
					possible = false;
					break;
				}
				const bool all_equal = current_bits[ 0 ] == current_bits[ 1 ] && current_bits[ 1 ] == current_bits[ 2 ];
				if ( chunk_start + bit < bits - 1 && !all_equal )
					++weight;
			}
			if ( possible )
				candidates.push_back( CddtChunkCandidate { free_chunk, weight } );
		}
		std::stable_sort( candidates.begin(), candidates.end(), []( const CddtChunkCandidate& left, const CddtChunkCandidate& right ) {
			if ( left.weight != right.weight )
				return left.weight < right.weight;
			return left.value < right.value;
		} );
		return candidates;
	}

	static CddtFreeSlotResult best_cddt_free_slot( int free_slot, std::uint32_t slot0, std::uint32_t slot1, std::uint32_t slot2, int bits, std::uint32_t cutoff_weight = std::numeric_limits<std::uint32_t>::max() )
	{
		if ( free_slot < 0 || free_slot > 2 || bits <= 0 || bits > 32 )
			return {};

		constexpr int cddt_chunk_bits = 8;
		const std::uint32_t mask = differential_oracle::mask_for_bits( bits );
		std::array<std::uint32_t, 3> fixed { slot0 & mask, slot1 & mask, slot2 & mask };
		std::array<CddtDpState, 2> states;
		states[ 0 ] = CddtDpState { true, 0, 0, 1 };

		CddtFreeSlotResult result;
		for ( int chunk_start = 0; chunk_start < bits; chunk_start += cddt_chunk_bits )
		{
			const int chunk_width = std::min( cddt_chunk_bits, bits - chunk_start );
			std::array<CddtDpState, 2> next_states;
			for ( int previous_free_msb = 0; previous_free_msb <= 1; ++previous_free_msb )
			{
				const CddtDpState& state = states[ previous_free_msb ];
				if ( !state.reachable )
					continue;
				std::array<int, 3> previous_msb {};
				for ( int slot = 0; slot < 3; ++slot )
					previous_msb[ slot ] = ( slot == free_slot ) ? previous_free_msb : ( chunk_start == 0 ? 0 : differential_oracle::word_bit( fixed[ slot ], chunk_start - 1 ) );

				const auto candidates = cddt_chunk_free_slot_candidates( free_slot, fixed, previous_msb, chunk_start, chunk_width, bits );
				for ( std::size_t index = 0; index < candidates.size(); ++index )
				{
					const CddtChunkCandidate& candidate = candidates[ index ];
					result.sorted_branch_candidates++;
					if ( state.weight != std::numeric_limits<std::uint32_t>::max() && candidate.weight > std::numeric_limits<std::uint32_t>::max() - state.weight )
						continue;
					const std::uint32_t next_weight = state.weight + candidate.weight;
					if ( next_weight > cutoff_weight )
					{
						result.sorted_branch_pruned_by_cutoff += static_cast<std::uint64_t>( candidates.size() - index );
						break;
					}
					const int next_free_msb = differential_oracle::word_bit( candidate.value, chunk_width - 1 );
					const std::uint32_t next_value = state.value | ( candidate.value << chunk_start );
					update_cddt_dp_state( next_states[ next_free_msb ], next_weight, next_value, state.count );
				}
			}
			states = next_states;
		}

		CddtDpState best;
		if ( states[ 0 ].reachable )
			update_cddt_dp_state( best, states[ 0 ].weight, states[ 0 ].value, states[ 0 ].count );
		if ( states[ 1 ].reachable )
			update_cddt_dp_state( best, states[ 1 ].weight, states[ 1 ].value, states[ 1 ].count );
		if ( !best.reachable )
			return result;
		result.possible = true;
		result.value = best.value & mask;
		result.weight = best.weight;
		result.best_value_count = best.count;
		return result;
	}

	static CddtBranchScore score_input_source_by_cddt( const ForestInputDifference& input, std::uint32_t cutoff_weight )
	{
		CddtBranchScore score;
		score.add_output = best_cddt_free_slot( 2, input.dA, input.dB, 0, WORD_SIZE, cutoff_weight );
		score.sub_output = best_cddt_free_slot( 0, 0, input.dB, input.dA, WORD_SIZE, cutoff_weight );
		score.possible = score.add_output.possible || score.sub_output.possible;
		if ( !score.possible )
		{
			const bool cutoff_pruned = score.add_output.sorted_branch_pruned_by_cutoff != 0 || score.sub_output.sorted_branch_pruned_by_cutoff != 0;
			score.prune_reason = cutoff_pruned ? "cddt_no_direct_input_source_addsub_output_hint_within_cutoff" : "cddt_no_two_variable_addsub_output";
			return score;
		}
		const double add_weight = score.add_output.possible ? static_cast<double>( score.add_output.weight ) : std::numeric_limits<double>::infinity();
		const double sub_weight = score.sub_output.possible ? static_cast<double>( score.sub_output.weight ) : std::numeric_limits<double>::infinity();
		score.score = add_weight + sub_weight;
		return score;
	}

	static std::string cddt_branch_score_string( const CddtBranchScore& score )
	{
		std::ostringstream output_stream;
		output_stream << "score=" << ( std::isfinite( score.score ) ? scientific_string( score.score ) : std::string( "inf" ) );
		if ( score.add_output.possible )
			output_stream << " add_best_weight=" << score.add_output.weight << " add_output_hint=0x" << hex32( score.add_output.value ) << " add_min_outputs=" << score.add_output.best_value_count
						  << " add_sorted_branches=" << score.add_output.sorted_branch_candidates << " add_cutoff_pruned=" << score.add_output.sorted_branch_pruned_by_cutoff;
		else
			output_stream << " add_best_weight=impossible";
		if ( score.sub_output.possible )
			output_stream << " sub_best_weight=" << score.sub_output.weight << " sub_output_hint=0x" << hex32( score.sub_output.value ) << " sub_min_outputs=" << score.sub_output.best_value_count
						  << " sub_sorted_branches=" << score.sub_output.sorted_branch_candidates << " sub_cutoff_pruned=" << score.sub_output.sorted_branch_pruned_by_cutoff;
		else
			output_stream << " sub_best_weight=impossible";
		return output_stream.str();
	}

	static std::string cddt_branch_order_message( const ForestAttemptLog& attempt )
	{
		return "ATTEMPT " + std::to_string( attempt.attempt_id ) + " CDDT_BRANCH_ORDER: rank=" + std::to_string( attempt.cddt_branch_rank ) + " " + cddt_branch_score_string( attempt.cddt_branch_score );
	}

	static void add_operator_step_result_to_run( ForestRunLog& run_log, const ForestAttemptLog& attempt )
	{
		run_log.operator_steps.verified_trails += attempt.operator_steps.verified_trails;
		run_log.operator_steps.trace_entries += attempt.operator_steps.trace_entries;
		run_log.operator_steps.modular_addition += attempt.operator_steps.modular_addition;
		run_log.operator_steps.modular_subtraction += attempt.operator_steps.modular_subtraction;
		run_log.operator_steps.fixed_public_constant_subtraction += attempt.operator_steps.fixed_public_constant_subtraction;
		run_log.operator_steps.fixed_public_constant_addition += attempt.operator_steps.fixed_public_constant_addition;
		run_log.operator_steps.public_xor_constant += attempt.operator_steps.public_xor_constant;
		run_log.operator_steps.linear_xor_bridge += attempt.operator_steps.linear_xor_bridge;
		run_log.operator_steps.joint_injection_derivative += attempt.operator_steps.joint_injection_derivative;
		run_log.operator_steps.other += attempt.operator_steps.other;
		run_log.milp_operator_step_total += attempt.operator_steps.trace_entries;
		run_log.milp_operator_step_modular_addition += attempt.operator_steps.modular_addition;
		run_log.milp_operator_step_modular_subtraction += attempt.operator_steps.modular_subtraction;
		run_log.milp_operator_step_fixed_public_constant_subtraction += attempt.operator_steps.fixed_public_constant_subtraction;
		run_log.milp_operator_step_fixed_public_constant_addition += attempt.operator_steps.fixed_public_constant_addition;
		run_log.milp_operator_step_public_xor_constant += attempt.operator_steps.public_xor_constant;
		run_log.milp_operator_step_linear_xor_bridge += attempt.operator_steps.linear_xor_bridge;
		run_log.milp_operator_step_joint_injection_derivative += attempt.operator_steps.joint_injection_derivative;
		run_log.milp_operator_step_other += attempt.operator_steps.other;
		run_log.cddt_operator_steps.two_variable_add_steps += attempt.cddt_operator_steps.two_variable_add_steps;
		run_log.cddt_operator_steps.two_variable_sub_steps += attempt.cddt_operator_steps.two_variable_sub_steps;
		run_log.cddt_operator_steps.local_bound_checks += attempt.cddt_operator_steps.local_bound_checks;
		run_log.cddt_operator_steps.branch_order_hints += attempt.cddt_operator_steps.branch_order_hints;
		run_log.cddt_operator_steps.sorted_branch_checks += attempt.cddt_operator_steps.sorted_branch_checks;
		run_log.cddt_operator_steps.sorted_branch_kept += attempt.cddt_operator_steps.sorted_branch_kept;
		run_log.cddt_operator_steps.early_prune_checks += attempt.cddt_operator_steps.early_prune_checks;
		run_log.cddt_operator_steps.early_pruned_by_objective_cutoff += attempt.cddt_operator_steps.early_pruned_by_objective_cutoff;
		run_log.cddt_operator_steps.solver_cutoff_constraints += attempt.cddt_operator_steps.solver_cutoff_constraints;
		run_log.cddt_operator_steps.cache_hits += attempt.cddt_operator_steps.cache_hits;
		run_log.cddt_operator_steps.cache_misses += attempt.cddt_operator_steps.cache_misses;
		run_log.cddt_operator_steps.impossible_transitions += attempt.cddt_operator_steps.impossible_transitions;
		run_log.cddt_operator_steps.weight_mismatches += attempt.cddt_operator_steps.weight_mismatches;
		run_log.cddt_operator_steps.local_bound_weight_sum += attempt.cddt_operator_steps.local_bound_weight_sum;
		run_log.cddt_operator_steps.model_weight_sum += attempt.cddt_operator_steps.model_weight_sum;
	}

	static void add_operator_step_result_to_attempt( ForestAttemptLog& attempt, const ScipSolveResult& result )
	{
		++attempt.operator_steps.verified_trails;
		attempt.operator_steps.trace_entries += static_cast<int>( result.weight_trace.size() );
		for ( const auto& trace_entry : result.weight_trace )
		{
			if ( trace_entry.operation == "modular_addition" )
				++attempt.operator_steps.modular_addition;
			else if ( trace_entry.operation == "modular_subtraction" )
				++attempt.operator_steps.modular_subtraction;
			else if ( trace_entry.operation == "fixed_public_constant_subtraction" )
				++attempt.operator_steps.fixed_public_constant_subtraction;
			else if ( trace_entry.operation == "fixed_public_constant_addition" )
				++attempt.operator_steps.fixed_public_constant_addition;
			else if ( trace_entry.operation == "public_xor_constant" )
				++attempt.operator_steps.public_xor_constant;
			else if ( trace_entry.operation == "linear_xor_bridge" )
				++attempt.operator_steps.linear_xor_bridge;
			else if ( trace_entry.operation == "joint_injection_derivative" )
				++attempt.operator_steps.joint_injection_derivative;
			else
				++attempt.operator_steps.other;
		}
	}

	static bool is_two_variable_addsub_step( const WeightTraceEntry& trace_entry )
	{
		return trace_entry.operation == "modular_addition" || trace_entry.operation == "modular_subtraction";
	}

	static void add_cddt_operator_step_result_to_attempt( ForestAttemptLog& attempt, const ScipSolveResult& result )
	{
		for ( const auto& trace_entry : result.weight_trace )
		{
			if ( !is_two_variable_addsub_step( trace_entry ) )
				continue;
			++attempt.cddt_operator_steps.local_bound_checks;
			++attempt.cddt_operator_steps.branch_order_hints;
			++attempt.cddt_operator_steps.sorted_branch_checks;
			if ( trace_entry.operation == "modular_addition" )
			{
				++attempt.cddt_operator_steps.two_variable_add_steps;
				const auto oracle_result = differential_oracle::oracle_add2( trace_entry.local_input0, trace_entry.local_input1, trace_entry.local_output, WORD_SIZE );
				if ( !oracle_result.possible )
				{
					++attempt.cddt_operator_steps.impossible_transitions;
					continue;
				}
				attempt.cddt_operator_steps.local_bound_weight_sum += static_cast<double>( oracle_result.weight );
				++attempt.cddt_operator_steps.sorted_branch_kept;
				if ( std::fabs( static_cast<double>( oracle_result.weight ) - trace_entry.local_weight ) > 1e-6 )
					++attempt.cddt_operator_steps.weight_mismatches;
			}
			else
			{
				++attempt.cddt_operator_steps.two_variable_sub_steps;
				const auto oracle_result = differential_oracle::oracle_sub2( trace_entry.local_input0, trace_entry.local_input1, trace_entry.local_output, WORD_SIZE );
				if ( !oracle_result.possible )
				{
					++attempt.cddt_operator_steps.impossible_transitions;
					continue;
				}
				attempt.cddt_operator_steps.local_bound_weight_sum += static_cast<double>( oracle_result.weight );
				++attempt.cddt_operator_steps.sorted_branch_kept;
				if ( std::fabs( static_cast<double>( oracle_result.weight ) - trace_entry.local_weight ) > 1e-6 )
					++attempt.cddt_operator_steps.weight_mismatches;
			}
			attempt.cddt_operator_steps.model_weight_sum += trace_entry.local_weight;
		}
	}

	static bool forest_weight_matches( double left, double right )
	{
		return std::fabs( left - right ) <= 1e-6;
	}

	static InjectionKind injection_kind_from_trace_entry( const WeightTraceEntry& entry )
	{
		if ( entry.stage.find( "B_to_A" ) != std::string::npos || entry.injection_name.find( "J0" ) != std::string::npos )
			return InjectionKind::B_TO_A_AFTER_RC4;
		if ( entry.stage.find( "A_to_B" ) != std::string::npos || entry.injection_name.find( "J1" ) != std::string::npos )
			return InjectionKind::A_TO_B_AFTER_RC9;
		throw std::runtime_error( "cannot infer injection kind from trace entry: " + entry.stage + " / " + entry.injection_name );
	}

	static void append_q1_error( ForestQ1Verification& verification, const WeightTraceEntry& entry, const std::string& message )
	{
		verification.valid = false;
		++verification.q1_failed;
		verification.errors.push_back( "round=" + std::to_string( entry.round ) + " step=" + std::to_string( entry.step ) + " stage=" + entry.stage + ": " + message );
	}

	static ForestQ1Verification verify_trail_by_q1_oracle( const SearchOptions& options, const ScipSolveResult& result )
	{
		ForestQ1Verification verification;
		if ( !result.feasible )
			return verification;
		if ( options.constant_model != ConstantModel::FIXED_PUBLIC_EXACT )
		{
			verification.valid = false;
			++verification.q1_failed;
			verification.errors.push_back( "forest Q1 verification requires fixed-public-exact constant model" );
			return verification;
		}
		if ( !result.weight_trace_available || !result.weight_trace_matches_objective )
		{
			verification.valid = false;
			++verification.q1_failed;
			verification.errors.push_back( "weight trace is unavailable or does not match SCIP objective" );
			return verification;
		}

		InjectionRankOracle injection_oracle;
		for ( const auto& entry : result.weight_trace )
		{
			if ( entry.operation == "modular_addition" || entry.operation == "modular_subtraction" )
			{
				++verification.q1_calls;
				const bool is_subtraction = entry.operation == "modular_subtraction";
				const auto oracle_result = lookup_two_input_cddt( is_subtraction, entry.local_input0, entry.local_input1, entry.local_output, WORD_SIZE );
				++verification.cddt_operator_steps.local_bound_checks;
				++verification.cddt_operator_steps.branch_order_hints;
				++verification.cddt_operator_steps.sorted_branch_checks;
				if ( is_subtraction )
					++verification.cddt_operator_steps.two_variable_sub_steps;
				else
					++verification.cddt_operator_steps.two_variable_add_steps;
				if ( oracle_result.cache_hit )
					++verification.cddt_operator_steps.cache_hits;
				else
					++verification.cddt_operator_steps.cache_misses;
				const std::string operation_name = is_subtraction ? "modular subtraction" : "modular addition";
				if ( !oracle_result.possible )
				{
					++verification.impossible_transitions;
					++verification.cddt_operator_steps.impossible_transitions;
					append_q1_error( verification, entry, operation_name + " transition is impossible" );
				}
				else if ( std::fabs( static_cast<double>( oracle_result.weight ) - entry.local_weight ) > 1e-6 )
				{
					++verification.weight_mismatches;
					++verification.cddt_operator_steps.weight_mismatches;
					verification.cddt_operator_steps.local_bound_weight_sum += static_cast<long double>( oracle_result.weight );
					verification.cddt_operator_steps.model_weight_sum += static_cast<long double>( entry.local_weight );
					append_q1_error( verification, entry, operation_name + " Q1 weight mismatch" );
				}
				else
				{
					++verification.cddt_operator_steps.sorted_branch_kept;
					verification.cddt_operator_steps.local_bound_weight_sum += static_cast<long double>( oracle_result.weight );
					verification.cddt_operator_steps.model_weight_sum += static_cast<long double>( entry.local_weight );
				}
			}
			else if ( entry.operation == "fixed_public_constant_subtraction" )
			{
				++verification.q1_calls;
				const auto oracle_result = differential_oracle::oracle_sub_const( entry.public_constant, entry.local_input0, entry.local_output, WORD_SIZE );
				if ( !oracle_result.possible )
				{
					++verification.impossible_transitions;
					append_q1_error( verification, entry, "fixed-constant subtraction transition is impossible" );
				}
				else if ( !forest_weight_matches( static_cast<double>( oracle_result.weight ), entry.local_weight ) )
				{
					++verification.weight_mismatches;
					append_q1_error( verification, entry, "fixed-constant subtraction Q1 weight mismatch" );
				}
			}
			else if ( entry.operation == "fixed_public_constant_addition" )
			{
				++verification.q1_calls;
				const auto oracle_result = differential_oracle::oracle_add_const( entry.public_constant, entry.local_input0, entry.local_output, WORD_SIZE );
				if ( !oracle_result.possible )
				{
					++verification.impossible_transitions;
					append_q1_error( verification, entry, "fixed-constant addition transition is impossible" );
				}
				else if ( !forest_weight_matches( static_cast<double>( oracle_result.weight ), entry.local_weight ) )
				{
					++verification.weight_mismatches;
					append_q1_error( verification, entry, "fixed-constant addition Q1 weight mismatch" );
				}
			}
			else if ( entry.operation == "joint_injection_derivative" )
			{
				++verification.q1_calls;
				const InjectionKind kind = injection_kind_from_trace_entry( entry );
				const auto		   oracle_result = injection_oracle.transition( kind, entry.injection_din, entry.injection_joint_dout );
				if ( !oracle_result.valid )
				{
					++verification.impossible_transitions;
					append_q1_error( verification, entry, "joint injection transition is impossible" );
				}
				else if ( !forest_weight_matches( static_cast<double>( oracle_result.rank ), entry.model_rank_weight ) || !forest_weight_matches( static_cast<double>( oracle_result.rank ), entry.local_weight ) )
				{
					++verification.weight_mismatches;
					append_q1_error( verification, entry, "joint injection Q1 rank weight mismatch" );
				}
			}
		}
		return verification;
	}

	static SearchOptions make_best_search_options_for_input( SearchOptions base_options, const ForestInputDifference& input, double remaining_hull_time_seconds )
	{
		base_options.fix_input_da = input.dA;
		base_options.fix_input_db = input.dB;
		base_options.fix_output_da.reset();
		base_options.fix_output_db.reset();
		base_options.objective_window_enabled = false;
		base_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
		base_options.objective_window_to = std::numeric_limits<double>::quiet_NaN();
		base_options.time_limit_seconds = remaining_hull_time_seconds;
		base_options.output_result_json.clear();
		base_options.output_weight_trace_json.clear();
		base_options.output_round_table_json.clear();
		return base_options;
	}

	static SearchOptions make_endpoint_enumeration_options( SearchOptions base_options, const ForestOptions& forest_options, const Endpoint& endpoint, double local_best_weight, double remaining_hull_time_seconds )
	{
		set_fixed_endpoint( base_options, endpoint );
		base_options.time_limit_seconds = remaining_hull_time_seconds;
		base_options.output_result_json.clear();
		base_options.output_weight_trace_json.clear();
		base_options.output_round_table_json.clear();
		if ( forest_options.hull_mode == HullMode::BOUNDED_ENDPOINT )
		{
			base_options.objective_window_enabled = true;
			base_options.objective_window_from = std::isnan( forest_options.enumerate_from ) ? local_best_weight - 1e-8 : forest_options.enumerate_from;
			base_options.objective_window_to = std::isnan( forest_options.enumerate_to ) ? local_best_weight + forest_options.enumerate_window : forest_options.enumerate_to;
		}
		else
		{
			base_options.objective_window_enabled = false;
			base_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
			base_options.objective_window_to = std::numeric_limits<double>::quiet_NaN();
		}
		return base_options;
	}

	static std::string forest_stop_reason_from_result( const ScipSolveResult& result, const std::string& infeasible_reason )
	{
		if ( result.scip_status == SCIP_STATUS_INFEASIBLE )
			return infeasible_reason;
		if ( result.hit_time_limit )
			return result.feasible ? "solver_timeout_with_incumbent" : "solver_timeout_no_incumbent";
		if ( result.hit_memory_limit )
			return "solver_memory_limit";
		if ( result.hit_solution_limit )
			return "solver_solution_limit";
		return "solver_not_optimal";
	}

	// ------------------------------------------------------------------------
	// Audit section 3: endpoint enumeration aggregation
	// ------------------------------------------------------------------------
	// Q2/HULL aggregation layer: each accepted characteristic is verified by
	// exact Q1 oracles, grouped by weight, and blocked by a semantic no-good cut
	// before the next endpoint enumeration solve. The probability sum is computed
	// only from enumerated characteristics.
	static void record_feasible_forest_trail( ForestAttemptLog& attempt, ForestRunLog& run_log, const SearchOptions& options, const ScipSolveResult& result, std::map<std::string, int>& grouped_counts, EnumerationState& enumeration )
	{
		const ForestQ1Verification verification = verify_trail_by_q1_oracle( options, result );
		add_q1_result_to_attempt( attempt, verification );
		if ( !verification.valid )
		{
			attempt.pruned = true;
			attempt.stop_reason = "q1_verification_failed";
			return;
		}
		add_operator_step_result_to_attempt( attempt, result );
		const long double contribution = std::pow( 2.0L, -static_cast<long double>( result.snapshot.objective ) );
		grouped_counts[ weight_key( result.snapshot.objective ) ]++;
		attempt.probability_sum += contribution;
		++attempt.found_trails;
		++run_log.found_trails;
		attempt.local_best_weight = std::isnan( attempt.local_best_weight ) ? result.snapshot.objective : std::min( attempt.local_best_weight, result.snapshot.objective );
		update_hull_growth( attempt, result.snapshot.objective, contribution );
		update_stage_weight_profile( attempt.stage_weight_profile, result.weight_trace );
		attempt.endpoint = endpoint_from_snapshot( result.snapshot );
		attempt.has_endpoint = true;
		enumeration.no_good_cuts.push_back( result.no_good );
		++attempt.blocking_cuts;
		const double forest_total_weight = attempt.cumulative_weight_before + result.snapshot.objective;
		if ( !run_log.has_global_best || forest_total_weight < run_log.global_best_weight - 1e-8 )
		{
			run_log.has_global_best = true;
			run_log.global_best_weight = forest_total_weight;
			run_log.global_best_endpoint = endpoint_from_snapshot( result.snapshot );
			run_log.global_best_trail_count = 1;
			attempt.updated_global_best = true;
		}
		else if ( std::fabs( forest_total_weight - run_log.global_best_weight ) <= 1e-8 )
		{
			++run_log.global_best_trail_count;
		}
	}

	static void finalize_attempt_distribution( ForestAttemptLog& attempt, const std::map<std::string, int>& grouped_counts )
	{
		attempt.weight_distribution = make_weight_distribution( grouped_counts );
		attempt.effective_weight = effective_weight_from_probability( attempt.probability_sum );
	}

	static void absorb_attempt_totals( ForestRunLog& run_log, const ForestAttemptLog& attempt )
	{
		run_log.solver_calls += attempt.solver_calls;
		run_log.q1_calls += attempt.q1_calls;
		run_log.q1_failed += attempt.q1_failed;
		run_log.impossible_transitions += attempt.impossible_transitions;
		run_log.weight_mismatches += attempt.weight_mismatches;
		add_operator_step_result_to_run( run_log, attempt );
		merge_stage_weight_profile( run_log.operator_heatmap, attempt.stage_weight_profile );
		if ( attempt.bnb_prune_checked )
			++run_log.bnb_prune_checks;
		if ( attempt.bnb_pruned )
		{
			++run_log.bnb_prune_applied;
			++run_log.bnb_pruned_attempts;
		}
		if ( attempt.bnb_prune_checked && !attempt.bnb_pruned && attempt.bnb_prune_message.find( "solver incomplete" ) != std::string::npos )
			++run_log.bnb_prune_deferred;
		run_log.bnb_objective_cutoff_constraints += attempt.cddt_operator_steps.solver_cutoff_constraints;
		if ( attempt.completed )
			++run_log.completed_attempts;
		if ( attempt.pruned )
			++run_log.pruned_branches;
		if ( attempt.cddt_branch_ordered )
		{
			++run_log.cddt_branch_ordered_attempts;
			++run_log.cddt_branch_score_checks;
		}
		run_log.cddt_early_prune_checks += attempt.cddt_operator_steps.early_prune_checks;
		if ( attempt.cddt_early_pruned && attempt.cddt_operator_steps.early_prune_checks == 0 )
			++run_log.cddt_early_prune_checks;
		if ( attempt.cddt_early_pruned )
			++run_log.cddt_early_pruned_attempts;
		if ( attempt.hit_time_limit || attempt.hit_memory_limit || attempt.hit_solution_limit )
			run_log.saw_partial_limit = true;
		print_cddt_audit( attempt );
	}

	static void write_forest_input_json( std::ostream& output_stream, const ForestInputDifference& input, const std::string& indent )
	{
		output_stream << indent << "{"
					  << "\"dA_in\": " << json_string( hex32_json( input.dA ) )
					  << ", \"dB_in\": " << json_string( hex32_json( input.dB ) )
					  << "}";
	}

	static void write_forest_string_array_json( std::ostream& output_stream, const std::vector<std::string>& values, const std::string& indent )
	{
		output_stream << "[";
		if ( !values.empty() )
			output_stream << "\n";
		for ( std::size_t i = 0; i < values.size(); ++i )
		{
			output_stream << indent << "  " << json_string( values[ i ] );
			if ( i + 1 != values.size() )
				output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static void write_operator_step_profile_json( std::ostream& output_stream, const OperatorStepProfile& profile, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"verified_trails\": " << profile.verified_trails << ",\n"
					  << indent << "  \"trace_entries\": " << profile.trace_entries << ",\n"
					  << indent << "  \"modular_addition\": " << profile.modular_addition << ",\n"
					  << indent << "  \"modular_subtraction\": " << profile.modular_subtraction << ",\n"
					  << indent << "  \"fixed_public_constant_subtraction\": " << profile.fixed_public_constant_subtraction << ",\n"
					  << indent << "  \"fixed_public_constant_addition\": " << profile.fixed_public_constant_addition << ",\n"
					  << indent << "  \"joint_injection_derivative\": " << profile.joint_injection_derivative << ",\n"
					  << indent << "  \"linear_xor_bridge\": " << profile.linear_xor_bridge << ",\n"
					  << indent << "  \"public_xor_constant\": " << profile.public_xor_constant << ",\n"
					  << indent << "  \"other\": " << profile.other << "\n"
					  << indent << "}";
	}

	static void write_cddt_operator_profile_json( std::ostream& output_stream, const CddtOperatorProfile& profile, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"two_variable_add_steps\": " << profile.two_variable_add_steps << ",\n"
					  << indent << "  \"two_variable_sub_steps\": " << profile.two_variable_sub_steps << ",\n"
					  << indent << "  \"local_bound_checks\": " << profile.local_bound_checks << ",\n"
					  << indent << "  \"branch_order_hints\": " << profile.branch_order_hints << ",\n"
					  << indent << "  \"sorted_branch_checks\": " << profile.sorted_branch_checks << ",\n"
					  << indent << "  \"sorted_branch_kept\": " << profile.sorted_branch_kept << ",\n"
					  << indent << "  \"early_prune_checks\": " << profile.early_prune_checks << ",\n"
					  << indent << "  \"early_pruned_by_objective_cutoff\": " << profile.early_pruned_by_objective_cutoff << ",\n"
					  << indent << "  \"solver_cutoff_constraints\": " << profile.solver_cutoff_constraints << ",\n"
					  << indent << "  \"cache_hits\": " << profile.cache_hits << ",\n"
					  << indent << "  \"cache_misses\": " << profile.cache_misses << ",\n"
					  << indent << "  \"impossible_transitions\": " << profile.impossible_transitions << ",\n"
					  << indent << "  \"weight_mismatches\": " << profile.weight_mismatches << ",\n"
					  << indent << "  \"local_bound_weight_sum\": " << json_number( profile.local_bound_weight_sum ) << ",\n"
					  << indent << "  \"model_weight_sum\": " << json_number( profile.model_weight_sum ) << "\n"
					  << indent << "}";
	}

	static void write_bnb_prune_audit_json( std::ostream& output_stream, const ForestAttemptLog& attempt, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"checked\": " << ( attempt.bnb_prune_checked ? "true" : "false" ) << ",\n"
					  << indent << "  \"applied\": " << ( attempt.bnb_pruned ? "true" : "false" ) << ",\n"
					  << indent << "  \"proven_by_solver\": " << ( attempt.bnb_prune_proven ? "true" : "false" ) << ",\n"
					  << indent << "  \"previous_global_best_weight\": " << json_number( attempt.bnb_previous_global_best ) << ",\n"
					  << indent << "  \"candidate_weight\": " << json_number( attempt.bnb_candidate_weight ) << ",\n"
					  << indent << "  \"cutoff_weight\": " << json_number( attempt.bnb_cutoff_weight ) << ",\n"
					  << indent << "  \"gap\": " << json_number( std::isfinite( attempt.bnb_previous_global_best ) && std::isfinite( attempt.bnb_candidate_weight ) ? ( attempt.bnb_candidate_weight - attempt.bnb_previous_global_best ) : std::numeric_limits<double>::quiet_NaN() ) << ",\n"
					  << indent << "  \"rule\": " << json_string( attempt.bnb_prune_rule ) << ",\n"
					  << indent << "  \"bound_expression\": " << json_string( attempt.bnb_bound_expression ) << ",\n"
					  << indent << "  \"message\": " << json_string( attempt.bnb_prune_message ) << "\n"
					  << indent << "}";
	}

	static void write_cddt_free_slot_json( std::ostream& output_stream, const CddtFreeSlotResult& result, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"possible\": " << ( result.possible ? "true" : "false" ) << ",\n"
					  << indent << "  \"best_value\": " << ( result.possible ? json_string( hex32_json( result.value ) ) : "null" ) << ",\n"
					  << indent << "  \"best_weight\": " << ( result.possible ? std::to_string( result.weight ) : "null" ) << ",\n"
					  << indent << "  \"best_value_count\": " << result.best_value_count << ",\n"
					  << indent << "  \"sorted_branch_candidates\": " << result.sorted_branch_candidates << ",\n"
					  << indent << "  \"sorted_branch_pruned_by_cutoff\": " << result.sorted_branch_pruned_by_cutoff << "\n"
					  << indent << "}";
	}

	static void write_cddt_branch_score_json( std::ostream& output_stream, const ForestAttemptLog& attempt, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"ordered\": " << ( attempt.cddt_branch_ordered ? "true" : "false" ) << ",\n"
					  << indent << "  \"rank\": " << attempt.cddt_branch_rank << ",\n"
					  << indent << "  \"message\": " << json_string( attempt.cddt_branch_order_message ) << ",\n"
					  << indent << "  \"possible\": " << ( attempt.cddt_branch_score.possible ? "true" : "false" ) << ",\n"
					  << indent << "  \"score\": " << json_number( attempt.cddt_branch_score.score ) << ",\n"
					  << indent << "  \"early_pruned\": " << ( attempt.cddt_early_pruned ? "true" : "false" ) << ",\n"
					  << indent << "  \"prune_reason\": " << json_string( attempt.cddt_branch_score.prune_reason ) << ",\n"
					  << indent << "  \"add_output_hint\": ";
		write_cddt_free_slot_json( output_stream, attempt.cddt_branch_score.add_output, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"sub_output_hint\": ";
		write_cddt_free_slot_json( output_stream, attempt.cddt_branch_score.sub_output, indent + "  " );
		output_stream << "\n" << indent << "}";
	}


	static void write_hull_growth_json( std::ostream& output_stream, const std::vector<HullGrowthPoint>& growth, const std::string& indent )
	{
		output_stream << "[";
		if ( !growth.empty() ) output_stream << "\n";
		for ( std::size_t index = 0; index < growth.size(); ++index )
		{
			const auto& point = growth[ index ];
			output_stream << indent << "  {\"window\": " << json_number( point.window )
						  << ", \"count\": " << point.count
						  << ", \"probability_sum\": " << json_string( scientific_string( point.probability_sum ) )
						  << ", \"effective_weight\": " << json_number( effective_weight_from_probability( point.probability_sum ) ) << "}";
			if ( index + 1 != growth.size() ) output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static void write_stage_weight_profile_json( std::ostream& output_stream, const std::map<std::string, StageWeightAggregate>& profile, const std::string& indent )
	{
		output_stream << "{";
		if ( !profile.empty() ) output_stream << "\n";
		std::size_t index = 0;
		for ( const auto& [ stage, aggregate ] : profile )
		{
			output_stream << indent << "  " << json_string( stage ) << ": {\"count\": " << aggregate.count
						  << ", \"sum_weight\": " << json_number( aggregate.sum_weight )
						  << ", \"average_weight\": " << json_number( aggregate.count > 0 ? aggregate.sum_weight / aggregate.count : std::numeric_limits<double>::quiet_NaN() )
						  << ", \"min_weight\": " << json_number( aggregate.min_weight )
						  << ", \"max_weight\": " << json_number( aggregate.max_weight ) << "}";
			if ( ++index != profile.size() ) output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "}";
	}

	static void write_layer_growth_json( std::ostream& output_stream, const std::map<int, LayerGrowthSummary>& layers, const std::string& indent )
	{
		output_stream << "[";
		if ( !layers.empty() ) output_stream << "\n";
		std::size_t index = 0;
		for ( const auto& [ layer_number, layer ] : layers )
		{
			output_stream << indent << "  {\"layer\": " << layer_number
						  << ", \"attempts\": " << layer.attempts
						  << ", \"feasible\": " << layer.feasible
						  << ", \"continuations_enqueued\": " << layer.continuations_enqueued
						  << ", \"cycles_detected\": " << layer.cycles_detected
						  << ", \"min_local_weight\": " << json_number( layer.min_local_weight )
						  << ", \"min_cumulative_weight\": " << json_number( layer.min_cumulative_weight )
						  << ", \"min_average_weight\": " << json_number( layer.min_average_weight ) << "}";
			if ( ++index != layers.size() ) output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static void write_forest_attempt_json( std::ostream& output_stream, const ForestAttemptLog& attempt, const std::string& indent )
	{
		output_stream << indent << "{\n"
					  << indent << "  \"attempt_id\": " << attempt.attempt_id << ",\n"
					  << indent << "  \"parent_attempt_id\": " << attempt.parent_attempt_id << ",\n"
					  << indent << "  \"tree_id\": " << attempt.tree_id << ",\n"
					  << indent << "  \"layer\": " << attempt.layer << ",\n"
					  << indent << "  \"source_kind\": " << json_string( attempt.source_kind ) << ",\n"
					  << indent << "  \"derived_root\": " << ( attempt.derived_root ? "true" : "false" ) << ",\n"
					  << indent << "  \"input_frequency\": " << attempt.input_frequency << ",\n"
					  << indent << "  \"output_frequency\": " << attempt.output_frequency << ",\n"
					  << indent << "  \"candidate_priority\": " << json_number( attempt.candidate_priority ) << ",\n"
					  << indent << "  \"candidate_bank_size_before\": " << attempt.candidate_bank_size_before << ",\n"
					  << indent << "  \"candidate_bank_size_after\": " << attempt.candidate_bank_size_after << ",\n"
					  << indent << "  \"cumulative_weight_before\": " << json_number( attempt.cumulative_weight_before ) << ",\n"
					  << indent << "  \"cumulative_weight_after\": " << json_number( attempt.cumulative_weight_after ) << ",\n"
					  << indent << "  \"average_weight_per_layer\": " << json_number( attempt.average_weight_per_layer ) << ",\n"
					  << indent << "  \"continuation_enqueued\": " << ( attempt.continuation_enqueued ? "true" : "false" ) << ",\n"
					  << indent << "  \"continuation_dominated\": " << ( attempt.continuation_dominated ? "true" : "false" ) << ",\n"
					  << indent << "  \"continuation_zero_state\": " << ( attempt.continuation_zero_state ? "true" : "false" ) << ",\n"
					  << indent << "  \"cycle_detected\": " << ( attempt.cycle_detected ? "true" : "false" ) << ",\n"
					  << indent << "  \"cycle_start_layer\": " << attempt.cycle_start_layer << ",\n"
					  << indent << "  \"cycle_length\": " << attempt.cycle_length << ",\n"
					  << indent << "  \"cycle_weight\": " << json_number( attempt.cycle_weight ) << ",\n"
					  << indent << "  \"cycle_average_weight\": " << json_number( attempt.cycle_average_weight ) << ",\n"
					  << indent << "  \"derive_seed\": " << json_string( "0x" + hex64_forest( attempt.derive_seed ) ) << ",\n"
					  << indent << "  \"derive_input_source\": ";
		if ( attempt.has_derive_input_source )
			write_forest_input_json( output_stream, attempt.derive_input_source, indent + "  " );
		else
			output_stream << "null";
		output_stream << ",\n"
					  << indent << "  \"old_input_source\": ";
		if ( attempt.has_old_input )
			write_forest_input_json( output_stream, attempt.old_input, indent + "  " );
		else
			output_stream << "null";
		output_stream << ",\n" << indent << "  \"new_input_source\": ";
		write_forest_input_json( output_stream, attempt.input, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"input_source_change_message\": " << json_string( attempt.input_source_change_message ) << ",\n"
					  << indent << "  \"propagation_trunk_message\": " << json_string( attempt.propagation_trunk_message ) << ",\n"
					  << indent << "  \"cddt_branch_order\": ";
		write_cddt_branch_score_json( output_stream, attempt, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"duplicate_input\": " << ( attempt.duplicate_input ? "true" : "false" ) << ",\n"
					  << indent << "  \"pruned\": " << ( attempt.pruned ? "true" : "false" ) << ",\n"
					  << indent << "  \"completed\": " << ( attempt.completed ? "true" : "false" ) << ",\n"
					  << indent << "  \"updated_global_best\": " << ( attempt.updated_global_best ? "true" : "false" ) << ",\n"
					  << indent << "  \"stop_reason\": " << json_string( attempt.stop_reason ) << ",\n"
					  << indent << "  \"solver_status\": " << json_string( attempt.solver_status ) << ",\n"
					  << indent << "  \"solver_calls\": " << attempt.solver_calls << ",\n"
					  << indent << "  \"found_trails\": " << attempt.found_trails << ",\n"
					  << indent << "  \"blocking_cuts\": " << attempt.blocking_cuts << ",\n"
					  << indent << "  \"q1_calls\": " << attempt.q1_calls << ",\n"
					  << indent << "  \"q1_failed\": " << attempt.q1_failed << ",\n"
					  << indent << "  \"impossible_transitions\": " << attempt.impossible_transitions << ",\n"
					  << indent << "  \"weight_mismatches\": " << attempt.weight_mismatches << ",\n"
					  << indent << "  \"solver_best_weight\": " << json_number( attempt.solver_best_weight ) << ",\n"
					  << indent << "  \"solver_best_complete\": " << ( attempt.solver_best_complete ? "true" : "false" ) << ",\n"
					  << indent << "  \"local_best_weight\": " << json_number( attempt.local_best_weight ) << ",\n"
					  << indent << "  \"milp_operator_steps\": ";
		write_operator_step_profile_json( output_stream, attempt.operator_steps, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"cddt_operator_steps\": ";
		write_cddt_operator_profile_json( output_stream, attempt.cddt_operator_steps, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"bnb_prune\": ";
		write_bnb_prune_audit_json( output_stream, attempt, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"endpoint\": ";
		if ( attempt.has_endpoint )
			write_endpoint_json( output_stream, attempt.endpoint, indent + "  " );
		else
			output_stream << "null";
		output_stream << ",\n"
					  << indent << "  \"weight_distribution\": ";
		write_weight_distribution_json( output_stream, attempt.weight_distribution, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"probability_polynomial_variable\": \"x=2^-1\",\n"
					  << indent << "  \"probability_polynomial\": " << json_string( probability_polynomial_string( attempt.weight_distribution ) ) << ",\n"
					  << indent << "  \"probability_polynomial_evaluation\": \"A(1/2)\",\n"
					  << indent << "  \"probability_sum\": " << json_string( scientific_string( attempt.probability_sum ) ) << ",\n"
					  << indent << "  \"effective_weight\": " << json_number( attempt.effective_weight ) << ",\n"
					  << indent << "  \"hull_growth\": ";
		write_hull_growth_json( output_stream, attempt.hull_growth, indent + "  " );
		output_stream << ",\n" << indent << "  \"operator_heatmap\": ";
		write_stage_weight_profile_json( output_stream, attempt.stage_weight_profile, indent + "  " );
		output_stream << ",\n" << indent << "  \"q1_errors\": ";
		write_forest_string_array_json( output_stream, attempt.q1_errors, indent + "  " );
		output_stream << "\n" << indent << "}";
	}

	// ------------------------------------------------------------------------
	// Audit section 4: JSON artifact writers
	// ------------------------------------------------------------------------
	static void write_forest_json_file( const std::string& path, const ForestRunLog& run_log )
	{
		if ( path.empty() )
			return;
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open forest JSON output file: " + path );
		out << "{\n"
			<< "  \"analysis_algorithm\": " << json_string( run_log.analysis_algorithm ) << ",\n"
			<< "  \"solve_target\": " << json_string( run_log.solve_target ) << ",\n"
			<< "  \"analysis\": \"xor_differential_forest_hull_search\",\n"
			<< "  \"cipher\": \"NeoAlzette\",\n"
			<< "  \"backend\": \"SCIP_C_API\",\n"
			<< "  \"scope\": \"time_bounded_multi_layer_difference_forest\",\n"
			<< "  \"result_type\": " << json_string( run_log.result_type ) << ",\n"
			<< "  \"rounds\": " << run_log.search_options.rounds << ",\n"
			<< "  \"constant_model\": " << json_string( constant_model_name( run_log.search_options.constant_model ) ) << ",\n"
			<< "  \"hull_mode\": " << json_string( hull_mode_name( run_log.forest_options.hull_mode ) ) << ",\n"
			<< "  \"forest_seed\": " << json_string( "0x" + hex64_forest( run_log.forest_options.seed ) ) << ",\n"
			<< "  \"forest_attempt_limit\": " << run_log.forest_options.attempts << ",\n"
			<< "  \"forest_attempt_limit_policy\": \"0_means_time_limit_only\",\n"
			<< "  \"growth_policy\": \"q1_valid_best_output_becomes_next_layer_input\",\n"
			<< "  \"time_budget_policy\": " << json_string( run_log.time_budget_policy ) << ",\n"
			<< "  \"time_budget_scope\": " << json_string( run_log.time_budget_scope ) << ",\n"
			<< "  \"time_limit_policy\": \"single_global_hull_time_limit_remaining_budget\",\n"
			<< "  \"hull_time_limit_seconds\": " << json_number( run_log.forest_options.hull_time_limit_seconds ) << ",\n"
			<< "  \"hit_global_time_limit\": " << ( run_log.hit_global_time_limit ? "true" : "false" ) << ",\n"
			<< "  \"total_attempts\": " << run_log.total_attempts << ",\n"
			<< "  \"completed_attempts\": " << run_log.completed_attempts << ",\n"
			<< "  \"pruned_branches\": " << run_log.pruned_branches << ",\n"
			<< "  \"candidate_bank_peak\": " << run_log.candidate_bank_peak << ",\n"
			<< "  \"derived_roots_enqueued\": " << run_log.derived_roots_enqueued << ",\n"
			<< "  \"continuations_enqueued\": " << run_log.continuations_enqueued << ",\n"
			<< "  \"continuations_dominated\": " << run_log.continuations_dominated << ",\n"
			<< "  \"cycles_detected\": " << run_log.cycles_detected << ",\n"
			<< "  \"max_layer\": " << run_log.max_layer << ",\n"
			<< "  \"unique_input_states\": " << run_log.unique_input_states << ",\n"
			<< "  \"unique_output_states\": " << run_log.unique_output_states << ",\n"
			<< "  \"cddt_branch_ordered_attempts\": " << run_log.cddt_branch_ordered_attempts << ",\n"
			<< "  \"cddt_branch_score_checks\": " << run_log.cddt_branch_score_checks << ",\n"
			<< "  \"cddt_early_prune_checks\": " << run_log.cddt_early_prune_checks << ",\n"
			<< "  \"cddt_early_pruned_attempts\": " << run_log.cddt_early_pruned_attempts << ",\n"
			<< "  \"solver_calls\": " << run_log.solver_calls << ",\n"
			<< "  \"found_trails\": " << run_log.found_trails << ",\n"
			<< "  \"q1_calls\": " << run_log.q1_calls << ",\n"
			<< "  \"q1_failed\": " << run_log.q1_failed << ",\n"
			<< "  \"impossible_transitions\": " << run_log.impossible_transitions << ",\n"
			<< "  \"weight_mismatches\": " << run_log.weight_mismatches << ",\n"
			<< "  \"milp_operator_steps\": ";
		write_operator_step_profile_json( out, run_log.operator_steps, "  " );
		out << ",\n"
			<< "  \"cddt_operator_steps\": ";
		write_cddt_operator_profile_json( out, run_log.cddt_operator_steps, "  " );
		out << ",\n"
			<< "  \"bnb_pruned_attempts\": " << run_log.bnb_pruned_attempts << ",\n"
			<< "  \"bnb_prune_checks\": " << run_log.bnb_prune_checks << ",\n"
			<< "  \"bnb_prune_applied\": " << run_log.bnb_prune_applied << ",\n"
			<< "  \"bnb_prune_deferred\": " << run_log.bnb_prune_deferred << ",\n"
			<< "  \"bnb_objective_cutoff_constraints\": " << run_log.bnb_objective_cutoff_constraints << ",\n"
			<< "  \"global_best_weight\": " << ( run_log.has_global_best ? json_number( run_log.global_best_weight ) : "null" ) << ",\n"
			<< "  \"global_best_trail_count\": " << run_log.global_best_trail_count << ",\n"
			<< "  \"global_best_endpoint\": ";
		if ( run_log.has_global_best )
			write_endpoint_json( out, run_log.global_best_endpoint, "  " );
		else
			out << "null";
		out << ",\n  \"layer_growth\": ";
		write_layer_growth_json( out, run_log.layer_growth, "  " );
		out << ",\n  \"operator_heatmap\": ";
		write_stage_weight_profile_json( out, run_log.operator_heatmap, "  " );
		out << ",\n  \"attempts\": [";
		if ( !run_log.attempts.empty() )
			out << "\n";
		for ( std::size_t i = 0; i < run_log.attempts.size(); ++i )
		{
			write_forest_attempt_json( out, run_log.attempts[ i ], "    " );
			if ( i + 1 != run_log.attempts.size() )
				out << ",";
			out << "\n";
		}
		out << "  ]\n}\n";
		std::cout << "FOREST_JSON_FILE=" << path << "\n";
	}

	static void validate_forest_configuration( const SearchOptions& options, const ForestOptions& forest_options )
	{
		if ( options.output_round_table_json.size() )
			throw std::runtime_error( "round-table output is implemented by neoalzette_scip_round_milp_search" );
		if ( options.constant_model != ConstantModel::FIXED_PUBLIC_EXACT )
			throw std::runtime_error( "forest Q1 mode requires --constant-model fixed-public-exact" );
		if ( ( *options.fix_input_da | *options.fix_input_db ) == 0 )
			throw std::runtime_error( "forest mode requires a nonzero input-difference source" );
		if ( options.fix_output_da || options.fix_output_db )
			throw std::runtime_error( "forest mode derives output endpoints from the solver; do not pass --fix-output-da/--fix-output-db" );
		if ( !std::isfinite( forest_options.hull_time_limit_seconds ) || forest_options.hull_time_limit_seconds <= 0.0 )
			throw std::runtime_error( "forest mode requires --hull-time-limit to avoid an unbounded run" );
		if ( forest_options.attempts < 0 )
			throw std::runtime_error( "--forest-attempts must be >= 0 (0 means time-limit-only)" );
		if ( forest_options.max_enumerate_solutions < 1 )
			throw std::runtime_error( "--max-enumerate-solutions must be >= 1" );
		if ( forest_options.hull_mode == HullMode::BEST_TRAIL && forest_options.hull_mode_explicit )
			throw std::runtime_error( "best-trail mode is implemented by neoalzette_scip_round_milp_search; hull_search runs forest hull search" );
		if ( forest_options.hull_mode == HullMode::ENDPOINT_CANDIDATE_SWEEP )
			throw std::runtime_error( "endpoint-candidate-sweep is disabled in forest Q1 mode; no Q2/candidate generation is used" );
	}

	static void finalize_run_result_type( ForestRunLog& run_log )
	{
		if ( run_log.hit_global_time_limit || run_log.saw_partial_limit ||
			( run_log.forest_options.attempts > 0 && run_log.total_attempts < run_log.forest_options.attempts ) )
		{
			run_log.result_type = "best_found_partial_forest";
			return;
		}
		if ( run_log.forest_options.hull_mode == HullMode::COMPLETE_ENDPOINT )
		{
			run_log.result_type = ( run_log.completed_attempts == run_log.total_attempts )
				? "complete_endpoint_under_current_model"
				: "best_found_partial_forest";
			return;
		}
		run_log.result_type = "threshold_bounded_hull_like_exploration";
	}

	// ------------------------------------------------------------------------
	// Audit section 5: forest hull driver
	// ------------------------------------------------------------------------
	// Driver flow:
	//   1. sample or derive input-difference attempts under the fixed source,
	//   2. solve best characteristics to discover endpoints,
	//   3. enumerate each selected endpoint with semantic no-good cuts, and
	//   4. accumulate a weight distribution for the characteristics found.
	// Scope note for ARX hull searches: the probability polynomial is the sum of
	// the enumerated characteristics. bounded-endpoint mode is windowed, while
	// complete-endpoint is complete only for a selected endpoint when enumeration
	// proves UNSAT after all recorded no-good cuts under the current MILP model.
	// Every SCIP call and enumeration pass consumes the single --hull-time-limit
	// budget, so partial JSON output is expected when that global budget expires.
	static void run_forest_hull_search( SearchOptions options, ForestOptions forest_options )
	{
		ForestRunLog run_log;
		run_log.forest_options = forest_options;
		run_log.search_options = options;
		run_log.analysis_algorithm = "MILP Solver Operator Steps + Multi-Layer Forest Growth";
		run_log.result_type = forest_options.hull_mode == HullMode::COMPLETE_ENDPOINT ? "complete_endpoint_under_current_model" : "threshold_bounded_hull_like_exploration";

		const auto forest_start = std::chrono::steady_clock::now();
		const ForestInputDifference initial_input { *options.fix_input_da, *options.fix_input_db };
		ForestCandidateQueue candidate_bank;
		std::map<std::uint64_t, std::vector<ForestStateFrontierEntry>> frontier;
		std::map<std::uint64_t, int> input_frequency;
		std::map<std::uint64_t, int> output_frequency;
		std::uint64_t candidate_serial = 0;
		int next_tree_id = 1;
		int derived_root_index = 0;

		auto enqueue_candidate = [&]( ForestCandidate candidate ) {
			if ( !register_forest_frontier( frontier, candidate ) )
				return false;
			candidate_bank.push( std::move( candidate ) );
			run_log.candidate_bank_peak = std::max( run_log.candidate_bank_peak, static_cast<int>( candidate_bank.size() ) );
			return true;
		};

		const std::uint64_t initial_key = pack_input_difference( initial_input );
		enqueue_candidate( make_forest_candidate( initial_input, -1, 0, 0, false,
			forest_options.seed ^ initial_key, 0.0, candidate_serial++, false, {}, { initial_key }, { 0.0 } ) );

		auto enqueue_derived_root = [&]() {
			for ( int retry = 0; retry < 4096; ++retry )
			{
				std::uint64_t derive_seed = 0;
				const int root_number = ++derived_root_index;
				const ForestInputDifference root_input = derive_next_input_difference( initial_input, forest_options.seed, root_number, derive_seed );
				const std::uint64_t root_key = pack_input_difference( root_input );
				ForestCandidate candidate = make_forest_candidate( root_input, -1, next_tree_id, 0, true,
					derive_seed, 0.0, candidate_serial++, true, initial_input, { root_key }, { 0.0 } );
				if ( enqueue_candidate( std::move( candidate ) ) )
				{
					++next_tree_id;
					++run_log.derived_roots_enqueued;
					return true;
				}
			}
			return false;
		};

		while ( !forest_deadline_expired( forest_options, forest_start ) &&
			( forest_options.attempts == 0 || run_log.total_attempts < forest_options.attempts ) )
		{
			if ( candidate_bank.empty() && !enqueue_derived_root() )
				break;

			const int bank_size_before = static_cast<int>( candidate_bank.size() );
			ForestCandidate candidate = candidate_bank.top();
			candidate_bank.pop();
			ForestAttemptLog attempt = make_forest_attempt( candidate, run_log.total_attempts, bank_size_before );
			if ( run_log.has_global_best )
			{
				const double local_cutoff = run_log.global_best_weight - candidate.cumulative_weight;
				attempt.bnb_prune_checked = true;
				attempt.bnb_previous_global_best = run_log.global_best_weight;
				attempt.bnb_cutoff_weight = local_cutoff;
				attempt.bnb_prune_rule = "cddt_q2_sorted_free_slot_bound";
				attempt.bnb_bound_expression = "candidate_cumulative_weight + CDDT_Q2_local_lower_bound <= global_best_weight";
				apply_cddt_cutoff_rescore_for_attempt( attempt, local_cutoff );
				attempt.bnb_candidate_weight = std::isfinite( attempt.cddt_branch_score.score ) ? candidate.cumulative_weight + attempt.cddt_branch_score.score : std::numeric_limits<double>::infinity();
				if ( local_cutoff < -1e-8 )
				{
					attempt.cddt_early_pruned = true;
					attempt.bnb_pruned = true;
					attempt.bnb_prune_proven = true;
					attempt.pruned = true;
					attempt.stop_reason = "cddt_q2_cutoff_negative_after_cumulative_weight";
					attempt.bnb_prune_message = "cumulative weight already exceeds the current forest global best";
					++attempt.cddt_operator_steps.early_pruned_by_objective_cutoff;
				}
				else if ( cddt_score_exceeds_cutoff( attempt.cddt_branch_score, local_cutoff ) )
				{
					attempt.bnb_prune_message = "CDDT Q2 helper exceeded cutoff; kept for SCIP because this source-level helper is a bound/order oracle, not a complete operator-level Matsui branch";
				}
				else
				{
					attempt.bnb_prune_message = "CDDT Q2 bound kept candidate; SCIP receives the same objective cutoff";
				}
			}
			attempt.input_frequency = ++input_frequency[ pack_input_difference( attempt.input ) ];
			attempt.duplicate_input = attempt.input_frequency > 1;
			run_log.max_layer = std::max( run_log.max_layer, attempt.layer );
			auto& layer_summary = run_log.layer_growth[ attempt.layer ];
			layer_summary.layer = attempt.layer;
			++layer_summary.attempts;

			print_input_source_change( attempt );
			print_cddt_branch_order( attempt );
			++run_log.total_attempts;


			if ( attempt.cddt_early_pruned )
			{
				attempt.candidate_bank_size_after = static_cast<int>( candidate_bank.size() );
				absorb_attempt_totals( run_log, attempt );
				run_log.attempts.push_back( std::move( attempt ) );
				continue;
			}

			const double best_remaining_hull_time = forest_remaining_budget_for_solver( forest_options, forest_start );
			SearchOptions best_options = make_best_search_options_for_input( options, attempt.input, best_remaining_hull_time );
			if ( attempt.bnb_prune_checked && std::isfinite( attempt.bnb_cutoff_weight ) )
			{
				best_options.objective_window_enabled = true;
				best_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
				best_options.objective_window_to = std::max( 0.0, attempt.bnb_cutoff_weight );
				++attempt.cddt_operator_steps.solver_cutoff_constraints;
			}
			EnumerationState best_enumeration;
			ScipSolveResult best_result = solve_in_model( best_options, best_enumeration, true );
			++attempt.solver_calls;
			print_time_limit_incumbent_note( attempt.attempt_id, "best_for_input", best_result );
			attempt.solver_status = scip_status_name( best_result.scip_status );
			attempt.hit_time_limit = best_result.hit_time_limit;
			attempt.hit_memory_limit = best_result.hit_memory_limit;
			attempt.hit_solution_limit = best_result.hit_solution_limit;
			if ( best_result.hit_time_limit ) run_log.hit_global_time_limit = true;

			if ( !best_result.feasible )
			{
				attempt.pruned = true;
				attempt.stop_reason = forest_stop_reason_from_result( best_result, "solver_infeasible_for_input" );
				attempt.candidate_bank_size_after = static_cast<int>( candidate_bank.size() );
				absorb_attempt_totals( run_log, attempt );
				run_log.attempts.push_back( std::move( attempt ) );
				if ( run_log.hit_global_time_limit ) break;
				continue;
			}

			std::map<std::string, int> grouped_counts;
			EnumerationState endpoint_enumeration;
			record_feasible_forest_trail( attempt, run_log, best_options, best_result, grouped_counts, endpoint_enumeration );
			attempt.solver_best_weight = best_result.snapshot.objective;
			attempt.solver_best_complete = best_result.complete;
			if ( attempt.stop_reason == "q1_verification_failed" )
			{
				finalize_attempt_distribution( attempt, grouped_counts );
				attempt.candidate_bank_size_after = static_cast<int>( candidate_bank.size() );
				absorb_attempt_totals( run_log, attempt );
				run_log.attempts.push_back( std::move( attempt ) );
				continue;
			}

			++layer_summary.feasible;
			attempt.cumulative_weight_after = candidate.cumulative_weight + best_result.snapshot.objective;
			attempt.average_weight_per_layer = attempt.cumulative_weight_after / static_cast<double>( attempt.layer + 1 );
			layer_summary.min_local_weight = std::min( layer_summary.min_local_weight, best_result.snapshot.objective );
			layer_summary.min_cumulative_weight = std::min( layer_summary.min_cumulative_weight, attempt.cumulative_weight_after );
			layer_summary.min_average_weight = std::min( layer_summary.min_average_weight, attempt.average_weight_per_layer );

			const Endpoint best_endpoint = endpoint_from_snapshot( best_result.snapshot );
			const ForestInputDifference child_input { best_endpoint.dA_out, best_endpoint.dB_out };
			attempt.output_frequency = ++output_frequency[ pack_input_difference( child_input ) ];
			if ( ( child_input.dA | child_input.dB ) == 0 )
			{
				attempt.continuation_zero_state = true;
			}
			else
			{
				const std::uint64_t child_key = pack_input_difference( child_input );
				const int cycle_start = find_cycle_start_index( candidate, child_key );
				if ( cycle_start >= 0 )
				{
					attempt.cycle_detected = true;
					attempt.cycle_start_layer = cycle_start;
					attempt.cycle_length = attempt.layer + 1 - cycle_start;
					attempt.cycle_weight = attempt.cumulative_weight_after - candidate.path_cumulative_weights[ static_cast<std::size_t>( cycle_start ) ];
					attempt.cycle_average_weight = attempt.cycle_weight / static_cast<double>( attempt.cycle_length );
					++run_log.cycles_detected;
					++layer_summary.cycles_detected;
				}
				else
				{
					auto child_path_inputs = candidate.path_inputs;
					auto child_path_weights = candidate.path_cumulative_weights;
					child_path_inputs.push_back( child_key );
					child_path_weights.push_back( attempt.cumulative_weight_after );
					ForestCandidate child = make_forest_candidate( child_input, attempt.attempt_id, candidate.tree_id, attempt.layer + 1,
						false, candidate.derive_seed, attempt.cumulative_weight_after, candidate_serial++, true, attempt.input,
						std::move( child_path_inputs ), std::move( child_path_weights ) );
					if ( enqueue_candidate( std::move( child ) ) )
					{
						attempt.continuation_enqueued = true;
						++run_log.continuations_enqueued;
						++layer_summary.continuations_enqueued;
					}
					else
					{
						attempt.continuation_dominated = true;
						++run_log.continuations_dominated;
					}
				}
			}

			if ( best_result.hit_time_limit )
			{
				if ( attempt.stop_reason.empty() ) attempt.stop_reason = "global_time_limit";
				finalize_attempt_distribution( attempt, grouped_counts );
				attempt.candidate_bank_size_after = static_cast<int>( candidate_bank.size() );
				absorb_attempt_totals( run_log, attempt );
				run_log.attempts.push_back( std::move( attempt ) );
				break;
			}

			const Endpoint fixed_endpoint = best_endpoint;
			if ( attempt.found_trails < forest_options.max_enumerate_solutions && !forest_deadline_expired( forest_options, forest_start ) )
			{
				const double initial_enum_budget = forest_remaining_budget_for_solver( forest_options, forest_start );
				SearchOptions enum_options = make_endpoint_enumeration_options( options, forest_options, fixed_endpoint, best_result.snapshot.objective, initial_enum_budget );
				DifferentialHullReoptimizationSession endpoint_session( enum_options, endpoint_enumeration );
				while ( attempt.found_trails < forest_options.max_enumerate_solutions )
				{
					if ( forest_deadline_expired( forest_options, forest_start ) )
					{
						attempt.hit_time_limit = true;
						attempt.stop_reason = "global_time_limit";
						run_log.hit_global_time_limit = true;
						break;
					}
					const double enum_remaining_hull_time = forest_remaining_budget_for_solver( forest_options, forest_start );
					ScipSolveResult enum_result = endpoint_session.solve_next( enum_remaining_hull_time, true );
					++attempt.solver_calls;
					print_time_limit_incumbent_note( attempt.attempt_id, "endpoint_enumeration", enum_result );
					attempt.solver_status = scip_status_name( enum_result.scip_status );
					attempt.hit_time_limit = attempt.hit_time_limit || enum_result.hit_time_limit;
					attempt.hit_memory_limit = attempt.hit_memory_limit || enum_result.hit_memory_limit;
					attempt.hit_solution_limit = attempt.hit_solution_limit || enum_result.hit_solution_limit;
					if ( enum_result.hit_time_limit ) run_log.hit_global_time_limit = true;
					if ( !enum_result.feasible )
					{
						attempt.stop_reason = forest_stop_reason_from_result( enum_result, "enumeration_unsat" );
						if ( attempt.stop_reason == "enumeration_unsat" ) attempt.completed = true;
						break;
					}
					record_feasible_forest_trail( attempt, run_log, enum_options, enum_result, grouped_counts, endpoint_enumeration );
					if ( attempt.stop_reason == "q1_verification_failed" || enum_result.hit_time_limit ) break;
					endpoint_session.exclude_characteristic( enum_result.no_good );
				}
			}

			if ( attempt.found_trails >= forest_options.max_enumerate_solutions && attempt.stop_reason.empty() )
			{
				attempt.hit_solution_limit = true;
				attempt.stop_reason = "enumeration_limit";
			}
			if ( attempt.stop_reason.empty() ) attempt.stop_reason = attempt.completed ? "enumeration_unsat" : "enumeration_limit";
			if ( attempt.stop_reason == "q1_verification_failed" ) attempt.pruned = true;
			finalize_attempt_distribution( attempt, grouped_counts );
			attempt.candidate_bank_size_after = static_cast<int>( candidate_bank.size() );
			absorb_attempt_totals( run_log, attempt );
			run_log.attempts.push_back( std::move( attempt ) );
			if ( run_log.hit_global_time_limit ) break;
		}

		if ( forest_deadline_expired( forest_options, forest_start ) ) run_log.hit_global_time_limit = true;
		run_log.unique_input_states = static_cast<int>( input_frequency.size() );
		run_log.unique_output_states = static_cast<int>( output_frequency.size() );
		finalize_run_result_type( run_log );
		write_forest_json_file( forest_options.hull_output_json, run_log );
		std::cout << "\n=== FOREST_HULL_SEARCH / SCIP ===\n"
				  << "analysis_algorithm=" << run_log.analysis_algorithm << "\n"
				  << "growth_policy=Q1-valid best output becomes next Forest Layer input\n"
				  << "hull_time_limit_seconds=" << std::setprecision( 12 ) << run_log.forest_options.hull_time_limit_seconds << "\n"
				  << "result_type=" << run_log.result_type << "\n"
				  << "total_attempts=" << run_log.total_attempts << " max_layer=" << run_log.max_layer
				  << " continuations=" << run_log.continuations_enqueued << " cycles=" << run_log.cycles_detected << "\n"
				  << "candidate_bank_peak=" << run_log.candidate_bank_peak << " derived_roots=" << run_log.derived_roots_enqueued << "\n"
				  << "found_trails=" << run_log.found_trails << " q1_failed=" << run_log.q1_failed << "\n";
		if ( run_log.has_global_best )
			std::cout << "global_best_weight=" << std::setprecision( 12 ) << run_log.global_best_weight
					  << " endpoint=ΔA_in=0x" << hex32( run_log.global_best_endpoint.dA_in )
					  << " ΔB_in=0x" << hex32( run_log.global_best_endpoint.dB_in )
					  << " ΔA_out=0x" << hex32( run_log.global_best_endpoint.dA_out )
					  << " ΔB_out=0x" << hex32( run_log.global_best_endpoint.dB_out ) << "\n";
		else
			std::cout << "global_best_weight=none\n";
	}

	// CLI-shaped C++ entry shared by the standalone executable and the
	// multi-process campaign worker. Keeping parsing, defaults, validation,
	// and execution here prevents the runner from drifting from HULL semantics.
	static void run_forest_hull_search_from_argv( int argc, char** argv )
	{
		auto parsed_forest = parse_forest_options_and_strip_argv( argc, argv );
		ForestOptions forest_options = parsed_forest.first;
		SearchOptions options = parse_base_options_from_strings( parsed_forest.second );
		if ( forest_options.hull_mode == HullMode::BEST_TRAIL && !forest_options.hull_mode_explicit )
			forest_options.hull_mode = HullMode::BOUNDED_ENDPOINT;
		validate_forest_configuration( options, forest_options );
		run_forest_hull_search( options, forest_options );
	}
}  // namespace neoalzette_diff_milp

#ifndef NEOALZETTE_HULL_LIBRARY_MODE
int main( int argc, char** argv )
{
	using namespace neoalzette_diff_milp;
	try
	{
		if ( raw_has_arg( argc, argv, "--help" ) )
		{
			print_help( argv[ 0 ] );
			print_forest_help();
			return 0;
		}
		run_forest_hull_search_from_argv( argc, argv );
		return 0;
	}
	catch ( const std::exception& e )
	{
		std::cerr << "Exception: " << e.what() << "\n";
		return 1;
	}
}

#endif  // NEOALZETTE_HULL_LIBRARY_MODE
