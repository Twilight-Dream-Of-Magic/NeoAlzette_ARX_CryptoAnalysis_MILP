#pragma once
#include <scip/pub_tree.h>
#include <scip/scipdefplugins.h>

#include <scip/scip.h>

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
#include <map>
#include <limits>
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
// NeoAlzette XOR-differential modular add/sub models and reference oracles
// ============================================================================
//
// This header is the single local home for the modular addition/subtraction
// differential math used by neoalzette_scip_round_milp_search.cpp / neoalzette_scip_round_hull_search.cpp.
//
// Scope:
//   * XOR differences only.
//   * Little-endian bit order: bit 0 is the least significant bit.
//   * Word size n is in [1, 32].
//   * Arithmetic is modulo 2^n.
//
// Bibliographic lineage and code mapping, with searchable paper data:
//   * Two-variable XOR-differential probability of modular addition:
//	   Helger Lipmaa and Shiho Moriai,
//	   "Efficient Algorithms for Computing Differential Properties of Addition",
//	   FSE 2001, LNCS 2355, pp. 336-350, 2002,
//	   DOI 10.1007/3-540-45473-X_28, IACR ePrint 2001/001.
//	 They define xdp+(alpha,beta -> gamma) for z = x boxplus y and reduce it
//	 to carry-difference conditions.  Their "good differential" feasibility
//	 rule is implemented by oracle_add2() and add_two_input_add_diff().
//   * Two-variable modular subtraction:
//	   This file uses the algebraic rewrite x - y = z  <=>  z + y = x.
//	   It is the same add/sub permutation used in ARX addition/subtraction
//	   treatments, e.g. Johan Wallen,
//	   "Linear Approximations of Addition Modulo 2^n",
//	   FSE 2003, LNCS 2887, pp. 261-273,
//	   DOI 10.1007/978-3-540-39887-5_20.
//	 The code mapping is oracle_sub2() and add_two_input_sub_diff(), both
//	 implemented as add(gamma,beta -> alpha).
//   * Fixed-addend / one public constant modular addition probability:
//	   Hiroshi Miyano,
//	   "Addend Dependency of Differential/Linear Probability of Addition",
//	   IEICE Transactions on Fundamentals E81-A(1), pp. 106-109, Jan. 1998.
//	   Alexis Warner Machado,
//	   "Differential Probability of Modular Addition with a Constant Operand",
//	   IACR ePrint 2001/052.
//	   Seyyed Arash Azimi, Adrian Ranea, Mahmoud Salmasizadeh, Javad Mohajeri,
//	   Mohammad Reza Aref, and Vincent Rijmen,
//	   "A Bit-Vector Differential Model for the Modular Addition by a Constant",
//	   ASIACRYPT 2020, LNCS 12491, pp. 385-414,
//	   DOI 10.1007/978-3-030-64837-4_13; extended journal version:
//	   "A bit-vector differential model for the modular addition by a constant
//	   and its applications to differential and impossible-differential
//	   cryptanalysis", Designs, Codes and Cryptography 90, pp. 1797-1855,
//	   2022, DOI 10.1007/s10623-022-01074-8, IACR ePrint 2022/512.
//	 These are the sources for oracle_add_const()/oracle_sub_const() and for
//	 the exact fixed-public-constant model add_fixed_public_constant_exact().
//   * Zero-difference operand / one-constant-input compact MILP comparison model:
//	   Elnaz Bagherzadeh and Zahra Ahmadian,
//	   "MILP-based automatic differential search for LEA and HIGHT block
//	   ciphers", IET Information Security 14(5), pp. 595-603, 2020,
//	   DOI 10.1049/iet-ifs.2018.5539.  The preprint title is
//	   "MILP-Based Automatic Differential Searches for LEA and HIGHT",
//	   IACR ePrint 2018/948.
//	 Search terms in that paper: "modular addition with one constant input",
//	 "one constant input", and "differential property of modular addition".
//	 This is the source for add_zero_diff_operand_average().  It deliberately
//	 ignores concrete public-constant bits, so it is only an optional comparison
//	 mode and is not the exact fixed-public-constant add/sub model.
//   * The exact-log fixed-public-constant MILP selector used here is an
//	 engineering MILP reformulation of the exact Machado/Azimi recurrence.  It
//	 is not the truncated BvWeight/apxlog2 approximation, and it is not stated
//	 verbatim as a MILP in the cited bit-vector/SMT paper.
//
// Important distinction:
//   * True two-variable add/sub:
//	   z = x boxplus y, or z = x boxminus y.
//	   Both input XOR differences are variables.  The local weight is an
//	   integer Hamming weight.
//   * Fixed-public-constant add/sub:
//	   y = x boxplus a, or y = x boxminus a, with public constant a.
//	   The concrete bits of a affect the probability.  The exact objective can
//	   contain real coefficients of the form -log2(p), so zero-chain selectors
//	   and precomputed exact-log coefficients are required.
//
// ============================================================================
// 1. Two-variable modular addition oracle
// ----------------------------------------------------------------------------
// Local transition:
//	 z = x boxplus y mod 2^n
//	 alpha = x xor x'
//	 beta  = y xor y'
//	 gamma = z xor z'
//
// Probability:
//	 DP+(alpha,beta -> gamma)
//	   = Pr_{x,y}[(x+y) xor ((x xor alpha)+(y xor beta)) = gamma].
//
// Define:
//	 t_i = alpha_i xor beta_i xor gamma_i
//	 alpha[-1] = beta[-1] = gamma[-1] = 0.
//
// Feasibility:
//	 for every i in [0,n-1],
//	 if alpha[i-1] = beta[i-1] = gamma[i-1], then
//		 t_i = alpha[i-1].
//
// Equivalent impossible condition:
//	 some i has alpha[i-1] = beta[i-1] = gamma[i-1] and
//	 alpha[i-1] != alpha_i xor beta_i xor gamma_i.
//
// If feasible:
//	 DP+ = 2^-W,
//	 W = sum_{i=0}^{n-2} [ (alpha_i,beta_i,gamma_i) not in {000,111} ].
//
// MILP:
//   Keep the zero-carry lowest-bit condition alpha_0 xor beta_0 = gamma_0
//   explicitly.  For each adjacent transition i -> i+1, create p_i and add
//   the thirteen Fu-Wang-Guo-Sun inequalities used for Speck-style ARX MILP
//   searches, also reproduced as formula (11) in the local Chinese S-box
//   paper.  The tuple is
//	   (alpha_i, beta_i, gamma_i, alpha_{i+1}, beta_{i+1}, gamma_{i+1}, p_i).
//   Here p_i is the differential weight bit:
//	   p_i = 0 for triples 000 and 111,
//	   p_i = 1 otherwise.
//   Objective:
//	   minimize sum_{i=0}^{n-2} p_i.
//
// ============================================================================
// 2. Two-variable modular subtraction oracle and MILP
// ----------------------------------------------------------------------------
// Local transition:
//	 z = x boxminus y mod 2^n, i.e. z = x - y.
//
// Rewrite:
//	 x - y = z  <=>  z + y = x.
//
// Therefore:
//	 DP-(alpha,beta -> gamma) = DP+(gamma,beta -> alpha),
//	 W-(alpha,beta -> gamma)  = W+(gamma,beta -> alpha).
//
// The MILP is the same addition MILP with
//	 alpha_add = gamma,
//	 beta_add  = beta,
//	 gamma_add = alpha.
//
// ============================================================================
// 3. Fixed-public-constant modular addition oracle
// ----------------------------------------------------------------------------
// Local transition:
//	 y = x boxplus a mod 2^n,
//	 u = x xor x',
//	 v = y xor y'.
//
// Probability:
//	 DP_a(u -> v)
//	   = Pr_x[(x+a) xor ((x xor u)+a) = v].
//
// Define:
//	 u[-1] = v[-1] = a[-1] = 0,
//	 e_i = u_i xor v_i,
//	 S_i = (u[i-1], v[i-1], e_i).
//
// State 001 is impossible.
//
// Base weight:
//	 sum_{i=1}^{n-1} [u[i-1] xor v[i-1] = 1],
//	 i.e. states 01* and 10* contribute 1.
//
// Chain states:
//	 For every S_i = 11*, i >= 1, let lambda_i be one plus the number of
//	 immediately preceding 000 states:
//		 lambda = 1;
//		 j = i - 1;
//		 while j >= 0 and S_j == 000: ++lambda, --j.
//
//	 q_i = u_i xor v_i xor a[i-1].
//
//	 B_{i,lambda}
//	   = a[i-2 : i-lambda] + a[i-lambda-1],
//	 where out-of-range constant bits are 0 and the slice keeps the original
//	 low-bit origin.
//
//	 p_{i,lambda,q}
//	   = B_{i,lambda}				  if q = 1,
//		 2^(lambda-1) - B_{i,lambda}   if q = 0.
//
//	 If p = 0, the transition is impossible.  Otherwise the chain contributes
//		 (lambda-1) - log2(p)
//	 to the weight.
//
// ============================================================================
// 4. Fixed-public-constant modular addition MILP
// ----------------------------------------------------------------------------
// Variables:
//	 u_i, v_i in {0,1}
//	 e_i = u_i xor v_i
//	 s_{r,i} in {0,1}, r in {0,...,7}, one-hot for S_i
//	 z_{i,lambda,q} in {0,1} for chain selection
//
// XOR constraints:
//	 e_i = u_i xor v_i.
//
// State one-hot and binding:
//	 sum_r s_{r,i} = 1,
//	 u[i-1] = s4_i+s5_i+s6_i+s7_i,
//	 v[i-1] = s2_i+s3_i+s6_i+s7_i,
//	 e_i	= s1_i+s3_i+s5_i+s7_i.
// For i=0, u[-1]=v[-1]=0, so the same binding forces states with first or
// second state bit 1 to zero.
//
// Forbid:
//	 s1_i = 0.
//
// Base objective:
//	 b_i = s2_i+s3_i+s4_i+s5_i, i=1..n-1.
//
// Chain trigger:
//	 sum_{lambda=1}^{i+1} sum_{q=0}^1 z_{i,lambda,q} = s6_i+s7_i.
//
// q binding:
//	 if a[i-1] = 0:
//		 sum_lambda z_{i,lambda,1} = s7_i,
//		 sum_lambda z_{i,lambda,0} = s6_i.
//	 if a[i-1] = 1:
//		 sum_lambda z_{i,lambda,1} = s6_i,
//		 sum_lambda z_{i,lambda,0} = s7_i.
//
// Exact zero-chain length:
//	 z_{i,lambda,q} <= s0_{i-k}	  for k=1..lambda-1,
//	 z_{i,lambda,q} <= 1-s0_{i-lambda} if i-lambda >= 0.
// No boundary constraint is added when i-lambda < 0.
//
// p=0:
//	 If precomputed p_{i,lambda,q}=0, no selector z_{i,lambda,q}
//	 is created. This exact-preserving sparse construction avoids zero-fixed
//	 selector variables and their chain constraints.
//
// Branching / presolve tuning:
//	 The impossible state 001 is created as a fixed-zero binary variable instead
//	 of a normal binary plus an equality constraint.  The e, state, and selector
//	 variables receive high branch priorities so SCIP branches near the fixed-addend
//	 automaton before spending effort on less informative surrounding variables.
//	 These changes do not alter the feasible set or the objective.
//
// Objective:
//	 minimize
//	   sum_{i=1}^{n-1} (s2_i+s3_i+s4_i+s5_i)
//	   + sum_{i=1}^{n-1} sum_lambda sum_q
//		   ((lambda-1)-log2(p_{i,lambda,q})) z_{i,lambda,q}.
//
// ============================================================================
// 5. Fixed-public-constant modular subtraction
// ----------------------------------------------------------------------------
//	 x boxminus a == x boxplus (-a mod 2^n) mod 2^n.
//
// Therefore fixed-constant subtraction reuses the same oracle and MILP after
// replacing a by (-a mod 2^n).
// ============================================================================


namespace neoalzette_diff_milp::differential_oracle
{
	// ========================================================================
	// Audit section A: reference Q1 oracles for modular add/sub
	// ========================================================================
	// These functions evaluate exact local probabilities for trace validation.
	// The SCIP-facing model builders start in arithmetic_model below.
	struct Add2DiffOracleResult
	{
		bool		  possible = false;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
	};

	using Sub2DiffOracleResult = Add2DiffOracleResult;

	struct FixedAddendDiffOracleResult
	{
		bool		possible = false;
		long double weight = std::numeric_limits<long double>::infinity();
	};

	using FixedSubendDiffOracleResult = FixedAddendDiffOracleResult;

	[[nodiscard]] inline std::uint32_t mask_for_bits( int bits )
	{
		if ( bits <= 0 )
			return 0u;
		if ( bits >= 32 )
			return 0xFFFFFFFFu;
		return ( std::uint32_t( 1 ) << bits ) - 1u;
	}

	[[nodiscard]] inline int word_bit( std::uint32_t x, int i )
	{
		if ( i < 0 || i >= 32 )
			return 0;
		return static_cast<int>( ( x >> i ) & 1u );
	}

	[[nodiscard]] inline Add2DiffOracleResult oracle_add2( std::uint32_t alpha, std::uint32_t beta, std::uint32_t gamma, int bits )
	{
		if ( bits <= 0 || bits > 32 )
			return {};

		const std::uint32_t mask = mask_for_bits( bits );
		alpha &= mask;
		beta &= mask;
		gamma &= mask;

		for ( int i = 0; i < bits; ++i )
		{
			const int prev_a = ( i == 0 ) ? 0 : word_bit( alpha, i - 1 );
			const int prev_b = ( i == 0 ) ? 0 : word_bit( beta, i - 1 );
			const int prev_g = ( i == 0 ) ? 0 : word_bit( gamma, i - 1 );
			const int t = word_bit( alpha, i ) ^ word_bit( beta, i ) ^ word_bit( gamma, i );

			if ( prev_a == prev_b && prev_b == prev_g && t != prev_a )
				return {};
		}

		std::uint32_t weight = 0;
		for ( int i = 0; i < bits - 1; ++i )
		{
			const int a = word_bit( alpha, i );
			const int b = word_bit( beta, i );
			const int g = word_bit( gamma, i );
			if ( !( a == b && b == g ) )
				++weight;
		}

		return { true, weight };
	}

	[[nodiscard]] inline Sub2DiffOracleResult oracle_sub2( std::uint32_t alpha, std::uint32_t beta, std::uint32_t gamma, int bits )
	{
		return oracle_add2( gamma, beta, alpha, bits );
	}

	[[nodiscard]] inline std::uint64_t constant_slice_value_with_origin( std::uint32_t constant, int high, int low )
	{
		if ( high < low )
			return 0;

		std::uint64_t value = 0;
		const int	  first_real_position = ( low > 0 ) ? low : 0;
		for ( int position = first_real_position; position <= high; ++position )
		{
			if ( word_bit( constant, position ) != 0 )
				value += ( std::uint64_t( 1 ) << ( position - low ) );
		}
		return value;
	}

	[[nodiscard]] inline int fixed_addend_state( std::uint32_t u, std::uint32_t v, int i )
	{
		const int previous_u = word_bit( u, i - 1 );
		const int previous_v = word_bit( v, i - 1 );
		const int e = word_bit( u, i ) ^ word_bit( v, i );
		return ( previous_u << 2 ) | ( previous_v << 1 ) | e;
	}

	[[nodiscard]] inline std::uint64_t fixed_addend_block_value( std::uint32_t constant, int i, int lambda )
	{
		return constant_slice_value_with_origin( constant, i - 2, i - lambda ) + static_cast<std::uint64_t>( word_bit( constant, i - lambda - 1 ) );
	}

	[[nodiscard]] inline std::uint64_t fixed_addend_chain_probability_numerator( std::uint32_t constant, int i, int lambda, int q )
	{
		// Azimi/Machado fixed-addend zero-chain numerator. For a trigger state
		// S_i = 11*, lambda is the length of the immediately preceding zero run
		// plus the trigger bit. q chooses which side of the public-constant block
		// is counted. p=0 is a true impossible branch, not an infinite penalty.
		const std::uint64_t B = fixed_addend_block_value( constant, i, lambda );
		const std::uint64_t denominator = std::uint64_t( 1 ) << ( lambda - 1 );
		return ( q != 0 ) ? B : ( denominator - B );
	}

	[[nodiscard]] inline FixedAddendDiffOracleResult oracle_add_const( std::uint32_t constant, std::uint32_t u, std::uint32_t v, int bits )
	{
		if ( bits <= 0 || bits > 32 )
			return {};

		const std::uint32_t mask = mask_for_bits( bits );
		constant &= mask;
		u &= mask;
		v &= mask;

		long double weight = 0.0L;
		for ( int i = 0; i < bits; ++i )
		{
			const int state = fixed_addend_state( u, v, i );
			if ( state == 0b001 )
				return {};

			if ( i >= 1 && ( word_bit( u, i - 1 ) ^ word_bit( v, i - 1 ) ) != 0 )
				weight += 1.0L;

			if ( i >= 1 && ( ( state >> 1 ) == 0b11 ) )
			{
				int lambda = 1;
				int j = i - 1;
				while ( j >= 0 && fixed_addend_state( u, v, j ) == 0b000 )
				{
					++lambda;
					--j;
				}

				const int			q = word_bit( u, i ) ^ word_bit( v, i ) ^ word_bit( constant, i - 1 );
				const std::uint64_t p = fixed_addend_chain_probability_numerator( constant, i, lambda, q );
				if ( p == 0 )
					return {};

				weight += static_cast<long double>( lambda - 1 ) - std::log2( static_cast<long double>( p ) );
			}
		}

		return { true, weight };
	}

	[[nodiscard]] inline FixedSubendDiffOracleResult oracle_sub_const( std::uint32_t constant, std::uint32_t u, std::uint32_t v, int bits )
	{
		const std::uint32_t mask = mask_for_bits( bits );
		const std::uint32_t neg_constant = ( std::uint32_t( 0 ) - ( constant & mask ) ) & mask;
		return oracle_add_const( neg_constant, u, v, bits );
	}
}  // namespace neoalzette_diff_milp::differential_oracle


// ============================================================================
// NeoAlzette XOR-differential modular add/sub MILP constraints
// ============================================================================
//
// This header contains the solver-facing MILP/CIP construction helpers for the
// operator models documented in neoalzette_scip_operator_analysis_oracle.hpp.
// The arithmetic rules, bibliographic lineage, and reference oracles were
// copied there before the legacy combined header was removed.
// ============================================================================

namespace neoalzette_diff_milp::arithmetic_model
{
	// ========================================================================
	// Audit section B: shared fixed-addend helper functions
	// ========================================================================
	// These small helpers are used by both the exact oracle and the exact MILP
	// selector construction.
	[[nodiscard]] inline int bit_at( std::uint32_t x, int i )
	{
		return differential_oracle::word_bit( x, i );
	}

	[[nodiscard]] inline std::string state_name( int s )
	{
		std::string r;
		r.push_back( ( ( s >> 2 ) & 1 ) ? '1' : '0' );
		r.push_back( ( ( s >> 1 ) & 1 ) ? '1' : '0' );
		r.push_back( ( s & 1 ) ? '1' : '0' );
		return r;
	}

	[[nodiscard]] inline std::uint64_t constant_slice_value_with_origin( std::uint32_t constant, int high, int low )
	{
		return differential_oracle::constant_slice_value_with_origin( constant, high, low );
	}

	[[nodiscard]] inline std::uint64_t chain_probability_numerator( std::uint32_t constant, int i, int lambda, int q )
	{
		return differential_oracle::fixed_addend_chain_probability_numerator( constant, i, lambda, q );
	}

	[[nodiscard]] inline double chain_exact_log_weight_coefficient( std::uint32_t constant, int i, int lambda, int q )
	{
		// The MILP objective uses the exact real coefficient
		// (lambda-1)-log2(p_i). This is the exact fixed-public-constant model,
		// not the Bagherzadeh-Ahmadian average constant-input model and not
		// Azimi's truncated fixed-point approximation.
		const std::uint64_t p_i = chain_probability_numerator( constant, i, lambda, q );
		if ( p_i == 0 )
			return std::numeric_limits<double>::infinity();
		return static_cast<double>( lambda - 1 ) - std::log2( static_cast<double>( p_i ) );
	}

	template <class Builder>
	inline void set_cddt_carry_chain_branch_priorities( Builder& model_builder, const BitVector& bits, int base_priority, int stride )
	{
		const int bit_count = static_cast<int>( bits.size() );
		for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
		{
			const int priority = base_priority + ( bit_count - 1 - bit_index ) * stride;
			model_builder.set_branch_priority( BitVector { bits[ static_cast<std::size_t>( bit_index ) ] }, priority );
		}
	}

	template <class Builder>
	inline void add_two_input_add_diff( Builder& model_builder, const BitVector& alpha, const BitVector& beta, const BitVector& gamma, const std::string& prefix )
	{
		// ====================================================================
		// Audit section C: two-variable Lipmaa-Moriai / Fu-Wang-Guo-Sun box
		// ====================================================================
		// One call emits the complete local two-input addition differential box.
		// Subtraction reuses this through gamma,beta -> alpha below.
		if ( alpha.size() != beta.size() || alpha.size() != gamma.size() )
		{
			throw std::invalid_argument( "add_two_input_add_diff: alpha, beta, gamma must have the same bit width" );
		}

		const int bit_count = static_cast<int>( alpha.size() );

		if ( bit_count <= 0 )
		{
			throw std::invalid_argument( "add_two_input_add_diff: bit width must be positive" );
		}

		// CDDT-style carry-chain ordering:
		// branch on the local weight bits first, then on the source bits that
		// steer the carry chain from low to high bits.
		constexpr int cddt_weight_priority_base = 180000;
		constexpr int cddt_input_priority_base = 170000;
		constexpr int cddt_output_priority_base = 160000;

		// Lowest bit has zero carry-in: alpha_0 xor beta_0 = gamma_0.
		model_builder.add_greater_or_equal_zero_constraint( prefix + "_least_significant_bit_xor_alpha_plus_beta_minus_gamma", { { alpha[ 0 ], 1 }, { beta[ 0 ], 1 }, { gamma[ 0 ], -1 } } );
		model_builder.add_greater_or_equal_zero_constraint( prefix + "_least_significant_bit_xor_alpha_minus_beta_plus_gamma", { { alpha[ 0 ], 1 }, { beta[ 0 ], -1 }, { gamma[ 0 ], 1 } } );
		model_builder.add_greater_or_equal_zero_constraint( prefix + "_least_significant_bit_xor_minus_alpha_plus_beta_plus_gamma", { { alpha[ 0 ], -1 }, { beta[ 0 ], 1 }, { gamma[ 0 ], 1 } } );
		model_builder.add_linear( prefix + "_least_significant_bit_xor_not_all_one", { { alpha[ 0 ], -1 }, { beta[ 0 ], -1 }, { gamma[ 0 ], -1 } }, -2.0, INF );

		for ( int bit_index = 0; bit_index + 1 < bit_count; ++bit_index )
		{
			const std::string bit_suffix = std::to_string( bit_index );
			const ScipVariable transition_weight = model_builder.create_binary_variable( prefix + "_two_input_addition_differential_weight_" + bit_suffix );

			model_builder.set_objective_coefficient( transition_weight, 1.0 );
			model_builder.set_branch_priority( BitVector { transition_weight }, cddt_weight_priority_base + ( bit_count - 2 - bit_index ) );

			// The first five inequalities force p_i to be exactly the local
			// modular-addition differential weight bit:
			// p_i = 0 for 000 or 111, and p_i = 1 otherwise.
			// The third inequality must be -alpha_i + gamma_i + p_i >= 0:
			// together with beta_i - gamma_i + p_i >= 0 and
			// alpha_i - beta_i + p_i >= 0, p_i = 0 forces
			// alpha_i = beta_i = gamma_i.
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_weight_beta_minus_gamma_" + bit_suffix, { { beta[ bit_index ], 1 }, { gamma[ bit_index ], -1 }, { transition_weight, 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_weight_alpha_minus_beta_" + bit_suffix, { { alpha[ bit_index ], 1 }, { beta[ bit_index ], -1 }, { transition_weight, 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_weight_gamma_minus_alpha_" + bit_suffix, { { alpha[ bit_index ], -1 }, { gamma[ bit_index ], 1 }, { transition_weight, 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_weight_sum_ge_weight_" + bit_suffix, { { alpha[ bit_index ], 1 }, { beta[ bit_index ], 1 }, { gamma[ bit_index ], 1 }, { transition_weight, -1 } } );
			model_builder.add_linear( prefix + "_fu_weight_sum_le_three_minus_weight_" + bit_suffix, { { alpha[ bit_index ], -1 }, { beta[ bit_index ], -1 }, { gamma[ bit_index ], -1 }, { transition_weight, -1 } }, -3.0, INF );

			// The remaining eight inequalities only bite when p_i = 0; then
			// alpha_i = beta_i = gamma_i and they enforce
			// alpha_{i+1} xor beta_{i+1} xor gamma_{i+1} = alpha_i.
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_transition_0_" + bit_suffix, { { alpha[ bit_index + 1 ], 1 }, { beta[ bit_index ], -1 }, { beta[ bit_index + 1 ], 1 }, { gamma[ bit_index + 1 ], 1 }, { transition_weight, 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_transition_1_" + bit_suffix, { { beta[ bit_index ], 1 }, { alpha[ bit_index + 1 ], 1 }, { beta[ bit_index + 1 ], -1 }, { gamma[ bit_index + 1 ], 1 }, { transition_weight, 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_transition_2_" + bit_suffix, { { beta[ bit_index ], 1 }, { alpha[ bit_index + 1 ], -1 }, { beta[ bit_index + 1 ], 1 }, { gamma[ bit_index + 1 ], 1 }, { transition_weight, 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_fu_transition_3_" + bit_suffix, { { alpha[ bit_index ], 1 }, { alpha[ bit_index + 1 ], 1 }, { beta[ bit_index + 1 ], 1 }, { gamma[ bit_index + 1 ], -1 }, { transition_weight, 1 } } );
			model_builder.add_linear( prefix + "_fu_transition_4_" + bit_suffix, { { gamma[ bit_index ], 1 }, { alpha[ bit_index + 1 ], -1 }, { beta[ bit_index + 1 ], -1 }, { gamma[ bit_index + 1 ], -1 }, { transition_weight, 1 } }, -2.0, INF );
			model_builder.add_linear( prefix + "_fu_transition_5_" + bit_suffix, { { beta[ bit_index ], -1 }, { alpha[ bit_index + 1 ], 1 }, { beta[ bit_index + 1 ], -1 }, { gamma[ bit_index + 1 ], -1 }, { transition_weight, 1 } }, -2.0, INF );
			model_builder.add_linear( prefix + "_fu_transition_6_" + bit_suffix, { { beta[ bit_index ], -1 }, { alpha[ bit_index + 1 ], -1 }, { beta[ bit_index + 1 ], 1 }, { gamma[ bit_index + 1 ], -1 }, { transition_weight, 1 } }, -2.0, INF );
			model_builder.add_linear( prefix + "_fu_transition_7_" + bit_suffix, { { beta[ bit_index ], -1 }, { alpha[ bit_index + 1 ], -1 }, { beta[ bit_index + 1 ], -1 }, { gamma[ bit_index + 1 ], 1 }, { transition_weight, 1 } }, -2.0, INF );
		}

		set_cddt_carry_chain_branch_priorities( model_builder, alpha, cddt_input_priority_base, 8 );
		set_cddt_carry_chain_branch_priorities( model_builder, beta, cddt_input_priority_base, 8 );
		set_cddt_carry_chain_branch_priorities( model_builder, gamma, cddt_output_priority_base, 8 );
	}

	template <class Builder>
	inline void add_two_input_sub_diff( Builder& model_builder, const BitVector& alpha, const BitVector& beta, const BitVector& gamma, const std::string& prefix )
	{
		add_two_input_add_diff( model_builder, gamma, beta, alpha, prefix );
	}

	template <class Builder>
	inline void add_zero_diff_operand_average( Builder& model_builder, const BitVector& beta, const BitVector& gamma, const std::string& prefix )
	{
		// Literature note: Bagherzadeh--Ahmadian, "MILP-based automatic
		// differential search for LEA and HIGHT block ciphers", IET Information
		// Security 14(5), 595-603, 2020, describe a compact MILP model for modular
		// addition with one constant input.  This optional comparison model is the
		// zero-input-difference-operand/constant-input style model: it ignores the
		// concrete public constant bits, so it is not the exact fixed-public-constant
		// add/sub model implemented below.
		if ( beta.size() != gamma.size() )
			throw std::invalid_argument( "add_zero_diff_operand_average: beta and gamma must have the same bit width for " + prefix );
		const int n = static_cast<int>( beta.size() );
		if ( n <= 0 || n > WORD_SIZE )
			throw std::invalid_argument( "add_zero_diff_operand_average: bit width must be in 1..32 for " + prefix );
		model_builder.add_equality_to_zero_constraint( prefix + "_lsb_beta_eq_gamma", { { beta[ 0 ], 1 }, { gamma[ 0 ], -1 } } );
		BitVector e = model_builder.create_bit_vector( prefix + "_e", n - 1 );
		for ( int i = 0; i < n - 1; ++i )
		{
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_e_ge_beta_" + std::to_string( i ), { { e[ i ], 1 }, { beta[ i ], -1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_e_ge_gamma_" + std::to_string( i ), { { e[ i ], 1 }, { gamma[ i ], -1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_e_or_" + std::to_string( i ), { { beta[ i ], 1 }, { gamma[ i ], 1 }, { e[ i ], -1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_next_pos_" + std::to_string( i ), { { beta[ i + 1 ], 1 }, { gamma[ i + 1 ], -1 }, { e[ i ], 1 } } );
			model_builder.add_greater_or_equal_zero_constraint( prefix + "_next_neg_" + std::to_string( i ), { { beta[ i + 1 ], -1 }, { gamma[ i + 1 ], 1 }, { e[ i ], 1 } } );
			model_builder.set_objective_coefficient( e[ i ], 1.0 );
		}
	}

	template <class Builder>
	inline void add_fixed_public_constant_exact( Builder& model_builder, std::uint32_t constant, const BitVector& u, const BitVector& v, const std::string& prefix )
	{
		// ====================================================================
		// Audit section D: fixed-public-constant exact zero-chain MILP
		// ====================================================================
		// One call emits the exact addend-dependent model for X -> X+constant.
		// The optional average-constant comparison model is intentionally kept
		// in add_zero_diff_operand_average() above.
		// Exact fixed-public-addend DP model. Miyano/Machado/Azimi explain why
		// this cannot be obtained by plugging a zero difference into the second
		// operand of two-variable addition: the public bits of constant steer
		// carry chains and therefore change the probability space.
		if ( u.size() != v.size() )
			throw std::invalid_argument( "add_fixed_public_constant_exact: input and output differences must have the same bit width for " + prefix );
		const int n = static_cast<int>( u.size() );
		if ( n <= 0 || n > WORD_SIZE )
			throw std::invalid_argument( "add_fixed_public_constant_exact: bit width must be in 1..32 for " + prefix );
		BitVector	  e = model_builder.create_bit_vector( prefix + "_e", n );
		for ( int i = 0; i < n; ++i )
		{
			model_builder.add_linear( prefix + "_e_le_u_plus_v_" + std::to_string( i ), { { e[ i ], 1 }, { u[ i ], -1 }, { v[ i ], -1 } }, -INF, 0.0 );
			model_builder.add_linear( prefix + "_e_ge_u_minus_v_" + std::to_string( i ), { { e[ i ], 1 }, { u[ i ], -1 }, { v[ i ], 1 } }, 0.0, INF );
			model_builder.add_linear( prefix + "_e_ge_v_minus_u_" + std::to_string( i ), { { e[ i ], 1 }, { u[ i ], 1 }, { v[ i ], -1 } }, 0.0, INF );
			model_builder.add_linear( prefix + "_e_le_2_minus_u_minus_v_" + std::to_string( i ), { { e[ i ], 1 }, { u[ i ], 1 }, { v[ i ], 1 } }, -INF, 2.0 );
		}
		model_builder.set_branch_priority( e, 500000 );

		std::vector<std::vector<ScipVariable>> s( n, std::vector<ScipVariable>( 8 ) );
		for ( int i = 0; i < n; ++i )
		{
			std::vector<LinearTerm> sum_terms;
			for ( int st = 0; st < 8; ++st )
			{
				const std::string state_variable_name = prefix + "_s_" + std::to_string( i ) + "_" + state_name( st );
				if ( st == 0b001 )
					s[ i ][ st ] = model_builder.create_variable( state_variable_name, 0.0, 0.0, 0.0, SCIP_VARTYPE_BINARY );
				else
				{
					s[ i ][ st ] = model_builder.create_binary_variable( state_variable_name );
					SCIP_CALL_THROW( SCIPchgVarBranchPriority( model_builder.scip, s[ i ][ st ].var, 480000 ) );
				}
				sum_terms.push_back( { s[ i ][ st ], 1 } );
			}
			model_builder.add_equality_to_constant_constraint( prefix + "_state_onehot_" + std::to_string( i ), sum_terms, 1.0 );

			std::vector<LinearTerm> a_terms, b_terms, c_terms;
			for ( int st = 0; st < 8; ++st )
			{
				if ( ( st >> 2 ) & 1 )
					a_terms.push_back( { s[ i ][ st ], 1 } );
				if ( ( st >> 1 ) & 1 )
					b_terms.push_back( { s[ i ][ st ], 1 } );
				if ( st & 1 )
					c_terms.push_back( { s[ i ][ st ], 1 } );
			}
			if ( i > 0 )
				a_terms.push_back( { u[ i - 1 ], -1 } );
			if ( i > 0 )
				b_terms.push_back( { v[ i - 1 ], -1 } );
			c_terms.push_back( { e[ i ], -1 } );
			model_builder.add_equality_to_zero_constraint( prefix + "_bind_A_" + std::to_string( i ), a_terms );
			model_builder.add_equality_to_zero_constraint( prefix + "_bind_B_" + std::to_string( i ), b_terms );
			model_builder.add_equality_to_zero_constraint( prefix + "_bind_C_" + std::to_string( i ), c_terms );
			// State 001 is impossible and was created as a fixed-zero binary variable.
			if ( i >= 1 )
			{
				model_builder.set_objective_coefficient( s[ i ][ 0b010 ], 1.0 );
				model_builder.set_objective_coefficient( s[ i ][ 0b011 ], 1.0 );
				model_builder.set_objective_coefficient( s[ i ][ 0b100 ], 1.0 );
				model_builder.set_objective_coefficient( s[ i ][ 0b101 ], 1.0 );
			}
		}

		for ( int i = 1; i < n; ++i )
		{
			// Each S_i = 11* trigger selects exactly one zero-chain length lambda
			// and one q branch. Selectors with p=0 are omitted, so the remaining
			// equalities make impossible q-classes infeasible automatically.
			std::vector<LinearTerm> trigger_terms { { s[ i ][ 0b110 ], -1 }, { s[ i ][ 0b111 ], -1 } };
			std::vector<LinearTerm> q0_terms;
			std::vector<LinearTerm> q1_terms;

			for ( int lambda = 1; lambda <= i + 1; ++lambda )
			{
				for ( int q = 0; q <= 1; ++q )
				{
					const double coefficient = chain_exact_log_weight_coefficient( constant, i, lambda, q );
					if ( !std::isfinite( coefficient ) )
					{
						// Exact-preserving sparse construction: p_{i,lambda,q}=0 means this
						// chain branch is impossible.  Do not create a selector and then fix it
						// to zero; omit the selector entirely.  The q-binding equations below
						// will force the triggering state to zero when all branches of the
						// required q-class are impossible.
						continue;
					}

					ScipVariable selector = model_builder.create_binary_variable( prefix + "_z_" + std::to_string( i ) + "_lambda_" + std::to_string( lambda ) + "_q_" + std::to_string( q ), coefficient );
					SCIP_CALL_THROW( SCIPchgVarBranchPriority( model_builder.scip, selector.var, 460000 ) );
					trigger_terms.push_back( { selector, 1 } );
					( q == 0 ? q0_terms : q1_terms ).push_back( { selector, 1 } );

					for ( int k = 1; k <= lambda - 1; ++k )
						model_builder.add_less_or_equal_zero_constraint( prefix + "_z_le_zero_" + std::to_string( i ) + "_lambda_" + std::to_string( lambda ) + "_q_" + std::to_string( q ) + "_k_" + std::to_string( k ), { { selector, 1 }, { s[ i - k ][ 0b000 ], -1 } } );
					if ( i - lambda >= 0 )
						model_builder.add_linear( prefix + "_z_boundary_not_zero_" + std::to_string( i ) + "_lambda_" + std::to_string( lambda ) + "_q_" + std::to_string( q ), { { selector, 1 }, { s[ i - lambda ][ 0b000 ], 1 } }, -INF, 1.0 );
				}
			}

			model_builder.add_equality_to_zero_constraint( prefix + "_z_trigger_" + std::to_string( i ), trigger_terms );
			if ( bit_at( constant, i - 1 ) == 0 )
			{
				q1_terms.push_back( { s[ i ][ 0b111 ], -1 } );
				q0_terms.push_back( { s[ i ][ 0b110 ], -1 } );
			}
			else
			{
				q1_terms.push_back( { s[ i ][ 0b110 ], -1 } );
				q0_terms.push_back( { s[ i ][ 0b111 ], -1 } );
			}
			model_builder.add_equality_to_zero_constraint( prefix + "_z_q0_bind_" + std::to_string( i ), q0_terms );
			model_builder.add_equality_to_zero_constraint( prefix + "_z_q1_bind_" + std::to_string( i ), q1_terms );
		}
	}

	template <class Builder>
	inline void add_fixed_public_constant_sub_exact( Builder& model_builder, std::uint32_t constant, const BitVector& u, const BitVector& v, const std::string& prefix )
	{
		add_fixed_public_constant_exact( model_builder, static_cast<std::uint32_t>( -constant ), u, v, prefix );
	}
}  // namespace neoalzette_diff_milp::arithmetic_model

namespace neoalzette_diff_milp
{
	// ========================================================================
	// Audit section E: generic SCIP model-builder facade
	// ========================================================================
	// This layer provides naming, variable creation, and primitive constraint
	// helpers. The differential theory belongs to the oracle/arithmetic sections.
	class ScipModelBuilder
	{
	public:
		SCIP*						scip = nullptr;
		std::vector<SCIP_VAR*>		all_scip_variables;
		std::vector<SCIP_CONS*>		all_scip_constraints;
		std::map<std::string, ScipVariable> scip_variable_by_name;
		std::vector<LinearTerm>		objective_terms;
		int							counter = 0;

		ScipModelBuilder( bool quiet, double debug_time_limit_seconds = std::numeric_limits<double>::quiet_NaN() )
		{
			SCIP_CALL_THROW( SCIPcreate( &scip ) );
			SCIP_CALL_THROW( SCIPincludeDefaultPlugins( scip ) );
			SCIP_CALL_THROW( include_injection_rank_const_handler( scip ) );
			SCIP_CALL_THROW( SCIPcreateProbBasic( scip, "neoalzette_xor_differential_milp" ) );
			SCIP_CALL_THROW( SCIPsetObjsense( scip, SCIP_OBJSENSE_MINIMIZE ) );
			if ( quiet )
			{
				SCIP_CALL_THROW( SCIPsetIntParam( scip, "display/verblevel", 0 ) );
			}
			if ( std::isfinite( debug_time_limit_seconds ) && debug_time_limit_seconds > 0.0 )
			{
				SCIP_CALL_THROW( SCIPsetRealParam( scip, "limits/time", debug_time_limit_seconds ) );
			}
		}
		~ScipModelBuilder()
		{
			if ( scip )
			{
				for ( SCIP_VAR* var : all_scip_variables )
				{
					if ( var != nullptr )
					{
						const SCIP_RETCODE retcode = SCIPreleaseVar( scip, &var );
						if ( retcode != SCIP_OKAY )
						{
							std::cerr << "SCIPreleaseVar failed during ScipModelBuilder destruction, code=" << retcode << "\n";
						}
					}
				}
				const SCIP_RETCODE retcode = SCIPfree( &scip );
				if ( retcode != SCIP_OKAY )
				{
					std::cerr << "SCIPfree failed during ScipModelBuilder destruction, code=" << retcode << "\n";
				}
			}
		}

		ScipVariable create_variable( const std::string& name, double lower_bound, double upper_bound, double objective_coefficient, SCIP_VARTYPE variable_type )
		{
			SCIP_VAR* scip_variable = nullptr;
			SCIP_CALL_THROW( SCIPcreateVarBasic( scip, &scip_variable, name.c_str(), lower_bound, upper_bound, objective_coefficient, variable_type ) );
			SCIP_CALL_THROW( SCIPaddVar( scip, scip_variable ) );
			all_scip_variables.push_back( scip_variable );
			ScipVariable variable_handle { scip_variable, name };
			scip_variable_by_name[ name ] = variable_handle;
			if ( std::fabs( objective_coefficient ) > 1e-15 )
				objective_terms.push_back( { variable_handle, objective_coefficient } );
			return variable_handle;
		}
		ScipVariable create_binary_variable( const std::string& name, double objective_coefficient = 0.0 )
		{
			return create_variable( name, 0.0, 1.0, objective_coefficient, SCIP_VARTYPE_BINARY );
		}
		ScipVariable create_integer_variable( const std::string& name, int lower_bound, int upper_bound )
		{
			return create_variable( name, lower_bound, upper_bound, 0.0, SCIP_VARTYPE_INTEGER );
		}
		ScipVariable create_continuous_variable( const std::string& name, double lower_bound, double upper_bound, double objective_coefficient = 0.0 )
		{
			return create_variable( name, lower_bound, upper_bound, objective_coefficient, SCIP_VARTYPE_CONTINUOUS );
		}
		void set_branch_priority( const BitVector& bits, int priority )
		{
			for ( const auto& bit : bits )
				SCIP_CALL_THROW( SCIPchgVarBranchPriority( scip, bit.var, priority ) );
		}
		void set_objective_coefficient( const ScipVariable& variable, double objective_coefficient )
		{
			SCIP_CALL_THROW( SCIPchgVarObj( scip, variable.var, objective_coefficient ) );
			if ( std::fabs( objective_coefficient ) > 1e-15 )
				objective_terms.push_back( { variable, objective_coefficient } );
		}

		ScipVariable find_var_or_throw( const std::string& name ) const
		{
			auto it = scip_variable_by_name.find( name );
			if ( it == scip_variable_by_name.end() )
				throw std::runtime_error( "SCIP variable not found for cut: " + name );
			return it->second;
		}

		void add_objective_bound( const std::string& name, double lower_bound, double upper_bound )
		{
			add_linear( name, objective_terms, lower_bound, upper_bound );
		}

		BitVector create_bit_vector( const std::string& prefix, int bit_count = WORD_SIZE )
		{
			if ( bit_count < 0 )
				throw std::runtime_error( "negative bit-vector size for " + prefix );
			BitVector output_bits;
			output_bits.reserve( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				output_bits.push_back( create_binary_variable( prefix + "_" + std::to_string( bit_index ) ) );
			return output_bits;
		}
		void add_linear( const std::string& name, const std::vector<LinearTerm>& terms, double lower_bound, double upper_bound )
		{
			SCIP_CONS*			   scip_constraint = nullptr;
			std::vector<SCIP_VAR*> scip_variables;
			std::vector<SCIP_Real> coefficients;
			scip_variables.reserve( terms.size() );
			coefficients.reserve( terms.size() );
			for ( const auto& term : terms )
			{
				scip_variables.push_back( term.variable.var );
				coefficients.push_back( term.coefficient );
			}
			SCIP_CALL_THROW( SCIPcreateConsBasicLinear( scip, &scip_constraint, name.c_str(), static_cast<int>( scip_variables.size() ), scip_variables.data(), coefficients.data(), lower_bound, upper_bound ) );
			SCIP_CALL_THROW( SCIPaddCons( scip, scip_constraint ) );
			all_scip_constraints.push_back( scip_constraint );
			SCIP_CALL_THROW( SCIPreleaseCons( scip, &scip_constraint ) );
		}
		void add_equality_to_zero_constraint( const std::string& name, const std::vector<LinearTerm>& terms )
		{
			add_linear( name, terms, 0.0, 0.0 );
		}
		void add_greater_or_equal_zero_constraint( const std::string& name, const std::vector<LinearTerm>& terms )
		{
			add_linear( name, terms, 0.0, INF );
		}
		void add_less_or_equal_zero_constraint( const std::string& name, const std::vector<LinearTerm>& terms )
		{
			add_linear( name, terms, -INF, 0.0 );
		}
		void add_equality_to_constant_constraint( const std::string& name, const std::vector<LinearTerm>& terms, double value )
		{
			add_linear( name, terms, value, value );
		}

		ScipVariable create_fixed_binary_variable( const std::string& name, int value )
		{
			ScipVariable variable = create_binary_variable( name );
			add_equality_to_constant_constraint( name + "_fix", { { variable, 1 } }, value ? 1.0 : 0.0 );
			return variable;
		}
		BitVector create_constant_word( const std::string& prefix, std::uint32_t value, int bit_count = WORD_SIZE )
		{
			if ( bit_count < 0 || bit_count > WORD_SIZE )
				throw std::runtime_error( "create_constant_word size outside 0..32 for " + prefix );
			BitVector output_bits;
			output_bits.reserve( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				output_bits.push_back( create_fixed_binary_variable( prefix + "_" + std::to_string( bit_index ), ( value >> bit_index ) & 1u ) );
			return output_bits;
		}
		void add_two_input_xor_constraint( const ScipVariable& x, const ScipVariable& y, const ScipVariable& z, const std::string& name )
		{
			ScipVariable parity_quotient = create_binary_variable( name + "_q" );
			add_equality_to_zero_constraint( name, { { x, 1 }, { y, 1 }, { z, 1 }, { parity_quotient, -2 } } );
		}
		void add_three_input_xor_constraint( const ScipVariable& x, const ScipVariable& y, const ScipVariable& z, const ScipVariable& out, const std::string& name )
		{
			ScipVariable parity_quotient = create_integer_variable( name + "_q", 0, 2 );
			add_equality_to_zero_constraint( name, { { x, 1 }, { y, 1 }, { z, 1 }, { out, 1 }, { parity_quotient, -2 } } );
		}
		ScipVariable create_negated_binary_variable( const ScipVariable& x, const std::string& name )
		{
			ScipVariable negated = create_binary_variable( name );
			add_equality_to_constant_constraint( name + "_not", { { x, 1 }, { negated, 1 } }, 1.0 );
			return negated;
		}
		void add_two_input_and_constraint( const ScipVariable& x, const ScipVariable& y, const ScipVariable& z, const std::string& name )
		{
			add_linear( name + "_ub_x", { { z, 1 }, { x, -1 } }, -INF, 0.0 );
			add_linear( name + "_ub_y", { { z, 1 }, { y, -1 } }, -INF, 0.0 );
			add_linear( name + "_lb", { { z, 1 }, { x, -1 }, { y, -1 } }, -1.0, INF );
		}
		void add_two_input_or_constraint( const ScipVariable& x, const ScipVariable& y, const ScipVariable& z, const std::string& name )
		{
			add_linear( name + "_lb_x", { { z, 1 }, { x, -1 } }, 0.0, INF );
			add_linear( name + "_lb_y", { { z, 1 }, { y, -1 } }, 0.0, INF );
			add_linear( name + "_ub", { { z, 1 }, { x, -1 }, { y, -1 } }, -INF, 0.0 );
		}
		BitVector create_xor_bit_vector( const BitVector& left_bits, const BitVector& right_bits, const std::string& prefix )
		{
			if ( left_bits.size() != right_bits.size() )
				throw std::runtime_error( "create_xor_bit_vector size mismatch for " + prefix );
			BitVector output_bits = create_bit_vector( prefix, static_cast<int>( left_bits.size() ) );
			for ( int bit_index = 0; bit_index < static_cast<int>( left_bits.size() ); ++bit_index )
				add_two_input_xor_constraint( left_bits[ bit_index ], right_bits[ bit_index ], output_bits[ bit_index ], prefix + "_xor_" + std::to_string( bit_index ) );
			return output_bits;
		}
		BitVector create_constant_xor_bit_vector( const BitVector& input_bits, std::uint32_t constant, const std::string& prefix )
		{
			if ( input_bits.size() > WORD_SIZE )
				throw std::runtime_error( "create_constant_xor_bit_vector only supports up to 32 bits for " + prefix );
			BitVector output_bits;
			output_bits.reserve( input_bits.size() );
			for ( int bit_index = 0; bit_index < static_cast<int>( input_bits.size() ); ++bit_index )
			{
				if ( ( constant >> bit_index ) & 1u )
					output_bits.push_back( create_negated_binary_variable( input_bits[ bit_index ], prefix + "_not_" + std::to_string( bit_index ) ) );
				else
					output_bits.push_back( input_bits[ bit_index ] );
			}
			return output_bits;
		}
		BitVector create_multiple_xor_bit_vector( const std::vector<BitVector>& inputs, const std::string& prefix )
		{
			if ( inputs.empty() )
				return create_constant_word( prefix + "_zero", 0u );
			const std::size_t bit_count = inputs.front().size();
			for ( const auto& input : inputs )
			{
				if ( input.size() != bit_count )
					throw std::runtime_error( "create_multiple_xor_bit_vector size mismatch for " + prefix );
			}
			BitVector accumulated_bits = inputs.front();
			for ( int input_index = 1; input_index < static_cast<int>( inputs.size() ); ++input_index )
				accumulated_bits = create_xor_bit_vector( accumulated_bits, inputs[ input_index ], prefix + "_xor_" + std::to_string( input_index ) );
			return accumulated_bits;
		}
		BitVector create_negated_bit_vector( const BitVector& input_bits, const std::string& prefix )
		{
			BitVector output_bits;
			output_bits.reserve( input_bits.size() );
			for ( int bit_index = 0; bit_index < static_cast<int>( input_bits.size() ); ++bit_index )
				output_bits.push_back( create_negated_binary_variable( input_bits[ bit_index ], prefix + "_" + std::to_string( bit_index ) ) );
			return output_bits;
		}
		BitVector create_and_bit_vector( const BitVector& left_bits, const BitVector& right_bits, const std::string& prefix )
		{
			if ( left_bits.size() != right_bits.size() )
				throw std::runtime_error( "create_and_bit_vector size mismatch for " + prefix );
			BitVector output_bits = create_bit_vector( prefix, static_cast<int>( left_bits.size() ) );
			for ( int bit_index = 0; bit_index < static_cast<int>( left_bits.size() ); ++bit_index )
				add_two_input_and_constraint( left_bits[ bit_index ], right_bits[ bit_index ], output_bits[ bit_index ], prefix + "_and_" + std::to_string( bit_index ) );
			return output_bits;
		}
		BitVector create_or_bit_vector( const BitVector& left_bits, const BitVector& right_bits, const std::string& prefix )
		{
			if ( left_bits.size() != right_bits.size() )
				throw std::runtime_error( "create_or_bit_vector size mismatch for " + prefix );
			BitVector output_bits = create_bit_vector( prefix, static_cast<int>( left_bits.size() ) );
			for ( int bit_index = 0; bit_index < static_cast<int>( left_bits.size() ); ++bit_index )
				add_two_input_or_constraint( left_bits[ bit_index ], right_bits[ bit_index ], output_bits[ bit_index ], prefix + "_or_" + std::to_string( bit_index ) );
			return output_bits;
		}
		BitVector shift_left_bit_vector( const BitVector& input_bits, int shift_amount, const std::string& prefix )
		{
			if ( shift_amount < 0 )
				throw std::runtime_error( "shift_left_bit_vector called with negative shift for " + prefix );
			const int bit_count = static_cast<int>( input_bits.size() );
			BitVector output_bits;
			output_bits.reserve( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
			{
				if ( bit_index - shift_amount >= 0 )
					output_bits.push_back( input_bits[ bit_index - shift_amount ] );
				else
					output_bits.push_back( create_fixed_binary_variable( prefix + "_zero_" + std::to_string( bit_index ), 0 ) );
			}
			return output_bits;
		}
		BitVector shift_right_bit_vector( const BitVector& input_bits, int shift_amount, const std::string& prefix )
		{
			if ( shift_amount < 0 )
				throw std::runtime_error( "shift_right_bit_vector called with negative shift for " + prefix );
			const int bit_count = static_cast<int>( input_bits.size() );
			BitVector output_bits;
			output_bits.reserve( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
			{
				if ( bit_index + shift_amount < bit_count )
					output_bits.push_back( input_bits[ bit_index + shift_amount ] );
				else
					output_bits.push_back( create_fixed_binary_variable( prefix + "_zero_" + std::to_string( bit_index ), 0 ) );
			}
			return output_bits;
		}
		static BitVector rotate_left( const BitVector& input_bits, int rotation_amount )
		{
			int bit_count = static_cast<int>( input_bits.size() );
			if ( bit_count <= 0 )
				throw std::runtime_error( "rotate_left called with empty bit-vector" );
			if ( rotation_amount < 0 )
				throw std::runtime_error( "rotate_left called with negative rotation" );
			rotation_amount %= bit_count;
			BitVector rotated_bits( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				rotated_bits[ bit_index ] = input_bits[ ( bit_index - rotation_amount + bit_count ) % bit_count ];
			return rotated_bits;
		}
		static BitVector rotate_right( const BitVector& input_bits, int rotation_amount )
		{
			int bit_count = static_cast<int>( input_bits.size() );
			if ( bit_count <= 0 )
				throw std::runtime_error( "rotate_right called with empty bit-vector" );
			if ( rotation_amount < 0 )
				throw std::runtime_error( "rotate_right called with negative rotation" );
			rotation_amount %= bit_count;
			BitVector rotated_bits( bit_count );
			for ( int bit_index = 0; bit_index < bit_count; ++bit_index )
				rotated_bits[ bit_index ] = input_bits[ ( bit_index + rotation_amount ) % bit_count ];
			return rotated_bits;
		}
	};

}  // namespace neoalzette_diff_milp
