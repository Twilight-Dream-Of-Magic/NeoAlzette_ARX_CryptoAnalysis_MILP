#pragma once

#include <scip/scip.h>
#include <scip/cons_xor.h>
#include <scip/pub_tree.h>
#include <scip/scipdefplugins.h>
#include <scip/scip_tree.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "neoalzette_scip_operator_analysis_oracle.hpp"

// ============================================================================
// NeoAlzette LINEAR MILP construction layer
// ============================================================================
//
// This file is the MILP/CIP construction drawing for the linear backend.
// It contains:
//   * Wallen/Fu-Wang-Guo two-variable modular-addition linear constraints;
//   * Miyano fixed-addend exact LAP transfer constraints with exact log-weight MILP epigraph;
//   * SCIP custom constraint handler for joint injection Walsh support/rank;
//   * the SCIP model builder used by the round-search layer.
//
// Boundary rule:
//   * no XOR-differential derivative witness variable x is created here;
//   * injection constraints are Walsh-support/rank constraints on alpha/beta;
//   * the 64-bit beta is a single joint output mask: low 32 bits mask the XOR
//     side, high 32 bits mask the injected addend used by the next modular add;
//   * fixed-addend add/sub is encoded by the 2-state signed transfer matrix
//     strict log-weight MILP for Y = X + K mod 2^n.
// ============================================================================

namespace neoalzette_linear_milp::arithmetic_model
{
	// ========================================================================
	// Audit section A: two-variable modular-addition linear MILP box
	// ========================================================================
	// Theory: Wallen correlation condition, encoded with Fu-Wang-Guo's compact
	// transition inequalities. This section only builds local add/sub boxes.
	struct TwoInputLinearBoxHandle
	{
		BitVec weight_bits;
	};

	template <class Builder>
	inline void add_fu_wang_guo_linear_addition_transition_constraints(
		Builder& model_builder,
		const SVar& next_state,
		const SVar& output_mask_bit,
		const SVar& first_input_mask_bit,
		const SVar& second_input_mask_bit,
		const SVar& current_state,
		const std::string& name )
	{
		// Fu-Wang-Guo FSE 2016, linear modular-addition transition:
		// (s_{i+1}, Gamma_i, A_i, B_i, s_i).  These eight inequalities are the
		// convex-hull form for the ten possible nonzero-correlation transitions.
		model_builder.add_linear_constraint( name + "_ineq_0", { { next_state, 1 }, { output_mask_bit, -1 }, { first_input_mask_bit, -1 }, { second_input_mask_bit, 1 }, { current_state, 1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_1", { { next_state, 1 }, { output_mask_bit, 1 }, { first_input_mask_bit, -1 }, { second_input_mask_bit, -1 }, { current_state, 1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_2", { { next_state, 1 }, { output_mask_bit, 1 }, { first_input_mask_bit, -1 }, { second_input_mask_bit, 1 }, { current_state, -1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_3", { { next_state, -1 }, { output_mask_bit, 1 }, { first_input_mask_bit, 1 }, { second_input_mask_bit, 1 }, { current_state, 1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_4", { { next_state, 1 }, { output_mask_bit, 1 }, { first_input_mask_bit, 1 }, { second_input_mask_bit, -1 }, { current_state, -1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_5", { { next_state, 1 }, { output_mask_bit, -1 }, { first_input_mask_bit, 1 }, { second_input_mask_bit, -1 }, { current_state, 1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_6", { { next_state, 1 }, { output_mask_bit, -1 }, { first_input_mask_bit, 1 }, { second_input_mask_bit, 1 }, { current_state, -1 } }, 0.0, INF );
		model_builder.add_linear_constraint( name + "_ineq_7", { { next_state, 1 }, { output_mask_bit, 1 }, { first_input_mask_bit, 1 }, { second_input_mask_bit, 1 }, { current_state, 1 } }, -INF, 4.0 );
	}

	template <class Builder>
	inline TwoInputLinearBoxHandle add_two_input_modular_addition_linear_characteristic_constraints( Builder& model_builder, const BitVec& output_mask, const BitVec& first_input_mask, const BitVec& second_input_mask, const std::string& prefix )
	{
		if ( output_mask.size() != first_input_mask.size() || output_mask.size() != second_input_mask.size() )
			throw std::invalid_argument( "add_two_input_modular_addition_linear_characteristic_constraints: masks must have the same width" );
		const int bit_count = static_cast<int>( output_mask.size() );
		if ( bit_count <= 0 )
			throw std::invalid_argument( "add_two_input_modular_addition_linear_characteristic_constraints: bit width must be positive" );

		BitVec weight_bits = model_builder.create_bit_vector( prefix + "_linear_weight_bits", bit_count, 1.0 );
		model_builder.add_constant_equality_constraint( prefix + "_most_significant_weight_bit_zero", { { weight_bits[ bit_count - 1 ], 1 } }, 0.0 );

		// The lower-boundary state is intentionally free.  Fixing it to zero
		// incorrectly removes valid perfect linear approximations such as the
		// all-one one-bit addition mask.
		SVar lower_boundary_state = model_builder.create_binary_variable( prefix + "_linear_lower_boundary_state" );
		for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
		{
			const SVar& current_state = ( bit_index == 0 ) ? lower_boundary_state : weight_bits[ bit_index - 1 ];
			add_fu_wang_guo_linear_addition_transition_constraints(
				model_builder,
				weight_bits[ bit_index ],
				output_mask[ bit_index ],
				first_input_mask[ bit_index ],
				second_input_mask[ bit_index ],
				current_state,
				prefix + "_fu_wang_guo_transition_" + std::to_string( bit_index ) );
		}
		return { weight_bits };
	}

	template <class Builder>
	inline TwoInputLinearBoxHandle add_two_input_modular_subtraction_linear_characteristic_constraints( Builder& model_builder, const BitVec& output_mask, const BitVec& first_input_mask, const BitVec& second_input_mask, const std::string& prefix )
	{
		return add_two_input_modular_addition_linear_characteristic_constraints( model_builder, first_input_mask, output_mask, second_input_mask, prefix );
	}

	struct FixedConstExactThresholdMilpHandle
	{
		BitVec selector_00;
		BitVec selector_01;
		BitVec selector_10;
		BitVec selector_11;
		std::vector<SVar> scaled_state_0;
		std::vector<SVar> scaled_state_1;
		std::vector<std::array<SVar, 4>> split_state_0;
		std::vector<std::array<SVar, 4>> split_state_1;
		SVar final_signed_numerator;
		SVar final_absolute_numerator;
		SVar exact_log_weight;
		SVar final_sign_is_negative;
		long double threshold_weight = 0.0L;
		std::uint64_t threshold_numerator = 0;
	};

	// ========================================================================
	// Audit section B: fixed-public-addend exact numerator MILP
	// ========================================================================
	// Theory: Miyano's addend-dependent two-state signed carry transfer.
	// This section builds |2^n*C_K(alpha,beta)| threshold constraints; the
	// nonlinear exact log objective is added later by a SCIP handler.
	[[nodiscard]] inline std::uint64_t fixed_addend_threshold_numerator_from_weight( int bit_count, long double threshold_weight )
	{
		if ( bit_count <= 0 || bit_count > 32 )
			throw std::invalid_argument( "fixed_addend_threshold_numerator_from_weight: bit width must be in [1, 32]" );
		if ( !std::isfinite( static_cast<double>( threshold_weight ) ) )
			throw std::invalid_argument( "fixed_addend_threshold_numerator_from_weight: threshold weight must be finite" );
		const long double exponent = static_cast<long double>( bit_count ) - threshold_weight;
		if ( exponent <= 0.0L )
			return 1u;
		const long double raw = std::ceil( std::pow( 2.0L, exponent ) - 1.0e-18L );
		const long double maximum = std::ldexp( 1.0L, bit_count );
		if ( raw > maximum )
			return static_cast<std::uint64_t>( maximum ) + 1u;
		return static_cast<std::uint64_t>( raw < 1.0L ? 1.0L : raw );
	}

	[[nodiscard]] inline std::uint64_t fixed_addend_sanitize_threshold_numerator( int bit_count, std::uint64_t threshold_numerator )
	{
		if ( bit_count <= 0 || bit_count > 32 )
			throw std::invalid_argument( "fixed_addend_sanitize_threshold_numerator: bit width must be in [1, 32]" );
		const std::uint64_t maximum = std::uint64_t( 1 ) << bit_count;
		if ( threshold_numerator == 0u )
			return 1u;
		if ( threshold_numerator > maximum )
			return maximum + 1u;
		return threshold_numerator;
	}

	[[nodiscard]] inline long double fixed_addend_weight_from_threshold_numerator( int bit_count, std::uint64_t threshold_numerator )
	{
		const std::uint64_t sanitized = fixed_addend_sanitize_threshold_numerator( bit_count, threshold_numerator );
		const std::uint64_t maximum = std::uint64_t( 1 ) << bit_count;
		if ( sanitized > maximum )
			return -std::numeric_limits<long double>::infinity();
		return static_cast<long double>( bit_count ) - std::log2( static_cast<long double>( sanitized ) );
	}

	[[nodiscard]] inline std::pair<int, int> fixed_addend_scaled_transition_coefficients( int constant_bit, int input_mask_bit, int output_mask_bit, int next_state_index )
	{
		// The integer state S_i = 2^i * (cur_0, cur_1) follows Miyano's
		// fixed-addend 2-state signed transfer without calling an oracle.
		// Return coefficients (coef_state0, coef_state1) for S_{i+1}[next_state_index].
		int coefficient_from_state0 = 0;
		int coefficient_from_state1 = 0;
		for ( int carry_state = 0; carry_state <= 1; ++carry_state )
		{
			int contribution_to_state0 = 0;
			int contribution_to_state1 = 0;
			if ( carry_state == constant_bit )
			{
				if ( input_mask_bit == output_mask_bit )
				{
					if ( constant_bit == 0 )
						contribution_to_state0 += 2;
					else
						contribution_to_state1 += 2;
				}
			}
			else
			{
				contribution_to_state0 += output_mask_bit ? -1 : 1;
				contribution_to_state1 += input_mask_bit ? -1 : 1;
			}

			if ( next_state_index == 0 )
			{
				if ( carry_state == 0 )
					coefficient_from_state0 = contribution_to_state0;
				else
					coefficient_from_state1 = contribution_to_state0;
			}
			else
			{
				if ( carry_state == 0 )
					coefficient_from_state0 = contribution_to_state1;
				else
					coefficient_from_state1 = contribution_to_state1;
			}
		}
		return { coefficient_from_state0, coefficient_from_state1 };
	}

	template <class Builder>
	inline void add_binary_continuous_product_constraints(
		Builder& model_builder,
		const SVar& product,
		const SVar& selector,
		const SVar& source,
		double source_lower_bound,
		double source_upper_bound,
		const std::string& prefix )
	{
		// Convex-hull linearization of product = selector * source for binary
		// selector and source in [source_lower_bound, source_upper_bound].
		model_builder.add_linear_constraint( prefix + "_lower_selector_bound", { { product, 1.0 }, { selector, -source_lower_bound } }, 0.0, SCIPinfinity( model_builder.scip ) );
		model_builder.add_linear_constraint( prefix + "_upper_selector_bound", { { product, 1.0 }, { selector, -source_upper_bound } }, -SCIPinfinity( model_builder.scip ), 0.0 );
		model_builder.add_linear_constraint( prefix + "_lower_source_bound", { { product, 1.0 }, { source, -1.0 }, { selector, -source_upper_bound } }, -source_upper_bound, SCIPinfinity( model_builder.scip ) );
		model_builder.add_linear_constraint( prefix + "_upper_source_bound", { { product, 1.0 }, { source, -1.0 }, { selector, -source_lower_bound } }, -SCIPinfinity( model_builder.scip ), -source_lower_bound );
	}

	template <class Builder>
	inline void add_highest_active_bit_equality_constraints(
		Builder& model_builder,
		const BitVec& input_mask,
		const BitVec& output_mask,
		const std::string& prefix )
	{
		if ( input_mask.size() != output_mask.size() )
			throw std::invalid_argument( "add_highest_active_bit_equality_constraints: masks must have the same width" );
		const int bit_count = static_cast<int>( input_mask.size() );
		if ( bit_count <= 0 || bit_count > 32 )
			throw std::invalid_argument( "add_highest_active_bit_equality_constraints: bit width must be in [1, 32]" );

		BitVec input_suffix_nonzero = model_builder.create_bit_vector( prefix + "_input_suffix_nonzero", bit_count );
		BitVec output_suffix_nonzero = model_builder.create_bit_vector( prefix + "_output_suffix_nonzero", bit_count );

		auto bind_suffix_nonzero = [&]( const BitVec& mask, const BitVec& suffix_nonzero, const std::string& local_prefix )
		{
			for ( int bit_index = bit_count - 1; bit_index >= 0; --bit_index )
			{
				if ( bit_index == bit_count - 1 )
				{
					model_builder.add_zero_equality_constraint( local_prefix + "_top", { { suffix_nonzero[ bit_index ], 1.0 }, { mask[ bit_index ], -1.0 } } );
					continue;
				}

				model_builder.add_linear_constraint(
					local_prefix + "_ge_mask_" + std::to_string( bit_index ),
					{ { suffix_nonzero[ bit_index ], 1.0 }, { mask[ bit_index ], -1.0 } },
					0.0,
					SCIPinfinity( model_builder.scip ) );
				model_builder.add_linear_constraint(
					local_prefix + "_ge_higher_" + std::to_string( bit_index ),
					{ { suffix_nonzero[ bit_index ], 1.0 }, { suffix_nonzero[ bit_index + 1 ], -1.0 } },
					0.0,
					SCIPinfinity( model_builder.scip ) );
				model_builder.add_linear_constraint(
					local_prefix + "_le_or_" + std::to_string( bit_index ),
					{ { suffix_nonzero[ bit_index ], 1.0 }, { mask[ bit_index ], -1.0 }, { suffix_nonzero[ bit_index + 1 ], -1.0 } },
					-SCIPinfinity( model_builder.scip ),
					0.0 );
			}
		};

		bind_suffix_nonzero( input_mask, input_suffix_nonzero, prefix + "_input_suffix" );
		bind_suffix_nonzero( output_mask, output_suffix_nonzero, prefix + "_output_suffix" );

		// Miyano's exact fixed-addend oracle is zero unless the two masks have
		// the same highest active bit, with the all-zero/all-zero case included.
		for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
		{
			model_builder.add_zero_equality_constraint(
				prefix + "_same_highest_suffix_" + std::to_string( bit_index ),
				{ { input_suffix_nonzero[ bit_index ], 1.0 }, { output_suffix_nonzero[ bit_index ], -1.0 } } );
		}
	}

	template <class Builder>
	inline FixedConstExactThresholdMilpHandle add_fixed_public_constant_linear_exact_numerator_threshold_constraints(
		Builder& model_builder,
		std::uint32_t constant,
		const BitVec& input_mask,
		const BitVec& output_mask,
		std::uint64_t requested_threshold_numerator,
		const std::string& prefix )
	{
		// Pure static numerator-threshold MILP for Miyano fixed-addend LAP.
		// It compiles the 2-state signed carry-transfer recurrence into ordinary
		// MILP constraints.  No oracle is called during solving.  It enforces
		// |2^n * C_K(a,b)| >= requested_threshold_numerator exactly.
		if ( input_mask.size() != output_mask.size() )
			throw std::invalid_argument( "add_fixed_public_constant_linear_exact_threshold_constraints: masks must have the same width" );
		const int bit_count = static_cast<int>( input_mask.size() );
		if ( bit_count <= 0 || bit_count > 32 )
			throw std::invalid_argument( "add_fixed_public_constant_linear_exact_threshold_constraints: bit width must be in [1, 32]" );

		constant &= linear_oracle::mask_for_bits( bit_count );
		const std::uint64_t threshold_numerator = fixed_addend_sanitize_threshold_numerator( bit_count, requested_threshold_numerator );
		const long double threshold_weight = fixed_addend_weight_from_threshold_numerator( bit_count, threshold_numerator );
		if ( threshold_numerator > ( std::uint64_t( 1 ) << bit_count ) )
		{
			// Impossible threshold; add a direct contradiction instead of silently
			// building a numerically huge disjunction.
			SVar impossible = model_builder.create_binary_variable( prefix + "_impossible_threshold_guard" );
			model_builder.add_constant_equality_constraint( prefix + "_impossible_threshold_guard_zero", { { impossible, 1.0 } }, 0.0 );
			model_builder.add_constant_equality_constraint( prefix + "_impossible_threshold_guard_one", { { impossible, 1.0 } }, 1.0 );
			return {};
		}

		FixedConstExactThresholdMilpHandle handle;
		handle.threshold_weight = threshold_weight;
		handle.threshold_numerator = threshold_numerator;
		handle.selector_00 = model_builder.create_bit_vector( prefix + "_mask_selector_00", bit_count );
		handle.selector_01 = model_builder.create_bit_vector( prefix + "_mask_selector_01", bit_count );
		handle.selector_10 = model_builder.create_bit_vector( prefix + "_mask_selector_10", bit_count );
		handle.selector_11 = model_builder.create_bit_vector( prefix + "_mask_selector_11", bit_count );

		add_highest_active_bit_equality_constraints(
			model_builder,
			input_mask,
			output_mask,
			prefix + "_highest_active_bit" );

		handle.scaled_state_0.reserve( bit_count + 1 );
		handle.scaled_state_1.reserve( bit_count + 1 );
		handle.split_state_0.reserve( bit_count );
		handle.split_state_1.reserve( bit_count );
		for ( int i = 0; i <= bit_count; ++i )
		{
			const double bound = std::ldexp( 1.0, i );
			handle.scaled_state_0.push_back( model_builder.create_continuous_variable( prefix + "_scaled_state0_" + std::to_string( i ), -bound, bound ) );
			handle.scaled_state_1.push_back( model_builder.create_continuous_variable( prefix + "_scaled_state1_" + std::to_string( i ), -bound, bound ) );
		}
		model_builder.add_constant_equality_constraint( prefix + "_scaled_state0_initial_one", { { handle.scaled_state_0[ 0 ], 1.0 } }, 1.0 );
		model_builder.add_constant_equality_constraint( prefix + "_scaled_state1_initial_zero", { { handle.scaled_state_1[ 0 ], 1.0 } }, 0.0 );

		for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
		{
			const SVar& z00 = handle.selector_00[ bit_index ];
			const SVar& z01 = handle.selector_01[ bit_index ];
			const SVar& z10 = handle.selector_10[ bit_index ];
			const SVar& z11 = handle.selector_11[ bit_index ];
			model_builder.add_constant_equality_constraint( prefix + "_selector_exactly_one_" + std::to_string( bit_index ), { { z00, 1.0 }, { z01, 1.0 }, { z10, 1.0 }, { z11, 1.0 } }, 1.0 );
			model_builder.add_zero_equality_constraint( prefix + "_selector_binds_input_" + std::to_string( bit_index ), { { input_mask[ bit_index ], 1.0 }, { z10, -1.0 }, { z11, -1.0 } } );
			model_builder.add_zero_equality_constraint( prefix + "_selector_binds_output_" + std::to_string( bit_index ), { { output_mask[ bit_index ], 1.0 }, { z01, -1.0 }, { z11, -1.0 } } );

			const int constant_bit = linear_oracle::word_bit( constant, bit_index );
			const double source_bound = std::ldexp( 1.0, bit_index );
			const std::array<std::tuple<int, int, SVar, const char*>, 4> cases = {
				std::tuple<int, int, SVar, const char*>( 0, 0, z00, "00" ),
				std::tuple<int, int, SVar, const char*>( 0, 1, z01, "01" ),
				std::tuple<int, int, SVar, const char*>( 1, 0, z10, "10" ),
				std::tuple<int, int, SVar, const char*>( 1, 1, z11, "11" ) };

			std::array<SVar, 4> split_state0;
			std::array<SVar, 4> split_state1;
			for ( int case_index = 0; case_index < static_cast<int>( cases.size() ); ++case_index )
			{
				const auto& [ input_bit, output_bit, selector, case_tag ] = cases[ case_index ];
				const std::string case_prefix = prefix + "_bit_" + std::to_string( bit_index ) + "_case_" + case_tag;
				split_state0[ case_index ] = model_builder.create_continuous_variable( case_prefix + "_split_state0", -source_bound, source_bound );
				split_state1[ case_index ] = model_builder.create_continuous_variable( case_prefix + "_split_state1", -source_bound, source_bound );
				add_binary_continuous_product_constraints(
					model_builder,
					split_state0[ case_index ],
					selector,
					handle.scaled_state_0[ bit_index ],
					-source_bound,
					source_bound,
					case_prefix + "_state0_product" );
				add_binary_continuous_product_constraints(
					model_builder,
					split_state1[ case_index ],
					selector,
					handle.scaled_state_1[ bit_index ],
					-source_bound,
					source_bound,
					case_prefix + "_state1_product" );
			}
			handle.split_state_0.push_back( split_state0 );
			handle.split_state_1.push_back( split_state1 );

			std::vector<LinearTerm> state0_disaggregation { { handle.scaled_state_0[ bit_index ], 1.0 } };
			std::vector<LinearTerm> state1_disaggregation { { handle.scaled_state_1[ bit_index ], 1.0 } };
			std::vector<LinearTerm> state0_transition { { handle.scaled_state_0[ bit_index + 1 ], 1.0 } };
			std::vector<LinearTerm> state1_transition { { handle.scaled_state_1[ bit_index + 1 ], 1.0 } };
			for ( int case_index = 0; case_index < static_cast<int>( cases.size() ); ++case_index )
			{
				const auto& [ input_bit, output_bit, selector, case_tag ] = cases[ case_index ];
				(void)selector;
				(void)case_tag;
				state0_disaggregation.push_back( { split_state0[ case_index ], -1.0 } );
				state1_disaggregation.push_back( { split_state1[ case_index ], -1.0 } );

				const auto [ c00, c01 ] = fixed_addend_scaled_transition_coefficients( constant_bit, input_bit, output_bit, 0 );
				const auto [ c10, c11 ] = fixed_addend_scaled_transition_coefficients( constant_bit, input_bit, output_bit, 1 );
				if ( c00 != 0 )
					state0_transition.push_back( { split_state0[ case_index ], -static_cast<double>( c00 ) } );
				if ( c01 != 0 )
					state0_transition.push_back( { split_state1[ case_index ], -static_cast<double>( c01 ) } );
				if ( c10 != 0 )
					state1_transition.push_back( { split_state0[ case_index ], -static_cast<double>( c10 ) } );
				if ( c11 != 0 )
					state1_transition.push_back( { split_state1[ case_index ], -static_cast<double>( c11 ) } );
			}
			model_builder.add_zero_equality_constraint( prefix + "_state0_disaggregation_" + std::to_string( bit_index ), state0_disaggregation );
			model_builder.add_zero_equality_constraint( prefix + "_state1_disaggregation_" + std::to_string( bit_index ), state1_disaggregation );
			model_builder.add_zero_equality_constraint( prefix + "_state0_transition_disaggregated_" + std::to_string( bit_index ), state0_transition );
			model_builder.add_zero_equality_constraint( prefix + "_state1_transition_disaggregated_" + std::to_string( bit_index ), state1_transition );
		}

		handle.final_signed_numerator = model_builder.create_continuous_variable( prefix + "_final_signed_numerator", -std::ldexp( 1.0, bit_count ), std::ldexp( 1.0, bit_count ) );
		model_builder.add_zero_equality_constraint( prefix + "_final_numerator_sum", { { handle.final_signed_numerator, 1.0 }, { handle.scaled_state_0[ bit_count ], -1.0 }, { handle.scaled_state_1[ bit_count ], -1.0 } } );
		handle.final_sign_is_negative = model_builder.create_binary_variable( prefix + "_final_sign_is_negative" );
		const double threshold = static_cast<double>( threshold_numerator );
		const double final_big_m = std::ldexp( 1.0, bit_count ) + threshold;
		model_builder.add_linear_constraint( prefix + "_abs_numerator_positive_or_negative_lower", { { handle.final_signed_numerator, 1.0 }, { handle.final_sign_is_negative, final_big_m } }, threshold, SCIPinfinity( model_builder.scip ) );
		model_builder.add_linear_constraint( prefix + "_abs_numerator_positive_or_negative_upper", { { handle.final_signed_numerator, 1.0 }, { handle.final_sign_is_negative, final_big_m } }, -SCIPinfinity( model_builder.scip ), final_big_m - threshold );
		return handle;
	}





	template <class Builder>
	inline FixedConstExactThresholdMilpHandle add_fixed_public_constant_linear_exact_threshold_constraints(
		Builder& model_builder,
		std::uint32_t constant,
		const BitVec& input_mask,
		const BitVec& output_mask,
		long double threshold_weight,
		const std::string& prefix )
	{
		const int bit_count = static_cast<int>( input_mask.size() );
		const std::uint64_t threshold_numerator = fixed_addend_threshold_numerator_from_weight( bit_count, threshold_weight );
		return add_fixed_public_constant_linear_exact_numerator_threshold_constraints( model_builder, constant, input_mask, output_mask, threshold_numerator, prefix );
	}

	template <class Builder>
	inline FixedConstExactThresholdMilpHandle add_fixed_public_constant_subtraction_linear_exact_threshold_constraints(
		Builder& model_builder,
		std::uint32_t constant,
		const BitVec& input_mask,
		const BitVec& output_mask,
		long double threshold_weight,
		const std::string& prefix )
	{
		const std::uint32_t mask = linear_oracle::mask_for_bits( static_cast<int>( input_mask.size() ) );
		const std::uint32_t neg_constant = ( std::uint32_t( 0 ) - ( constant & mask ) ) & mask;
		return add_fixed_public_constant_linear_exact_threshold_constraints( model_builder, neg_constant, input_mask, output_mask, threshold_weight, prefix );
	}

	template <class Builder>
	inline FixedConstExactThresholdMilpHandle add_fixed_public_constant_subtraction_linear_exact_numerator_threshold_constraints(
		Builder& model_builder,
		std::uint32_t constant,
		const BitVec& input_mask,
		const BitVec& output_mask,
		std::uint64_t threshold_numerator,
		const std::string& prefix )
	{
		const std::uint32_t mask = linear_oracle::mask_for_bits( static_cast<int>( input_mask.size() ) );
		const std::uint32_t neg_constant = ( std::uint32_t( 0 ) - ( constant & mask ) ) & mask;
		return add_fixed_public_constant_linear_exact_numerator_threshold_constraints( model_builder, neg_constant, input_mask, output_mask, threshold_numerator, prefix );
	}
}  // namespace neoalzette_linear_milp::arithmetic_model

namespace neoalzette_linear_milp
{
	// ========================================================================
	// Audit section C: SCIP handler for joint injection linear Walsh constraints
	// ========================================================================
	// This block owns all dynamic cuts for the quadratic injection oracle:
	// beta exclusions, alpha support parities, rank/2 epigraph cuts, and handler
	// lifecycle callbacks.
	// Per-SCIP-constraint state for one joint injection Walsh constraint.
	// beta is the packed 64-bit output mask of the injection map and alpha is
	// the 32-bit source mask.  The seen sets keep dynamically generated cuts
	// local to the relevant SCIP search context so the handler can add support,
	// rank, and tuple-exclusion rows without flooding the model with duplicates.
	// The contract is exactly the quadratic-Walsh bridge in the oracle: beta
	// fixes the quadratic form, support parities restrict alpha, and weight is
	// the rank/2 epigraph for a nonzero Walsh coefficient.
	struct LinearInjectionWalshConsData
	{
		InjectionKind kind = InjectionKind::B_TO_A_JOINT;
		std::array<SCIP_VAR*, WORD_SIZE> alpha {};
		std::array<SCIP_VAR*, JOINT_INJECTION_BETA_SIZE> beta {};
		SCIP_VAR* weight = nullptr;
		std::string name;
		std::set<std::uint64_t> beta_exclusion_seen;
		std::set<std::uint64_t> rank_epigraph_seen;
		std::set<std::pair<std::uint32_t, std::uint64_t>> tuple_exclusion_seen;
		std::map<std::tuple<std::uint64_t, std::uint32_t, int>, SCIP_Longint> local_support_seen;
	};

	static constexpr const char* LINEAR_INJECTION_WALSH_CONSHDLR_NAME = "linear_injection_walsh";
	static constexpr std::size_t MAX_DYNAMIC_RANK_EPIGRAPH_CUTS_PER_INJECTION = 1024;

	static bool local_fixed_binary_value( SCIP* scip, SCIP_VAR* var, int& value )
	{
		const SCIP_Real lb = SCIPvarGetLbLocal( var );
		const SCIP_Real ub = SCIPvarGetUbLocal( var );
		if ( SCIPisFeasGE( scip, lb, 1.0 ) )
		{
			value = 1;
			return true;
		}
		if ( SCIPisFeasLE( scip, ub, 0.0 ) )
		{
			value = 0;
			return true;
		}
		return false;
	}


	static SCIP_RETCODE capture_linear_injection_vars( SCIP* scip, LinearInjectionWalshConsData* data )
	{
		if ( data->weight == nullptr )
			return SCIP_INVALIDDATA;
		SCIP_CALL( SCIPcaptureVar( scip, data->weight ) );
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			if ( data->alpha[ i ] == nullptr )
				return SCIP_INVALIDDATA;
			SCIP_CALL( SCIPcaptureVar( scip, data->alpha[ i ] ) );
		}
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			if ( data->beta[ i ] == nullptr )
				return SCIP_INVALIDDATA;
			SCIP_CALL( SCIPcaptureVar( scip, data->beta[ i ] ) );
		}
		return SCIP_OKAY;
	}

	static SCIP_RETCODE release_linear_injection_vars( SCIP* scip, LinearInjectionWalshConsData* data )
	{
		if ( data->weight != nullptr )
		{
			SCIP_CALL( SCIPreleaseVar( scip, &data->weight ) );
			data->weight = nullptr;
		}
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			if ( data->alpha[ i ] != nullptr )
			{
				SCIP_CALL( SCIPreleaseVar( scip, &data->alpha[ i ] ) );
				data->alpha[ i ] = nullptr;
			}
		}
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			if ( data->beta[ i ] != nullptr )
			{
				SCIP_CALL( SCIPreleaseVar( scip, &data->beta[ i ] ) );
				data->beta[ i ] = nullptr;
			}
		}
		return SCIP_OKAY;
	}

	static std::uint32_t read_solution_bits( SCIP* scip, SCIP_SOL* sol, const std::array<SCIP_VAR*, WORD_SIZE>& bits )
	{
		std::uint32_t x = 0u;
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			const double v = SCIPgetSolVal( scip, sol, bits[ i ] );
			if ( v > 0.5 )
				x |= ( 1u << i );
		}
		return x;
	}

	static std::uint64_t read_solution_bits64( SCIP* scip, SCIP_SOL* sol, const std::array<SCIP_VAR*, JOINT_INJECTION_BETA_SIZE>& bits )
	{
		std::uint64_t x = 0u;
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			const double v = SCIPgetSolVal( scip, sol, bits[ i ] );
			if ( v > 0.5 )
				x |= ( std::uint64_t( 1 ) << i );
		}
		return x;
	}

	static double read_solution_value( SCIP* scip, SCIP_SOL* sol, SCIP_VAR* var )
	{
		return SCIPgetSolVal( scip, sol, var );
	}

	static bool solution_bits_are_integral( SCIP* scip, SCIP_SOL* sol, const std::array<SCIP_VAR*, WORD_SIZE>& bits )
	{
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			const double v = read_solution_value( scip, sol, bits[ i ] );
			if ( !SCIPisFeasIntegral( scip, v ) )
				return false;
		}
		return true;
	}

	static bool solution_bits_are_integral64( SCIP* scip, SCIP_SOL* sol, const std::array<SCIP_VAR*, JOINT_INJECTION_BETA_SIZE>& bits )
	{
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			const double v = read_solution_value( scip, sol, bits[ i ] );
			if ( !SCIPisFeasIntegral( scip, v ) )
				return false;
		}
		return true;
	}

	static bool injection_solution_masks_are_integral( SCIP* scip, SCIP_SOL* sol, const LinearInjectionWalshConsData* data )
	{
		return solution_bits_are_integral( scip, sol, data->alpha ) && solution_bits_are_integral64( scip, sol, data->beta );
	}

	static bool local_binary_value( SCIP* scip, SCIP_VAR* var, int& value )
	{
		const SCIP_Real lb = SCIPvarGetLbLocal( var );
		const SCIP_Real ub = SCIPvarGetUbLocal( var );
		if ( SCIPisFeasGE( scip, lb, 1.0 ) )
		{
			value = 1;
			return true;
		}
		if ( SCIPisFeasLE( scip, ub, 0.0 ) )
		{
			value = 0;
			return true;
		}
		return false;
	}

	static bool beta_is_locally_fixed( SCIP* scip, const LinearInjectionWalshConsData* data, std::uint64_t& beta )
	{
		beta = 0u;
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			int bit = 0;
			if ( !local_binary_value( scip, data->beta[ i ], bit ) )
				return false;
			if ( bit != 0 )
				beta |= ( std::uint64_t( 1 ) << i );
		}
		return true;
	}

	static SCIP_Longint current_node_number( SCIP* scip )
	{
		SCIP_NODE* node = SCIPgetCurrentNode( scip );
		return node == nullptr ? -1 : SCIPnodeGetNumber( node );
	}

	static std::set<SCIP_Longint> current_path_node_numbers( SCIP* scip )
	{
		std::set<SCIP_Longint> path;
		for ( SCIP_NODE* node = SCIPgetCurrentNode( scip ); node != nullptr; node = SCIPnodeGetParent( node ) )
			path.insert( SCIPnodeGetNumber( node ) );
		return path;
	}

	static void prune_local_support_seen_to_current_path( SCIP* scip, LinearInjectionWalshConsData* data )
	{
		const auto path = current_path_node_numbers( scip );
		for ( auto it = data->local_support_seen.begin(); it != data->local_support_seen.end(); )
		{
			if ( path.find( it->second ) == path.end() )
				it = data->local_support_seen.erase( it );
			else
				++it;
		}
	}

	static SCIP_RETCODE add_named_linear_constraint(
		SCIP* scip,
		const std::string& name,
		const std::vector<SCIP_VAR*>& vars,
		const std::vector<SCIP_Real>& vals,
		SCIP_Real lhs,
		SCIP_Real rhs,
		bool& added )
	{
		SCIP_CONS* cons = nullptr;
		SCIP_CALL( SCIPcreateConsBasicLinear(
			scip,
			&cons,
			name.c_str(),
			static_cast<int>( vars.size() ),
			const_cast<SCIP_VAR**>( vars.data() ),
			const_cast<SCIP_Real*>( vals.data() ),
			lhs,
			rhs ) );
		SCIP_CALL( SCIPaddCons( scip, cons ) );
		SCIP_CALL( SCIPreleaseCons( scip, &cons ) );
		added = true;
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_beta_exclusion_bound( SCIP* scip, LinearInjectionWalshConsData* data, std::uint64_t beta_star, bool& added )
	{
		added = false;
		if ( !data->beta_exclusion_seen.insert( beta_star ).second )
			return SCIP_OKAY;

		std::vector<SCIP_VAR*> vars;
		std::vector<SCIP_Real> vals;
		vars.reserve( JOINT_INJECTION_BETA_SIZE );
		vals.reserve( JOINT_INJECTION_BETA_SIZE );
		int ones = 0;
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			vars.push_back( data->beta[ i ] );
			if ( ( beta_star >> i ) & 1ull )
			{
				vals.push_back( -1.0 );
				++ones;
			}
			else
			{
				vals.push_back( 1.0 );
			}
		}
		const SCIP_Real lhs = 1.0 - static_cast<SCIP_Real>( ones );
		SCIP_CALL( add_named_linear_constraint( scip, data->name + "_exclude_beta_" + std::to_string( beta_star ), vars, vals, lhs, SCIPinfinity( scip ), added ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_rank_epigraph_bound(
		SCIP* scip,
		LinearInjectionWalshConsData* data,
		std::uint64_t beta_star,
		double required_weight,
		bool& added )
	{
		// Conditional epigraph cut:
		//   if beta == beta_star then weight >= required_weight.
		// For the quadratic injection oracle required_weight is rank(beta)/2.
		// The Hamming-distance guard below deactivates the bound as soon as any
		// beta bit differs from beta_star, so no extra indicator variable is used.
		added = false;
		if ( required_weight <= 0.0 )
			return SCIP_OKAY;
		if ( data->rank_epigraph_seen.size() >= MAX_DYNAMIC_RANK_EPIGRAPH_CUTS_PER_INJECTION )
			return SCIP_OKAY;
		if ( !data->rank_epigraph_seen.insert( beta_star ).second )
			return SCIP_OKAY;

		std::vector<SCIP_VAR*> vars;
		std::vector<SCIP_Real> vals;
		vars.reserve( JOINT_INJECTION_BETA_SIZE + 1 );
		vals.reserve( JOINT_INJECTION_BETA_SIZE + 1 );
		vars.push_back( data->weight );
		vals.push_back( 1.0 );

		int ones = 0;
		const SCIP_Real r = static_cast<SCIP_Real>( required_weight );
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			vars.push_back( data->beta[ i ] );
			if ( ( beta_star >> i ) & 1ull )
			{
				vals.push_back( -r );
				++ones;
			}
			else
			{
				vals.push_back( r );
			}
		}
		const SCIP_Real lhs = r - r * static_cast<SCIP_Real>( ones );
		SCIP_CALL( add_named_linear_constraint( scip, data->name + "_rank_epigraph_beta_" + std::to_string( beta_star ), vars, vals, lhs, SCIPinfinity( scip ), added ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_rank_or_exclusion_for_beta( SCIP* scip, LinearInjectionWalshConsData* data, std::uint64_t beta_star, bool& added )
	{
		LinearInjectionOracle& oracle = global_injection_oracle();
		const auto support = oracle.support( data->kind, beta_star );
		if ( !support.possible )
		{
			SCIP_CALL( add_beta_exclusion_bound( scip, data, beta_star, added ) );
			return SCIP_OKAY;
		}
		if ( data->rank_epigraph_seen.size() >= MAX_DYNAMIC_RANK_EPIGRAPH_CUTS_PER_INJECTION )
		{
			added = false;
			return SCIP_OKAY;
		}
		const double required_weight = static_cast<double>( support.rank ) / 2.0;
		SCIP_CALL( add_rank_epigraph_bound( scip, data, beta_star, required_weight, added ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_injection_tuple_exclusion_bound(
		SCIP* scip,
		LinearInjectionWalshConsData* data,
		std::uint32_t alpha_star,
		std::uint64_t beta_star,
		bool& added )
	{
		added = false;
		if ( !data->tuple_exclusion_seen.insert( { alpha_star, beta_star } ).second )
			return SCIP_OKAY;

		std::vector<SCIP_VAR*> vars;
		std::vector<SCIP_Real> vals;
		vars.reserve( WORD_SIZE + JOINT_INJECTION_BETA_SIZE );
		vals.reserve( WORD_SIZE + JOINT_INJECTION_BETA_SIZE );

		int zero_value_count = 0;
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			vars.push_back( data->alpha[ i ] );
			if ( ( alpha_star >> i ) & 1u )
			{
				vals.push_back( 1.0 );
			}
			else
			{
				vals.push_back( -1.0 );
				++zero_value_count;
			}
		}
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			vars.push_back( data->beta[ i ] );
			if ( ( beta_star >> i ) & 1ull )
			{
				vals.push_back( 1.0 );
			}
			else
			{
				vals.push_back( -1.0 );
				++zero_value_count;
			}
		}

		const int assignment_size = WORD_SIZE + JOINT_INJECTION_BETA_SIZE;
		const SCIP_Real rhs = static_cast<SCIP_Real>( assignment_size - 1 - zero_value_count );
		const std::string cons_name = data->name + "_exclude_alpha_beta_" + std::to_string( alpha_star ) + "_" + std::to_string( beta_star );
		SCIP_CALL( add_named_linear_constraint( scip, cons_name, vars, vals, -SCIPinfinity( scip ), rhs, added ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE preadd_small_weight_beta_rank_bounds( SCIP* scip, LinearInjectionWalshConsData* data )
	{
		bool added = false;
		SCIP_CALL( add_rank_or_exclusion_for_beta( scip, data, 0u, added ) );
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
			SCIP_CALL( add_rank_or_exclusion_for_beta( scip, data, std::uint64_t( 1 ) << i, added ) );
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			for ( int j = i + 1; j < JOINT_INJECTION_BETA_SIZE; ++j )
				SCIP_CALL( add_rank_or_exclusion_for_beta( scip, data, ( std::uint64_t( 1 ) << i ) | ( std::uint64_t( 1 ) << j ), added ) );
		}
		return SCIP_OKAY;
	}

	static SCIP_RETCODE propagate_local_alpha_parity(
		SCIP* scip,
		LinearInjectionWalshConsData* data,
		std::uint32_t alpha_mask,
		int rhs,
		bool& infeasible,
		bool& reduced_domain )
	{
		if ( alpha_mask == 0u )
		{
			if ( rhs != 0 )
				infeasible = true;
			return SCIP_OKAY;
		}

		int parity = 0;
		int free_count = 0;
		SCIP_VAR* free_var = nullptr;
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			if ( ( ( alpha_mask >> i ) & 1u ) == 0u )
				continue;
			int bit = 0;
			if ( local_fixed_binary_value( scip, data->alpha[ i ], bit ) )
			{
				parity ^= bit;
			}
			else
			{
				++free_count;
				free_var = data->alpha[ i ];
			}
		}

		if ( free_count == 0 )
		{
			if ( parity != rhs )
				infeasible = true;
			return SCIP_OKAY;
		}

		if ( free_count == 1 )
		{
			const int required = parity ^ rhs;
			SCIP_Bool cutoff = FALSE;
			SCIP_Bool tightened = FALSE;
			if ( required != 0 )
				SCIP_CALL( SCIPtightenVarLb( scip, free_var, 1.0, FALSE, &cutoff, &tightened ) );
			else
				SCIP_CALL( SCIPtightenVarUb( scip, free_var, 0.0, FALSE, &cutoff, &tightened ) );
			if ( cutoff )
				infeasible = true;
			if ( tightened )
				reduced_domain = true;
		}
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_local_alpha_parity_constraint(
		SCIP* scip,
		LinearInjectionWalshConsData* data,
		std::uint64_t beta,
		std::uint32_t alpha_mask,
		int rhs,
		bool& infeasible,
		bool& added )
	{
		added = false;
		if ( alpha_mask == 0u )
		{
			if ( rhs != 0 )
				infeasible = true;
			return SCIP_OKAY;
		}

		prune_local_support_seen_to_current_path( scip, data );
		const int rhs_bit = rhs & 1;
		const auto key = std::make_tuple( beta, alpha_mask, rhs_bit );
		if ( data->local_support_seen.find( key ) != data->local_support_seen.end() )
			return SCIP_OKAY;
		data->local_support_seen.emplace( key, current_node_number( scip ) );

		std::vector<SCIP_VAR*> vars;
		vars.reserve( WORD_SIZE );
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			if ( ( ( alpha_mask >> i ) & 1u ) != 0u )
				vars.push_back( data->alpha[ i ] );
		}
		if ( vars.empty() )
			return SCIP_OKAY;

		SCIP_CONS* cons = nullptr;
		const std::string cons_name = data->name + "_local_support_xor_" + std::to_string( beta ) + "_" + std::to_string( alpha_mask ) + "_" + std::to_string( rhs_bit );
		SCIP_CALL( SCIPcreateConsXor(
			scip,
			&cons,
			cons_name.c_str(),
			rhs_bit != 0 ? TRUE : FALSE,
			static_cast<int>( vars.size() ),
			vars.data(),
			FALSE,
			TRUE,
			TRUE,
			TRUE,
			TRUE,
			TRUE,
			FALSE,
			FALSE,
			FALSE,
			TRUE ) );
		SCIP_CALL( SCIPaddConsLocal( scip, cons, nullptr ) );
		SCIP_CALL( SCIPreleaseCons( scip, &cons ) );
		added = true;
		return SCIP_OKAY;
	}

	static SCIP_RETCODE separate_linear_injection_walsh( SCIP* scip, SCIP_SOL* sol, LinearInjectionWalshConsData* data, SCIP_RESULT* result, bool enforcement )
	{
		// Integral separation mirrors the oracle contract in three steps:
		//   1. reject beta masks whose quadratic Walsh support is empty;
		//   2. reject or cut alpha values outside the affine support for beta;
		//   3. enforce weight >= rank(beta)/2.
		// Fractional LP points are handled by propagation/cuts for locally fixed
		// beta masks; this function only reasons about concrete masks.
		if ( !injection_solution_masks_are_integral( scip, sol, data ) )
			return SCIP_OKAY;

		LinearInjectionOracle& oracle = global_injection_oracle();
		const std::uint32_t alpha = read_solution_bits( scip, sol, data->alpha );
		const std::uint64_t beta = read_solution_bits64( scip, sol, data->beta );
		const double model_weight = read_solution_value( scip, sol, data->weight );
		const auto support = oracle.support( data->kind, beta );
		bool support_violated = false;
		bool weight_violated = false;

		if ( !support.possible )
		{
			if ( enforcement )
			{
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
			bool added = false;
			SCIP_CALL( add_beta_exclusion_bound( scip, data, beta, added ) );
			if ( added )
				*result = SCIP_CONSADDED;
			return SCIP_OKAY;
		}

		for ( const auto& [ mask, rhs ] : support.parities )
		{
			if ( parity32( alpha & mask ) != rhs )
				support_violated = true;
		}

		const double required_weight = static_cast<double>( support.rank ) / 2.0;
		if ( model_weight + 1e-7 < required_weight )
			weight_violated = true;

		if ( support_violated )
		{
			if ( enforcement )
			{
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
			bool added = false;
			SCIP_CALL( add_injection_tuple_exclusion_bound( scip, data, alpha, beta, added ) );
			if ( added )
				*result = SCIP_CONSADDED;
			return SCIP_OKAY;
		}

		if ( weight_violated )
		{
			if ( enforcement )
			{
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
			else
			{
				bool added = false;
				SCIP_CALL( add_rank_epigraph_bound( scip, data, beta, required_weight, added ) );
				if ( added )
					*result = SCIP_CONSADDED;
			}
		}
		return SCIP_OKAY;
	}

	static SCIP_RETCODE propagate_linear_injection_for_fixed_beta(
		SCIP* scip,
		LinearInjectionWalshConsData* data,
		std::uint64_t beta,
		bool& infeasible,
		bool& reduced_domain,
		bool& cons_added )
	{
		// Once SCIP locally fixes beta, the quadratic Walsh problem becomes an
		// affine alpha-support system plus a numeric rank/2 lower bound. Adding
		// these local rows early is cheaper than waiting for a full integral
		// alpha,beta tuple to violate the handler.
		LinearInjectionOracle& oracle = global_injection_oracle();
		const auto			  support = oracle.support( data->kind, beta );

		if ( !support.possible )
		{
			infeasible = true;
			return SCIP_OKAY;
		}

		const double required_weight = static_cast<double>( support.rank ) / 2.0;
		if ( required_weight > 0.0 )
		{
			SCIP_Bool cutoff = FALSE;
			SCIP_Bool tightened = FALSE;
			SCIP_CALL( SCIPtightenVarLb( scip, data->weight, required_weight, FALSE, &cutoff, &tightened ) );
			if ( cutoff )
				infeasible = true;
			if ( tightened )
				reduced_domain = true;
		}
		for ( const auto& [ mask, rhs ] : support.parities )
		{
			bool added = false;
			SCIP_CALL( add_local_alpha_parity_constraint( scip, data, beta, mask, rhs, infeasible, added ) );
			if ( infeasible )
				return SCIP_OKAY;
			if ( added )
				cons_added = true;
			SCIP_CALL( propagate_local_alpha_parity( scip, data, mask, rhs, infeasible, reduced_domain ) );
			if ( infeasible )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSDELETE(consDeleteLinearInjectionWalsh)
	{
		(void)conshdlr;
		if ( consdata != nullptr && *consdata != nullptr )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( *consdata );
			SCIP_CALL( release_linear_injection_vars( scip, data ) );
			delete data;
			*consdata = nullptr;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSTRANS(consTransLinearInjectionWalsh)
	{
		auto* source = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( sourcecons ) );
		auto* target = new LinearInjectionWalshConsData();
		target->kind = source->kind;
		target->name = source->name;
		target->beta_exclusion_seen = source->beta_exclusion_seen;
		target->rank_epigraph_seen = source->rank_epigraph_seen;
		target->tuple_exclusion_seen = source->tuple_exclusion_seen;
		SCIP_CALL( SCIPgetTransformedVar( scip, source->weight, &target->weight ) );
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			SCIP_CALL( SCIPgetTransformedVar( scip, source->alpha[ i ], &target->alpha[ i ] ) );
		}
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			SCIP_CALL( SCIPgetTransformedVar( scip, source->beta[ i ], &target->beta[ i ] ) );
		}
		SCIP_CALL( capture_linear_injection_vars( scip, target ) );
		SCIP_CALL( SCIPcreateCons(
			scip,
			targetcons,
			SCIPconsGetName( sourcecons ),
			conshdlr,
			reinterpret_cast<SCIP_CONSDATA*>( target ),
			SCIPconsIsInitial( sourcecons ),
			SCIPconsIsSeparated( sourcecons ),
			SCIPconsIsEnforced( sourcecons ),
			SCIPconsIsChecked( sourcecons ),
			SCIPconsIsPropagated( sourcecons ),
			SCIPconsIsLocal( sourcecons ),
			SCIPconsIsModifiable( sourcecons ),
			SCIPconsIsDynamic( sourcecons ),
			SCIPconsIsRemovable( sourcecons ),
			SCIPconsIsStickingAtNode( sourcecons ) ) );
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSENFOLP(consEnfolpLinearInjectionWalsh)
	{
		(void)conshdlr;
		(void)nusefulconss;
		(void)solinfeasible;
		*result = SCIP_FEASIBLE;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_linear_injection_walsh( scip, nullptr, data, result, true ) );
			if ( *result != SCIP_FEASIBLE )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSENFOPS(consEnfopsLinearInjectionWalsh)
	{
		(void)conshdlr;
		(void)nusefulconss;
		(void)solinfeasible;
		(void)objinfeasible;
		*result = SCIP_FEASIBLE;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_linear_injection_walsh( scip, nullptr, data, result, true ) );
			if ( *result != SCIP_FEASIBLE )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSSEPALP(consSepalpLinearInjectionWalsh)
	{
		(void)conshdlr;
		(void)nusefulconss;
		*result = SCIP_DIDNOTFIND;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( conss[ c ] ) );
			std::uint64_t beta_star = 0u;
			for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
			{
				const double v = SCIPgetSolVal( scip, nullptr, data->beta[ i ] );
				if ( v > 0.5 )
					beta_star |= std::uint64_t( 1 ) << i;
			}
			bool added = false;
			SCIP_CALL( add_rank_or_exclusion_for_beta( scip, data, beta_star, added ) );
			if ( added )
			{
				*result = SCIP_CONSADDED;
				return SCIP_OKAY;
			}
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSSEPASOL(consSepasolLinearInjectionWalsh)
	{
		(void)conshdlr;
		(void)nusefulconss;
		*result = SCIP_DIDNOTFIND;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_linear_injection_walsh( scip, sol, data, result, false ) );
			if ( *result == SCIP_CONSADDED || *result == SCIP_CUTOFF )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSCHECK(consCheckLinearInjectionWalsh)
	{
		(void)conshdlr;
		(void)checkintegrality;
		(void)checklprows;
		(void)printreason;
		(void)completely;
		LinearInjectionOracle& oracle = global_injection_oracle();
		*result = SCIP_FEASIBLE;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( conss[ c ] ) );
			if ( !injection_solution_masks_are_integral( scip, sol, data ) )
				continue;
			const std::uint32_t alpha = read_solution_bits( scip, sol, data->alpha );
			const std::uint64_t beta = read_solution_bits64( scip, sol, data->beta );
			const double model_weight = read_solution_value( scip, sol, data->weight );
			const auto res = oracle.transition( data->kind, alpha, beta );
			if ( !res.valid || model_weight + 1e-7 < res.weight )
			{
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSPROP(consPropLinearInjectionWalsh)
	{
		(void)conshdlr;
		(void)nusefulconss;
		(void)nmarkedconss;
		(void)proptiming;
		*result = SCIP_DIDNOTFIND;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( conss[ c ] ) );
			std::uint64_t beta = 0u;
			if ( !beta_is_locally_fixed( scip, data, beta ) )
				continue;

			bool infeasible = false;
			bool reduced_domain = false;
			bool cons_added = false;
			SCIP_CALL( propagate_linear_injection_for_fixed_beta( scip, data, beta, infeasible, reduced_domain, cons_added ) );
			if ( infeasible )
			{
				*result = SCIP_CUTOFF;
				return SCIP_OKAY;
			}
			if ( cons_added || reduced_domain )
				*result = SCIP_REDUCEDDOM;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSLOCK(consLockLinearInjectionWalsh)
	{
		auto* data = reinterpret_cast<LinearInjectionWalshConsData*>( SCIPconsGetData( cons ) );
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			SCIP_CALL( SCIPaddVarLocksType( scip, data->alpha[ i ], locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		}
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			SCIP_CALL( SCIPaddVarLocksType( scip, data->beta[ i ], locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		}
		SCIP_CALL( SCIPaddVarLocksType( scip, data->weight, locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE include_linear_injection_walsh_conshdlr( SCIP* scip )
	{
		if ( SCIPfindConshdlr( scip, LINEAR_INJECTION_WALSH_CONSHDLR_NAME ) != nullptr )
			return SCIP_OKAY;
		SCIP_CONSHDLR* conshdlr = nullptr;
		SCIP_CALL( SCIPincludeConshdlrBasic(
			scip,
			&conshdlr,
			LINEAR_INJECTION_WALSH_CONSHDLR_NAME,
			"NeoAlzette joint quadratic injection linear Walsh support/rank constraint",
			-100000,
			-100000,
			1,
			TRUE,
			consEnfolpLinearInjectionWalsh,
			consEnfopsLinearInjectionWalsh,
			consCheckLinearInjectionWalsh,
			consLockLinearInjectionWalsh,
			nullptr ) );
		SCIP_CALL( SCIPsetConshdlrDelete( scip, conshdlr, consDeleteLinearInjectionWalsh ) );
		SCIP_CALL( SCIPsetConshdlrTrans( scip, conshdlr, consTransLinearInjectionWalsh ) );
		SCIP_CALL( SCIPsetConshdlrSepa( scip, conshdlr, consSepalpLinearInjectionWalsh, consSepasolLinearInjectionWalsh, 1, -100000, TRUE ) );
		SCIP_CALL( SCIPsetConshdlrProp( scip, conshdlr, consPropLinearInjectionWalsh, 1, FALSE, SCIP_PROPTIMING_ALWAYS ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_linear_injection_walsh_constraint(
		SCIP* scip,
		const std::string& name,
		InjectionKind kind,
		const BitVec& alpha,
		const BitVec& beta,
		const SVar& weight )
	{
		SCIP_CONSHDLR* conshdlr = SCIPfindConshdlr( scip, LINEAR_INJECTION_WALSH_CONSHDLR_NAME );
		if ( conshdlr == nullptr )
			return SCIP_INVALIDCALL;
		if ( alpha.size() != WORD_SIZE || beta.size() != JOINT_INJECTION_BETA_SIZE || weight.var == nullptr )
			return SCIP_INVALIDCALL;
		auto* data = new LinearInjectionWalshConsData();
		data->kind = kind;
		data->weight = weight.var;
		data->name = name;
		for ( int i = 0; i < WORD_SIZE; ++i )
		{
			data->alpha[ i ] = alpha[ i ].var;
			SCIP_CALL( SCIPchgVarBranchPriority( scip, data->alpha[ i ], 100000 ) );
		}
		for ( int i = 0; i < JOINT_INJECTION_BETA_SIZE; ++i )
		{
			data->beta[ i ] = beta[ i ].var;
			SCIP_CALL( SCIPchgVarBranchPriority( scip, data->beta[ i ], 200000 ) );
		}
		SCIP_CALL( capture_linear_injection_vars( scip, data ) );
		SCIP_CONS* cons = nullptr;
		SCIP_CALL( SCIPcreateCons(
			scip,
			&cons,
			( name + "_walsh_constraint" ).c_str(),
			conshdlr,
			reinterpret_cast<SCIP_CONSDATA*>( data ),
			TRUE,
			TRUE,
			TRUE,
			TRUE,
			TRUE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE ) );
		SCIP_CALL( SCIPaddCons( scip, cons ) );
		SCIP_CALL( SCIPreleaseCons( scip, &cons ) );
		SCIP_CALL( preadd_small_weight_beta_rank_bounds( scip, data ) );
		return SCIP_OKAY;
	}


	// ========================================================================
	// Audit section D: fixed-addend exact log-weight epigraph handler
	// ========================================================================
	//
	// Literature and derivation note.
	// -------------------------------
	// Miyano, "Addend Dependency of Differential/Linear Probability of
	// Addition", IEICE Trans. Fundamentals, E81-A(1), 1998, formulates LAP for
	// Y = X + K (mod 2^w) as a function of the fixed addend K.  In the notation
	// used by this backend, alpha is the input mask, beta is the output mask,
	// mu_i = alpha_i xor beta_i, nu_i = beta_i, and the carry state lambda_i is
	// scanned from low to high.
	//
	// The exact signed numerator recurrence is the 2-state row-vector transfer
	//
	//     (P_0,Q_0) = (1,0),
	//     (P_{i+1},Q_{i+1}) = (P_i,Q_i) M_{K_i,mu_i,nu_i},
	//     N = P_w + Q_w,
	//     C_K(alpha,beta) = 2^{-w} N.
	//
	// The static MILP above already compiles this transfer recurrence exactly.
	// The missing objective is the exact linear-correlation weight
	//
	//     W = -log2 |C_K(alpha,beta)| = w - log2 |N|.
	//
	// We do not approximate W by a ladder and we do not run an outer numerator
	// sweep.  Instead we bind A = |N| exactly, add W as a SCIP objective variable,
	// and enforce the convex epigraph of f(A)=w-log2(A) lazily by tangent cuts:
	//
	//     W >= f(a) + f'(a)(A-a)
	//       = -(1/(a ln 2)) A + w - log2(a) + 1/ln 2.
	//
	// Whenever an integer solution proposes a concrete A, the tangent at that A
	// is exact, so minimization forces W=f(A).  This gives a strict single-trail
	// MILP/CIP objective directly, without an outer numerator-threshold sweep.


	struct FixedAddendLogWeightConsData
	{
		SCIP_VAR* absolute_numerator = nullptr;
		SCIP_VAR* weight = nullptr;
		int bit_count = 0;
		std::string name;
		std::set<std::uint64_t> tangent_seen;
	};

	static constexpr const char* FIXED_ADDEND_LOG_WEIGHT_CONSHDLR_NAME = "fixed_addend_exact_log_weight";

	[[nodiscard]] static double fixed_addend_exact_log_weight_value( int bit_count, double absolute_numerator )
	{
		if ( absolute_numerator < 1.0 )
			return std::numeric_limits<double>::infinity();
		return static_cast<double>( bit_count ) - std::log2( absolute_numerator );
	}

	static SCIP_RETCODE capture_fixed_addend_log_weight_vars( SCIP* scip, FixedAddendLogWeightConsData* data )
	{
		SCIP_CALL( SCIPcaptureVar( scip, data->absolute_numerator ) );
		SCIP_CALL( SCIPcaptureVar( scip, data->weight ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE release_fixed_addend_log_weight_vars( SCIP* scip, FixedAddendLogWeightConsData* data )
	{
		if ( data->absolute_numerator != nullptr )
			SCIP_CALL( SCIPreleaseVar( scip, &data->absolute_numerator ) );
		if ( data->weight != nullptr )
			SCIP_CALL( SCIPreleaseVar( scip, &data->weight ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_fixed_addend_log_weight_tangent_cut(
		SCIP* scip,
		FixedAddendLogWeightConsData* data,
		std::uint64_t tangent_absolute_numerator,
		bool& added )
	{
		added = false;
		if ( tangent_absolute_numerator == 0u )
			tangent_absolute_numerator = 1u;
		const std::uint64_t maximum = std::uint64_t( 1 ) << data->bit_count;
		if ( tangent_absolute_numerator > maximum )
			tangent_absolute_numerator = maximum;
		if ( !data->tangent_seen.insert( tangent_absolute_numerator ).second )
			return SCIP_OKAY;

		constexpr double inverse_log_two = 1.442695040888963407359924681001892137;
		const double a = static_cast<double>( tangent_absolute_numerator );
		const double slope = -inverse_log_two / a;
		const double intercept = static_cast<double>( data->bit_count ) - std::log2( a ) + inverse_log_two;

		// W >= slope*A + intercept  <=>  W - slope*A >= intercept.
		std::vector<SCIP_VAR*> vars { data->weight, data->absolute_numerator };
		std::vector<SCIP_Real> vals { 1.0, -slope };
		SCIP_CONS* cons = nullptr;
		SCIP_CALL( SCIPcreateConsBasicLinear(
			scip,
			&cons,
			( data->name + "_tangent_at_" + std::to_string( tangent_absolute_numerator ) ).c_str(),
			2,
			vars.data(),
			vals.data(),
			intercept,
			SCIPinfinity( scip ) ) );
		SCIP_CALL( SCIPaddCons( scip, cons ) );
		SCIP_CALL( SCIPreleaseCons( scip, &cons ) );
		added = true;
		return SCIP_OKAY;
	}

	static std::uint64_t fixed_addend_tangent_point_from_solution_value( int bit_count, double absolute_numerator_value )
	{
		const double maximum = std::ldexp( 1.0, bit_count );
		double clamped = absolute_numerator_value;
		if ( !std::isfinite( clamped ) )
			clamped = 1.0;
		clamped = std::max( 1.0, std::min( maximum, clamped ) );
		return static_cast<std::uint64_t>( std::llround( clamped ) );
	}

	static SCIP_RETCODE separate_fixed_addend_log_weight(
		SCIP* scip,
		SCIP_SOL* sol,
		FixedAddendLogWeightConsData* data,
		SCIP_RESULT* result )
	{
		// Add tangents near the current integer/LP numerator. The center cut is
		// exact when A is integral; neighbor cuts reduce numerical wobble around
		// adjacent numerator values without changing the epigraph.
		const double absolute_numerator_value = SCIPgetSolVal( scip, sol, data->absolute_numerator );
		const double weight_value = SCIPgetSolVal( scip, sol, data->weight );
		const double required_weight = fixed_addend_exact_log_weight_value( data->bit_count, std::max( 1.0, absolute_numerator_value ) );
		if ( weight_value + 1.0e-8 >= required_weight )
			return SCIP_OKAY;

		bool added = false;
		const std::uint64_t center = fixed_addend_tangent_point_from_solution_value( data->bit_count, absolute_numerator_value );
		SCIP_CALL( add_fixed_addend_log_weight_tangent_cut( scip, data, center, added ) );
		if ( center > 1u )
		{
			bool local_added = false;
			SCIP_CALL( add_fixed_addend_log_weight_tangent_cut( scip, data, center - 1u, local_added ) );
			added = added || local_added;
		}
		const std::uint64_t maximum = std::uint64_t( 1 ) << data->bit_count;
		if ( center < maximum )
		{
			bool local_added = false;
			SCIP_CALL( add_fixed_addend_log_weight_tangent_cut( scip, data, center + 1u, local_added ) );
			added = added || local_added;
		}
		if ( added )
			*result = SCIP_CONSADDED;
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSDELETE(consDeleteFixedAddendLogWeight)
	{
		(void)conshdlr;
		if ( consdata != nullptr && *consdata != nullptr )
		{
			auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( *consdata );
			SCIP_CALL( release_fixed_addend_log_weight_vars( scip, data ) );
			delete data;
			*consdata = nullptr;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSTRANS(consTransFixedAddendLogWeight)
	{
		auto* source = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( sourcecons ) );
		auto* target = new FixedAddendLogWeightConsData();
		target->bit_count = source->bit_count;
		target->name = source->name;
		target->tangent_seen = source->tangent_seen;
		SCIP_CALL( SCIPgetTransformedVar( scip, source->absolute_numerator, &target->absolute_numerator ) );
		SCIP_CALL( SCIPgetTransformedVar( scip, source->weight, &target->weight ) );
		SCIP_CALL( capture_fixed_addend_log_weight_vars( scip, target ) );
		SCIP_CALL( SCIPcreateCons(
			scip,
			targetcons,
			SCIPconsGetName( sourcecons ),
			conshdlr,
			reinterpret_cast<SCIP_CONSDATA*>( target ),
			SCIPconsIsInitial( sourcecons ),
			SCIPconsIsSeparated( sourcecons ),
			SCIPconsIsEnforced( sourcecons ),
			SCIPconsIsChecked( sourcecons ),
			SCIPconsIsPropagated( sourcecons ),
			SCIPconsIsLocal( sourcecons ),
			SCIPconsIsModifiable( sourcecons ),
			SCIPconsIsDynamic( sourcecons ),
			SCIPconsIsRemovable( sourcecons ),
			SCIPconsIsStickingAtNode( sourcecons ) ) );
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSENFOLP(consEnfolpFixedAddendLogWeight)
	{
		(void)conshdlr;
		(void)nusefulconss;
		(void)solinfeasible;
		*result = SCIP_FEASIBLE;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_fixed_addend_log_weight( scip, nullptr, data, result ) );
			if ( *result != SCIP_FEASIBLE )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSENFOPS(consEnfopsFixedAddendLogWeight)
	{
		(void)conshdlr;
		(void)nusefulconss;
		(void)solinfeasible;
		(void)objinfeasible;
		*result = SCIP_FEASIBLE;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_fixed_addend_log_weight( scip, nullptr, data, result ) );
			if ( *result != SCIP_FEASIBLE )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSSEPALP(consSepalpFixedAddendLogWeight)
	{
		(void)conshdlr;
		(void)nusefulconss;
		*result = SCIP_DIDNOTFIND;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_fixed_addend_log_weight( scip, nullptr, data, result ) );
			if ( *result == SCIP_CONSADDED )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSSEPASOL(consSepasolFixedAddendLogWeight)
	{
		(void)conshdlr;
		(void)nusefulconss;
		*result = SCIP_DIDNOTFIND;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( conss[ c ] ) );
			SCIP_CALL( separate_fixed_addend_log_weight( scip, sol, data, result ) );
			if ( *result == SCIP_CONSADDED )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSCHECK(consCheckFixedAddendLogWeight)
	{
		(void)conshdlr;
		(void)checkintegrality;
		(void)checklprows;
		(void)printreason;
		(void)completely;
		*result = SCIP_FEASIBLE;
		for ( int c = 0; c < nconss; ++c )
		{
			auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( conss[ c ] ) );
			const double absolute_numerator_value = SCIPgetSolVal( scip, sol, data->absolute_numerator );
			const double weight_value = SCIPgetSolVal( scip, sol, data->weight );
			if ( !SCIPisFeasIntegral( scip, absolute_numerator_value ) )
			{
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
			const double required_weight = fixed_addend_exact_log_weight_value( data->bit_count, std::max( 1.0, absolute_numerator_value ) );
			if ( weight_value + 1.0e-7 < required_weight )
			{
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSLOCK(consLockFixedAddendLogWeight)
	{
		auto* data = reinterpret_cast<FixedAddendLogWeightConsData*>( SCIPconsGetData( cons ) );
		SCIP_CALL( SCIPaddVarLocksType( scip, data->absolute_numerator, locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		SCIP_CALL( SCIPaddVarLocksType( scip, data->weight, locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE include_fixed_addend_log_weight_conshdlr( SCIP* scip )
	{
		if ( SCIPfindConshdlr( scip, FIXED_ADDEND_LOG_WEIGHT_CONSHDLR_NAME ) != nullptr )
			return SCIP_OKAY;
		SCIP_CONSHDLR* conshdlr = nullptr;
		SCIP_CALL( SCIPincludeConshdlrBasic(
			scip,
			&conshdlr,
			FIXED_ADDEND_LOG_WEIGHT_CONSHDLR_NAME,
			"strict fixed-addend exact log-weight epigraph W >= n-log2(|N|)",
			-90000,
			-90000,
			1,
			TRUE,
			consEnfolpFixedAddendLogWeight,
			consEnfopsFixedAddendLogWeight,
			consCheckFixedAddendLogWeight,
			consLockFixedAddendLogWeight,
			nullptr ) );
		SCIP_CALL( SCIPsetConshdlrDelete( scip, conshdlr, consDeleteFixedAddendLogWeight ) );
		SCIP_CALL( SCIPsetConshdlrTrans( scip, conshdlr, consTransFixedAddendLogWeight ) );
		SCIP_CALL( SCIPsetConshdlrSepa( scip, conshdlr, consSepalpFixedAddendLogWeight, consSepasolFixedAddendLogWeight, 1, -90000, TRUE ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_fixed_addend_log_weight_epigraph_constraint(
		SCIP* scip,
		const std::string& name,
		const SVar& absolute_numerator,
		const SVar& weight,
		int bit_count )
	{
		SCIP_CONSHDLR* conshdlr = SCIPfindConshdlr( scip, FIXED_ADDEND_LOG_WEIGHT_CONSHDLR_NAME );
		if ( conshdlr == nullptr )
			return SCIP_INVALIDCALL;
		auto* data = new FixedAddendLogWeightConsData();
		data->absolute_numerator = absolute_numerator.var;
		data->weight = weight.var;
		data->bit_count = bit_count;
		data->name = name;
		SCIP_CALL( capture_fixed_addend_log_weight_vars( scip, data ) );
		SCIP_CONS* cons = nullptr;
		SCIP_CALL( SCIPcreateCons(
			scip,
			&cons,
			( name + "_exact_log_weight_epigraph" ).c_str(),
			conshdlr,
			reinterpret_cast<SCIP_CONSDATA*>( data ),
			TRUE,
			TRUE,
			TRUE,
			TRUE,
			TRUE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE ) );
		SCIP_CALL( SCIPaddCons( scip, cons ) );
		SCIP_CALL( SCIPreleaseCons( scip, &cons ) );

		bool added = false;
		SCIP_CALL( add_fixed_addend_log_weight_tangent_cut( scip, data, 1u, added ) );
		bool added_max = false;
		SCIP_CALL( add_fixed_addend_log_weight_tangent_cut( scip, data, std::uint64_t( 1 ) << bit_count, added_max ) );
		(void)added;
		(void)added_max;
		return SCIP_OKAY;
	}

	template <class Builder>
	inline void add_fixed_addend_exact_log_weight_milp_objective(
		Builder& model_builder,
		arithmetic_model::FixedConstExactThresholdMilpHandle& handle,
		int bit_count,
		const std::string& prefix )
	{
		if ( bit_count <= 0 || bit_count > 32 )
			throw std::invalid_argument( "add_fixed_addend_exact_log_weight_milp_objective: bit width must be in [1, 32]" );
		if ( handle.final_signed_numerator.var == nullptr )
			return;
		const double numerator_bound = std::ldexp( 1.0, bit_count );
		const double big_m = 2.0 * numerator_bound + 2.0;

		handle.final_absolute_numerator = model_builder.add_variable( prefix + "_final_absolute_numerator", 1.0, numerator_bound, 0.0, SCIP_VARTYPE_INTEGER );
		handle.exact_log_weight = model_builder.create_continuous_variable( prefix + "_exact_log_weight", 0.0, static_cast<double>( bit_count ), 1.0 );

		const SVar& N = handle.final_signed_numerator;
		const SVar& A = handle.final_absolute_numerator;
		const SVar& z = handle.final_sign_is_negative;

		// Existing threshold constraints already force the sign branch and |N|>=1.
		// We now bind A exactly to |N|.  z=0 means positive branch, z=1 negative.
		model_builder.add_linear_constraint( prefix + "_abs_positive_upper", { { A, 1.0 }, { N, -1.0 }, { z, -big_m } }, -SCIPinfinity( model_builder.scip ), 0.0 );
		model_builder.add_linear_constraint( prefix + "_abs_positive_lower", { { A, 1.0 }, { N, -1.0 }, { z, big_m } }, 0.0, SCIPinfinity( model_builder.scip ) );
		model_builder.add_linear_constraint( prefix + "_abs_negative_upper", { { A, 1.0 }, { N, 1.0 }, { z, big_m } }, -SCIPinfinity( model_builder.scip ), big_m );
		model_builder.add_linear_constraint( prefix + "_abs_negative_lower", { { A, 1.0 }, { N, 1.0 }, { z, -big_m } }, -big_m, SCIPinfinity( model_builder.scip ) );

		SCIP_CALL_THROW( add_fixed_addend_log_weight_epigraph_constraint( model_builder.scip, prefix, A, handle.exact_log_weight, bit_count ) );
	}


	// ========================================================================
	// Audit section E: generic SCIP model-builder facade
	// ========================================================================
	// Low-level variable/constraint helpers used by the round builders. Theory
	// should stay in the operator sections above; this facade just emits SCIP API
	// objects and records objective terms for traces.
	// The fixed-addend layer is now solver-internal: the two-state numerator
	// recurrence is static MILP, A=|N| is exact MILP, and the nonlinear
	// log-weight objective is enforced by the fixed_addend_exact_log_weight
	// SCIP constraint handler above.  The oracle header remains as an offline
	// Q1 validator and trace-audit source; it is not the pricing mechanism.

	class ScipModelBuilder
	{
	public:
		SCIP*						scip = nullptr;
		std::map<std::string, SVar> var_by_name;
		std::vector<LinearTerm>		objective_terms;

		ScipModelBuilder( bool quiet, double time_limit_seconds )
		{
			SCIP_CALL_THROW( SCIPcreate( &scip ) );
			SCIP_CALL_THROW( SCIPincludeDefaultPlugins( scip ) );
			SCIP_CALL_THROW( include_linear_injection_walsh_conshdlr( scip ) );
			SCIP_CALL_THROW( include_fixed_addend_log_weight_conshdlr( scip ) );
			SCIP_CALL_THROW( SCIPcreateProbBasic( scip, "neoalzette_linear_milp" ) );
			SCIP_CALL_THROW( SCIPsetObjsense( scip, SCIP_OBJSENSE_MINIMIZE ) );
			if ( quiet )
				SCIP_CALL_THROW( SCIPsetIntParam( scip, "display/verblevel", 0 ) );
			if ( std::isfinite( time_limit_seconds ) && time_limit_seconds > 0.0 )
				SCIP_CALL_THROW( SCIPsetRealParam( scip, "limits/time", time_limit_seconds ) );
		}

		~ScipModelBuilder()
		{
			if ( scip )
				SCIPfree( &scip );
		}

		SVar add_variable( const std::string& name, double lower_bound, double upper_bound, double objective_coefficient, SCIP_VARTYPE variable_type )
		{
			SCIP_VAR* scip_variable = nullptr;
			SCIP_CALL_THROW( SCIPcreateVarBasic( scip, &scip_variable, name.c_str(), lower_bound, upper_bound, objective_coefficient, variable_type ) );
			SCIP_CALL_THROW( SCIPaddVar( scip, scip_variable ) );
			SVar symbolic_variable { scip_variable, name };
			var_by_name[ name ] = symbolic_variable;
			if ( std::fabs( objective_coefficient ) > 1e-15 )
				objective_terms.push_back( { symbolic_variable, objective_coefficient } );
			SCIP_CALL_THROW( SCIPreleaseVar( scip, &scip_variable ) );
			return symbolic_variable;
		}

		SVar create_binary_variable( const std::string& name, double objective_coefficient = 0.0 )
		{
			return add_variable( name, 0.0, 1.0, objective_coefficient, SCIP_VARTYPE_BINARY );
		}

		SVar create_integer_variable( const std::string& name, int lower_bound, int upper_bound )
		{
			return add_variable( name, lower_bound, upper_bound, 0.0, SCIP_VARTYPE_INTEGER );
		}

		SVar create_integer_variable( const std::string& name, double lower_bound, double upper_bound, double objective_coefficient = 0.0 )
		{
			return add_variable( name, lower_bound, upper_bound, objective_coefficient, SCIP_VARTYPE_INTEGER );
		}

		SVar create_continuous_variable( const std::string& name, double lower_bound, double upper_bound, double objective_coefficient = 0.0 )
		{
			return add_variable( name, lower_bound, upper_bound, objective_coefficient, SCIP_VARTYPE_CONTINUOUS );
		}

		BitVec create_bit_vector( const std::string& prefix, int bit_count = WORD_SIZE, double objective_coefficient = 0.0 )
		{
			BitVec bit_vector;
			bit_vector.reserve( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				bit_vector.push_back( create_binary_variable( prefix + "_" + std::to_string( bit_index ), objective_coefficient ) );
			return bit_vector;
		}

		void add_linear_constraint( const std::string& name, const std::vector<LinearTerm>& terms, double left_hand_side, double right_hand_side )
		{
			SCIP_CONS*			   scip_constraint = nullptr;
			std::vector<SCIP_VAR*> scip_variables;
			std::vector<SCIP_Real> coefficients;
			scip_variables.reserve( terms.size() );
			coefficients.reserve( terms.size() );
			for ( const auto& term : terms )
			{
				scip_variables.push_back( term.v.var );
				coefficients.push_back( term.c );
			}
			SCIP_CALL_THROW( SCIPcreateConsBasicLinear( scip, &scip_constraint, name.c_str(), static_cast<int>( scip_variables.size() ), scip_variables.data(), coefficients.data(), left_hand_side, right_hand_side ) );
			SCIP_CALL_THROW( SCIPaddCons( scip, scip_constraint ) );
			SCIP_CALL_THROW( SCIPreleaseCons( scip, &scip_constraint ) );
		}

		void add_zero_equality_constraint( const std::string& name, const std::vector<LinearTerm>& terms )
		{
			add_linear_constraint( name, terms, 0.0, 0.0 );
		}

		void add_constant_equality_constraint( const std::string& name, const std::vector<LinearTerm>& terms, double value )
		{
			add_linear_constraint( name, terms, value, value );
		}

		void add_two_input_xor_constraint( const SVar& first_input, const SVar& second_input, const SVar& output, const std::string& name )
		{
			SVar parity_quotient = create_binary_variable( name + "_parity_quotient" );
			add_zero_equality_constraint( name, { { first_input, 1 }, { second_input, 1 }, { output, 1 }, { parity_quotient, -2 } } );
		}

		BitVec create_xor_vector( const BitVec& first_input, const BitVec& second_input, const std::string& prefix )
		{
			if ( first_input.size() != second_input.size() )
				throw std::invalid_argument( "create_xor_vector: masks must have the same width" );
			BitVec output = create_bit_vector( prefix, static_cast<int>( first_input.size() ) );
			for ( int bit_index = 0; bit_index < static_cast<int>( first_input.size() ); ++bit_index )
				add_two_input_xor_constraint( first_input[ bit_index ], second_input[ bit_index ], output[ bit_index ], prefix + "_xor_" + std::to_string( bit_index ) );
			return output;
		}

		void add_vector_equality_constraints( const BitVec& first_input, const BitVec& second_input, const std::string& prefix )
		{
			if ( first_input.size() != second_input.size() )
				throw std::invalid_argument( "add_vector_equality_constraints: masks must have the same width" );
			for ( int bit_index = 0; bit_index < static_cast<int>( first_input.size() ); ++bit_index )
				add_zero_equality_constraint( prefix + "_eq_" + std::to_string( bit_index ), { { first_input[ bit_index ], 1 }, { second_input[ bit_index ], -1 } } );
		}

		void add_xor_vector_equality_constraints( const BitVec& output, const BitVec& first_input, const BitVec& second_input, const std::string& prefix )
		{
			BitVec temporary_xor_output = create_xor_vector( first_input, second_input, prefix + "_temporary_xor_output" );
			add_vector_equality_constraints( output, temporary_xor_output, prefix + "_out" );
		}

		void add_objective_bound_constraint( const std::string& name, double left_hand_side, double right_hand_side )
		{
			add_linear_constraint( name, objective_terms, left_hand_side, right_hand_side );
		}

		static BitVec rotate_left( const BitVec& input, int rotation_amount )
		{
			const int bit_count = static_cast<int>( input.size() );
			if ( bit_count <= 0 )
				throw std::invalid_argument( "rotate_left: bit vector must be nonempty" );
			rotation_amount = ( rotation_amount % bit_count + bit_count ) % bit_count;
			BitVec output( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				output[ bit_index ] = input[ ( bit_index - rotation_amount + bit_count ) % bit_count ];
			return output;
		}

		static BitVec rotate_right( const BitVec& input, int rotation_amount )
		{
			const int bit_count = static_cast<int>( input.size() );
			if ( bit_count <= 0 )
				throw std::invalid_argument( "rotate_right: bit vector must be nonempty" );
			rotation_amount = ( rotation_amount % bit_count + bit_count ) % bit_count;
			BitVec output( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				output[ bit_index ] = input[ ( bit_index + rotation_amount ) % bit_count ];
			return output;
		}
	};


}  // namespace neoalzette_linear_milp
