// ============================================================================
// NeoAlzette LINEAR Forest Hull Search -- SCIP C API backend
// ============================================================================
//
// This entry point is LINEAR / Walsh-correlation only.
// It keeps the MILP model, Q1 oracles, and build layout untouched, and adds the
// engineering forest search shell locally in this source file.
//
// Audit map:
//   1. forest options, endpoint keys, and run logs;
//   2. Q1/CLAT verification and branch-order scoring helpers;
//   3. endpoint enumeration and signed contribution aggregation;
//   4. JSON artifact writers;
//   5. run_forest_hull_search() driver.
//
// Q2/Q1 literature anchor used by this HULL driver:
//   Zhengbin Liu, Yongqiang Li, Lin Jiao, and Mingsheng Wang,
//   "A New Method for Searching Optimal Differential and Linear Trails in ARX Ciphers",
//   IEEE Transactions on Information Theory 67(2), pp. 1054-1068,
//   2021, DOI 10.1109/TIT.2020.3040543; earlier IACR ePrint 2019/1438.
//   Their ARX method uses carry-bit-dependent local tables and an adapted
//   Matsui/Q2 layer that calls Q1 local transition-weight computations.  This
//   file follows that split for NeoAlzette HULL search: the forest layer derives
//   endpoints, orders and cuts branches, and enumerates no-good-blocked trails,
//   while two-variable modular add/sub weights are scored and verified through
//   local Q1 CLAT/oracle calls.
// ============================================================================

#include "model/neoalzette_scip_search_round_function.hpp"

#include <queue>

namespace neoalzette_linear_milp
{
	enum class HullMode
	{
		BOUNDED_ENDPOINT,
		COMPLETE_ENDPOINT,
		STRONG_HULL
	};

	struct Endpoint
	{
		std::uint32_t input_mask_A_value = 0;
		std::uint32_t input_mask_B_value = 0;
		std::uint32_t output_mask_A_value = 0;
		std::uint32_t output_mask_B_value = 0;
	};

	struct HullWeightDistributionEntry
	{
		std::string weight_key;
		double		 weight = 0.0;
		int			 positive_count = 0;
		int			 negative_count = 0;
		int			 coefficient = 0;
	};

	struct HullCharacteristicRecord
	{
		int						 index = 0;
		Endpoint				 endpoint;
		double					 weight = 0.0;
		int						 sign = 0;
		long double				 signed_contribution = 0.0L;
		long double				 abs_contribution = 0.0L;
		bool					 complete_after_record = false;
		std::vector<WeightTraceEntry> weight_trace;
	};

	// ------------------------------------------------------------------------
	// Audit section 1: forest options, endpoints, and run-log records
	// ------------------------------------------------------------------------
	static const char* hull_mode_name( HullMode mode )
	{
		switch ( mode )
		{
		case HullMode::BOUNDED_ENDPOINT:
			return "bounded-endpoint";
		case HullMode::COMPLETE_ENDPOINT:
			return "complete-endpoint";
		case HullMode::STRONG_HULL:
			return "strong-hull";
		}
		return "unknown";
	}

	static HullMode parse_forest_hull_mode( const std::string& text )
	{
		if ( text == "bounded-endpoint" || text == "best-trail" )
			return HullMode::BOUNDED_ENDPOINT;
		if ( text == "complete-endpoint" )
			return HullMode::COMPLETE_ENDPOINT;
		if ( text == "strong-hull" )
			return HullMode::STRONG_HULL;
		throw std::runtime_error( "unknown --hull-mode: " + text );
	}

	static std::string forest_weight_key( double w )
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision( 8 ) << w;
		return oss.str();
	}

	static long double forest_pow2_neg_ld( long double weight )
	{
		return std::pow( 2.0L, -weight );
	}

	static long double forest_effective_weight_from_abs( long double x )
	{
		return x == 0.0L ? std::numeric_limits<long double>::infinity() : -std::log2( std::fabs( x ) );
	}

	static Endpoint endpoint_from_snapshot( const SolutionSnapshot& snapshot )
	{
		return Endpoint { snapshot.input_mask_A_value, snapshot.input_mask_B_value, snapshot.output_mask_A_value, snapshot.output_mask_B_value };
	}

	static void set_fixed_endpoint( SearchOptions& options, const Endpoint& endpoint )
	{
		options.fix_input_ma = endpoint.input_mask_A_value;
		options.fix_input_mb = endpoint.input_mask_B_value;
		options.fix_output_ma = endpoint.output_mask_A_value;
		options.fix_output_mb = endpoint.output_mask_B_value;
	}

	static void write_forest_weight_distribution_json( std::ostream& output_stream, const std::vector<HullWeightDistributionEntry>& distribution, const std::string& indent )
	{
		output_stream << "[";
		if ( !distribution.empty() )
			output_stream << "\n";
		for ( std::size_t i = 0; i < distribution.size(); ++i )
		{
			const auto& entry = distribution[ i ];
			output_stream << indent << "  {\"weight\": " << json_number( entry.weight )
						  << ", \"positive_count\": " << entry.positive_count
						  << ", \"negative_count\": " << entry.negative_count
						  << ", \"coefficient\": " << entry.coefficient
						  << ", \"term_value\": " << json_number( static_cast<double>( static_cast<long double>( entry.coefficient ) * forest_pow2_neg_ld( entry.weight ) ) )
						  << "}";
			if ( i + 1 != distribution.size() )
				output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static void append_forest_characteristic_jsonl( const std::string& path, const SearchOptions& options, const HullCharacteristicRecord& rec )
	{
		if ( path.empty() )
			return;
		ensure_parent_directory( path );
		std::ofstream out( path, std::ios::app );
		if ( !out )
			throw std::runtime_error( "failed to append linear forest characteristic JSONL: " + path );
		out << "{"
			<< "\"analysis\":\"linear_forest_characteristic\","
			<< "\"cipher\":\"NeoAlzette\","
			<< "\"rounds\":" << options.rounds << ","
			<< "\"index\":" << rec.index << ","
			<< "\"endpoint\":{"
			<< "\"mA_in\":" << json_string( "0x" + hex32( rec.endpoint.input_mask_A_value ) ) << ","
			<< "\"mB_in\":" << json_string( "0x" + hex32( rec.endpoint.input_mask_B_value ) ) << ","
			<< "\"mA_out\":" << json_string( "0x" + hex32( rec.endpoint.output_mask_A_value ) ) << ","
			<< "\"mB_out\":" << json_string( "0x" + hex32( rec.endpoint.output_mask_B_value ) ) << "},"
			<< "\"weight\":" << json_number( rec.weight ) << ","
			<< "\"sign\":" << rec.sign << ","
			<< "\"signed_contribution\":" << json_number( static_cast<double>( rec.signed_contribution ) ) << ","
			<< "\"abs_contribution\":" << json_number( static_cast<double>( rec.abs_contribution ) ) << ","
			<< "\"complete_after_record\":" << ( rec.complete_after_record ? "true" : "false" ) << ","
			<< "\"weight_trace\":";
		std::ostringstream trace_os;
		write_weight_trace_json_array( trace_os, rec.weight_trace, "" );
		std::string trace_json = trace_os.str();
		for ( char& ch : trace_json )
			if ( ch == '\n' || ch == '\r' )
				ch = ' ';
		out << trace_json;
		out << "}\n";
	}

	struct ForestInputMask
	{
		std::uint32_t mA = 0;
		std::uint32_t mB = 0;
	};

	struct HullGrowthPoint
	{
		double window = 0.0;
		int count = 0;
		long double signed_correlation_sum = 0.0L;
		long double abs_correlation_sum = 0.0L;
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


	struct ForestOptions
	{
		int			  attempts = 0;
		std::uint64_t seed = 0;
		HullMode	  hull_mode = HullMode::BOUNDED_ENDPOINT;
		bool		  hull_mode_explicit = false;
		double		  time_limit_seconds = std::numeric_limits<double>::quiet_NaN();
		int			  max_enumerate_solutions = 10000;
		double		  enumerate_from = std::numeric_limits<double>::quiet_NaN();
		double		  enumerate_to = std::numeric_limits<double>::quiet_NaN();
		double		  enumerate_window = 8.0;
		std::string	  hull_output_json = "linear_scip_hull_summary.json";
		std::string	  hull_characteristics_jsonl = "linear_scip_hull_characteristics.jsonl";
	};

	struct ForestQ1Verification
	{
		bool					 valid = true;
		int						 q1_calls = 0;
		int						 q1_failed = 0;
		int						 impossible_transitions = 0;
		int						 zero_correlations = 0;
		int						 weight_mismatches = 0;
		std::int64_t			 clat_lookups = 0;
		std::int64_t			 clat_hits = 0;
		std::int64_t			 clat_misses = 0;
		std::int64_t			 clat_add_steps = 0;
		std::int64_t			 clat_sub_steps = 0;
		std::int64_t			 clat_impossible = 0;
		std::int64_t			 clat_sign_mismatches = 0;
		std::int64_t			 clat_weight_mismatches = 0;
		double					 clat_local_bound_weight_sum = 0.0;
		double					 clat_branch_ordering_score = 0.0;
		std::vector<std::string> errors;
	};

	enum class ForestClatOperation
	{
		ADDITION,
		SUBTRACTION
	};

	struct ClatFreeSlotResult
	{
		bool		  possible = false;
		std::uint32_t value = 0;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
		std::uint64_t best_value_count = 0;
	};

	struct ClatBranchScore
	{
		bool			   possible = false;
		double			   score = std::numeric_limits<double>::infinity();
		ClatFreeSlotResult add_output;
		ClatFreeSlotResult sub_output;
		std::string		   prune_reason;
	};

	struct ForestCandidate
	{
		ForestInputMask input;
		bool has_parent_input = false;
		ForestInputMask parent_input;
		int parent_attempt_id = -1;
		int tree_id = 0;
		int layer = 0;
		bool derived_root = false;
		std::uint64_t derive_seed = 0;
		double cumulative_weight = 0.0;
		double priority_score = std::numeric_limits<double>::infinity();
		std::uint64_t serial = 0;
		ClatBranchScore clat_branch_score;
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


	struct ForestClatCache
	{
		using Key = std::tuple<int, std::uint32_t, std::uint32_t, std::uint32_t, int>;
		std::map<Key, linear_oracle::LinearOracleResult> table;

		linear_oracle::LinearOracleResult lookup(
			ForestClatOperation operation,
			std::uint32_t output_mask,
			std::uint32_t first_input_mask,
			std::uint32_t second_input_mask,
			int bits,
			ForestQ1Verification& verification )
		{
			++verification.clat_lookups;
			const Key key { static_cast<int>( operation ), output_mask, first_input_mask, second_input_mask, bits };
			const auto it = table.find( key );
			if ( it != table.end() )
			{
				++verification.clat_hits;
				return it->second;
			}

			++verification.clat_misses;
			const auto result = operation == ForestClatOperation::ADDITION
				? linear_oracle::oracle_add2( output_mask, first_input_mask, second_input_mask, bits )
				: linear_oracle::oracle_sub2( output_mask, first_input_mask, second_input_mask, bits );
			table.emplace( key, result );
			return result;
		}

		std::size_t size() const
		{
			return table.size();
		}
	};

	struct ForestAttemptLog
	{
		int								 attempt_id = 0;
		int								 parent_attempt_id = -1;
		int								 tree_id = 0;
		int								 layer = 0;
		bool								 derived_root = false;
		std::string						 source_kind = "initial_root";
		int								 input_frequency = 0;
		int								 output_frequency = 0;
		double							 candidate_priority = std::numeric_limits<double>::infinity();
		double							 cumulative_weight_before = 0.0;
		double							 cumulative_weight_after = std::numeric_limits<double>::quiet_NaN();
		double							 average_weight_per_layer = std::numeric_limits<double>::quiet_NaN();
		bool								 continuation_enqueued = false;
		bool								 continuation_dominated = false;
		bool								 continuation_zero_state = false;
		bool								 cycle_detected = false;
		int								 cycle_start_layer = -1;
		int								 cycle_length = 0;
		double							 cycle_weight = std::numeric_limits<double>::quiet_NaN();
		double							 cycle_average_weight = std::numeric_limits<double>::quiet_NaN();
		int								 candidate_bank_size_before = 0;
		int								 candidate_bank_size_after = 0;
		std::uint64_t					 derive_seed = 0;
		bool							 has_old_input = false;
		ForestInputMask					 old_input;
		ForestInputMask					 input;
		std::string						 input_source_change_message;
		std::string						 propagation_trunk_message;
		int								 clat_branch_rank = 0;
		ClatBranchScore					 clat_branch_score;
		bool							 clat_branch_ordered = false;
		bool							 clat_early_pruned = false;
		std::string						 clat_branch_order_message;
		bool							 duplicate_input = false;
		bool							 pruned = false;
		bool							 completed = false;
		bool							 updated_global_best = false;
		bool							 hit_time_limit = false;
		bool							 hit_memory_limit = false;
		bool							 hit_solution_limit = false;
		std::string						 stop_reason;
		std::string						 solver_status = "not_run";
		int								 solver_calls = 0;
		int								 found_trails = 0;
		int								 blocking_cuts = 0;
		int								 q1_calls = 0;
		int								 q1_failed = 0;
		int								 impossible_transitions = 0;
		int								 zero_correlations = 0;
		int								 weight_mismatches = 0;
		std::int64_t					 clat_lookups = 0;
		std::int64_t					 clat_hits = 0;
		std::int64_t					 clat_misses = 0;
		std::int64_t					 clat_add_steps = 0;
		std::int64_t					 clat_sub_steps = 0;
		std::int64_t					 clat_impossible = 0;
		std::int64_t					 clat_sign_mismatches = 0;
		std::int64_t					 clat_weight_mismatches = 0;
		double							 clat_local_bound_weight_sum = 0.0;
		double							 clat_branch_ordering_score = 0.0;
		bool							 bnb_prune_checked = false;
		bool							 bnb_pruned = false;
		bool							 bnb_prune_proven = false;
		double							 bnb_previous_global_best = std::numeric_limits<double>::infinity();
		double							 bnb_candidate_weight = std::numeric_limits<double>::infinity();
		double							 bnb_cutoff_weight = std::numeric_limits<double>::infinity();
		double							 bnb_prune_gap = std::numeric_limits<double>::quiet_NaN();
		std::string						 bnb_prune_rule;
		std::string						 bnb_bound_expression;
		std::string						 bnb_prune_message;
		double							 solver_best_weight = std::numeric_limits<double>::quiet_NaN();
		bool							 solver_best_complete = false;
		bool							 has_endpoint = false;
		Endpoint						 endpoint;
		double							 local_best_weight = std::numeric_limits<double>::quiet_NaN();
		std::vector<HullWeightDistributionEntry> weight_distribution;
		long double						 signed_correlation_sum = 0.0L;
		long double						 abs_correlation_sum = 0.0L;
		long double						 effective_weight_signed_abs = std::numeric_limits<long double>::infinity();
		long double						 effective_weight_abs_sum = std::numeric_limits<long double>::infinity();
		std::vector<HullGrowthPoint>		 hull_growth;
		std::map<std::string, StageWeightAggregate> stage_weight_profile;
		std::vector<std::string>			 q1_errors;
	};

	struct ForestRunLog
	{
		ForestOptions					 forest_options;
		SearchOptions					 search_options;
		std::string						 result_type = "best_found_partial_forest";
		bool							 hit_global_time_limit = false;
		bool							 has_global_best = false;
		double							 global_best_weight = std::numeric_limits<double>::infinity();
		Endpoint						 global_best_endpoint;
		int								 global_best_trail_count = 0;
		int								 total_attempts = 0;
		int								 completed_attempts = 0;
		int								 pruned_branches = 0;
		int								 solver_calls = 0;
		int								 found_trails = 0;
		int								 q1_calls = 0;
		int								 q1_failed = 0;
		int								 impossible_transitions = 0;
		int								 zero_correlations = 0;
		int								 weight_mismatches = 0;
		int								 bnb_pruned_attempts = 0;
		int								 bnb_prune_checks = 0;
		int								 bnb_prune_applied = 0;
		int								 bnb_prune_deferred = 0;
		int								 bnb_objective_cutoff_constraints = 0;
		int								 clat_branch_ordered_attempts = 0;
		int								 clat_early_prune_checks = 0;
		int								 clat_early_pruned_attempts = 0;
		std::int64_t					 clat_lookups = 0;
		std::int64_t					 clat_hits = 0;
		std::int64_t					 clat_misses = 0;
		std::int64_t					 clat_add_steps = 0;
		std::int64_t					 clat_sub_steps = 0;
		std::int64_t					 clat_impossible = 0;
		std::int64_t					 clat_sign_mismatches = 0;
		std::int64_t					 clat_weight_mismatches = 0;
		double							 clat_local_bound_weight_sum = 0.0;
		double							 clat_branch_ordering_score = 0.0;
		std::size_t						 clat_cache_entries = 0;
		bool							 saw_partial_limit = false;
		int								 candidate_bank_peak = 0;
		int								 derived_roots_enqueued = 0;
		int								 continuations_enqueued = 0;
		int								 continuations_dominated = 0;
		int								 cycles_detected = 0;
		int								 max_layer = 0;
		int								 unique_input_states = 0;
		int								 unique_output_states = 0;
		std::map<int, LayerGrowthSummary> layer_growth;
		std::map<std::string, StageWeightAggregate> operator_heatmap;
		std::vector<ForestAttemptLog>	 attempts;
	};

	static std::string get_required_forest_arg( int argc, char** argv, const std::string& name )
	{
		for ( int i = 1; i < argc; ++i )
		{
			if ( argv[ i ] == name )
			{
				if ( i + 1 >= argc )
					throw std::runtime_error( "missing value for option: " + name );
				const std::string value = argv[ i + 1 ];
				if ( value.rfind( "--", 0 ) == 0 )
					throw std::runtime_error( "missing value for option: " + name );
				return value;
			}
		}
		throw std::runtime_error( "missing option: " + name );
	}

	static bool forest_value_option( const std::string& arg )
	{
		return arg == "--forest-attempts" || arg == "--forest-seed" || arg == "--hull-mode" ||
			   arg == "--enumerate-window" || arg == "--enumerate-weight-from" || arg == "--enumerate-weight-to" ||
			   arg == "--max-enumerate-solutions" || arg == "--hull-output-json" || arg == "--hull-characteristics-jsonl";
	}

	static std::pair<ForestOptions, std::vector<std::string>> parse_forest_options_and_strip_argv( int argc, char** argv )
	{
		ForestOptions options;
		std::vector<std::string> stripped_args;
		stripped_args.push_back( argv[ 0 ] );
		for ( int i = 1; i < argc; ++i )
		{
			const std::string arg = argv[ i ];
			if ( arg == "--forest-solve-time-limit" || arg == "--hull-time-limit" )
				throw std::runtime_error( "linear forest uses exactly one total N-round MILP search budget: use --time-limit" );
			if ( forest_value_option( arg ) )
			{
				if ( i + 1 >= argc )
					throw std::runtime_error( "missing value for option: " + arg );
				const std::string value = argv[ ++i ];
				if ( value.rfind( "--", 0 ) == 0 )
					throw std::runtime_error( "missing value for option: " + arg );
				if ( arg == "--forest-attempts" )
					options.attempts = std::stoi( value );
				else if ( arg == "--forest-seed" )
					options.seed = parse_u64_hex_or_dec( value );
				else if ( arg == "--hull-mode" )
				{
					options.hull_mode = parse_forest_hull_mode( value );
					options.hull_mode_explicit = true;
				}
				else if ( arg == "--enumerate-weight-from" )
					options.enumerate_from = std::stod( value );
				else if ( arg == "--enumerate-weight-to" )
					options.enumerate_to = std::stod( value );
				else if ( arg == "--enumerate-window" )
					options.enumerate_window = std::stod( value );
				else if ( arg == "--max-enumerate-solutions" )
					options.max_enumerate_solutions = std::stoi( value );
				else if ( arg == "--hull-output-json" )
					options.hull_output_json = value;
				else if ( arg == "--hull-characteristics-jsonl" )
					options.hull_characteristics_jsonl = value;
				continue;
			}
			stripped_args.push_back( arg );
		}
		return { options, stripped_args };
	}

	static SearchOptions parse_base_options_from_strings( const std::vector<std::string>& args )
	{
		std::vector<char*> argv_copy;
		argv_copy.reserve( args.size() );
		for ( const auto& arg : args )
			argv_copy.push_back( const_cast<char*>( arg.c_str() ) );
		return parse_options( static_cast<int>( argv_copy.size() ), argv_copy.data() );
	}

	static void print_forest_help( const char* argv0 )
	{
		std::cout << "Usage: " << argv0 << " [forest options]\n\n"
				  << "Linear forest hull search:\n"
				  << "  --rounds R                       number of NeoAlzette rounds, default 1\n"
				  << "  --fix-input-ma X                 optional input A linear mask source, default 0x00000001\n"
				  << "  --fix-input-mb X                 optional input B linear mask source, default 0x00000001\n"
				  << "  --time-limit S                   total N-round MILP search wall-clock budget\n"
				  << "  --constant-model fixed-addend-exact-log-weight-milp  strict fixed-addend model\n"
				  << "  --forest-attempts N             optional processed-node cap; 0 means run until the global time limit (default)\n"
				  << "  --forest-seed X                 deterministic SplitMix64 derivation seed, default 0\n"
				  << "  --hull-mode MODE                bounded-endpoint | complete-endpoint\n"
				  << "  --enumerate-window W            default upper window from best weight, default 8\n"
				  << "  --enumerate-weight-from W       explicit lower weight bound\n"
				  << "  --enumerate-weight-to W         explicit upper weight bound\n"
				  << "  --max-enumerate-solutions N     per-input enumeration cap, default 10000\n"
				  << "  --hull-output-json FILE         write forest hull JSON\n"
				  << "  --hull-characteristics-jsonl FILE append every enumerated signed characteristic with full trace\n"
				  << "  --quiet                         reduce SCIP display output\n"
				  << "  --help                          print this help\n"
				  << "\nForest mode requires --time-limit. If --fix-input-ma/--fix-input-mb are omitted, both default to 0x00000001.\n"
				  << "Output masks are derived by the solver and fed back as the next Forest Layer input.\n"
				  << "Each attempt prints an input-source arrow before solving.\n";
	}

	static std::uint64_t splitmix64_next( std::uint64_t& state )
	{
		std::uint64_t z = ( state += 0x9E3779B97F4A7C15ull );
		z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ull;
		z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBull;
		return z ^ ( z >> 31 );
	}

	static std::uint64_t pack_input_mask( const ForestInputMask& input )
	{
		return ( std::uint64_t( input.mA ) << 32 ) | std::uint64_t( input.mB );
	}

	static ForestInputMask derive_next_input_mask( const ForestInputMask& previous_input, std::uint64_t forest_seed, int attempt_id, std::uint64_t& derive_seed )
	{
		derive_seed = forest_seed ^ pack_input_mask( previous_input ) ^ ( 0xD1B54A32D192ED03ull * static_cast<std::uint64_t>( attempt_id + 1 ) );
		std::uint64_t state = derive_seed;
		ForestInputMask next;
		next.mA = static_cast<std::uint32_t>( splitmix64_next( state ) >> 32 );
		next.mB = static_cast<std::uint32_t>( splitmix64_next( state ) >> 32 );
		if ( ( next.mA | next.mB ) == 0u )
			next.mA = 1u;
		return next;
	}

	static std::string forest_input_string( const ForestInputMask& input )
	{
		return "ΛA_in=0x" + hex32( input.mA ) + " ΛB_in=0x" + hex32( input.mB );
	}

	static std::string forest_arrow_message( int attempt_id, const ForestAttemptLog& attempt )
	{
		if ( attempt.has_old_input )
			return "ATTEMPT " + std::to_string( attempt_id ) + " INPUT_SOURCE_CHANGED: " + forest_input_string( attempt.old_input ) + " -> " + forest_input_string( attempt.input );
		return "ATTEMPT " + std::to_string( attempt_id ) + " INPUT_SOURCE_SELECTED: external_seed -> " + forest_input_string( attempt.input );
	}

	static std::string forest_trunk_change_message( const ForestAttemptLog& attempt )
	{
		if ( attempt.has_old_input )
			return "PROPAGATION_TRUNK_CHANGED: " + forest_input_string( attempt.old_input ) + " -> " + forest_input_string( attempt.input );
		return "PROPAGATION_TRUNK_CHANGED: external_seed -> " + forest_input_string( attempt.input );
	}

	static std::uint64_t saturating_add_u64( std::uint64_t left, std::uint64_t right )
	{
		if ( std::numeric_limits<std::uint64_t>::max() - left < right )
			return std::numeric_limits<std::uint64_t>::max();
		return left + right;
	}

	struct ClatDpState
	{
		bool		  reachable = false;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
		std::uint32_t value = 0;
		std::uint64_t count = 0;
	};

	static void update_clat_dp_state( ClatDpState& state, std::uint32_t weight, std::uint32_t value, std::uint64_t count )
	{
		if ( !state.reachable || weight < state.weight || ( weight == state.weight && value < state.value ) )
		{
			state.reachable = true;
			state.weight = weight;
			state.value = value;
			state.count = count;
			return;
		}
		if ( weight == state.weight )
			state.count = saturating_add_u64( state.count, count );
	}

	static bool clat_box_transition_allowed( int next_state, int output_bit, int first_input_bit, int second_input_bit, int current_state )
	{
		return next_state - output_bit - first_input_bit + second_input_bit + current_state >= 0 &&
			   next_state + output_bit - first_input_bit - second_input_bit + current_state >= 0 &&
			   next_state + output_bit - first_input_bit + second_input_bit - current_state >= 0 &&
			   -next_state + output_bit + first_input_bit + second_input_bit + current_state >= 0 &&
			   next_state + output_bit + first_input_bit - second_input_bit - current_state >= 0 &&
			   next_state - output_bit + first_input_bit - second_input_bit + current_state >= 0 &&
			   next_state - output_bit + first_input_bit + second_input_bit - current_state >= 0 &&
			   next_state + output_bit + first_input_bit + second_input_bit + current_state <= 4;
	}

	static ClatFreeSlotResult best_clat_free_slot( int free_slot, std::uint32_t slot0, std::uint32_t slot1, std::uint32_t slot2, int bits )
	{
		if ( free_slot < 0 || free_slot > 2 || bits <= 0 || bits > 32 )
			return {};

		const std::uint32_t		 mask = linear_oracle::mask_for_bits( bits );
		std::array<std::uint32_t, 3> fixed { slot0 & mask, slot1 & mask, slot2 & mask };
		std::array<ClatDpState, 2> states;
		states[ 0 ] = ClatDpState { true, 0, 0, 1 };
		states[ 1 ] = ClatDpState { true, 0, 0, 1 };

		for ( int bit = 0; bit < bits; ++bit )
		{
			std::array<ClatDpState, 2> next_states;
			for ( int current_state = 0; current_state <= 1; ++current_state )
			{
				const ClatDpState& state = states[ current_state ];
				if ( !state.reachable )
					continue;
				for ( int free_bit = 0; free_bit <= 1; ++free_bit )
				{
					std::array<int, 3> slot_bits {};
					for ( int slot = 0; slot < 3; ++slot )
						slot_bits[ slot ] = ( slot == free_slot ) ? free_bit : linear_oracle::word_bit( fixed[ slot ], bit );
					for ( int next_state = 0; next_state <= 1; ++next_state )
					{
						if ( bit == bits - 1 && next_state != 0 )
							continue;
						if ( !clat_box_transition_allowed( next_state, slot_bits[ 0 ], slot_bits[ 1 ], slot_bits[ 2 ], current_state ) )
							continue;
						const std::uint32_t next_weight = state.weight + static_cast<std::uint32_t>( next_state );
						const std::uint32_t next_value = state.value | ( static_cast<std::uint32_t>( free_bit ) << bit );
						update_clat_dp_state( next_states[ next_state ], next_weight, next_value, state.count );
					}
				}
			}
			states = next_states;
		}

		const ClatDpState& best = states[ 0 ];
		if ( !best.reachable )
			return {};
		return ClatFreeSlotResult { true, best.value & mask, best.weight, best.count };
	}

	// ------------------------------------------------------------------------
	// Audit section 2: Q1/CLAT verification and branch-order helpers
	// ------------------------------------------------------------------------
	// Liu-Li-Jiao-Wang Q2 hook for the linear side: CLAT provides the local Q1
	// weight/order signal for two-variable modular add/sub branches, and the
	// surrounding forest driver acts as the Q2 layer. CLAT is used only as a
	// local hint for ordering and early pruning; every accepted characteristic is
	// still checked by exact Q1 trace verification before aggregation.
	static ClatBranchScore score_input_source_by_clat( const ForestInputMask& input )
	{
		ClatBranchScore score;
		score.add_output = best_clat_free_slot( 0, 0, input.mA, input.mB, WORD_SIZE );
		score.sub_output = best_clat_free_slot( 1, input.mA, 0, input.mB, WORD_SIZE );
		score.possible = score.add_output.possible || score.sub_output.possible;
		if ( !score.possible )
		{
			score.prune_reason = "clat_no_direct_input_source_addsub_output_hint";
			return score;
		}
		const double add_weight = score.add_output.possible ? static_cast<double>( score.add_output.weight ) : std::numeric_limits<double>::infinity();
		const double sub_weight = score.sub_output.possible ? static_cast<double>( score.sub_output.weight ) : std::numeric_limits<double>::infinity();
		score.score = add_weight + sub_weight;
		return score;
	}

	static std::string clat_free_slot_string( const char* label, const ClatFreeSlotResult& result )
	{
		std::ostringstream oss;
		if ( result.possible )
			oss << " " << label << "_best_weight=" << result.weight
				<< " " << label << "_output_hint=0x" << hex32( result.value )
				<< " " << label << "_min_outputs=" << result.best_value_count;
		else
			oss << " " << label << "_best_weight=impossible";
		return oss.str();
	}

	static std::string clat_branch_score_string( const ClatBranchScore& score )
	{
		std::ostringstream oss;
		oss << "score=";
		if ( std::isfinite( score.score ) )
			oss << std::fixed << std::setprecision( 6 ) << score.score;
		else
			oss << "inf";
		oss << clat_free_slot_string( "add", score.add_output );
		oss << clat_free_slot_string( "sub", score.sub_output );
		return oss.str();
	}

	static std::string clat_branch_order_message( const ForestAttemptLog& attempt )
	{
		return "ATTEMPT " + std::to_string( attempt.attempt_id ) + " CLAT_BRANCH_ORDER: rank=" + std::to_string( attempt.clat_branch_rank ) + " " + clat_branch_score_string( attempt.clat_branch_score );
	}

	static std::vector<HullGrowthPoint> make_hull_growth_points()
	{
		return { { 0.0, 0, 0.0L, 0.0L }, { 2.0, 0, 0.0L, 0.0L }, { 4.0, 0, 0.0L, 0.0L }, { 6.0, 0, 0.0L, 0.0L }, { 8.0, 0, 0.0L, 0.0L } };
	}

	static double forest_candidate_priority( int layer, double cumulative_weight, const ClatBranchScore& score, const ForestInputMask& input )
	{
		const double hint = score.possible && std::isfinite( score.score )
			? score.score
			: static_cast<double>( std::popcount( input.mA ) + std::popcount( input.mB ) );
		return ( cumulative_weight + hint ) / static_cast<double>( std::max( 1, layer + 1 ) );
	}

	static ForestCandidate make_forest_candidate(
		const ForestInputMask& input,
		int parent_attempt_id,
		int tree_id,
		int layer,
		bool derived_root,
		std::uint64_t derive_seed,
		double cumulative_weight,
		std::uint64_t serial,
		bool has_parent_input,
		const ForestInputMask& parent_input,
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
		candidate.clat_branch_score = score_input_source_by_clat( input );
		candidate.priority_score = forest_candidate_priority( layer, cumulative_weight, candidate.clat_branch_score, input );
		return candidate;
	}

	static bool register_forest_frontier(
		std::map<std::uint64_t, std::vector<ForestStateFrontierEntry>>& frontier,
		const ForestCandidate& candidate )
	{
		auto& states = frontier[ pack_input_mask( candidate.input ) ];
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
		attempt.input_source_change_message = candidate.parent_attempt_id >= 0
			? "ATTEMPT " + std::to_string( attempt_id ) + " FOREST_LAYER_CONTINUATION: output_of_attempt=" + std::to_string( candidate.parent_attempt_id ) + " -> " + forest_input_string( candidate.input )
			: forest_arrow_message( attempt_id, attempt );
		attempt.propagation_trunk_message = "ATTEMPT " + std::to_string( attempt_id ) + " FOREST_LAYER: tree=" + std::to_string( candidate.tree_id ) + " layer=" + std::to_string( candidate.layer ) + " input=" + forest_input_string( candidate.input );
		attempt.clat_branch_score = candidate.clat_branch_score;
		attempt.clat_branch_ordered = true;
		attempt.clat_branch_rank = attempt_id;
		attempt.clat_branch_order_message = clat_branch_order_message( attempt );
		if ( attempt.clat_branch_score.possible )
		{
			if ( attempt.clat_branch_score.add_output.possible )
				attempt.clat_branch_ordering_score += static_cast<double>( attempt.clat_branch_score.add_output.weight );
			if ( attempt.clat_branch_score.sub_output.possible )
				attempt.clat_branch_ordering_score += static_cast<double>( attempt.clat_branch_score.sub_output.weight );
		}
		attempt.hull_growth = make_hull_growth_points();
		return attempt;
	}

	static void update_hull_growth( ForestAttemptLog& attempt, double weight, long double signed_contribution, long double abs_contribution )
	{
		if ( !std::isfinite( attempt.local_best_weight ) )
			return;
		for ( auto& point : attempt.hull_growth )
		{
			if ( weight <= attempt.local_best_weight + point.window + 1e-8 )
			{
				++point.count;
				point.signed_correlation_sum += signed_contribution;
				point.abs_correlation_sum += abs_contribution;
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

	static void print_clat_branch_order( const ForestAttemptLog& attempt )
	{
		std::cout << attempt.clat_branch_order_message << "\n";
		if ( attempt.clat_early_pruned )
			std::cout << "ATTEMPT " << attempt.attempt_id << " CLAT_EARLY_PRUNE: reason=" << attempt.clat_branch_score.prune_reason << "\n";
	}

	static void print_bnb_prune_audit( const ForestAttemptLog& attempt )
	{
		if ( !attempt.bnb_prune_checked )
			return;
		std::cout << "BNB_PRUNE_AUDIT: solver_complete=" << ( attempt.solver_best_complete ? "yes" : "no" )
				  << " proven_by_solver=" << ( attempt.bnb_prune_proven ? "yes" : "no" )
				  << " previous_global_best=" << ( std::isfinite( attempt.bnb_previous_global_best ) ? std::to_string( attempt.bnb_previous_global_best ) : "none" )
				  << " candidate_best=" << ( std::isfinite( attempt.bnb_candidate_weight ) ? std::to_string( attempt.bnb_candidate_weight ) : "none" )
				  << " cutoff=" << ( std::isfinite( attempt.bnb_cutoff_weight ) ? std::to_string( attempt.bnb_cutoff_weight ) : "none" )
				  << " gap=" << ( std::isfinite( attempt.bnb_candidate_weight ) && std::isfinite( attempt.bnb_previous_global_best ) ? std::to_string( attempt.bnb_candidate_weight - attempt.bnb_previous_global_best ) : "none" )
				  << " rule=" << ( attempt.bnb_bound_expression.empty() ? attempt.bnb_prune_rule : attempt.bnb_bound_expression )
				  << " -> prune=" << ( attempt.bnb_pruned ? "yes" : "no" );
		if ( !attempt.bnb_prune_message.empty() )
			std::cout << " (" << attempt.bnb_prune_message << ")";
		std::cout << "\n";
	}

	static void print_hull_time_limit_incumbent( const ScipSolveResult& result, const std::string& context )
	{
		std::cout << "\n=== LINEAR_FOREST_TIME_LIMIT_INCUMBENT / SCIP ===\n"
				  << "context=" << context << "\n";
		if ( !result.feasible )
		{
			std::cout << "current_incumbent=none\n";
			return;
		}
		print_solution_summary( result, "SCIP CURRENT LINEAR FOREST INCUMBENT / WALSH MODEL" );
		print_checkpoint_trace( result );
		print_weight_trace( result );
	}

	static void print_hull_current_time_limit_incumbent( const std::optional<ScipSolveResult>& current_best_result, const std::string& context )
	{
		if ( current_best_result )
		{
			print_hull_time_limit_incumbent( *current_best_result, context );
			return;
		}
		ScipSolveResult empty_result;
		print_hull_time_limit_incumbent( empty_result, context );
	}

	static void print_hull_time_limit_incumbent_or_current( const ScipSolveResult& limit_result, const std::optional<ScipSolveResult>& current_best_result, const std::string& context )
	{
		const ScipSolveResult* selected = limit_result.feasible ? &limit_result : nullptr;
		if ( current_best_result && ( selected == nullptr || current_best_result->snapshot.objective <= selected->snapshot.objective + 1e-8 ) )
			selected = &*current_best_result;
		if ( selected != nullptr )
		{
			print_hull_time_limit_incumbent( *selected, context );
			return;
		}
		ScipSolveResult empty_result;
		print_hull_time_limit_incumbent( empty_result, context );
	}

	static double forest_elapsed_seconds( std::chrono::steady_clock::time_point start )
	{
		return std::chrono::duration<double>( std::chrono::steady_clock::now() - start ).count();
	}

	static bool forest_deadline_expired( const ForestOptions& options, std::chrono::steady_clock::time_point start )
	{
		return std::isfinite( options.time_limit_seconds ) && options.time_limit_seconds > 0.0 && forest_elapsed_seconds( start ) >= options.time_limit_seconds;
	}

	static double forest_remaining_seconds( const ForestOptions& options, std::chrono::steady_clock::time_point start )
	{
		if ( !std::isfinite( options.time_limit_seconds ) || options.time_limit_seconds <= 0.0 )
			return std::numeric_limits<double>::quiet_NaN();
		return options.time_limit_seconds - forest_elapsed_seconds( start );
	}

	static double forest_remaining_total_budget_for_next_milp( const ForestOptions& forest_options, std::chrono::steady_clock::time_point start )
	{
		const double remaining = forest_remaining_seconds( forest_options, start );
		return std::max( 0.001, std::isfinite( remaining ) ? remaining : 0.001 );
	}

	static void add_q1_result_to_attempt( ForestAttemptLog& attempt, const ForestQ1Verification& verification )
	{
		attempt.q1_calls += verification.q1_calls;
		attempt.q1_failed += verification.q1_failed;
		attempt.impossible_transitions += verification.impossible_transitions;
		attempt.zero_correlations += verification.zero_correlations;
		attempt.weight_mismatches += verification.weight_mismatches;
		attempt.clat_lookups += verification.clat_lookups;
		attempt.clat_hits += verification.clat_hits;
		attempt.clat_misses += verification.clat_misses;
		attempt.clat_add_steps += verification.clat_add_steps;
		attempt.clat_sub_steps += verification.clat_sub_steps;
		attempt.clat_impossible += verification.clat_impossible;
		attempt.clat_sign_mismatches += verification.clat_sign_mismatches;
		attempt.clat_weight_mismatches += verification.clat_weight_mismatches;
		attempt.clat_local_bound_weight_sum += verification.clat_local_bound_weight_sum;
		attempt.clat_branch_ordering_score += verification.clat_branch_ordering_score;
		attempt.q1_errors.insert( attempt.q1_errors.end(), verification.errors.begin(), verification.errors.end() );
	}

	static void append_q1_failure( ForestQ1Verification& verification, const std::string& message )
	{
		verification.valid = false;
		++verification.q1_failed;
		verification.errors.push_back( message );
	}

	static std::string forest_clat_step_label( const WeightTraceEntry& entry )
	{
		return "round=" + std::to_string( entry.round ) + " step=" + std::to_string( entry.step ) + " stage=" + entry.stage + " prefix=" + entry.prefix;
	}

	static bool forest_clat_operation_from_trace( const WeightTraceEntry& entry, ForestClatOperation& operation )
	{
		if ( entry.operation == "two_variable_modular_addition_linear" )
		{
			operation = ForestClatOperation::ADDITION;
			return true;
		}
		if ( entry.operation == "two_variable_modular_subtraction_linear" )
		{
			operation = ForestClatOperation::SUBTRACTION;
			return true;
		}
		return false;
	}

	static void verify_two_variable_clat_step( ForestQ1Verification& verification, ForestClatCache& clat_cache, const WeightTraceEntry& entry )
	{
		ForestClatOperation operation = ForestClatOperation::ADDITION;
		if ( !forest_clat_operation_from_trace( entry, operation ) )
			return;

		if ( operation == ForestClatOperation::ADDITION )
			++verification.clat_add_steps;
		else
			++verification.clat_sub_steps;

		if ( !entry.has_local_input0 || !entry.has_local_input1 || !entry.has_local_output )
		{
			++verification.clat_impossible;
			++verification.impossible_transitions;
			append_q1_failure( verification, "linear CLAT step lacks local masks: " + forest_clat_step_label( entry ) );
			return;
		}

		const auto clat_result = clat_cache.lookup( operation, entry.local_output, entry.local_input0, entry.local_input1, WORD_SIZE, verification );
		if ( !clat_result.possible )
		{
			++verification.clat_impossible;
			++verification.impossible_transitions;
			++verification.zero_correlations;
			append_q1_failure( verification, "linear CLAT rejects two-variable add/sub transition: " + forest_clat_step_label( entry ) );
			return;
		}

		int clat_sign = clat_result.sign;
		if ( entry.has_public_constant )
			clat_sign *= parity32( entry.local_input1 & entry.public_constant ) ? -1 : 1;
		if ( clat_sign == 0 || clat_sign != entry.local_sign )
		{
			++verification.clat_sign_mismatches;
			append_q1_failure( verification, "linear CLAT sign mismatch at " + forest_clat_step_label( entry ) );
		}

		const double clat_weight = static_cast<double>( clat_result.weight );
		verification.clat_local_bound_weight_sum += clat_weight;
		verification.clat_branch_ordering_score += clat_weight;
		if ( std::fabs( entry.local_weight - clat_weight ) > 1e-6 )
		{
			++verification.clat_weight_mismatches;
			++verification.weight_mismatches;
			append_q1_failure( verification, "linear CLAT weight mismatch at " + forest_clat_step_label( entry ) );
		}
	}

	static ForestQ1Verification verify_trail_by_q1_trace( const ScipSolveResult& result, ForestClatCache& clat_cache )
	{
		ForestQ1Verification verification;
		verification.q1_calls = static_cast<int>( result.weight_trace.size() );

		if ( !result.weight_trace_available )
			append_q1_failure( verification, "linear Q1 verification requires a weight trace" );
		if ( !result.weight_trace_matches_objective )
		{
			++verification.weight_mismatches;
			append_q1_failure( verification, "linear Q1 trace total does not match SCIP objective" );
		}
		if ( !result.weight_trace_oracles_valid )
			append_q1_failure( verification, "linear Q1 oracle metadata marks at least one local transition invalid" );
		if ( !result.weight_trace_oracle_weights_consistent )
		{
			++verification.weight_mismatches;
			append_q1_failure( verification, "linear Q1 exact oracle weights differ from the MILP objective" );
		}
		if ( result.characteristic_sign == 0 )
		{
			++verification.zero_correlations;
			append_q1_failure( verification, "linear Q1 exact sign is zero" );
		}

		for ( const auto& entry : result.weight_trace )
		{
			verify_two_variable_clat_step( verification, clat_cache, entry );
			if ( entry.has_injection_oracle && !entry.injection_valid )
				++verification.impossible_transitions;
			if ( has_fixed_addend_exact_lap_metadata( entry ) && !entry.fixed_const_exact_possible )
				++verification.impossible_transitions;
			if ( entry.local_sign == 0 )
				++verification.zero_correlations;
			if ( has_fixed_addend_exact_lap_metadata( entry ) && entry.fixed_const_exact_possible && entry.fixed_const_exact_correlation == 0.0L )
				++verification.zero_correlations;
		}
		return verification;
	}

	static SearchOptions make_best_search_options_for_input( SearchOptions base_options, const ForestInputMask& input, double remaining_total_budget_seconds )
	{
		base_options.fix_input_ma = input.mA;
		base_options.fix_input_mb = input.mB;
		base_options.fix_output_ma.reset();
		base_options.fix_output_mb.reset();
		base_options.time_limit_seconds = remaining_total_budget_seconds;
		base_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
		base_options.objective_window_to = std::numeric_limits<double>::quiet_NaN();
		base_options.output_result_json.clear();
		base_options.output_weight_trace_json.clear();
		base_options.output_round_table_json.clear();
		return base_options;
	}

	static SearchOptions make_endpoint_enumeration_options( SearchOptions base_options, const Endpoint& endpoint, double local_best_weight, double remaining_total_budget_seconds, const ForestOptions& forest_options )
	{
		set_fixed_endpoint( base_options, endpoint );
		base_options.time_limit_seconds = remaining_total_budget_seconds;
		base_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
		base_options.objective_window_to = std::numeric_limits<double>::quiet_NaN();
		base_options.output_result_json.clear();
		base_options.output_weight_trace_json.clear();
		base_options.output_round_table_json.clear();
		if ( forest_options.hull_mode == HullMode::COMPLETE_ENDPOINT )
		{
			base_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
			base_options.objective_window_to = std::numeric_limits<double>::quiet_NaN();
		}
		else
		{
			const double lower = std::isfinite( forest_options.enumerate_from ) ? forest_options.enumerate_from : local_best_weight - 1e-8;
			const double upper = std::isfinite( forest_options.enumerate_to ) ? forest_options.enumerate_to : local_best_weight + forest_options.enumerate_window;
			base_options.objective_window_from = lower;
			base_options.objective_window_to = upper;
		}
		return base_options;
	}

	static std::string forest_stop_reason_from_result( const ScipSolveResult& result, const std::string& infeasible_reason )
	{
		if ( result.hit_time_limit || result.scip_status == SCIP_STATUS_TIMELIMIT )
			return "solver_timeout_no_incumbent";
		if ( result.hit_memory_limit || result.scip_status == SCIP_STATUS_MEMLIMIT )
			return "solver_memory_limit";
		if ( result.scip_status == SCIP_STATUS_INFEASIBLE )
			return infeasible_reason;
		if ( !result.complete )
			return "solver_incomplete_no_incumbent";
		return infeasible_reason;
	}

	static void finalize_attempt_distribution( ForestAttemptLog& attempt, const std::map<std::string, HullWeightDistributionEntry>& grouped_counts )
	{
		attempt.weight_distribution.clear();
		for ( const auto& [ _, entry ] : grouped_counts )
			attempt.weight_distribution.push_back( entry );
		attempt.effective_weight_signed_abs = forest_effective_weight_from_abs( attempt.signed_correlation_sum );
		attempt.effective_weight_abs_sum = forest_effective_weight_from_abs( attempt.abs_correlation_sum );
	}

	// ------------------------------------------------------------------------
	// Audit section 3: endpoint enumeration aggregation
	// ------------------------------------------------------------------------
	// Each accepted characteristic is verified by local Q1 oracles, then added to
	// signed and absolute sums. The no-good cut records the semantic bit pattern
	// so the next solve enumerates a distinct characteristic.
	static void record_feasible_forest_trail(
		ForestAttemptLog& attempt,
		ForestRunLog& run_log,
		const SearchOptions& record_options,
		const std::string& characteristic_jsonl_path,
		const ScipSolveResult& result,
		std::map<std::string, HullWeightDistributionEntry>& grouped_counts,
		NoGoodStore& enumeration,
		ForestClatCache& clat_cache )
	{
		const ForestQ1Verification verification = verify_trail_by_q1_trace( result, clat_cache );
		add_q1_result_to_attempt( attempt, verification );
		if ( !verification.valid )
		{
			attempt.pruned = true;
			attempt.stop_reason = "q1_verification_failed";
			return;
		}

		const Endpoint endpoint = endpoint_from_snapshot( result.snapshot );
		attempt.endpoint = endpoint;
		attempt.has_endpoint = true;
		attempt.local_best_weight = std::isnan( attempt.local_best_weight ) ? result.snapshot.objective : std::min( attempt.local_best_weight, result.snapshot.objective );
		++attempt.found_trails;
		++run_log.found_trails;

		const std::string key = forest_weight_key( result.snapshot.objective );
		auto& entry = grouped_counts[ key ];
		entry.weight_key = key;
		entry.weight = result.snapshot.objective;
		if ( result.characteristic_sign > 0 ) ++entry.positive_count;
		else ++entry.negative_count;
		entry.coefficient = entry.positive_count - entry.negative_count;
		attempt.signed_correlation_sum += result.signed_correlation_contribution;
		attempt.abs_correlation_sum += result.abs_correlation_contribution;
		update_hull_growth( attempt, result.snapshot.objective, result.signed_correlation_contribution, result.abs_correlation_contribution );
		update_stage_weight_profile( attempt.stage_weight_profile, result.weight_trace );

		if ( !run_log.has_global_best || result.snapshot.objective < run_log.global_best_weight - 1e-8 )
		{
			run_log.has_global_best = true;
			run_log.global_best_weight = result.snapshot.objective;
			run_log.global_best_endpoint = endpoint;
			run_log.global_best_trail_count = 1;
			attempt.updated_global_best = true;
		}
		else if ( std::fabs( result.snapshot.objective - run_log.global_best_weight ) <= 1e-8 )
		{
			++run_log.global_best_trail_count;
		}

		enumeration.no_good_cuts.push_back( result.no_good );
		++attempt.blocking_cuts;

		HullCharacteristicRecord record;
		record.index = run_log.found_trails;
		record.endpoint = endpoint;
		record.weight = result.snapshot.objective;
		record.sign = result.characteristic_sign;
		record.signed_contribution = result.signed_correlation_contribution;
		record.abs_contribution = result.abs_correlation_contribution;
		record.complete_after_record = false;
		record.weight_trace = result.weight_trace;
		append_forest_characteristic_jsonl( characteristic_jsonl_path, record_options, record );
	}

	static void absorb_attempt_totals( ForestRunLog& run_log, const ForestAttemptLog& attempt )
	{
		run_log.solver_calls += attempt.solver_calls;
		run_log.q1_calls += attempt.q1_calls;
		run_log.q1_failed += attempt.q1_failed;
		run_log.impossible_transitions += attempt.impossible_transitions;
		run_log.zero_correlations += attempt.zero_correlations;
		run_log.weight_mismatches += attempt.weight_mismatches;
		run_log.clat_lookups += attempt.clat_lookups;
		run_log.clat_hits += attempt.clat_hits;
		run_log.clat_misses += attempt.clat_misses;
		run_log.clat_add_steps += attempt.clat_add_steps;
		run_log.clat_sub_steps += attempt.clat_sub_steps;
		run_log.clat_impossible += attempt.clat_impossible;
		run_log.clat_sign_mismatches += attempt.clat_sign_mismatches;
		run_log.clat_weight_mismatches += attempt.clat_weight_mismatches;
		run_log.clat_local_bound_weight_sum += attempt.clat_local_bound_weight_sum;
		run_log.clat_branch_ordering_score += attempt.clat_branch_ordering_score;
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
		if ( attempt.bnb_prune_checked )
			++run_log.bnb_objective_cutoff_constraints;
		if ( attempt.completed )
			++run_log.completed_attempts;
		if ( attempt.pruned )
			++run_log.pruned_branches;
		if ( attempt.clat_branch_ordered )
			++run_log.clat_branch_ordered_attempts;
		if ( attempt.clat_branch_score.possible || !attempt.clat_branch_score.prune_reason.empty() )
			++run_log.clat_early_prune_checks;
		if ( attempt.clat_early_pruned )
			++run_log.clat_early_pruned_attempts;
		if ( attempt.hit_time_limit || attempt.hit_memory_limit || attempt.hit_solution_limit )
			run_log.saw_partial_limit = true;
		print_bnb_prune_audit( attempt );
	}

	static void write_forest_input_json( std::ostream& output_stream, const ForestInputMask& input, const std::string& indent )
	{
		output_stream << indent << "{"
					  << "\"mA_in\": " << json_string( "0x" + hex32( input.mA ) )
					  << ", \"mB_in\": " << json_string( "0x" + hex32( input.mB ) )
					  << "}";
	}

	static void write_forest_endpoint_json( std::ostream& output_stream, const Endpoint& endpoint, const std::string& indent )
	{
		output_stream << indent << "{\n"
					  << indent << "  \"mA_in\": " << json_string( "0x" + hex32( endpoint.input_mask_A_value ) ) << ",\n"
					  << indent << "  \"mB_in\": " << json_string( "0x" + hex32( endpoint.input_mask_B_value ) ) << ",\n"
					  << indent << "  \"mA_out\": " << json_string( "0x" + hex32( endpoint.output_mask_A_value ) ) << ",\n"
					  << indent << "  \"mB_out\": " << json_string( "0x" + hex32( endpoint.output_mask_B_value ) ) << "\n"
					  << indent << "}";
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

	static void write_clat_free_slot_json( std::ostream& output_stream, const ClatFreeSlotResult& result, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"possible\": " << ( result.possible ? "true" : "false" ) << ",\n"
					  << indent << "  \"best_value\": " << ( result.possible ? json_string( "0x" + hex32( result.value ) ) : "null" ) << ",\n"
					  << indent << "  \"best_weight\": " << ( result.possible ? std::to_string( result.weight ) : "null" ) << ",\n"
					  << indent << "  \"best_value_count\": " << result.best_value_count << "\n"
					  << indent << "}";
	}

	static void write_clat_branch_score_json( std::ostream& output_stream, const ForestAttemptLog& attempt, const std::string& indent )
	{
		output_stream << "{\n"
					  << indent << "  \"ordered\": " << ( attempt.clat_branch_ordered ? "true" : "false" ) << ",\n"
					  << indent << "  \"rank\": " << attempt.clat_branch_rank << ",\n"
					  << indent << "  \"message\": " << json_string( attempt.clat_branch_order_message ) << ",\n"
					  << indent << "  \"possible\": " << ( attempt.clat_branch_score.possible ? "true" : "false" ) << ",\n"
					  << indent << "  \"score\": " << json_number( attempt.clat_branch_score.score ) << ",\n"
					  << indent << "  \"early_pruned\": " << ( attempt.clat_early_pruned ? "true" : "false" ) << ",\n"
					  << indent << "  \"prune_reason\": " << json_string( attempt.clat_branch_score.prune_reason ) << ",\n"
					  << indent << "  \"add_output_hint\": ";
		write_clat_free_slot_json( output_stream, attempt.clat_branch_score.add_output, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"sub_output_hint\": ";
		write_clat_free_slot_json( output_stream, attempt.clat_branch_score.sub_output, indent + "  " );
		output_stream << "\n" << indent << "}";
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


	static void write_hull_growth_json( std::ostream& output_stream, const std::vector<HullGrowthPoint>& growth, const std::string& indent )
	{
		output_stream << "[";
		if ( !growth.empty() ) output_stream << "\n";
		for ( std::size_t index = 0; index < growth.size(); ++index )
		{
			const auto& point = growth[ index ];
			output_stream << indent << "  {\"window\": " << json_number( point.window )
						  << ", \"count\": " << point.count
						  << ", \"signed_correlation_sum\": " << json_number( static_cast<double>( point.signed_correlation_sum ) )
						  << ", \"abs_correlation_sum\": " << json_number( static_cast<double>( point.abs_correlation_sum ) )
						  << ", \"effective_weight_signed_abs\": " << json_number( static_cast<double>( forest_effective_weight_from_abs( point.signed_correlation_sum ) ) )
						  << ", \"effective_weight_abs_sum\": " << json_number( static_cast<double>( forest_effective_weight_from_abs( point.abs_correlation_sum ) ) ) << "}";
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
					  << indent << "  \"derive_seed\": " << json_string( "0x" + hex64( attempt.derive_seed ) ) << ",\n"
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
					  << indent << "  \"clat_branch_order\": ";
		write_clat_branch_score_json( output_stream, attempt, indent + "  " );
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
					  << indent << "  \"zero_correlations\": " << attempt.zero_correlations << ",\n"
					  << indent << "  \"weight_mismatches\": " << attempt.weight_mismatches << ",\n"
					  << indent << "  \"clat_scope\": \"two_variable_modular_addition_subtraction_linear\",\n"
					  << indent << "  \"clat_lookups\": " << attempt.clat_lookups << ",\n"
					  << indent << "  \"clat_hits\": " << attempt.clat_hits << ",\n"
					  << indent << "  \"clat_misses\": " << attempt.clat_misses << ",\n"
					  << indent << "  \"clat_add_steps\": " << attempt.clat_add_steps << ",\n"
					  << indent << "  \"clat_sub_steps\": " << attempt.clat_sub_steps << ",\n"
					  << indent << "  \"clat_impossible\": " << attempt.clat_impossible << ",\n"
					  << indent << "  \"clat_sign_mismatches\": " << attempt.clat_sign_mismatches << ",\n"
					  << indent << "  \"clat_weight_mismatches\": " << attempt.clat_weight_mismatches << ",\n"
					  << indent << "  \"clat_local_bound_weight_sum\": " << json_number( attempt.clat_local_bound_weight_sum ) << ",\n"
					  << indent << "  \"clat_branch_ordering_score\": " << json_number( attempt.clat_branch_ordering_score ) << ",\n"
					  << indent << "  \"solver_best_weight\": " << json_number( attempt.solver_best_weight ) << ",\n"
					  << indent << "  \"solver_best_complete\": " << ( attempt.solver_best_complete ? "true" : "false" ) << ",\n"
					  << indent << "  \"local_best_weight\": " << json_number( attempt.local_best_weight ) << ",\n"
					  << indent << "  \"bnb_prune\": ";
		write_bnb_prune_audit_json( output_stream, attempt, indent + "  " );
		output_stream << ",\n"
					  << indent << "  \"endpoint\": ";
		if ( attempt.has_endpoint )
			write_forest_endpoint_json( output_stream, attempt.endpoint, indent + "  " );
		else
			output_stream << "null";
		output_stream << ",\n"
					  << indent << "  \"signed_correlation_sum\": " << json_number( static_cast<double>( attempt.signed_correlation_sum ) ) << ",\n"
					  << indent << "  \"abs_correlation_sum\": " << json_number( static_cast<double>( attempt.abs_correlation_sum ) ) << ",\n"
					  << indent << "  \"effective_weight_signed_abs\": " << json_number( static_cast<double>( attempt.effective_weight_signed_abs ) ) << ",\n"
					  << indent << "  \"effective_weight_abs_sum\": " << json_number( static_cast<double>( attempt.effective_weight_abs_sum ) ) << ",\n"
					  << indent << "  \"hull_growth\": ";
		write_hull_growth_json( output_stream, attempt.hull_growth, indent + "  " );
		output_stream << ",\n" << indent << "  \"operator_heatmap\": ";
		write_stage_weight_profile_json( output_stream, attempt.stage_weight_profile, indent + "  " );
		output_stream << ",\n" << indent << "  \"weight_distribution\": ";
		write_forest_weight_distribution_json( output_stream, attempt.weight_distribution, indent + "  " );
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
		ensure_parent_directory( path );
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open linear forest JSON output file: " + path );
		out << "{\n"
			<< "  \"analysis\": \"linear_forest_hull_search\",\n"
			<< "  \"cipher\": \"NeoAlzette\",\n"
			<< "  \"backend\": \"SCIP_C_API\",\n"
			<< "  \"algorithm\": \"MILP Solver Operator Steps + BNB Prune\",\n"
			<< "  \"scope\": \"time_bounded_multi_layer_mask_forest\",\n"
			<< "  \"clat_scope\": \"operator_step_local_bound_branch_ordering_two_variable_modular_add_sub_only\",\n"
			<< "  \"result_type\": " << json_string( run_log.result_type ) << ",\n"
			<< "  \"rounds\": " << run_log.search_options.rounds << ",\n"
			<< "  \"constant_model\": \"fixed-addend-exact-log-weight-milp\",\n"
			<< "  \"hull_mode\": " << json_string( hull_mode_name( run_log.forest_options.hull_mode ) ) << ",\n"
			<< "  \"forest_seed\": " << json_string( "0x" + hex64( run_log.forest_options.seed ) ) << ",\n"
			<< "  \"forest_attempt_limit\": " << run_log.forest_options.attempts << ",\n"
			<< "  \"forest_attempt_limit_policy\": \"0_means_time_limit_only\",\n"
			<< "  \"growth_policy\": \"q1_valid_best_output_becomes_next_layer_input\",\n"
			<< "  \"time_limit_seconds\": " << json_number( run_log.forest_options.time_limit_seconds ) << ",\n"
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
			<< "  \"clat_branch_ordered_attempts\": " << run_log.clat_branch_ordered_attempts << ",\n"
			<< "  \"clat_early_prune_checks\": " << run_log.clat_early_prune_checks << ",\n"
			<< "  \"clat_early_pruned_attempts\": " << run_log.clat_early_pruned_attempts << ",\n"
			<< "  \"solver_calls\": " << run_log.solver_calls << ",\n"
			<< "  \"found_trails\": " << run_log.found_trails << ",\n"
			<< "  \"q1_calls\": " << run_log.q1_calls << ",\n"
			<< "  \"q1_failed\": " << run_log.q1_failed << ",\n"
			<< "  \"impossible_transitions\": " << run_log.impossible_transitions << ",\n"
			<< "  \"zero_correlations\": " << run_log.zero_correlations << ",\n"
			<< "  \"weight_mismatches\": " << run_log.weight_mismatches << ",\n"
			<< "  \"clat_lookups\": " << run_log.clat_lookups << ",\n"
			<< "  \"clat_hits\": " << run_log.clat_hits << ",\n"
			<< "  \"clat_misses\": " << run_log.clat_misses << ",\n"
			<< "  \"clat_cache_entries\": " << run_log.clat_cache_entries << ",\n"
			<< "  \"clat_add_steps\": " << run_log.clat_add_steps << ",\n"
			<< "  \"clat_sub_steps\": " << run_log.clat_sub_steps << ",\n"
			<< "  \"clat_impossible\": " << run_log.clat_impossible << ",\n"
			<< "  \"clat_sign_mismatches\": " << run_log.clat_sign_mismatches << ",\n"
			<< "  \"clat_weight_mismatches\": " << run_log.clat_weight_mismatches << ",\n"
			<< "  \"clat_local_bound_weight_sum\": " << json_number( run_log.clat_local_bound_weight_sum ) << ",\n"
			<< "  \"clat_branch_ordering_score\": " << json_number( run_log.clat_branch_ordering_score ) << ",\n"
			<< "  \"bnb_pruned_attempts\": " << run_log.bnb_pruned_attempts << ",\n"
			<< "  \"bnb_prune_checks\": " << run_log.bnb_prune_checks << ",\n"
			<< "  \"bnb_prune_applied\": " << run_log.bnb_prune_applied << ",\n"
			<< "  \"bnb_prune_deferred\": " << run_log.bnb_prune_deferred << ",\n"
			<< "  \"bnb_objective_cutoff_constraints\": " << run_log.bnb_objective_cutoff_constraints << ",\n"
			<< "  \"global_best_weight\": " << ( run_log.has_global_best ? json_number( run_log.global_best_weight ) : "null" ) << ",\n"
			<< "  \"global_best_trail_count\": " << run_log.global_best_trail_count << ",\n"
			<< "  \"global_best_endpoint\": ";
		if ( run_log.has_global_best )
			write_forest_endpoint_json( out, run_log.global_best_endpoint, "  " );
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
		std::cout << "LINEAR_FOREST_JSON_FILE=" << path << "\n";
	}

	static void validate_forest_configuration( const SearchOptions& options, const ForestOptions& forest_options )
	{
		if ( !options.output_round_table_json.empty() )
			throw std::runtime_error( "round-table output is handled by neoalzette_scip_round_milp_search" );
		if ( options.continuous_best_trail )
			throw std::runtime_error( "--continuous-best-trail is handled by neoalzette_scip_round_milp_search" );
		if ( ( *options.fix_input_ma | *options.fix_input_mb ) == 0u )
			throw std::runtime_error( "linear forest mode requires a nonzero input-mask source" );
		if ( options.fix_output_ma || options.fix_output_mb )
			throw std::runtime_error( "linear forest mode derives output endpoints from the solver; do not pass --fix-output-ma/--fix-output-mb" );
		if ( !std::isfinite( forest_options.time_limit_seconds ) || forest_options.time_limit_seconds <= 0.0 )
			throw std::runtime_error( "linear forest mode requires --time-limit as the total N-round MILP search wall-clock budget" );
		if ( forest_options.attempts < 0 )
			throw std::runtime_error( "--forest-attempts must be >= 0 (0 means time-limit-only)" );
		if ( forest_options.max_enumerate_solutions < 1 )
			throw std::runtime_error( "--max-enumerate-solutions must be >= 1" );
		if ( forest_options.hull_mode == HullMode::STRONG_HULL )
			throw std::runtime_error( "strong-hull / endpoint candidate sweep is disabled in forest Q1 mode; no Q2 candidate generation is used" );
		if ( forest_options.hull_mode != HullMode::BOUNDED_ENDPOINT && forest_options.hull_mode != HullMode::COMPLETE_ENDPOINT )
			throw std::runtime_error( "linear forest hull_search supports bounded-endpoint or complete-endpoint modes" );
	}

	static void set_forest_time_limit_from_search_options( const SearchOptions& options, ForestOptions& forest_options )
	{
		forest_options.time_limit_seconds = options.time_limit_seconds;
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
			run_log.result_type = ( run_log.completed_attempts == run_log.total_attempts ) ? "complete_endpoint_under_current_model" : "best_found_partial_forest";
			return;
		}
		run_log.result_type = "threshold_bounded_hull_like_exploration";
	}

	// ------------------------------------------------------------------------
	// Audit section 5: forest hull driver
	// ------------------------------------------------------------------------
	// Driver flow:
	//   1. solve the best characteristic for the fixed input-mask source,
	//   2. choose the endpoint to enumerate,
	//   3. rerun the same MILP with semantic no-good cuts, and
	//   4. aggregate signed terms by weight as coefficient * 2^-W.
	// Scope note for linear-hull literature: bounded-endpoint mode is a
	// threshold/windowed enumeration around the selected endpoint. complete-endpoint
	// is complete only when the endpoint enumeration proves UNSAT after the last
	// no-good cut, and only for the fixed endpoint under this MILP model.
	// `--time-limit` is treated as one total wall-clock budget for the whole
	// command, not as a fresh budget for every endpoint enumeration solve.
	static void run_forest_hull_search( SearchOptions options, ForestOptions forest_options )
	{
		ForestRunLog run_log;
		run_log.forest_options = forest_options;
		run_log.search_options = options;
		run_log.result_type = forest_options.hull_mode == HullMode::COMPLETE_ENDPOINT ? "complete_endpoint_under_current_model" : "threshold_bounded_hull_like_exploration";

		truncate_file_if_requested( forest_options.hull_characteristics_jsonl );

		const auto forest_start = std::chrono::steady_clock::now();
		const ForestInputMask initial_input { *options.fix_input_ma, *options.fix_input_mb };
		ForestCandidateQueue candidate_bank;
		std::map<std::uint64_t, std::vector<ForestStateFrontierEntry>> frontier;
		std::map<std::uint64_t, int> input_frequency;
		std::map<std::uint64_t, int> output_frequency;
		ForestClatCache clat_cache;
		std::optional<ScipSolveResult> current_best_result;
		std::uint64_t candidate_serial = 0;
		int next_tree_id = 1;
		int derived_root_index = 0;

		auto enqueue_candidate = [&]( ForestCandidate candidate ) {
			if ( !register_forest_frontier( frontier, candidate ) ) return false;
			candidate_bank.push( std::move( candidate ) );
			run_log.candidate_bank_peak = std::max( run_log.candidate_bank_peak, static_cast<int>( candidate_bank.size() ) );
			return true;
		};

		const std::uint64_t initial_key = pack_input_mask( initial_input );
		enqueue_candidate( make_forest_candidate( initial_input, -1, 0, 0, false,
			forest_options.seed ^ initial_key, 0.0, candidate_serial++, false, {}, { initial_key }, { 0.0 } ) );

		auto enqueue_derived_root = [&]() {
			for ( int retry = 0; retry < 4096; ++retry )
			{
				std::uint64_t derive_seed = 0;
				const int root_number = ++derived_root_index;
				const ForestInputMask root_input = derive_next_input_mask( initial_input, forest_options.seed, root_number, derive_seed );
				const std::uint64_t root_key = pack_input_mask( root_input );
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
			if ( candidate_bank.empty() && !enqueue_derived_root() ) break;

			const int bank_size_before = static_cast<int>( candidate_bank.size() );
			ForestCandidate candidate = candidate_bank.top();
			candidate_bank.pop();
			ForestAttemptLog attempt = make_forest_attempt( candidate, run_log.total_attempts, bank_size_before );
			attempt.input_frequency = ++input_frequency[ pack_input_mask( attempt.input ) ];
			attempt.duplicate_input = attempt.input_frequency > 1;
			run_log.max_layer = std::max( run_log.max_layer, attempt.layer );
			auto& layer_summary = run_log.layer_growth[ attempt.layer ];
			layer_summary.layer = attempt.layer;
			++layer_summary.attempts;

			print_input_source_change( attempt );
			print_clat_branch_order( attempt );
			++run_log.total_attempts;

			const double best_remaining_total_budget = forest_remaining_total_budget_for_next_milp( forest_options, forest_start );
			SearchOptions best_options = make_best_search_options_for_input( options, attempt.input, best_remaining_total_budget );
			NoGoodStore best_no_goods;
			ScipSolveResult best_result = solve_linear_model( best_options, best_no_goods, true );
			++attempt.solver_calls;
			attempt.solver_status = scip_status_name( best_result.scip_status );
			attempt.hit_time_limit = best_result.hit_time_limit;
			attempt.hit_memory_limit = best_result.hit_memory_limit;
			attempt.solver_best_complete = best_result.complete;
			if ( best_result.hit_time_limit )
			{
				run_log.hit_global_time_limit = true;
				print_hull_time_limit_incumbent_or_current( best_result, current_best_result, "best_trail_solve_attempt_" + std::to_string( attempt.attempt_id ) );
			}

			if ( !best_result.feasible )
			{
				attempt.pruned = true;
				attempt.stop_reason = best_result.hit_time_limit ? "global_time_limit" : forest_stop_reason_from_result( best_result, "solver_infeasible_for_input" );
				attempt.candidate_bank_size_after = static_cast<int>( candidate_bank.size() );
				absorb_attempt_totals( run_log, attempt );
				run_log.attempts.push_back( std::move( attempt ) );
				if ( run_log.hit_global_time_limit ) break;
				continue;
			}

			std::map<std::string, HullWeightDistributionEntry> grouped_counts;
			NoGoodStore endpoint_enumeration;
			record_feasible_forest_trail( attempt, run_log, best_options, forest_options.hull_characteristics_jsonl, best_result, grouped_counts, endpoint_enumeration, clat_cache );
			attempt.solver_best_weight = best_result.snapshot.objective;
			if ( run_log.has_global_best && std::fabs( best_result.snapshot.objective - run_log.global_best_weight ) <= 1e-8 ) current_best_result = best_result;
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
			const ForestInputMask child_input { best_endpoint.output_mask_A_value, best_endpoint.output_mask_B_value };
			attempt.output_frequency = ++output_frequency[ pack_input_mask( child_input ) ];
			if ( ( child_input.mA | child_input.mB ) == 0u )
			{
				attempt.continuation_zero_state = true;
			}
			else
			{
				const std::uint64_t child_key = pack_input_mask( child_input );
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
				const double initial_enum_budget = forest_remaining_total_budget_for_next_milp( forest_options, forest_start );
				SearchOptions enum_options = make_endpoint_enumeration_options( options, fixed_endpoint, best_result.snapshot.objective, initial_enum_budget, forest_options );
				LinearHullReoptimizationSession endpoint_session( enum_options, endpoint_enumeration );
				while ( attempt.found_trails < forest_options.max_enumerate_solutions )
				{
					if ( forest_deadline_expired( forest_options, forest_start ) )
					{
						attempt.hit_time_limit = true;
						attempt.stop_reason = "global_time_limit";
						run_log.hit_global_time_limit = true;
						print_hull_current_time_limit_incumbent( current_best_result, "global_time_limit_before_endpoint_enumeration" );
						break;
					}

					const double remaining_total_budget = forest_remaining_total_budget_for_next_milp( forest_options, forest_start );
					ScipSolveResult enum_result = endpoint_session.solve_next( remaining_total_budget, true );
					++attempt.solver_calls;
					attempt.solver_status = scip_status_name( enum_result.scip_status );
					attempt.hit_time_limit = attempt.hit_time_limit || enum_result.hit_time_limit;
					attempt.hit_memory_limit = attempt.hit_memory_limit || enum_result.hit_memory_limit;
					if ( enum_result.hit_time_limit )
					{
						run_log.hit_global_time_limit = true;
						print_hull_time_limit_incumbent_or_current( enum_result, current_best_result, "endpoint_enumeration_attempt_" + std::to_string( attempt.attempt_id ) );
					}
					if ( !enum_result.feasible )
					{
						attempt.stop_reason = enum_result.hit_time_limit ? "global_time_limit" : forest_stop_reason_from_result( enum_result, "enumeration_unsat" );
						if ( attempt.stop_reason == "enumeration_unsat" ) attempt.completed = true;
						break;
					}
					record_feasible_forest_trail( attempt, run_log, enum_options, forest_options.hull_characteristics_jsonl, enum_result, grouped_counts, endpoint_enumeration, clat_cache );
					if ( run_log.has_global_best && std::fabs( enum_result.snapshot.objective - run_log.global_best_weight ) <= 1e-8 ) current_best_result = enum_result;
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
		run_log.clat_cache_entries = clat_cache.size();
		finalize_run_result_type( run_log );
		write_forest_json_file( forest_options.hull_output_json, run_log );
		std::cout << "\n=== LINEAR_FOREST_HULL_SEARCH / SCIP ===\n"
				  << "growth_policy=Q1-valid best output becomes next Forest Layer input\n"
				  << "result_type=" << run_log.result_type << "\n"
				  << "total_attempts=" << run_log.total_attempts << " max_layer=" << run_log.max_layer
				  << " continuations=" << run_log.continuations_enqueued << " cycles=" << run_log.cycles_detected << "\n"
				  << "candidate_bank_peak=" << run_log.candidate_bank_peak << " derived_roots=" << run_log.derived_roots_enqueued << "\n"
				  << "found_trails=" << run_log.found_trails << " q1_failed=" << run_log.q1_failed
				  << " zero_correlations=" << run_log.zero_correlations << "\n";
		if ( run_log.has_global_best )
			std::cout << "global_best_weight=" << std::setprecision( 12 ) << run_log.global_best_weight
					  << " endpoint=ΛA_in=0x" << hex32( run_log.global_best_endpoint.input_mask_A_value )
					  << " ΛB_in=0x" << hex32( run_log.global_best_endpoint.input_mask_B_value )
					  << " ΛA_out=0x" << hex32( run_log.global_best_endpoint.output_mask_A_value )
					  << " ΛB_out=0x" << hex32( run_log.global_best_endpoint.output_mask_B_value ) << "\n";
		else
			std::cout << "global_best_weight=none\n";
	}

	// CLI-shaped C++ entry shared by the standalone executable and the
	// multi-process campaign worker. Keeping parsing, total-budget transfer,
	// validation, and execution here prevents the runner from drifting from HULL semantics.
	static void run_forest_hull_search_from_argv( int argc, char** argv )
	{
		auto [ forest_options, stripped_args ] = parse_forest_options_and_strip_argv( argc, argv );
		SearchOptions options = parse_base_options_from_strings( stripped_args );
		set_forest_time_limit_from_search_options( options, forest_options );
		validate_forest_configuration( options, forest_options );
		run_forest_hull_search( options, forest_options );
	}
}  // namespace neoalzette_linear_milp

#ifndef NEOALZETTE_HULL_LIBRARY_MODE
int main( int argc, char** argv )
{
	using namespace neoalzette_linear_milp;
	try
	{
		if ( has_arg( argc, argv, "--help" ) )
		{
			print_forest_help( argv[ 0 ] );
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
