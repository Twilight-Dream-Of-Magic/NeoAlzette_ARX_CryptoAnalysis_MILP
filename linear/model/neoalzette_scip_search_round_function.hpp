#pragma once

#include "neoalzette_scip_operator_analysis_milp_constraint.hpp"

// ============================================================================
// NeoAlzette LINEAR round-search orchestration
// ============================================================================
//
// Shared implementation for the linear round-search entry points.
//
// This layer instantiates the forward NeoAlzette round in transpose-mask order,
// records signed per-step trace metadata, and serializes JSON for the shared
// best-trail / round-table workflows.
//
// Audit map:
//   1. result/trace data structures and JSON helpers;
//   2. NeoAlzetteScipLinearSearch: round-level MILP construction;
//   3. solution finalization and oracle-based weight trace audit;
//   4. CLI parsing and run modes.
// ============================================================================

namespace neoalzette_linear_milp
{
	// ------------------------------------------------------------------------
	// Audit section 1: result/trace data structures and JSON helpers
	// ------------------------------------------------------------------------
	// This first block contains solution readers, trace/result records, and JSON
	// serialization helpers. It does not add SCIP constraints.
	static std::uint32_t value_from_solution( SCIP* scip, SCIP_SOL* sol, const BitVec& bits )
	{
		if ( bits.size() > 32 )
			throw std::runtime_error( "value_from_solution: bit vector wider than 32 bits" );
		std::uint32_t x = 0u;
		for ( int i = 0; i < static_cast<int>( bits.size() ); ++i )
			if ( SCIPgetSolVal( scip, sol, bits[ i ].var ) > 0.5 )
				x |= ( std::uint32_t( 1 ) << i );
		return x;
	}

	static std::uint64_t value64_from_solution( SCIP* scip, SCIP_SOL* sol, const BitVec& bits )
	{
		if ( bits.size() > 64 )
			throw std::runtime_error( "value64_from_solution: bit vector wider than 64 bits" );
		std::uint64_t x = 0u;
		for ( int i = 0; i < static_cast<int>( bits.size() ); ++i )
			if ( SCIPgetSolVal( scip, sol, bits[ i ].var ) > 0.5 )
				x |= ( std::uint64_t( 1 ) << i );
		return x;
	}

	struct InjectionInstance
	{
		std::string	  name;
		InjectionKind kind;
		BitVec		  alpha_mask;
		BitVec		  beta_xor_mask;
		BitVec		  beta_addend_mask;
		SVar		  weight;
	};

	struct NoGoodCut
	{
		std::vector<std::pair<std::string, int>> assignment;
	};

	struct NoGoodStore
	{
		std::vector<NoGoodCut> no_good_cuts;
	};

	enum class FixedConstantModel
	{
		FIXED_ADDEND_EXACT_STATIC_THRESHOLD_MILP
	};

	struct SearchOptions
	{
		int							 rounds = 1;
		bool						 quiet = false;
		bool						 continuous_best_trail = false;
		double						 time_limit_seconds = std::numeric_limits<double>::quiet_NaN();
		FixedConstantModel		 constant_model = FixedConstantModel::FIXED_ADDEND_EXACT_STATIC_THRESHOLD_MILP;
		bool						 require_nonzero_input_mask = true;
		bool						 require_nonzero_output_mask = true;
		std::string					 output_result_json = "linear_scip_best_result.json";
		std::string					 output_weight_trace_json = "linear_scip_weight_trace.json";
		std::string					 output_round_table_json;
		std::optional<std::uint32_t> fix_input_ma;
		std::optional<std::uint32_t> fix_input_mb;
		std::optional<std::uint32_t> fix_output_ma;
		std::optional<std::uint32_t> fix_output_mb;
		double					 objective_window_from = std::numeric_limits<double>::quiet_NaN();
		double					 objective_window_to = std::numeric_limits<double>::quiet_NaN();
	};

	struct SolutionSnapshot
	{
		double		  objective = 0.0;
		std::uint32_t input_mask_A_value = 0;
		std::uint32_t input_mask_B_value = 0;
		std::uint32_t output_mask_A_value = 0;
		std::uint32_t output_mask_B_value = 0;
	};

	struct WeightStepSpec
	{
		int			round = 0;
		int			step = 0;
		std::string stage;
		std::string operation;
		std::string prefix;
		std::string weight_model;
		BitVec		A_before, B_before, A_after, B_after;
		bool		has_local_input0 = false;
		bool		has_local_input1 = false;
		bool		has_local_output = false;
		std::string local_input0_label, local_input1_label, local_output_label;
		BitVec		local_input0, local_input1, local_output;
		bool		has_public_constant = false;
		std::uint32_t public_constant = 0;
		std::uint32_t effective_constant = 0;
		bool		has_fixed_characteristic = false;
		BitVec		fixed_carry;
		BitVec		fixed_bad;
		std::string injection_name;
		std::size_t objective_begin = 0;
		std::size_t objective_end = 0;
		bool has_fixed_exact_oracle = false;
		SVar fixed_exact_weight;
	};

	struct WeightTraceTerm
	{
		std::string variable;
		double		coefficient = 0.0;
		double		value = 0.0;
		double		contribution = 0.0;
	};

	struct WeightTraceEntry
	{
		int			round = 0;
		int			step = 0;
		std::string stage;
		std::string operation;
		std::string prefix;
		std::string weight_model;
		std::uint32_t A_before = 0, B_before = 0, A_after = 0, B_after = 0;
		bool		has_local_input0 = false, has_local_input1 = false, has_local_output = false;
		std::string local_input0_label, local_input1_label, local_output_label;
		std::uint32_t local_input0 = 0, local_input1 = 0, local_output = 0;
		bool		has_public_constant = false;
		std::uint32_t public_constant = 0;
		std::uint32_t effective_constant = 0;
		bool		has_injection_oracle = false;
		std::string injection_name;
		bool		injection_valid = false;
		int			injection_rank = 0;
		double		model_oracle_weight = 0.0;
		double		injection_oracle_weight = 0.0;
		std::uint32_t injection_alpha_mask = 0;
		std::uint32_t injection_beta_xor_mask = 0;
		std::uint32_t injection_beta_addend_mask = 0;
		std::uint64_t injection_packed_beta_mask = 0;
		bool		has_fixed_characteristic = false;
		std::uint64_t fixed_carry_mask = 0;
		std::uint32_t fixed_bad_mask = 0;
		bool		fixed_const_exact_possible = false;
		int		fixed_const_exact_sign = 0;
		double		fixed_const_exact_weight = std::numeric_limits<double>::infinity();
		long double fixed_const_exact_correlation = 0.0L;
		long double fixed_const_exact_abs_correlation = 0.0L;
		int			local_sign = 0;
		double		local_weight = 0.0;
		double		cumulative_weight = 0.0;
		int			objective_term_count = 0;
		int			selected_term_count = 0;
		std::vector<WeightTraceTerm> selected_terms;
	};

	struct ScipSolveResult
	{
		bool			 feasible = false;
		bool			 complete = false;
		bool			 hit_time_limit = false;
		bool			 hit_memory_limit = false;
		SCIP_STATUS		 scip_status = SCIP_STATUS_UNKNOWN;
		std::string		 error_message;
		SolutionSnapshot snapshot;
		NoGoodCut		 no_good;
		int				 characteristic_sign = 0;
		long double		 signed_correlation_contribution = 0.0L;
		long double		 abs_correlation_contribution = 0.0L;
		std::vector<WeightTraceEntry> weight_trace;
		double						 weight_trace_total = 0.0;
		bool						 exact_evaluated_total_weight_available = false;
		double						 exact_evaluated_total_weight = std::numeric_limits<double>::infinity();
		double						 weight_trace_objective_delta = 0.0;
		bool						 weight_trace_available = false;
		bool						 weight_trace_matches_objective = false;
		bool						 weight_trace_oracles_valid = false;
		bool						 weight_trace_oracle_weights_consistent = false;
		bool						 paper_usable_characteristic = false;
	};

	static void write_best_result_json_file( const std::string& path, const SearchOptions& options, const ScipSolveResult& r );
	static void write_weight_trace_json_file( const std::string& path, const SearchOptions& options, const ScipSolveResult& r );
	static void write_weight_trace_json_array( std::ostream& output_stream, const std::vector<WeightTraceEntry>& trace, const std::string& indent );
	static void write_round_table_json_file( const std::string& path, const SearchOptions& base_options, const std::vector<ScipSolveResult>& rows );
	static double exact_evaluated_total_weight_from_trace( const std::vector<WeightTraceEntry>& trace );

	static std::string hex32( std::uint32_t x )
	{
		std::ostringstream oss;
		oss << std::hex << std::setw( 8 ) << std::setfill( '0' ) << x;
		return oss.str();
	}

	static std::string hex64( std::uint64_t x )
	{
		std::ostringstream oss;
		oss << std::hex << std::setw( 16 ) << std::setfill( '0' ) << x;
		return oss.str();
	}

	static std::string hex32_json( std::uint32_t x )
	{
		return "0x" + hex32( x );
	}

	static std::string hex64_json( std::uint64_t x )
	{
		return "0x" + hex64( x );
	}

	static std::string json_string( const std::string& s )
	{
		std::ostringstream oss;
		oss << '"';
		for ( char ch : s )
		{
			if ( ch == '"' || ch == '\\' )
				oss << '\\' << ch;
			else if ( ch == '\n' )
				oss << "\\n";
			else
				oss << ch;
		}
		oss << '"';
		return oss.str();
	}

	static std::string json_number( double v )
	{
		if ( !std::isfinite( v ) )
			return "null";
		std::ostringstream oss;
		oss << std::setprecision( 17 ) << v;
		return oss.str();
	}

	static std::string json_number( long double v )
	{
		if ( !std::isfinite( static_cast<double>( v ) ) )
			return "null";
		std::ostringstream oss;
		oss << std::setprecision( 21 ) << v;
		return oss.str();
	}

	static const char* constant_model_name( FixedConstantModel mode )
	{
		switch ( mode )
		{
		case FixedConstantModel::FIXED_ADDEND_EXACT_STATIC_THRESHOLD_MILP:
			return "fixed-addend-exact-log-weight-milp";
		}
		return "unknown";
	}

	static std::optional<std::chrono::steady_clock::time_point> make_sweep_deadline( const SearchOptions& options )
	{
		if ( !std::isfinite( options.time_limit_seconds ) || options.time_limit_seconds <= 0.0 )
			return std::nullopt;
		return std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>( std::chrono::duration<double>( options.time_limit_seconds ) );
	}

	static bool sweep_deadline_expired( const std::optional<std::chrono::steady_clock::time_point>& deadline )
	{
		return deadline.has_value() && std::chrono::steady_clock::now() >= *deadline;
	}

	static double sweep_remaining_seconds( const std::optional<std::chrono::steady_clock::time_point>& deadline )
	{
		if ( !deadline )
			return std::numeric_limits<double>::quiet_NaN();
		const auto now = std::chrono::steady_clock::now();
		if ( now >= *deadline )
			return 0.0;
		return std::chrono::duration<double>( *deadline - now ).count();
	}

	static const char* scip_status_name( SCIP_STATUS s )
	{
		switch ( s )
		{
		case SCIP_STATUS_UNKNOWN:
			return "unknown";
		case SCIP_STATUS_USERINTERRUPT:
			return "user_interrupt";
		case SCIP_STATUS_NODELIMIT:
			return "node_limit";
		case SCIP_STATUS_TOTALNODELIMIT:
			return "total_node_limit";
		case SCIP_STATUS_STALLNODELIMIT:
			return "stall_node_limit";
		case SCIP_STATUS_TIMELIMIT:
			return "time_limit";
		case SCIP_STATUS_MEMLIMIT:
			return "memory_limit";
		case SCIP_STATUS_GAPLIMIT:
			return "gap_limit";
		case SCIP_STATUS_SOLLIMIT:
			return "solution_limit";
		case SCIP_STATUS_BESTSOLLIMIT:
			return "best_solution_limit";
		case SCIP_STATUS_RESTARTLIMIT:
			return "restart_limit";
		case SCIP_STATUS_OPTIMAL:
			return "optimal";
		case SCIP_STATUS_INFEASIBLE:
			return "infeasible";
		case SCIP_STATUS_UNBOUNDED:
			return "unbounded";
		case SCIP_STATUS_INFORUNBD:
			return "infeasible_or_unbounded";
		}
		return "other";
	}

	static void ensure_parent_directory( const std::string& path )
	{
		if ( path.empty() )
			return;
		const std::filesystem::path p( path );
		const auto				   parent = p.parent_path();
		if ( !parent.empty() )
			std::filesystem::create_directories( parent );
	}

	static void truncate_file_if_requested( const std::string& path )
	{
		if ( path.empty() )
			return;
		ensure_parent_directory( path );
		std::ofstream out( path, std::ios::trunc );
		if ( !out )
			throw std::runtime_error( "failed to truncate output file: " + path );
	}

	static std::string correlation_string_from_weight( double weight )
	{
		std::ostringstream oss;
		oss << "2^-" << std::setprecision( 12 ) << weight;
		return oss.str();
	}

	static long double pow2_neg_ld( long double weight )
	{
		return std::pow( 2.0L, -weight );
	}

	struct NeoAlzetteScipLinearSearch
	{
		// --------------------------------------------------------------------
		// Audit section 2: round-level linear MILP construction
		// --------------------------------------------------------------------
		// The helper methods below are intentionally grouped by operator type:
		// injection Walsh bridge, two-variable addition box, fixed-addend
		// subtraction, and finally build_one_round() as the schedule.
		SearchOptions				   options;
		std::vector<InjectionInstance> injections;
		std::vector<WeightStepSpec>	   weight_steps;
		BitVec						   input_mask_A_bits, input_mask_B_bits, output_mask_A_bits, output_mask_B_bits;

		explicit NeoAlzetteScipLinearSearch( SearchOptions opt ) : options( std::move( opt ) )
		{
			if ( options.rounds < 1 )
				throw std::runtime_error( "--rounds must be >= 1" );
		}

		BitVec create_xor_of_two_right_rotated_masks( ScipModelBuilder& model_builder, const BitVec& mask, int first_rotation_amount, int second_rotation_amount, const std::string& prefix )
		{
			return model_builder.create_xor_vector(
				ScipModelBuilder::rotate_right( mask, first_rotation_amount ),
				ScipModelBuilder::rotate_right( mask, second_rotation_amount ),
				prefix );
		}

		void add_weight_step( WeightStepSpec spec )
		{
			spec.step = static_cast<int>( weight_steps.size() );
			weight_steps.push_back( std::move( spec ) );
		}

		void fix_bit_vector_to_value( ScipModelBuilder& model_builder, const BitVec& bits, std::uint32_t value, const std::string& name )
		{
			if ( bits.size() > 32 )
				throw std::invalid_argument( "fix_bit_vector_to_value: bit vector wider than 32 bits" );
			for ( int bit_index = 0; bit_index < static_cast<int>( bits.size() ); ++bit_index )
				model_builder.add_constant_equality_constraint( name + "_bit_" + std::to_string( bit_index ), { { bits[ bit_index ], 1 } }, static_cast<double>( ( value >> bit_index ) & 1u ) );
		}

		static bool mask_pair_is_fully_fixed( const std::optional<std::uint32_t>& first_mask, const std::optional<std::uint32_t>& second_mask )
		{
			return first_mask.has_value() && second_mask.has_value();
		}

		static void add_nonzero_mask_pair_constraint( ScipModelBuilder& model_builder, const BitVec& first_mask, const BitVec& second_mask, const std::string& name )
		{
			std::vector<LinearTerm> nonzero_terms;
			for ( const auto& mask_bit : first_mask )
				nonzero_terms.push_back( { mask_bit, 1 } );
			for ( const auto& mask_bit : second_mask )
				nonzero_terms.push_back( { mask_bit, 1 } );
			model_builder.add_linear_constraint( name, nonzero_terms, 1.0, INF );
		}

		BitVec pack_joint_beta_bits( const BitVec& beta_xor_mask, const BitVec& beta_addend_mask )
		{
			if ( beta_xor_mask.size() != WORD_SIZE || beta_addend_mask.size() != WORD_SIZE )
				throw std::invalid_argument( "pack_joint_beta_bits: both beta masks must be 32-bit" );
			BitVec packed = beta_xor_mask;
			packed.insert( packed.end(), beta_addend_mask.begin(), beta_addend_mask.end() );
			return packed;
		}

		BitVec create_joint_injection_alpha_with_walsh_constraint(
			ScipModelBuilder& model_builder,
			const BitVec& beta_xor_mask,
			const BitVec& beta_addend_mask,
			const std::string& prefix,
			InjectionKind kind )
		{
			BitVec alpha_mask = model_builder.create_bit_vector( prefix + "_alpha", WORD_SIZE );
			BitVec packed_beta = pack_joint_beta_bits( beta_xor_mask, beta_addend_mask );
			SVar   linear_weight = model_builder.create_continuous_variable( prefix + "_linear_weight", 0.0, 32.0, 1.0 );
			SCIP_CALL_THROW( add_linear_injection_walsh_constraint( model_builder.scip, prefix, kind, alpha_mask, packed_beta, linear_weight ) );
			injections.push_back( { prefix, kind, alpha_mask, beta_xor_mask, beta_addend_mask, linear_weight } );
			return alpha_mask;
		}

		void add_modular_addition_with_rotated_linear_addend( ScipModelBuilder& model_builder,
										 const BitVec& changed_mask_before_addition,
										 const BitVec& controller_mask_before_addition,
										 BitVec& changed_mask_after_addition,
										 BitVec& controller_mask_after_addition,
										 const std::string& prefix,
										 const std::string& stage,
										 std::uint32_t public_constant,
										 bool changed_branch_is_A,
										 int round )
		{
			changed_mask_after_addition = model_builder.create_bit_vector( prefix + "_changed_after", WORD_SIZE );
			controller_mask_after_addition = model_builder.create_bit_vector( prefix + "_controller_after", WORD_SIZE );
			BitVec rotated_xor_addend_mask = model_builder.create_bit_vector( prefix + "_rot_xor_addend_mask", WORD_SIZE );
			const std::size_t objective_begin = model_builder.objective_terms.size();
			arithmetic_model::add_two_input_modular_addition_linear_characteristic_constraints( model_builder, changed_mask_after_addition, changed_mask_before_addition, rotated_xor_addend_mask, prefix + "_ADD_LINEAR_BOX" );
			BitVec controller_transpose_extra_mask = create_xor_of_two_right_rotated_masks( model_builder, rotated_xor_addend_mask, 31, 17, prefix + "_controller_transpose_extra" );
			model_builder.add_xor_vector_equality_constraints( controller_mask_before_addition, controller_mask_after_addition, controller_transpose_extra_mask, prefix + "_controller_before_relation" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  stage,
											  "two_variable_modular_addition_linear",
											  prefix,
											  "wallen_two_input_add_linear_characteristic",
											  changed_branch_is_A ? changed_mask_before_addition : controller_mask_before_addition,
											  changed_branch_is_A ? controller_mask_before_addition : changed_mask_before_addition,
											  changed_branch_is_A ? changed_mask_after_addition : controller_mask_after_addition,
											  changed_branch_is_A ? controller_mask_after_addition : changed_mask_after_addition,
											  true,
											  true,
											  true,
											  changed_branch_is_A ? "mask_A_before" : "mask_B_before",
											  changed_branch_is_A ? "mask_rotB_addend" : "mask_rotA_addend",
											  changed_branch_is_A ? "mask_A_after" : "mask_B_after",
											  changed_mask_before_addition,
											  rotated_xor_addend_mask,
											  changed_mask_after_addition,
											  true,
											  public_constant,
											  public_constant,
											  false,
											  BitVec {},
											  BitVec {},
											  "",
											  objective_begin,
											  model_builder.objective_terms.size() } );
		}

		void add_modular_addition_with_direct_xor_constant_addend( ScipModelBuilder& model_builder,
										 const BitVec& changed_mask_before_addition,
										 const BitVec& controller_mask_before_addition,
										 BitVec& changed_mask_after_addition,
										 BitVec& controller_mask_after_addition,
										 const std::string& prefix,
										 const std::string& stage,
										 std::uint32_t public_constant,
										 bool changed_branch_is_A,
										 int round )
		{
			changed_mask_after_addition = model_builder.create_bit_vector( prefix + "_changed_after", WORD_SIZE );
			controller_mask_after_addition = model_builder.create_bit_vector( prefix + "_controller_after", WORD_SIZE );
			BitVec direct_addend_mask = model_builder.create_bit_vector( prefix + "_direct_addend_mask", WORD_SIZE );
			const std::size_t objective_begin = model_builder.objective_terms.size();
			arithmetic_model::add_two_input_modular_addition_linear_characteristic_constraints( model_builder, changed_mask_after_addition, changed_mask_before_addition, direct_addend_mask, prefix + "_ADD_LINEAR_BOX" );
			model_builder.add_xor_vector_equality_constraints( controller_mask_before_addition, controller_mask_after_addition, direct_addend_mask, prefix + "_controller_before_relation" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  stage,
											  "two_variable_modular_addition_linear",
											  prefix,
											  "wallen_two_input_add_linear_characteristic",
											  changed_branch_is_A ? changed_mask_before_addition : controller_mask_before_addition,
											  changed_branch_is_A ? controller_mask_before_addition : changed_mask_before_addition,
											  changed_branch_is_A ? changed_mask_after_addition : controller_mask_after_addition,
											  changed_branch_is_A ? controller_mask_after_addition : changed_mask_after_addition,
											  true,
											  true,
											  true,
											  changed_branch_is_A ? "mask_A_before" : "mask_B_before",
											  changed_branch_is_A ? "mask_B_xor_constant_addend" : "mask_A_xor_constant_addend",
											  changed_branch_is_A ? "mask_A_after" : "mask_B_after",
											  changed_mask_before_addition,
											  direct_addend_mask,
											  changed_mask_after_addition,
											  true,
											  public_constant,
											  public_constant,
											  false,
											  BitVec {},
											  BitVec {},
											  "",
											  objective_begin,
											  model_builder.objective_terms.size() } );
		}


		void add_fixed_public_constant_subtraction_step(
			ScipModelBuilder& model_builder,
			const BitVec& unchanged_other_mask,
			const BitVec& input_mask,
			BitVec& output_mask,
			const std::string& prefix,
			const std::string& stage,
			std::uint32_t public_constant,
			bool changed_branch_is_A,
			int round )
		{
			output_mask = model_builder.create_bit_vector( prefix + "_after", WORD_SIZE );
			const std::uint32_t effective_constant = static_cast<std::uint32_t>( -public_constant );
			const std::size_t objective_begin = model_builder.objective_terms.size();

			// Strict fixed-addend linear model:
			// Y = X + K mod 2^n is encoded by Miyano's 2-state signed
			// transfer matrix.  The MILP builds the exact signed numerator
			//
			//     N = 2^n * C_K(alpha,beta),
			//
			// binds A = |N|, and charges the exact log-weight
			//
			//     W = n - log2(A)
			//
			// through a lazy tangent-cut epigraph constraint handler.  There is
			// no outer numerator sweep and no logarithmic ladder approximation.
			arithmetic_model::FixedConstExactThresholdMilpHandle fixed_addend_handle =
				arithmetic_model::add_fixed_public_constant_subtraction_linear_exact_numerator_threshold_constraints(
					model_builder,
					public_constant,
					input_mask,
					output_mask,
					1u,
					prefix );

			add_fixed_addend_exact_log_weight_milp_objective(
				model_builder,
				fixed_addend_handle,
				WORD_SIZE,
				prefix + "_EXACT_LOG_WEIGHT_MILP" );

			add_weight_step( WeightStepSpec { round,
										  0,
										  stage,
										  "fixed_public_constant_subtraction_linear_exact_log_weight_milp",
										  prefix,
										  "miyano_fixed_addend_exact_log_weight_milp",
										  changed_branch_is_A ? input_mask : unchanged_other_mask,
										  changed_branch_is_A ? unchanged_other_mask : input_mask,
										  changed_branch_is_A ? output_mask : unchanged_other_mask,
										  changed_branch_is_A ? unchanged_other_mask : output_mask,
										  true,
										  false,
										  true,
										  changed_branch_is_A ? "mask_A_before" : "mask_B_before",
										  "",
										  changed_branch_is_A ? "mask_A_after" : "mask_B_after",
										  input_mask,
										  BitVec {},
										  output_mask,
										  true,
										  public_constant,
										  effective_constant,
										  false,
										  BitVec {},
										  BitVec {},
										  "",
										  objective_begin,
										  model_builder.objective_terms.size(),
										  true,
										  SVar {} } );
		}

		// Build one NeoAlzette linear round in the same order as the value-domain
		// core, but with masks propagated through transpose relations.  Each
		// nonlinear or arithmetic step records a WeightStepSpec so the JSON trace
		// can audit the SCIP objective term slice that belongs to that step.
		std::pair<BitVec, BitVec> build_one_round( ScipModelBuilder& model_builder, int round, const BitVec& mask_A_at_round_start, const BitVec& mask_B_at_round_start )
		{
			const std::string round_prefix = "r" + std::to_string( round ) + "_";

			BitVec mask_B_after_sub_RC1;
			add_fixed_public_constant_subtraction_step(
				model_builder,
				mask_A_at_round_start,
				mask_B_at_round_start,
				mask_B_after_sub_RC1,
				round_prefix + "CONST_SUB_B_RC1",
				"CONST_SUB_B_RC1",
				ROUND_CONSTANTS[ 1 ],
				false,
				round );

			std::size_t objective_begin = model_builder.objective_terms.size();
			BitVec mask_A_after_B_to_A_injection = model_builder.create_bit_vector( round_prefix + "A_after_B_to_A_injection", WORD_SIZE );
			BitVec mask_B_after_B_to_A_injection = model_builder.create_bit_vector( round_prefix + "B_after_B_to_A_injection", WORD_SIZE );
			BitVec beta_B_to_A_addend = model_builder.create_bit_vector( round_prefix + "B_to_A_injection_beta_addend", WORD_SIZE );
			BitVec alpha_B_to_A = create_joint_injection_alpha_with_walsh_constraint(
				model_builder,
				mask_A_after_B_to_A_injection,
				beta_B_to_A_addend,
				round_prefix + "INJECTION_B_TO_A_JOINT",
				InjectionKind::B_TO_A_JOINT );
			model_builder.add_vector_equality_constraints( mask_A_at_round_start, mask_A_after_B_to_A_injection, round_prefix + "B_TO_A_INJECTION_A_relation" );
			model_builder.add_xor_vector_equality_constraints( mask_B_after_sub_RC1, mask_B_after_B_to_A_injection, alpha_B_to_A, round_prefix + "B_TO_A_INJECTION_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "INJECTION_B_TO_A_JOINT", "joint_quadratic_vectorial_boolean_linear_milp_constraints", round_prefix + "INJECTION_B_TO_A_JOINT", "joint_walsh_support_rank_milp", mask_A_at_round_start, mask_B_after_sub_RC1, mask_A_after_B_to_A_injection, mask_B_after_B_to_A_injection, true, true, true, "alpha_mask_on_B", "beta_addend_mask_to_A_add", "beta_xor_mask_to_A_xor", alpha_B_to_A, beta_B_to_A_addend, mask_A_after_B_to_A_injection, false, 0, 0, false, BitVec {}, BitVec {}, round_prefix + "INJECTION_B_TO_A_JOINT", objective_begin, model_builder.objective_terms.size() } );

			objective_begin = model_builder.objective_terms.size();
			BitVec mask_A_after_add_RC0 = model_builder.create_bit_vector( round_prefix + "A_after_add_B_to_A_addend_RC0", WORD_SIZE );
			BitVec mask_B_after_add_RC0 = model_builder.create_bit_vector( round_prefix + "B_after_add_B_to_A_addend_RC0", WORD_SIZE );
			arithmetic_model::add_two_input_modular_addition_linear_characteristic_constraints( model_builder, mask_A_after_add_RC0, mask_A_after_B_to_A_injection, beta_B_to_A_addend, round_prefix + "ADD_A_WITH_B_TO_A_ADDEND_RC0_LINEAR_BOX" );
			model_builder.add_vector_equality_constraints( mask_B_after_B_to_A_injection, mask_B_after_add_RC0, round_prefix + "ADD_A_WITH_B_TO_A_ADDEND_RC0_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "ADD_A_WITH_B_TO_A_ADDEND_RC0", "two_variable_modular_addition_linear", round_prefix + "ADD_A_WITH_B_TO_A_ADDEND_RC0", "wallen_two_input_add_linear_characteristic", mask_A_after_B_to_A_injection, mask_B_after_B_to_A_injection, mask_A_after_add_RC0, mask_B_after_add_RC0, true, true, true, "mask_A_before", "mask_B_to_A_addend", "mask_A_after", mask_A_after_B_to_A_injection, beta_B_to_A_addend, mask_A_after_add_RC0, true, ROUND_CONSTANTS[ 0 ], ROUND_CONSTANTS[ 0 ], false, BitVec {}, BitVec {}, "", objective_begin, model_builder.objective_terms.size() } );

			BitVec mask_A_after_xor_B_rotA22_RC4 = model_builder.create_bit_vector( round_prefix + "A_after_xor_B_rotA22_RC4", WORD_SIZE );
			BitVec mask_B_after_xor_B_rotA22_RC4 = model_builder.create_bit_vector( round_prefix + "B_after_xor_B_rotA22_RC4", WORD_SIZE );
			BitVec rotA22_transposed_mask_into_A = ScipModelBuilder::rotate_right( mask_B_after_xor_B_rotA22_RC4, FIRST_BRIDGE_ROTATE0 );
			model_builder.add_xor_vector_equality_constraints( mask_A_after_add_RC0, mask_A_after_xor_B_rotA22_RC4, rotA22_transposed_mask_into_A, round_prefix + "XOR_B_WITH_ROTL_A_22_A_relation" );
			model_builder.add_vector_equality_constraints( mask_B_after_add_RC0, mask_B_after_xor_B_rotA22_RC4, round_prefix + "XOR_B_WITH_ROTL_A_22_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "XOR_B_WITH_ROTL_A_22_RC4", "linear_xor_rotation_bridge_with_public_constant_B", round_prefix + "XOR_B_WITH_ROTL_A_22_RC4", "zero_weight_linear_transpose", mask_A_after_add_RC0, mask_B_after_add_RC0, mask_A_after_xor_B_rotA22_RC4, mask_B_after_xor_B_rotA22_RC4, false, false, false, "", "", "", BitVec {}, BitVec {}, BitVec {}, true, ROUND_CONSTANTS[ 4 ], ROUND_CONSTANTS[ 4 ], false, BitVec {}, BitVec {}, "", model_builder.objective_terms.size(), model_builder.objective_terms.size() } );

			BitVec mask_A_after_xor_A_rotB13 = model_builder.create_bit_vector( round_prefix + "A_after_xor_A_rotB13", WORD_SIZE );
			BitVec mask_B_after_xor_A_rotB13 = model_builder.create_bit_vector( round_prefix + "B_after_xor_A_rotB13", WORD_SIZE );
			BitVec rotB13_transposed_mask_into_B = ScipModelBuilder::rotate_right( mask_A_after_xor_A_rotB13, FIRST_BRIDGE_ROTATE1 );
			model_builder.add_vector_equality_constraints( mask_A_after_xor_B_rotA22_RC4, mask_A_after_xor_A_rotB13, round_prefix + "XOR_A_WITH_ROTL_B_13_A_relation" );
			model_builder.add_xor_vector_equality_constraints( mask_B_after_xor_B_rotA22_RC4, mask_B_after_xor_A_rotB13, rotB13_transposed_mask_into_B, round_prefix + "XOR_A_WITH_ROTL_B_13_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "XOR_A_WITH_ROTL_B_13", "linear_xor_rotation_bridge", round_prefix + "XOR_A_WITH_ROTL_B_13", "zero_weight_linear_transpose", mask_A_after_xor_B_rotA22_RC4, mask_B_after_xor_B_rotA22_RC4, mask_A_after_xor_A_rotB13, mask_B_after_xor_A_rotB13, false, false, false, "", "", "", BitVec {}, BitVec {}, BitVec {}, false, 0, 0, false, BitVec {}, BitVec {}, "", model_builder.objective_terms.size(), model_builder.objective_terms.size() } );

			BitVec mask_A_after_sub_RC6;
			add_fixed_public_constant_subtraction_step(
				model_builder,
				mask_B_after_xor_A_rotB13,
				mask_A_after_xor_A_rotB13,
				mask_A_after_sub_RC6,
				round_prefix + "CONST_SUB_A_RC6",
				"CONST_SUB_A_RC6",
				ROUND_CONSTANTS[ 6 ],
				true,
				round );

			objective_begin = model_builder.objective_terms.size();
			BitVec mask_A_after_A_to_B_injection = model_builder.create_bit_vector( round_prefix + "A_after_A_to_B_injection", WORD_SIZE );
			BitVec mask_B_after_A_to_B_injection = model_builder.create_bit_vector( round_prefix + "B_after_A_to_B_injection", WORD_SIZE );
			BitVec beta_A_to_B_addend = model_builder.create_bit_vector( round_prefix + "A_to_B_injection_beta_addend", WORD_SIZE );
			BitVec alpha_A_to_B = create_joint_injection_alpha_with_walsh_constraint(
				model_builder,
				mask_B_after_A_to_B_injection,
				beta_A_to_B_addend,
				round_prefix + "INJECTION_A_TO_B_JOINT",
				InjectionKind::A_TO_B_JOINT );
			model_builder.add_xor_vector_equality_constraints( mask_A_after_sub_RC6, mask_A_after_A_to_B_injection, alpha_A_to_B, round_prefix + "A_TO_B_INJECTION_A_relation" );
			model_builder.add_vector_equality_constraints( mask_B_after_xor_A_rotB13, mask_B_after_A_to_B_injection, round_prefix + "A_TO_B_INJECTION_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "INJECTION_A_TO_B_JOINT", "joint_quadratic_vectorial_boolean_linear_milp_constraints", round_prefix + "INJECTION_A_TO_B_JOINT", "joint_walsh_support_rank_milp", mask_A_after_sub_RC6, mask_B_after_xor_A_rotB13, mask_A_after_A_to_B_injection, mask_B_after_A_to_B_injection, true, true, true, "alpha_mask_on_A", "beta_addend_mask_to_B_add", "beta_xor_mask_to_B_xor", alpha_A_to_B, beta_A_to_B_addend, mask_B_after_A_to_B_injection, false, 0, 0, false, BitVec {}, BitVec {}, round_prefix + "INJECTION_A_TO_B_JOINT", objective_begin, model_builder.objective_terms.size() } );

			objective_begin = model_builder.objective_terms.size();
			BitVec mask_A_after_add_RC5 = model_builder.create_bit_vector( round_prefix + "A_after_add_A_to_B_addend_RC5", WORD_SIZE );
			BitVec mask_B_after_add_RC5 = model_builder.create_bit_vector( round_prefix + "B_after_add_A_to_B_addend_RC5", WORD_SIZE );
			arithmetic_model::add_two_input_modular_addition_linear_characteristic_constraints( model_builder, mask_B_after_add_RC5, mask_B_after_A_to_B_injection, beta_A_to_B_addend, round_prefix + "ADD_B_WITH_A_TO_B_ADDEND_RC5_LINEAR_BOX" );
			model_builder.add_vector_equality_constraints( mask_A_after_A_to_B_injection, mask_A_after_add_RC5, round_prefix + "ADD_B_WITH_A_TO_B_ADDEND_RC5_A_relation" );
			add_weight_step( WeightStepSpec { round, 0, "ADD_B_WITH_A_TO_B_ADDEND_RC5", "two_variable_modular_addition_linear", round_prefix + "ADD_B_WITH_A_TO_B_ADDEND_RC5", "wallen_two_input_add_linear_characteristic", mask_A_after_A_to_B_injection, mask_B_after_A_to_B_injection, mask_A_after_add_RC5, mask_B_after_add_RC5, true, true, true, "mask_B_before", "mask_A_to_B_addend", "mask_B_after", mask_B_after_A_to_B_injection, beta_A_to_B_addend, mask_B_after_add_RC5, true, ROUND_CONSTANTS[ 5 ], ROUND_CONSTANTS[ 5 ], false, BitVec {}, BitVec {}, "", objective_begin, model_builder.objective_terms.size() } );

			BitVec mask_A_after_xor_A_rotB5_RC9 = model_builder.create_bit_vector( round_prefix + "A_after_xor_A_rotB5_RC9", WORD_SIZE );
			BitVec mask_B_after_xor_A_rotB5_RC9 = model_builder.create_bit_vector( round_prefix + "B_after_xor_A_rotB5_RC9", WORD_SIZE );
			BitVec rotB5_transposed_mask_into_B = ScipModelBuilder::rotate_right( mask_A_after_xor_A_rotB5_RC9, SECOND_BRIDGE_ROT_B_TO_A );
			model_builder.add_vector_equality_constraints( mask_A_after_add_RC5, mask_A_after_xor_A_rotB5_RC9, round_prefix + "XOR_A_WITH_ROTL_B_5_A_relation" );
			model_builder.add_xor_vector_equality_constraints( mask_B_after_add_RC5, mask_B_after_xor_A_rotB5_RC9, rotB5_transposed_mask_into_B, round_prefix + "XOR_A_WITH_ROTL_B_5_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "XOR_A_WITH_ROTL_B_5_RC9", "linear_xor_rotation_bridge_with_public_constant_A", round_prefix + "XOR_A_WITH_ROTL_B_5_RC9", "zero_weight_linear_transpose", mask_A_after_add_RC5, mask_B_after_add_RC5, mask_A_after_xor_A_rotB5_RC9, mask_B_after_xor_A_rotB5_RC9, false, false, false, "", "", "", BitVec {}, BitVec {}, BitVec {}, true, ROUND_CONSTANTS[ 9 ], ROUND_CONSTANTS[ 9 ], false, BitVec {}, BitVec {}, "", model_builder.objective_terms.size(), model_builder.objective_terms.size() } );

			BitVec mask_A_after_xor_B_rotA25 = model_builder.create_bit_vector( round_prefix + "A_after_xor_B_rotA25", WORD_SIZE );
			BitVec mask_B_after_xor_B_rotA25 = model_builder.create_bit_vector( round_prefix + "B_after_xor_B_rotA25", WORD_SIZE );
			BitVec rotA25_transposed_mask_into_A = ScipModelBuilder::rotate_right( mask_B_after_xor_B_rotA25, SECOND_BRIDGE_ROT_A_TO_B );
			model_builder.add_xor_vector_equality_constraints( mask_A_after_xor_A_rotB5_RC9, mask_A_after_xor_B_rotA25, rotA25_transposed_mask_into_A, round_prefix + "XOR_B_WITH_ROTL_A_25_A_relation" );
			model_builder.add_vector_equality_constraints( mask_B_after_xor_A_rotB5_RC9, mask_B_after_xor_B_rotA25, round_prefix + "XOR_B_WITH_ROTL_A_25_B_relation" );
			add_weight_step( WeightStepSpec { round, 0, "XOR_B_WITH_ROTL_A_25", "linear_xor_rotation_bridge", round_prefix + "XOR_B_WITH_ROTL_A_25", "zero_weight_linear_transpose", mask_A_after_xor_A_rotB5_RC9, mask_B_after_xor_A_rotB5_RC9, mask_A_after_xor_B_rotA25, mask_B_after_xor_B_rotA25, false, false, false, "", "", "", BitVec {}, BitVec {}, BitVec {}, false, 0, 0, false, BitVec {}, BitVec {}, "", model_builder.objective_terms.size(), model_builder.objective_terms.size() } );

			add_weight_step( WeightStepSpec { round, 0, "FINAL_XOR_CONSTANTS_RC10_RC11", "linear_xor_public_constants", round_prefix + "FINAL_CONST_XOR", "zero_weight_sign_phase", mask_A_after_xor_B_rotA25, mask_B_after_xor_B_rotA25, mask_A_after_xor_B_rotA25, mask_B_after_xor_B_rotA25, false, false, false, "", "", "", BitVec {}, BitVec {}, BitVec {}, true, ROUND_CONSTANTS[ 10 ] ^ ROUND_CONSTANTS[ 11 ], 0, false, BitVec {}, BitVec {}, "", model_builder.objective_terms.size(), model_builder.objective_terms.size() } );
			return { mask_A_after_xor_B_rotA25, mask_B_after_xor_B_rotA25 };
		}

		void build( ScipModelBuilder& model_builder, const NoGoodStore& no_goods )
		{
			injections.clear();
			weight_steps.clear();
			input_mask_A_bits = model_builder.create_bit_vector( "mA_in", WORD_SIZE );
			input_mask_B_bits = model_builder.create_bit_vector( "mB_in", WORD_SIZE );
			BitVec mask_A_at_current_round = input_mask_A_bits;
			BitVec mask_B_at_current_round = input_mask_B_bits;
			for ( int round = 0; round < options.rounds; ++round )
			{
				auto masks_after_round = build_one_round( model_builder, round, mask_A_at_current_round, mask_B_at_current_round );
				mask_A_at_current_round = masks_after_round.first;
				mask_B_at_current_round = masks_after_round.second;
			}
			output_mask_A_bits = mask_A_at_current_round;
			output_mask_B_bits = mask_B_at_current_round;

			// Zero external masks are legal when they are explicitly fixed by the
			// user.  These anchors only prevent a
			// free-mask best-trail query from selecting the trivial all-zero
			// boundary; they never forbid internal zero masks, which must arise
			// through the chained propagation/oracle constraints themselves.
			if ( options.require_nonzero_input_mask && !mask_pair_is_fully_fixed( options.fix_input_ma, options.fix_input_mb ) )
				add_nonzero_mask_pair_constraint( model_builder, input_mask_A_bits, input_mask_B_bits, "nonzero_free_input_linear_mask" );
			if ( options.require_nonzero_output_mask && !mask_pair_is_fully_fixed( options.fix_output_ma, options.fix_output_mb ) )
				add_nonzero_mask_pair_constraint( model_builder, output_mask_A_bits, output_mask_B_bits, "nonzero_free_output_linear_mask" );

			if ( options.fix_input_ma )
				fix_bit_vector_to_value( model_builder, input_mask_A_bits, *options.fix_input_ma, "fix_input_ma" );
			if ( options.fix_input_mb )
				fix_bit_vector_to_value( model_builder, input_mask_B_bits, *options.fix_input_mb, "fix_input_mb" );
			if ( options.fix_output_ma )
				fix_bit_vector_to_value( model_builder, output_mask_A_bits, *options.fix_output_ma, "fix_output_ma" );
			if ( options.fix_output_mb )
				fix_bit_vector_to_value( model_builder, output_mask_B_bits, *options.fix_output_mb, "fix_output_mb" );
			if ( std::isfinite( options.objective_window_from ) || std::isfinite( options.objective_window_to ) )
			{
				const double left_hand_side = std::isnan( options.objective_window_from ) ? -INF : options.objective_window_from - 1e-8;
				const double right_hand_side = std::isnan( options.objective_window_to ) ? INF : options.objective_window_to + 1e-8;
				model_builder.add_objective_bound_constraint( "objective_window", left_hand_side, right_hand_side );
			}
			apply_no_good_cuts( model_builder, no_goods );
		}

		void apply_no_good_cuts( ScipModelBuilder& model_builder, const NoGoodStore& no_goods )
		{
			for ( std::size_t cut_index = 0; cut_index < no_goods.no_good_cuts.size(); ++cut_index )
			{
				std::vector<std::pair<SVar, int>> assignment;
				for ( const auto& [ name, value ] : no_goods.no_good_cuts[ cut_index ].assignment )
				{
					auto variable_iterator = model_builder.var_by_name.find( name );
					if ( variable_iterator != model_builder.var_by_name.end() )
						assignment.push_back( { variable_iterator->second, value } );
				}
				if ( !assignment.empty() )
					add_no_good( model_builder, "linear_characteristic_no_good_" + std::to_string( cut_index ), assignment );
			}
		}

		const InjectionInstance* find_injection( const std::string& name ) const
		{
			for ( const auto& injection : injections )
				if ( injection.name == name )
					return &injection;
			return nullptr;
		}

		static void add_no_good( ScipModelBuilder& model_builder, const std::string& name, const std::vector<std::pair<SVar, int>>& assignment )
		{
			std::vector<LinearTerm> terms;
			int						zero_value_count = 0;
			for ( const auto& [ variable, value ] : assignment )
			{
				if ( value )
					terms.push_back( { variable, 1.0 } );
				else
				{
					terms.push_back( { variable, -1.0 } );
					++zero_value_count;
				}
			}
			model_builder.add_linear_constraint( name, terms, -INF, static_cast<double>( assignment.size() - 1 - zero_value_count ) );
		}
	};

	static SolutionSnapshot make_snapshot( ScipModelBuilder& model_builder, const NeoAlzetteScipLinearSearch& search )
	{
		SCIP_SOL* solution = SCIPgetBestSol( model_builder.scip );
		if ( !solution )
			throw std::runtime_error( "SCIP produced no incumbent solution" );
		SolutionSnapshot snapshot;
		snapshot.objective = SCIPgetSolOrigObj( model_builder.scip, solution );
		snapshot.input_mask_A_value = value_from_solution( model_builder.scip, solution, search.input_mask_A_bits );
		snapshot.input_mask_B_value = value_from_solution( model_builder.scip, solution, search.input_mask_B_bits );
		snapshot.output_mask_A_value = value_from_solution( model_builder.scip, solution, search.output_mask_A_bits );
		snapshot.output_mask_B_value = value_from_solution( model_builder.scip, solution, search.output_mask_B_bits );
		return snapshot;
	}

	static void append_unique_bit_vector_assignment( NoGoodCut& cut, std::set<std::string>& seen, SCIP* scip, SCIP_SOL* solution, const BitVec& bits )
	{
		for ( const auto& variable : bits )
		{
			if ( seen.insert( variable.name ).second )
				cut.assignment.push_back( { variable.name, SCIPgetSolVal( scip, solution, variable.var ) > 0.5 ? 1 : 0 } );
		}
	}

	static NoGoodCut capture_semantic_no_good( SCIP* scip, SCIP_SOL* solution, const NeoAlzetteScipLinearSearch& search )
	{
		NoGoodCut		  cut;
		std::set<std::string> seen;
		append_unique_bit_vector_assignment( cut, seen, scip, solution, search.input_mask_A_bits );
		append_unique_bit_vector_assignment( cut, seen, scip, solution, search.input_mask_B_bits );
		append_unique_bit_vector_assignment( cut, seen, scip, solution, search.output_mask_A_bits );
		append_unique_bit_vector_assignment( cut, seen, scip, solution, search.output_mask_B_bits );
		for ( const auto& spec : search.weight_steps )
		{
			if ( spec.has_local_input0 )
				append_unique_bit_vector_assignment( cut, seen, scip, solution, spec.local_input0 );
			if ( spec.has_local_input1 )
				append_unique_bit_vector_assignment( cut, seen, scip, solution, spec.local_input1 );
			if ( spec.has_local_output )
				append_unique_bit_vector_assignment( cut, seen, scip, solution, spec.local_output );
			if ( spec.has_fixed_characteristic )
			{
				append_unique_bit_vector_assignment( cut, seen, scip, solution, spec.fixed_carry );
				append_unique_bit_vector_assignment( cut, seen, scip, solution, spec.fixed_bad );
			}
		}
		for ( const auto& injection : search.injections )
		{
			append_unique_bit_vector_assignment( cut, seen, scip, solution, injection.alpha_mask );
			append_unique_bit_vector_assignment( cut, seen, scip, solution, injection.beta_xor_mask );
			append_unique_bit_vector_assignment( cut, seen, scip, solution, injection.beta_addend_mask );
		}
		return cut;
	}

	static int characteristic_sign_from_trace( const std::vector<WeightTraceEntry>& trace )
	{
		int sign = 1;
		for ( const auto& e : trace )
		{
			if ( e.local_sign == 0 )
				return 0;
			sign *= e.local_sign;
		}
		return sign;
	}

	static bool has_fixed_addend_exact_lap_metadata( const WeightTraceEntry& e )
	{
		return e.has_fixed_characteristic ||
			   e.weight_model == "miyano_fixed_addend_exact_lap_oracle_metadata" ||
			   e.weight_model == "miyano_fixed_addend_exact_log_weight_milp";
	}

	static bool trace_oracles_valid( const std::vector<WeightTraceEntry>& trace )
	{
		for ( const auto& e : trace )
		{
			if ( e.has_injection_oracle && !e.injection_valid )
				return false;
			if ( has_fixed_addend_exact_lap_metadata( e ) && !e.fixed_const_exact_possible )
				return false;
			if ( e.local_sign == 0 )
				return false;
		}
		return true;
	}

	static bool trace_oracle_weights_consistent( const std::vector<WeightTraceEntry>& trace )
	{
		for ( const auto& e : trace )
		{
			if ( e.has_injection_oracle && std::fabs( e.model_oracle_weight - e.injection_oracle_weight ) > 1e-6 )
				return false;
			if ( e.has_fixed_characteristic )
			{
				const double removed_internal_path_weight = static_cast<double>( popcount32( e.fixed_bad_mask ) );
				if ( std::fabs( e.local_weight - removed_internal_path_weight ) > 1e-6 )
					return false;
			}
			if ( e.weight_model == "miyano_fixed_addend_exact_lap_oracle_metadata" )
			{
				if ( !e.fixed_const_exact_possible )
					return false;
				if ( std::fabs( e.local_weight - e.fixed_const_exact_weight ) > 1e-6 )
					return false;
			}
			if ( e.weight_model == "miyano_fixed_addend_exact_log_weight_milp" )
			{
				if ( !e.fixed_const_exact_possible )
					return false;
				if ( std::fabs( e.local_weight - e.fixed_const_exact_weight ) > 1e-6 )
					return false;
			}
		}
		return true;
	}

	static const InjectionInstance* find_injection_instance( const NeoAlzetteScipLinearSearch& search, const std::string& name )
	{
		for ( const auto& inst : search.injections )
			if ( inst.name == name )
				return &inst;
		return nullptr;
	}

	// ------------------------------------------------------------------------
	// Audit section 3: incumbent finalization and trace/oracle audit
	// ------------------------------------------------------------------------
	// These functions read the SCIP incumbent back into semantic checkpoints and
	// compare local model weights against the exact Q1 or injection oracles.
	static std::vector<WeightTraceEntry> collect_weight_trace( ScipModelBuilder& model_builder, const NeoAlzetteScipLinearSearch& search, double& total )
	{
		SCIP_SOL* solution = SCIPgetBestSol( model_builder.scip );
		if ( !solution )
			throw std::runtime_error( "SCIP produced no incumbent solution" );
		LinearInjectionOracle& oracle = global_injection_oracle();
		std::vector<WeightTraceEntry> trace_entries;
		total = 0.0;
		for ( const auto& spec : search.weight_steps )
		{
			WeightTraceEntry trace_entry;
			trace_entry.round = spec.round;
			trace_entry.step = spec.step;
			trace_entry.stage = spec.stage;
			trace_entry.operation = spec.operation;
			trace_entry.prefix = spec.prefix;
			trace_entry.weight_model = spec.weight_model;
			trace_entry.A_before = value_from_solution( model_builder.scip, solution, spec.A_before );
			trace_entry.B_before = value_from_solution( model_builder.scip, solution, spec.B_before );
			trace_entry.A_after = value_from_solution( model_builder.scip, solution, spec.A_after );
			trace_entry.B_after = value_from_solution( model_builder.scip, solution, spec.B_after );
			trace_entry.has_local_input0 = spec.has_local_input0;
			trace_entry.has_local_input1 = spec.has_local_input1;
			trace_entry.has_local_output = spec.has_local_output;
			trace_entry.local_input0_label = spec.local_input0_label;
			trace_entry.local_input1_label = spec.local_input1_label;
			trace_entry.local_output_label = spec.local_output_label;
			if ( spec.has_local_input0 )
				trace_entry.local_input0 = value_from_solution( model_builder.scip, solution, spec.local_input0 );
			if ( spec.has_local_input1 )
				trace_entry.local_input1 = value_from_solution( model_builder.scip, solution, spec.local_input1 );
			if ( spec.has_local_output )
				trace_entry.local_output = value_from_solution( model_builder.scip, solution, spec.local_output );
			trace_entry.has_public_constant = spec.has_public_constant;
			trace_entry.public_constant = spec.public_constant;
			trace_entry.effective_constant = spec.effective_constant;
			for ( std::size_t objective_term_index = spec.objective_begin; objective_term_index < spec.objective_end && objective_term_index < model_builder.objective_terms.size(); ++objective_term_index )
			{
				const auto& term = model_builder.objective_terms[ objective_term_index ];
				const double value = SCIPgetSolVal( model_builder.scip, solution, term.v.var );
				const double contribution = value * term.c;
				trace_entry.local_weight += contribution;
				++trace_entry.objective_term_count;
				if ( std::fabs( value ) > 1e-9 || std::fabs( contribution ) > 1e-9 )
				{
					trace_entry.selected_terms.push_back( { term.v.name, term.c, value, contribution } );
					++trace_entry.selected_term_count;
				}
			}
			if ( spec.operation == "two_variable_modular_addition_linear" )
			{
				const auto addition_oracle_result = linear_oracle::oracle_add2( trace_entry.local_output, trace_entry.local_input0, trace_entry.local_input1, WORD_SIZE );
				trace_entry.local_sign = addition_oracle_result.possible ? addition_oracle_result.sign : 0;
				if ( trace_entry.has_public_constant )
					trace_entry.local_sign *= parity32( trace_entry.local_input1 & trace_entry.public_constant ) ? -1 : 1;
			}
			else if ( spec.operation == "two_variable_modular_subtraction_linear" )
			{
				const auto subtraction_oracle_result = linear_oracle::oracle_sub2( trace_entry.local_output, trace_entry.local_input0, trace_entry.local_input1, WORD_SIZE );
				trace_entry.local_sign = subtraction_oracle_result.possible ? subtraction_oracle_result.sign : 0;
				if ( trace_entry.has_public_constant )
					trace_entry.local_sign *= parity32( trace_entry.local_input1 & trace_entry.public_constant ) ? -1 : 1;
			}
			else if ( spec.has_fixed_characteristic )
			{
				const std::uint64_t carry = value64_from_solution( model_builder.scip, solution, spec.fixed_carry );
				const std::uint32_t bad = value_from_solution( model_builder.scip, solution, spec.fixed_bad );
				trace_entry.has_fixed_characteristic = true;
				trace_entry.fixed_carry_mask = carry;
				trace_entry.fixed_bad_mask = bad;
				trace_entry.local_sign = linear_oracle::fixed_const_characteristic_sign( spec.effective_constant, trace_entry.local_input0, trace_entry.local_output, carry, bad, WORD_SIZE );

				// Exact Miyano fixed-addend LAP for the collapsed component, kept
				// next to the fixed-addend exact threshold state so traces cannot
				// silently confuse the two semantic layers.
				const auto exact_fixed_const_oracle = linear_oracle::oracle_add_const( spec.effective_constant, trace_entry.local_input0, trace_entry.local_output, WORD_SIZE );
				trace_entry.fixed_const_exact_possible = exact_fixed_const_oracle.possible;
				trace_entry.fixed_const_exact_sign = exact_fixed_const_oracle.sign;
				trace_entry.fixed_const_exact_weight = exact_fixed_const_oracle.possible ? static_cast<double>( exact_fixed_const_oracle.weight ) : std::numeric_limits<double>::infinity();
				trace_entry.fixed_const_exact_correlation = exact_fixed_const_oracle.correlation;
				trace_entry.fixed_const_exact_abs_correlation = exact_fixed_const_oracle.abs_correlation;
			}
			else if ( spec.has_fixed_exact_oracle )
			{
				const auto exact_fixed_const_oracle = linear_oracle::oracle_add_const( spec.effective_constant, trace_entry.local_input0, trace_entry.local_output, WORD_SIZE );
				trace_entry.fixed_const_exact_possible = exact_fixed_const_oracle.possible;
				trace_entry.fixed_const_exact_sign = exact_fixed_const_oracle.sign;
				trace_entry.fixed_const_exact_weight = exact_fixed_const_oracle.possible ? static_cast<double>( exact_fixed_const_oracle.weight ) : std::numeric_limits<double>::infinity();
				trace_entry.fixed_const_exact_correlation = exact_fixed_const_oracle.correlation;
				trace_entry.fixed_const_exact_abs_correlation = exact_fixed_const_oracle.abs_correlation;
				trace_entry.local_weight = trace_entry.fixed_const_exact_weight;
				trace_entry.local_sign = exact_fixed_const_oracle.possible ? exact_fixed_const_oracle.sign : 0;
			}
			else if ( !spec.injection_name.empty() )
			{
				const InjectionInstance* injection = find_injection_instance( search, spec.injection_name );
				if ( injection )
				{
					trace_entry.has_injection_oracle = true;
					trace_entry.injection_name = spec.injection_name;
					trace_entry.injection_alpha_mask = value_from_solution( model_builder.scip, solution, injection->alpha_mask );
					trace_entry.injection_beta_xor_mask = value_from_solution( model_builder.scip, solution, injection->beta_xor_mask );
					trace_entry.injection_beta_addend_mask = value_from_solution( model_builder.scip, solution, injection->beta_addend_mask );
					trace_entry.injection_packed_beta_mask = pack_joint_injection_output( trace_entry.injection_beta_xor_mask, trace_entry.injection_beta_addend_mask );
					const auto injection_oracle_result = oracle.transition( injection->kind, trace_entry.injection_alpha_mask, trace_entry.injection_packed_beta_mask );
					trace_entry.injection_valid = injection_oracle_result.valid;
					trace_entry.injection_rank = injection_oracle_result.rank;
					trace_entry.model_oracle_weight = SCIPgetSolVal( model_builder.scip, solution, injection->weight.var );
					trace_entry.injection_oracle_weight = injection_oracle_result.valid ? injection_oracle_result.weight : std::numeric_limits<double>::infinity();
					trace_entry.local_sign = injection_oracle_result.valid ? injection_oracle_result.sign : 0;
				}
			}
			else if ( spec.operation == "linear_xor_public_constants" )
			{
				const int public_constant_phase = parity32( trace_entry.A_after & ROUND_CONSTANTS[ 10 ] ) ^ parity32( trace_entry.B_after & ROUND_CONSTANTS[ 11 ] );
				trace_entry.local_sign = public_constant_phase ? -1 : 1;
			}
			else if ( spec.operation == "linear_xor_public_constant_A" )
			{
				trace_entry.local_sign = parity32( trace_entry.A_after & spec.public_constant ) ? -1 : 1;
			}
			else if ( spec.operation == "linear_xor_public_constant_B" )
			{
				trace_entry.local_sign = parity32( trace_entry.B_after & spec.public_constant ) ? -1 : 1;
			}
			else if ( spec.operation == "linear_xor_rotation_bridge_with_public_constant_A" )
			{
				trace_entry.local_sign = parity32( trace_entry.A_after & spec.public_constant ) ? -1 : 1;
			}
			else if ( spec.operation == "linear_xor_rotation_bridge_with_public_constant_B" )
			{
				trace_entry.local_sign = parity32( trace_entry.B_after & spec.public_constant ) ? -1 : 1;
			}
			else
			{
				trace_entry.local_sign = 1;
			}
			total += trace_entry.local_weight;
			trace_entry.cumulative_weight = total;
			trace_entries.push_back( std::move( trace_entry ) );
		}
		return trace_entries;
	}

	static void attach_weight_trace( ScipSolveResult& result, ScipModelBuilder& model_builder, const NeoAlzetteScipLinearSearch& search )
	{
		if ( !result.feasible )
			return;
		result.weight_trace = collect_weight_trace( model_builder, search, result.weight_trace_total );
		result.exact_evaluated_total_weight = exact_evaluated_total_weight_from_trace( result.weight_trace );
		result.exact_evaluated_total_weight_available = true;
		result.weight_trace_objective_delta = result.weight_trace_total - result.snapshot.objective;
		result.weight_trace_available = true;
		result.weight_trace_matches_objective = std::fabs( result.weight_trace_objective_delta ) <= 1e-6;
		result.characteristic_sign = characteristic_sign_from_trace( result.weight_trace );
		result.weight_trace_oracles_valid = trace_oracles_valid( result.weight_trace );
		result.weight_trace_oracle_weights_consistent = trace_oracle_weights_consistent( result.weight_trace );
		result.paper_usable_characteristic = result.weight_trace_available && result.weight_trace_matches_objective && result.weight_trace_oracles_valid && result.weight_trace_oracle_weights_consistent && result.characteristic_sign != 0;
		result.abs_correlation_contribution = pow2_neg_ld( result.snapshot.objective );
		result.signed_correlation_contribution = static_cast<long double>( result.characteristic_sign ) * result.abs_correlation_contribution;
	}

	static void print_weight_trace( const ScipSolveResult& r )
	{
		if ( r.weight_trace.empty() )
			return;
		std::cout << "\n=== LINEAR CHARACTERISTIC WEIGHT TRACE ===\n";
		for ( const auto& e : r.weight_trace )
		{
			std::cout << "round " << e.round << " step " << e.step << " " << e.stage
					  << " op=" << e.operation
					  << " model=" << e.weight_model
					  << " local_weight=" << std::setprecision( 12 ) << e.local_weight
					  << " cumulative=" << e.cumulative_weight
					  << " local_sign=" << e.local_sign << "\n";
			std::cout << "  masks: A 0x" << hex32( e.A_before ) << " -> 0x" << hex32( e.A_after )
					  << "  B 0x" << hex32( e.B_before ) << " -> 0x" << hex32( e.B_after ) << "\n";
			if ( e.has_public_constant )
				std::cout << "  public_constant=0x" << hex32( e.public_constant ) << " effective_constant=0x" << hex32( e.effective_constant ) << "\n";
			if ( e.has_local_input0 )
				std::cout << "  " << e.local_input0_label << "=0x" << hex32( e.local_input0 ) << "\n";
			if ( e.has_local_input1 )
				std::cout << "  " << e.local_input1_label << "=0x" << hex32( e.local_input1 ) << "\n";
			if ( e.has_local_output )
				std::cout << "  " << e.local_output_label << "=0x" << hex32( e.local_output ) << "\n";
			if ( e.has_fixed_characteristic )
			{
				std::cout << "  fixed_addend_removed_internal_characteristic: carry=0x" << hex64( e.fixed_carry_mask )
						  << " bad_q=0x" << hex32( e.fixed_bad_mask ) << "\n";
			}
			if ( has_fixed_addend_exact_lap_metadata( e ) )
			{
				std::cout << "  fixed_addend_exact_lap_oracle: possible=" << ( e.fixed_const_exact_possible ? "yes" : "NO" )
						  << " sign=" << e.fixed_const_exact_sign
						  << " weight=" << e.fixed_const_exact_weight
						  << " corr=" << static_cast<double>( e.fixed_const_exact_correlation )
						  << " abs_corr=" << static_cast<double>( e.fixed_const_exact_abs_correlation ) << "\n";
			}
			if ( e.has_injection_oracle )
				std::cout << "  joint_milp: " << e.injection_name
						  << " alpha=0x" << hex32( e.injection_alpha_mask )
						  << " beta_xor=0x" << hex32( e.injection_beta_xor_mask )
						  << " beta_addend=0x" << hex32( e.injection_beta_addend_mask )
						  << " beta64=0x" << hex64( e.injection_packed_beta_mask )
						  << " valid=" << ( e.injection_valid ? "yes" : "NO" )
						  << " rank=" << e.injection_rank
						  << " model_weight=" << e.model_oracle_weight
						  << " oracle_weight=" << e.injection_oracle_weight << "\n";
			std::cout << "  selected_objective_terms=" << e.selected_term_count << "/" << e.objective_term_count << "\n";
			for ( const auto& t : e.selected_terms )
				std::cout << "    " << t.variable << " coeff=" << t.coefficient << " value=" << t.value << " contribution=" << t.contribution << "\n";
		}
		const double exact_total = r.exact_evaluated_total_weight_available ? r.exact_evaluated_total_weight : exact_evaluated_total_weight_from_trace( r.weight_trace );
		std::cout << "TRACE_WEIGHT_TOTAL=" << std::setprecision( 12 ) << r.weight_trace_total
				  << " objective_weight=" << r.snapshot.objective
				  << " exact_evaluated_total=" << exact_total
				  << " delta=" << r.weight_trace_objective_delta
				  << " matches_objective=" << ( r.weight_trace_matches_objective ? "true" : "false" )
				  << " oracles_valid=" << ( r.weight_trace_oracles_valid ? "true" : "false" )
				  << " oracle_weights_consistent=" << ( r.weight_trace_oracle_weights_consistent ? "true" : "false" )
				  << " paper_usable=" << ( r.paper_usable_characteristic ? "true" : "false" ) << "\n";
	}

	static void print_checkpoint_trace( const ScipSolveResult& r )
	{
		if ( r.weight_trace.empty() )
			return;
		std::cout << "\nΛ = Linear Mask\n";
		std::cout << "\n=== CHARACTERISTIC CHECKPOINT TRACE ===\n";

		int current_round = std::numeric_limits<int>::min();
		std::uint32_t last_A = 0u;
		std::uint32_t last_B = 0u;
		for ( const auto& e : r.weight_trace )
		{
			if ( e.round != current_round )
			{
				if ( current_round != std::numeric_limits<int>::min() )
				{
					std::cout << "round " << current_round << " / end                          ΛA=0x" << hex32( last_A )
							  << " ΛB=0x" << hex32( last_B ) << "\n";
				}
				current_round = e.round;
				std::cout << "round " << current_round << " / start                        ΛA=0x" << hex32( e.A_before )
						  << " ΛB=0x" << hex32( e.B_before ) << "\n";
			}

			const std::string checkpoint = "after_" + e.stage;
			std::cout << "round " << e.round << " / " << std::left << std::setw( 40 ) << checkpoint << std::right
					  << " ΛA=0x" << hex32( e.A_after )
					  << " ΛB=0x" << hex32( e.B_after ) << "\n";
			last_A = e.A_after;
			last_B = e.B_after;
		}

		if ( current_round != std::numeric_limits<int>::min() )
		{
			std::cout << "round " << current_round << " / end                          ΛA=0x" << hex32( last_A )
					  << " ΛB=0x" << hex32( last_B ) << "\n";
		}
	}

	static void print_solution_summary( const ScipSolveResult& r, const std::string& title )
	{
		std::cout << "\n=== " << title << " ===\n";
		std::cout << "status=" << scip_status_name( r.scip_status )
				  << " complete=" << ( r.complete ? "true" : "false" ) << "\n";
		if ( !r.error_message.empty() )
			std::cout << "message=" << r.error_message << "\n";
		std::cout << "objective_weight=" << std::setprecision( 12 ) << r.snapshot.objective
			  << " abs_correlation≈" << correlation_string_from_weight( r.snapshot.objective ) << "\n";
		std::cout << "mA_in=0x" << hex32( r.snapshot.input_mask_A_value ) << " mB_in=0x" << hex32( r.snapshot.input_mask_B_value )
				  << " mA_out=0x" << hex32( r.snapshot.output_mask_A_value ) << " mB_out=0x" << hex32( r.snapshot.output_mask_B_value ) << "\n";
		if ( r.weight_trace_available )
		{
			const double exact_total = r.exact_evaluated_total_weight_available ? r.exact_evaluated_total_weight : exact_evaluated_total_weight_from_trace( r.weight_trace );
			std::cout << "exact_evaluated_total_weight=" << std::setprecision( 12 ) << exact_total
				  << " exact_minus_objective=" << ( exact_total - r.snapshot.objective ) << "\n";
		}
		std::cout << "paper_usable_characteristic=" << ( r.paper_usable_characteristic ? "true" : "false" )
				  << " oracle_valid=" << ( r.weight_trace_oracles_valid ? "true" : "false" )
				  << " oracle_weight_consistent=" << ( r.weight_trace_oracle_weights_consistent ? "true" : "false" )
				  << " trace_matches_objective=" << ( r.weight_trace_matches_objective ? "true" : "false" ) << "\n";
		if ( r.feasible && !r.paper_usable_characteristic )
			std::cout << "WARNING: current incumbent is saved for debugging/anytime data, but is not a paper-ready verified linear characteristic.\n";
	}

	static void fill_status( ScipSolveResult& result )
	{
		result.hit_time_limit = result.scip_status == SCIP_STATUS_TIMELIMIT;
		result.hit_memory_limit = result.scip_status == SCIP_STATUS_MEMLIMIT;
	}

	static void finalize_incumbent( ScipSolveResult& result, ScipModelBuilder& model_builder, NeoAlzetteScipLinearSearch& search )
	{
		result.snapshot = make_snapshot( model_builder, search );
		result.no_good = capture_semantic_no_good( model_builder.scip, SCIPgetBestSol( model_builder.scip ), search );
		attach_weight_trace( result, model_builder, search );
	}

	static ScipSolveResult solve_linear_model( const SearchOptions& options, NoGoodStore& no_goods, bool emit_output = true )
	{
		ScipModelBuilder builder( options.quiet, options.time_limit_seconds );
		NeoAlzetteScipLinearSearch search( options );
		search.build( builder, no_goods );
		SCIP_CALL_THROW( SCIPsolve( builder.scip ) );
		const SCIP_STATUS status = SCIPgetStatus( builder.scip );
		if ( emit_output )
			std::cout << "[SCIP linear Walsh model] status=" << scip_status_name( status ) << " primal=" << SCIPgetPrimalbound( builder.scip ) << "\n";

		ScipSolveResult result;
		result.feasible = SCIPgetBestSol( builder.scip ) != nullptr;
		result.scip_status = status;
		result.complete = status == SCIP_STATUS_OPTIMAL || status == SCIP_STATUS_INFEASIBLE;
		fill_status( result );
		if ( result.feasible )
		{
			finalize_incumbent( result, builder, search );
			if ( status != SCIP_STATUS_OPTIMAL )
				result.error_message = "SCIP did not prove optimality; reporting current incumbent from internal linear injection Walsh model";
			if ( emit_output )
			{
				print_solution_summary( result, status == SCIP_STATUS_OPTIMAL ? "SCIP BEST LINEAR SINGLE TRAIL / WALSH MODEL" : "SCIP CURRENT LINEAR INCUMBENT / WALSH MODEL" );
				print_checkpoint_trace( result );
				print_weight_trace( result );
				write_best_result_json_file( options.output_result_json, options, result );
				write_weight_trace_json_file( options.output_weight_trace_json, options, result );
			}
		}
		return result;
	}


	// Exact endpoint enumeration session.  Between solves only one semantic
	// no-good inequality is added, so SCIP's reoptimization tree can be reused
	// without changing the Walsh model, endpoint, objective, or weight window.
	class LinearHullReoptimizationSession
	{
	public:
		LinearHullReoptimizationSession( const SearchOptions& options, const NoGoodStore& initial_no_goods )
			: builder_( options.quiet, std::numeric_limits<double>::quiet_NaN() ), search_( options ), next_no_good_index_( initial_no_goods.no_good_cuts.size() )
		{
			SCIP_CALL_THROW( SCIPenableReoptimization( builder_.scip, TRUE ) );
			search_.build( builder_, initial_no_goods );
		}

		ScipSolveResult solve_next( double remaining_time_seconds, bool emit_output = false )
		{
			if ( std::isfinite( remaining_time_seconds ) && remaining_time_seconds > 0.0 )
				SCIP_CALL_THROW( SCIPsetRealParam( builder_.scip, "limits/time", remaining_time_seconds ) );
			SCIP_CALL_THROW( SCIPsolve( builder_.scip ) );
			const SCIP_STATUS status = SCIPgetStatus( builder_.scip );

			ScipSolveResult result;
			result.feasible = SCIPgetBestSol( builder_.scip ) != nullptr;
			result.scip_status = status;
			result.complete = status == SCIP_STATUS_OPTIMAL || status == SCIP_STATUS_INFEASIBLE;
			fill_status( result );
			if ( result.feasible )
			{
				finalize_incumbent( result, builder_, search_ );
				if ( status != SCIP_STATUS_OPTIMAL )
					result.error_message = "SCIP reoptimization did not prove optimality; reporting current incumbent from internal linear Walsh model";
				if ( emit_output )
				{
					print_solution_summary( result, status == SCIP_STATUS_OPTIMAL ? "SCIP REOPTIMIZED LINEAR ENDPOINT TRAIL" : "SCIP CURRENT LINEAR REOPTIMIZATION INCUMBENT" );
					print_checkpoint_trace( result );
					print_weight_trace( result );
				}
			}
			return result;
		}

		void exclude_characteristic( const NoGoodCut& no_good_cut )
		{
			SCIP_CALL_THROW( SCIPfreeReoptSolve( builder_.scip ) );
			std::vector<std::pair<SVar, int>> assignment;
			assignment.reserve( no_good_cut.assignment.size() );
			for ( const auto& [ name, value ] : no_good_cut.assignment )
			{
				auto variable_iterator = builder_.var_by_name.find( name );
				if ( variable_iterator == builder_.var_by_name.end() )
					throw std::runtime_error( "SCIP variable not found for linear reoptimization no-good: " + name );
				assignment.push_back( { variable_iterator->second, value } );
			}
			if ( assignment.empty() )
				throw std::runtime_error( "cannot add an empty linear semantic no-good cut" );
			NeoAlzetteScipLinearSearch::add_no_good( builder_, "linear_characteristic_no_good_reopt_" + std::to_string( next_no_good_index_++ ), assignment );
		}

	private:
		ScipModelBuilder builder_;
		NeoAlzetteScipLinearSearch search_;
		std::size_t next_no_good_index_ = 0;
	};


	static double exact_evaluated_total_weight_from_trace( const std::vector<WeightTraceEntry>& trace )
	{
		double total = 0.0;
		for ( const auto& e : trace )
		{
			if ( e.weight_model == "miyano_fixed_addend_exact_log_weight_milp" )
			{
				if ( !e.fixed_const_exact_possible || !std::isfinite( e.fixed_const_exact_weight ) )
					return std::numeric_limits<double>::infinity();
				total += e.fixed_const_exact_weight;
			}
			else
			{
				total += e.local_weight;
			}
		}
		return total;
	}

	static void write_weight_trace_terms_json( std::ostream& output_stream, const std::vector<WeightTraceTerm>& terms, const std::string& indent )
	{
		output_stream << "[";
		if ( !terms.empty() )
			output_stream << "\n";
		for ( std::size_t i = 0; i < terms.size(); ++i )
		{
			const auto& term = terms[ i ];
			output_stream << indent << "  {"
						  << "\"variable\": " << json_string( term.variable )
						  << ", \"coefficient\": " << json_number( term.coefficient )
						  << ", \"value\": " << json_number( term.value )
						  << ", \"contribution\": " << json_number( term.contribution )
						  << "}";
			if ( i + 1 != terms.size() )
				output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static void write_weight_trace_json_array( std::ostream& output_stream, const std::vector<WeightTraceEntry>& trace, const std::string& indent )
	{
		output_stream << "[";
		if ( !trace.empty() )
			output_stream << "\n";
		for ( std::size_t trace_index = 0; trace_index < trace.size(); ++trace_index )
		{
			const auto& e = trace[ trace_index ];
			const bool has_fixed_exact_metadata = has_fixed_addend_exact_lap_metadata( e );
			output_stream << indent << "  {\n"
						  << indent << "    \"round\": " << e.round << ",\n"
						  << indent << "    \"step\": " << e.step << ",\n"
						  << indent << "    \"stage\": " << json_string( e.stage ) << ",\n"
						  << indent << "    \"operation\": " << json_string( e.operation ) << ",\n"
						  << indent << "    \"prefix\": " << json_string( e.prefix ) << ",\n"
						  << indent << "    \"weight_model\": " << json_string( e.weight_model ) << ",\n"
						  << indent << "    \"state_before\": {\"A\": " << json_string( hex32_json( e.A_before ) ) << ", \"B\": " << json_string( hex32_json( e.B_before ) ) << "},\n"
						  << indent << "    \"state_after\": {\"A\": " << json_string( hex32_json( e.A_after ) ) << ", \"B\": " << json_string( hex32_json( e.B_after ) ) << "},\n"
						  << indent << "    \"public_constant\": " << ( e.has_public_constant ? json_string( hex32_json( e.public_constant ) ) : "null" ) << ",\n"
						  << indent << "    \"effective_constant\": " << ( e.has_public_constant ? json_string( hex32_json( e.effective_constant ) ) : "null" ) << ",\n"
						  << indent << "    \"local_input0\": " << ( e.has_local_input0 ? json_string( hex32_json( e.local_input0 ) ) : "null" ) << ",\n"
						  << indent << "    \"local_input0_label\": " << ( e.has_local_input0 ? json_string( e.local_input0_label ) : "null" ) << ",\n"
						  << indent << "    \"local_input1\": " << ( e.has_local_input1 ? json_string( hex32_json( e.local_input1 ) ) : "null" ) << ",\n"
						  << indent << "    \"local_input1_label\": " << ( e.has_local_input1 ? json_string( e.local_input1_label ) : "null" ) << ",\n"
						  << indent << "    \"local_output\": " << ( e.has_local_output ? json_string( hex32_json( e.local_output ) ) : "null" ) << ",\n"
						  << indent << "    \"local_output_label\": " << ( e.has_local_output ? json_string( e.local_output_label ) : "null" ) << ",\n"
						  << indent << "    \"joint_injection\": ";
			if ( e.has_injection_oracle )
			{
				output_stream << "{"
							  << "\"name\": " << json_string( e.injection_name )
							  << ", \"valid\": " << ( e.injection_valid ? "true" : "false" )
							  << ", \"rank\": " << e.injection_rank
							  << ", \"model_weight\": " << json_number( e.model_oracle_weight )
							  << ", \"oracle_weight\": " << json_number( e.injection_oracle_weight )
							  << ", \"alpha\": " << json_string( hex32_json( e.injection_alpha_mask ) )
							  << ", \"beta_xor\": " << json_string( hex32_json( e.injection_beta_xor_mask ) )
							  << ", \"beta_addend\": " << json_string( hex32_json( e.injection_beta_addend_mask ) )
							  << ", \"packed_beta\": " << json_string( hex64_json( e.injection_packed_beta_mask ) )
							  << "}";
			}
			else
			{
				output_stream << "null";
			}
			output_stream << ",\n"
						  << indent << "    \"has_fixed_characteristic\": " << ( e.has_fixed_characteristic ? "true" : "false" ) << ",\n"
						  << indent << "    \"fixed_carry_mask\": " << ( e.has_fixed_characteristic ? json_string( hex64_json( e.fixed_carry_mask ) ) : "null" ) << ",\n"
						  << indent << "    \"fixed_bad_mask\": " << ( e.has_fixed_characteristic ? json_string( hex32_json( e.fixed_bad_mask ) ) : "null" ) << ",\n"
						  << indent << "    \"fixed_const_exact_possible\": " << ( has_fixed_exact_metadata ? ( e.fixed_const_exact_possible ? "true" : "false" ) : "null" ) << ",\n"
						  << indent << "    \"fixed_const_exact_sign\": " << ( has_fixed_exact_metadata ? std::to_string( e.fixed_const_exact_sign ) : "null" ) << ",\n"
						  << indent << "    \"fixed_const_exact_weight\": " << ( has_fixed_exact_metadata ? json_number( e.fixed_const_exact_weight ) : "null" ) << ",\n"
						  << indent << "    \"fixed_const_exact_correlation\": " << ( has_fixed_exact_metadata ? json_number( static_cast<double>( e.fixed_const_exact_correlation ) ) : "null" ) << ",\n"
						  << indent << "    \"fixed_const_exact_abs_correlation\": " << ( has_fixed_exact_metadata ? json_number( static_cast<double>( e.fixed_const_exact_abs_correlation ) ) : "null" ) << ",\n"
						  << indent << "    \"local_sign\": " << e.local_sign << ",\n"
						  << indent << "    \"local_weight\": " << json_number( e.local_weight ) << ",\n"
						  << indent << "    \"cumulative_weight\": " << json_number( e.cumulative_weight ) << ",\n"
						  << indent << "    \"objective_term_count\": " << e.objective_term_count << ",\n"
						  << indent << "    \"selected_term_count\": " << e.selected_term_count << ",\n"
						  << indent << "    \"selected_objective_terms\": ";
			write_weight_trace_terms_json( output_stream, e.selected_terms, indent + "    " );
			output_stream << "\n" << indent << "  }";
			if ( trace_index + 1 != trace.size() )
				output_stream << ",";
			output_stream << "\n";
		}
		output_stream << "]";
	}

	static void write_best_result_json_object( std::ostream& output_stream, const SearchOptions& options, const ScipSolveResult& r, const std::string& indent, bool include_weight_trace = true )
	{
		const bool has_weight_trace = r.feasible && r.weight_trace_available;
		output_stream << indent << "{\n"
					  << indent << "  \"analysis\": \"linear_single_characteristic\",\n"
					  << indent << "  \"cipher\": \"NeoAlzette\",\n"
					  << indent << "  \"backend\": \"SCIP_C_API\",\n"
					  << indent << "  \"rounds\": " << options.rounds << ",\n"
					  << indent << "  \"constant_model\": " << json_string( constant_model_name( options.constant_model ) ) << ",\n"
					  << indent << "  \"injection_model\": \"joint-linear-walsh-milp-constraints\",\n"
					  << indent << "  \"solver_status\": " << json_string( scip_status_name( r.scip_status ) ) << ",\n"
					  << indent << "  \"complete\": " << ( r.complete ? "true" : "false" ) << ",\n"
					  << indent << "  \"feasible\": " << ( r.feasible ? "true" : "false" ) << ",\n"
					  << indent << "  \"hit_time_limit\": " << ( r.hit_time_limit ? "true" : "false" ) << ",\n"
					  << indent << "  \"hit_memory_limit\": " << ( r.hit_memory_limit ? "true" : "false" ) << ",\n"
					  << indent << "  \"error_message\": " << ( r.error_message.empty() ? "null" : json_string( r.error_message ) ) << ",\n"
					  << indent << "  \"objective_weight\": " << ( r.feasible ? json_number( r.snapshot.objective ) : "null" ) << ",\n"
					  << indent << "  \"abs_correlation\": " << ( r.feasible ? json_string( correlation_string_from_weight( r.snapshot.objective ) ) : "null" ) << ",\n"
					  << indent << "  \"characteristic_sign\": " << ( r.feasible ? std::to_string( r.characteristic_sign ) : "null" ) << ",\n"
					  << indent << "  \"signed_correlation_contribution\": " << ( r.feasible ? json_number( static_cast<double>( r.signed_correlation_contribution ) ) : "null" ) << ",\n"
					  << indent << "  \"masks\": {\n"
					  << indent << "    \"mA_in\": " << ( r.feasible ? json_string( hex32_json( r.snapshot.input_mask_A_value ) ) : "null" ) << ",\n"
					  << indent << "    \"mB_in\": " << ( r.feasible ? json_string( hex32_json( r.snapshot.input_mask_B_value ) ) : "null" ) << ",\n"
					  << indent << "    \"mA_out\": " << ( r.feasible ? json_string( hex32_json( r.snapshot.output_mask_A_value ) ) : "null" ) << ",\n"
					  << indent << "    \"mB_out\": " << ( r.feasible ? json_string( hex32_json( r.snapshot.output_mask_B_value ) ) : "null" ) << "\n"
					  << indent << "  },\n"
					  << indent << "  \"weight_trace_available\": " << ( has_weight_trace ? "true" : "false" ) << ",\n"
					  << indent << "  \"weight_trace_total\": " << ( has_weight_trace ? json_number( r.weight_trace_total ) : "null" ) << ",\n"
					  << indent << "  \"exact_evaluated_total_weight\": " << ( r.exact_evaluated_total_weight_available ? json_number( r.exact_evaluated_total_weight ) : "null" ) << ",\n"
					  << indent << "  \"weight_trace_objective_delta\": " << ( has_weight_trace ? json_number( r.weight_trace_objective_delta ) : "null" ) << ",\n"
					  << indent << "  \"weight_trace_matches_objective\": " << ( has_weight_trace && r.weight_trace_matches_objective ? "true" : "false" ) << ",\n"
					  << indent << "  \"weight_trace_oracles_valid\": " << ( has_weight_trace && r.weight_trace_oracles_valid ? "true" : "false" ) << ",\n"
					  << indent << "  \"weight_trace_oracle_weights_consistent\": " << ( has_weight_trace && r.weight_trace_oracle_weights_consistent ? "true" : "false" ) << ",\n"
					  << indent << "  \"paper_usable_characteristic\": " << ( r.paper_usable_characteristic ? "true" : "false" ) << ",\n";
		if ( include_weight_trace )
		{
			output_stream << indent << "  \"weight_trace\": ";
			write_weight_trace_json_array( output_stream, r.weight_trace, indent + "  " );
			output_stream << "\n";
		}
		else
		{
			output_stream << indent << "  \"weight_trace\": []\n";
		}
		output_stream << indent << "}";
	}

	static void write_best_result_json_file( const std::string& path, const SearchOptions& options, const ScipSolveResult& result )
	{
		if ( path.empty() )
			return;
		ensure_parent_directory( path );
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open JSON output file: " + path );
		write_best_result_json_object( out, options, result, "" );
		out << "\n";
		std::cout << "RESULT_JSON_FILE=" << path << "\n";
	}

	static void write_weight_trace_json_file( const std::string& path, const SearchOptions& options, const ScipSolveResult& result )
	{
		if ( path.empty() )
			return;
		ensure_parent_directory( path );
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open weight-trace JSON output file: " + path );
		out << "{\n"
			<< "  \"analysis\": \"linear_single_characteristic_weight_trace\",\n"
			<< "  \"cipher\": \"NeoAlzette\",\n"
			<< "  \"backend\": \"SCIP_C_API\",\n"
			<< "  \"rounds\": " << options.rounds << ",\n"
			<< "  \"constant_model\": " << json_string( constant_model_name( options.constant_model ) ) << ",\n"
			<< "  \"injection_model\": \"joint-linear-walsh-milp-constraints\",\n"
			<< "  \"solver_status\": " << json_string( scip_status_name( result.scip_status ) ) << ",\n"
			<< "  \"complete\": " << ( result.complete ? "true" : "false" ) << ",\n"
			<< "  \"feasible\": " << ( result.feasible ? "true" : "false" ) << ",\n"
			<< "  \"objective_weight\": " << ( result.feasible ? json_number( result.snapshot.objective ) : "null" ) << ",\n"
			<< "  \"weight_trace_available\": " << ( result.feasible && result.weight_trace_available ? "true" : "false" ) << ",\n"
			<< "  \"weight_trace_total\": " << ( result.feasible && result.weight_trace_available ? json_number( result.weight_trace_total ) : "null" ) << ",\n"
			<< "  \"exact_evaluated_total_weight\": " << ( result.exact_evaluated_total_weight_available ? json_number( result.exact_evaluated_total_weight ) : "null" ) << ",\n"
			<< "  \"weight_trace_objective_delta\": " << ( result.feasible && result.weight_trace_available ? json_number( result.weight_trace_objective_delta ) : "null" ) << ",\n"
			<< "  \"weight_trace_matches_objective\": " << ( result.feasible && result.weight_trace_available && result.weight_trace_matches_objective ? "true" : "false" ) << ",\n"
			<< "  \"weight_trace\": ";
		write_weight_trace_json_array( out, result.weight_trace, "  " );
		out << "\n}\n";
		std::cout << "WEIGHT_TRACE_JSON_FILE=" << path << "\n";
	}

	static void write_round_table_json_file( const std::string& path, const SearchOptions& base_options, const std::vector<ScipSolveResult>& rows )
	{
		if ( path.empty() )
			return;
		ensure_parent_directory( path );
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open round-table JSON output file: " + path );
		out << "{\n"
			<< "  \"analysis\": \"linear_round_table\",\n"
			<< "  \"cipher\": \"NeoAlzette\",\n"
			<< "  \"backend\": \"SCIP_C_API\",\n"
			<< "  \"constant_model\": " << json_string( constant_model_name( base_options.constant_model ) ) << ",\n"
			<< "  \"table_kind\": \"prefix_best_single_characteristics\",\n"
			<< "  \"rows\": [";
		if ( !rows.empty() )
			out << "\n";
		for ( std::size_t i = 0; i < rows.size(); ++i )
		{
			SearchOptions row_options = base_options;
			row_options.rounds = static_cast<int>( i + 1 );
			out << "    {\n"
				<< "      \"rounds\": " << row_options.rounds << ",\n"
				<< "      \"best_trail\": ";
			write_best_result_json_object( out, row_options, rows[ i ], "      ", true );
			out << "\n"
				<< "    }";
			if ( i + 1 != rows.size() )
				out << ",";
			out << "\n";
		}
		out << "  ]\n}\n";
		std::cout << "ROUND_TABLE_JSON_FILE=" << path << "\n";
	}

	static void run_continuous_best_trail( SearchOptions options )
	{
		const auto deadline = make_sweep_deadline( options );
		if ( !deadline )
			throw std::runtime_error( "--continuous-best-trail requires a finite --time-limit" );
		NoGoodStore no_goods;
		int trail_count = 0;
		bool stopped_by_time = false;

		while ( true )
		{
			if ( sweep_deadline_expired( deadline ) )
			{
				stopped_by_time = true;
				break;
			}
			SearchOptions trial_options = options;
			trial_options.continuous_best_trail = false;
			if ( deadline )
			{
				trial_options.time_limit_seconds = sweep_remaining_seconds( deadline );
				if ( trial_options.time_limit_seconds <= 0.0 )
				{
					stopped_by_time = true;
					break;
				}
			}

			ScipSolveResult result = solve_linear_model( trial_options, no_goods, true );
			if ( !result.feasible )
			{
				std::cout << "CONTINUOUS_BEST_TRAIL_STOP status=" << scip_status_name( result.scip_status )
						  << " feasible=false trails=" << trail_count << "\n";
				break;
			}
			++trail_count;
			no_goods.no_good_cuts.push_back( result.no_good );
			std::cout << "CONTINUOUS_BEST_TRAIL_TRAIL " << trail_count
					  << " objective_weight=" << std::setprecision( 12 ) << result.snapshot.objective
					  << " exact_evaluated_total_weight=" << result.exact_evaluated_total_weight << "\n";
			if ( result.hit_time_limit || result.hit_memory_limit )
			{
				stopped_by_time = result.hit_time_limit || stopped_by_time;
				break;
			}
		}

		std::cout << "CONTINUOUS_BEST_TRAIL_SUMMARY trails=" << trail_count
				  << " stopped_by_time_limit=" << ( stopped_by_time ? "true" : "false" ) << "\n";
	}

	static std::uint32_t parse_u32_hex_or_dec( const std::string& text )
	{
		std::size_t idx = 0;
		const unsigned long long value = std::stoull( text, &idx, 0 );
		if ( idx != text.size() || value > 0xFFFFFFFFull )
			throw std::runtime_error( "invalid uint32 value: " + text );
		return static_cast<std::uint32_t>( value );
	}

	static std::uint64_t parse_u64_hex_or_dec( const std::string& text )
	{
		std::size_t idx = 0;
		const unsigned long long value = std::stoull( text, &idx, 0 );
		if ( idx != text.size() )
			throw std::runtime_error( "invalid uint64 value: " + text );
		return static_cast<std::uint64_t>( value );
	}

	static std::string get_arg( int argc, char** argv, const std::string& name, const std::string& def )
	{
		for ( int i = 1; i + 1 < argc; ++i )
			if ( argv[ i ] == name )
				return argv[ i + 1 ];
		return def;
	}

	static bool has_arg( int argc, char** argv, const std::string& name )
	{
		for ( int i = 1; i < argc; ++i )
			if ( argv[ i ] == name )
				return true;
		return false;
	}

	static void reject_unknown_options( int argc, char** argv )
	{
		static const std::set<std::string> value_options {
			"--rounds",
			"--constant-model",
			"--fix-input-ma",
			"--fix-input-mb",
			"--fix-output-ma",
			"--fix-output-mb",
			"--fix-input-da",
			"--fix-input-db",
			"--fix-output-da",
			"--fix-output-db",
			"--time-limit",
			"--output-result-json",
			"--output-weight-trace-json",
			"--output-round-table-json"
		};
		static const std::set<std::string> flag_options {
			"--quiet",
			"--continuous-best-trail",
			"--help"
		};
		for ( int i = 1; i < argc; ++i )
		{
			const std::string arg = argv[ i ];
			if ( arg.rfind( "--", 0 ) != 0 )
				continue;
			if ( value_options.find( arg ) != value_options.end() )
			{
				if ( i + 1 >= argc )
					throw std::runtime_error( "missing value for option: " + arg );
				++i;
				continue;
			}
			if ( flag_options.find( arg ) != flag_options.end() )
				continue;
			throw std::runtime_error( "unknown option: " + arg );
		}
	}

	static std::optional<std::uint32_t> get_optional_u32_arg( int argc, char** argv, const std::string& name )
	{
		for ( int i = 1; i + 1 < argc; ++i )
			if ( argv[ i ] == name )
				return parse_u32_hex_or_dec( argv[ i + 1 ] );
		return std::nullopt;
	}

	static std::optional<std::uint32_t> get_optional_u32_arg_any( int argc, char** argv, const std::string& primary, const std::string& alias )
	{
		auto v = get_optional_u32_arg( argc, argv, primary );
		if ( v )
			return v;
		return get_optional_u32_arg( argc, argv, alias );
	}

	// ------------------------------------------------------------------------
	// Audit section 4: CLI parsing and run modes
	// ------------------------------------------------------------------------
	static FixedConstantModel parse_fixed_constant_model( const std::string& text )
	{
		if ( text == "fixed-addend-exact-log-weight-milp" ||
			 text == "miyano-fixed-addend-exact-log-weight-milp" ||
			 text == "fixed-addend-exact-log-weight-milp" ||
			 text == "miyano-fixed-addend-exact-log-weight-milp" )
			return FixedConstantModel::FIXED_ADDEND_EXACT_STATIC_THRESHOLD_MILP;
		throw std::runtime_error( "unknown --constant-model: " + text + " (use fixed-addend-exact-log-weight-milp)" );
	}


	static SearchOptions parse_options( int argc, char** argv )
	{
		reject_unknown_options( argc, argv );
		SearchOptions opt;
		opt.rounds = std::stoi( get_arg( argc, argv, "--rounds", "1" ) );
		opt.quiet = has_arg( argc, argv, "--quiet" );
		opt.continuous_best_trail = has_arg( argc, argv, "--continuous-best-trail" );
		opt.require_nonzero_input_mask = true;
		opt.require_nonzero_output_mask = true;
		if ( has_arg( argc, argv, "--time-limit" ) )
			opt.time_limit_seconds = std::stod( get_arg( argc, argv, "--time-limit", "0" ) );
		opt.output_result_json = get_arg( argc, argv, "--output-result-json", opt.output_result_json );
		opt.output_weight_trace_json = get_arg( argc, argv, "--output-weight-trace-json", opt.output_weight_trace_json );
		opt.output_round_table_json = get_arg( argc, argv, "--output-round-table-json", "" );
		if ( has_arg( argc, argv, "--constant-model" ) )
			opt.constant_model = parse_fixed_constant_model( get_arg( argc, argv, "--constant-model", "fixed-addend-exact-log-weight-milp" ) );
		opt.fix_input_ma = get_optional_u32_arg_any( argc, argv, "--fix-input-ma", "--fix-input-da" ).value_or( 0x00000001u );
		opt.fix_input_mb = get_optional_u32_arg_any( argc, argv, "--fix-input-mb", "--fix-input-db" ).value_or( 0x00000001u );
		opt.fix_output_ma = get_optional_u32_arg_any( argc, argv, "--fix-output-ma", "--fix-output-da" );
		opt.fix_output_mb = get_optional_u32_arg_any( argc, argv, "--fix-output-mb", "--fix-output-db" );
		if ( opt.rounds < 1 )
			throw std::runtime_error( "--rounds must be >= 1" );
		return opt;
	}

	static void print_help( const char* argv0 )
	{
		std::cout << "Usage: " << argv0 << " [options]\n\n"
				  << "Single-solve / best-trail mode:\n"
				  << "  --rounds R                       number of NeoAlzette rounds, default 1\n"
				  << "  --fix-input-ma X                 optional input A linear mask source, default 0x00000001\n"
				  << "  --fix-input-mb X                 optional input B linear mask source, default 0x00000001\n"
				  << "  --fix-output-ma X                fix output A linear mask\n"
				  << "  --fix-output-mb X                fix output B linear mask\n"
				  << "  --time-limit S                   SCIP time limit for one solve\n"
				  << "  --constant-model fixed-addend-exact-log-weight-milp  strict fixed-addend model: exact numerator + A=|N| + log-weight epigraph cuts\n"
				  << "  --continuous-best-trail                keep solving distinct best trails until --time-limit expires\n"
				  << "\nOutput / misc:\n"
				  << "  --output-result-json FILE              write result JSON, default linear_scip_best_result.json\n"
				  << "  --output-weight-trace-json FILE        write per-step weight trace JSON, default linear_scip_weight_trace.json\n"
				  << "  --output-round-table-json FILE         run and write best-trail table for rounds 1..R\n"
				  << "  --quiet                                reduce SCIP display output\n"
				  << "  --help                                 print this help\n"
				  << "\nNote: best-trail uses the strict fixed-addend exact-log-weight MILP/CIP model directly.\n";
	}

	static void run_round_table( SearchOptions options )
	{
		const int max_rounds = options.rounds;
		std::vector<ScipSolveResult> rows;
		rows.reserve( max_rounds );
		for ( int r = 1; r <= max_rounds; ++r )
		{
			SearchOptions row = options;
			row.rounds = r;
			std::cout << "\n=== LINEAR ROUND TABLE SEARCH rounds=" << r << " ===\n";
			NoGoodStore no_goods;
			ScipSolveResult result = solve_linear_model( row, no_goods );
			rows.push_back( result );
			write_best_result_json_file( row.output_result_json, row, result );
			write_weight_trace_json_file( row.output_weight_trace_json, row, result );
		}
		write_round_table_json_file( options.output_round_table_json, options, rows );
	}

}  // namespace neoalzette_linear_milp
