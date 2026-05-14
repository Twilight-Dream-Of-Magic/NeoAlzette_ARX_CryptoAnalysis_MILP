#pragma once
#include <sstream>
#include <set>
#include <iostream>
#include <cassert>
#include <bit>
#include <scip/scip_tree.h>
#include <scip/pub_tree.h>
#include <scip/cons_xor.h>
#include <scip/scip.h>

#include <array>
#include <map>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <stdexcept>

namespace neoalzette_diff_milp
{
	// ========================================================================
	// Audit section 0: shared SCIP symbolic value types
	// ========================================================================
	// Only handle wrappers live here. Injection theory and SCIP callbacks are
	// separated into the sections below.
	inline constexpr double INF = 1e20;

	struct ScipVariable
	{
		SCIP_VAR*	var = nullptr;
		std::string name;
	};

	using BitVector = std::vector<ScipVariable>;

	struct LinearTerm
	{
		ScipVariable variable;
		double		 coefficient;
	};
}  // namespace neoalzette_diff_milp



namespace neoalzette_diff_milp
{
	// ========================================================================
	// Audit section 1: NeoAlzette joint injection value functions
	// ========================================================================
	// These constexpr functions define the actual vector map J(x). Both the
	// explicit witness MILP and the affine-rank oracle derive from this map.
	static constexpr int		   WORD_SIZE = 32;
	static constexpr int		   JOINT_OUTPUT_SIZE = 64;
	static constexpr std::uint32_t WORD_MASK = 0xFFFFFFFFu;
	static constexpr int		   FIRST_BRIDGE_ROTATE0 = 22;
	static constexpr int		   FIRST_BRIDGE_ROTATE1 = 13;
	static constexpr int		   SECOND_BRIDGE_ROTATE0 = 5;
	static constexpr int		   SECOND_BRIDGE_ROTATE1 = 25;

	static constexpr std::uint32_t ROUND_CONSTANTS[ 16 ] = { 0x16B2C40B, 0xC117176A, 0x0F9A2598, 0xA1563ACA, 0x243F6A88, 0x85A308D3, 0x13198A2E, 0x03707344, 0x9E3779B9, 0x7F4A7C15, 0xF39CC060, 0x5CEDC834, 0xB7E15162, 0x8AED2A6A, 0xBF715880, 0x9CF4F3C7 };

#define SCIP_CALL_THROW( x )                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
	do                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
	{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
		SCIP_RETCODE _retcode = ( x );                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
		if ( _retcode != SCIP_OKAY )                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           \
		{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
			std::ostringstream _oss;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           \
			_oss << "SCIP call failed at " << __FILE__ << ":" << __LINE__ << " code=" << _retcode;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
			throw std::runtime_error( _oss.str() );                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
		}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      \
	} while ( false )

	static constexpr std::uint32_t rotate_left_32( std::uint32_t word, int rotation )
	{
		rotation &= 31;
		if ( rotation == 0 )
			return word;
		return static_cast<std::uint32_t>( ( word << rotation ) | ( word >> ( 32 - rotation ) ) );
	}
	static constexpr std::uint32_t rotate_right_32( std::uint32_t word, int rotation )
	{
		rotation &= 31;
		if ( rotation == 0 )
			return word;
		return static_cast<std::uint32_t>( ( word >> rotation ) | ( word << ( 32 - rotation ) ) );
	}
	static constexpr std::uint32_t bitwise_not_32( std::uint32_t word )
	{
		return ~word;
	}

	static constexpr std::uint64_t pack_joint_output_difference( std::uint32_t xor_output_difference, std::uint32_t add_operand_difference )
	{
		return static_cast<std::uint64_t>( xor_output_difference ) | ( static_cast<std::uint64_t>( add_operand_difference ) << 32 );
	}

	static constexpr std::uint32_t generate_dynamic_diffusion_mask0_value( std::uint32_t word )
	{
		const std::uint32_t stage0 = word;
		const std::uint32_t stage1 = stage0 ^ rotate_left_32( stage0, 2 );
		const std::uint32_t stage2 = stage0 ^ rotate_left_32( stage1, 17 );
		const std::uint32_t stage3 = stage0 ^ rotate_left_32( stage2, 4 );
		const std::uint32_t stage4 = stage3 ^ rotate_left_32( stage3, 24 );
		return stage2 ^ rotate_left_32( stage4, 7 );
	}
	static constexpr std::uint32_t generate_dynamic_diffusion_mask1_value( std::uint32_t word )
	{
		const std::uint32_t stage0 = word;
		const std::uint32_t stage1 = stage0 ^ rotate_right_32( stage0, 2 );
		const std::uint32_t stage2 = stage0 ^ rotate_right_32( stage1, 17 );
		const std::uint32_t stage3 = stage0 ^ rotate_right_32( stage2, 4 );
		const std::uint32_t stage4 = stage3 ^ rotate_right_32( stage3, 24 );
		return stage2 ^ rotate_right_32( stage4, 7 );
	}

	static const std::uint32_t ROUND_CONSTANT_7_ROTATED_RIGHT_24 = rotate_right_32( ROUND_CONSTANTS[ 7 ], 24 );
	static const std::uint32_t ROUND_CONSTANT_8_ROTATED_RIGHT_24 = rotate_right_32( ROUND_CONSTANTS[ 8 ], 24 );
	static const std::uint32_t ROUND_CONSTANT_13_ROTATED_RIGHT_24 = rotate_right_32( ROUND_CONSTANTS[ 13 ], 24 );
	static const std::uint32_t ROUND_CONSTANT_2_ROTATED_LEFT_8 = rotate_left_32( ROUND_CONSTANTS[ 2 ], 8 );
	static const std::uint32_t ROUND_CONSTANT_3_ROTATED_LEFT_8 = rotate_left_32( ROUND_CONSTANTS[ 3 ], 8 );
	static const std::uint32_t ROUND_CONSTANT_12_ROTATED_LEFT_8 = rotate_left_32( ROUND_CONSTANTS[ 12 ], 8 );
	static const std::uint32_t DIFFUSION_MASK0_FROM_ROUND_CONSTANT_7 = generate_dynamic_diffusion_mask0_value( ROUND_CONSTANTS[ 7 ] );
	static const std::uint32_t DIFFUSION_MASK1_FROM_ROUND_CONSTANT_2 = generate_dynamic_diffusion_mask1_value( ROUND_CONSTANTS[ 2 ] );

	static constexpr std::pair<std::uint32_t, std::uint32_t> cd_injection_from_B_value( std::uint32_t B )
	{
		const std::uint32_t companion0 = rotate_right_32( B, 24 );
		const std::uint32_t mask = generate_dynamic_diffusion_mask0_value( B );
		const std::uint32_t companion_mask = rotate_right_32( mask, 24 ) ^ DIFFUSION_MASK0_FROM_ROUND_CONSTANT_7;
		const std::uint32_t mask_r1 = rotate_right_32( mask, 5 );
		const std::uint32_t x0 = companion0 ^ mask;
		const std::uint32_t x1 = B ^ mask;
		const std::uint32_t view = companion0 ^ companion_mask;
		const std::uint32_t bridge_state = rotate_right_32( B, 19 ) ^ ( B << 9 );
		const std::uint32_t q_state_na = ROUND_CONSTANT_7_ROTATED_RIGHT_24 ^ bitwise_not_32( B & mask );
		const std::uint32_t q_comp_no = companion0 ^ B ^ ROUND_CONSTANT_8_ROTATED_RIGHT_24 ^ bitwise_not_32( companion0 | mask_r1 );
		const std::uint32_t q_bridge = bridge_state ^ B ^ ROUND_CONSTANT_13_ROTATED_RIGHT_24 ^ bitwise_not_32( bridge_state & companion_mask );
		const std::uint32_t q_shared = q_state_na ^ q_comp_no;
		const std::uint32_t cross_q = ( B ^ mask_r1 ) & rotate_right_32( mask ^ companion_mask, 7 );
		const std::uint32_t anti_q = ( ( x1 >> 3 ) ^ ( view >> 5 ) ^ mask_r1 ) & ( B ^ rotate_right_32( x0, 11 ) );
		const std::uint32_t c = q_shared ^ rotate_right_32( q_comp_no, 5 ) ^ rotate_right_32( q_comp_no, 11 ) ^ anti_q;
		const std::uint32_t d = q_shared ^ rotate_right_32( q_state_na, 5 ) ^ rotate_right_32( q_bridge, 13 ) ^ cross_q ^ anti_q;
		return { c, d };
	}
	static constexpr std::pair<std::uint32_t, std::uint32_t> cd_injection_from_A_value( std::uint32_t A )
	{
		const std::uint32_t companion0 = rotate_left_32( A, 8 );
		const std::uint32_t mask = generate_dynamic_diffusion_mask1_value( A );
		const std::uint32_t companion_mask = rotate_left_32( mask, 8 ) ^ DIFFUSION_MASK1_FROM_ROUND_CONSTANT_2;
		const std::uint32_t mask_r1 = rotate_right_32( mask, 5 );
		const std::uint32_t x0 = companion0 ^ mask;
		const std::uint32_t x1 = A ^ mask;
		const std::uint32_t view = companion0 ^ companion_mask;
		const std::uint32_t bridge_state = rotate_left_32( A, 19 ) ^ ( A >> 9 );
		const std::uint32_t q_state_no = ROUND_CONSTANT_2_ROTATED_LEFT_8 ^ bitwise_not_32( A | mask );
		const std::uint32_t q_comp_na = companion0 ^ A ^ ROUND_CONSTANT_3_ROTATED_LEFT_8 ^ bitwise_not_32( companion0 & mask_r1 );
		const std::uint32_t q_bridge = bridge_state ^ A ^ ROUND_CONSTANT_12_ROTATED_LEFT_8 ^ bitwise_not_32( bridge_state | companion_mask );
		const std::uint32_t q_shared = q_state_no ^ q_comp_na;
		const std::uint32_t cross_q = ( A ^ mask_r1 ) & rotate_left_32( mask ^ companion_mask, 13 );
		const std::uint32_t anti_q = ( ( x1 << 3 ) ^ ( view << 5 ) ^ mask_r1 ) | ( A ^ rotate_left_32( x0, 11 ) );
		const std::uint32_t c = q_shared ^ rotate_left_32( q_comp_na, 5 ) ^ rotate_left_32( q_comp_na, 11 ) ^ anti_q;
		const std::uint32_t d = q_shared ^ rotate_left_32( q_state_no, 5 ) ^ rotate_left_32( q_bridge, 13 ) ^ cross_q ^ anti_q;
		return { c, d };
	}
	static constexpr std::uint32_t injection_JB_value( std::uint32_t B )
	{
		const auto [ c, d ] = cd_injection_from_B_value( B );
		return rotate_left_32( ( c << 2 ) ^ ( d >> 2 ), 24 ) ^ rotate_left_32( d, 16 ) ^ rotate_left_32( ( c >> 5 ) ^ ( d << 5 ), 8 );
	}
	static constexpr std::uint32_t injection_JA_value( std::uint32_t A )
	{
		const auto [ c, d ] = cd_injection_from_A_value( A );
		return rotate_right_32( ( c >> 3 ) ^ ( d << 3 ), 24 ) ^ rotate_right_32( d, 16 ) ^ rotate_right_32( ( c << 1 ) ^ ( d >> 1 ), 8 );
	}

	static constexpr std::uint64_t joint_injection_from_B_value( std::uint32_t B )
	{
		const auto [ c, d ] = cd_injection_from_B_value( B );
		const std::uint32_t xor_output_difference = rotate_left_32( B, 24 ) ^ rotate_left_32( c, 16 ) ^ rotate_left_32( B, 8 );
		const std::uint32_t cd0 = ( c << 2 ) ^ ( d >> 2 );
		const std::uint32_t cd1 = ( c >> 5 ) ^ ( d << 5 );
		const std::uint32_t add_operand_difference = rotate_left_32( cd0, 31 ) ^ rotate_left_32( cd1, 17 );
		return pack_joint_output_difference( xor_output_difference, add_operand_difference );
	}

	static constexpr std::uint64_t joint_injection_from_A_value( std::uint32_t A )
	{
		const auto [ c, d ] = cd_injection_from_A_value( A );
		const std::uint32_t xor_output_difference = rotate_right_32( A, 24 ) ^ rotate_right_32( d, 16 ) ^ rotate_right_32( A, 8 );
		const std::uint32_t cd2 = ( c >> 3 ) ^ ( d << 3 );
		const std::uint32_t cd3 = ( c << 1 ) ^ ( d >> 1 );
		const std::uint32_t add_operand_difference = cd2 ^ cd3;
		return pack_joint_output_difference( xor_output_difference, add_operand_difference );
	}

	// ============================================================================
	// Audit section 2: GF(2) linear algebra and polar fast path
	// ----------------------------------------------------------------------------
	// WARNING / mathematical precondition:
	//   This optimization is valid because the current NeoAlzette injected XOR-term
	//   maps are vector quadratic Boolean functions over GF(2).  For a fixed input
	//   XOR difference Delta,
	//       D_Delta J(x) = J(x) xor J(x xor Delta)
	//   is therefore affine:
	//       D_Delta J(x) = M_Delta x xor c_Delta.
	//
	//   The slow sampler builds M_Delta column-by-column with
	//       col_i(M_Delta) = D_Delta J(e_i) xor D_Delta J(0),
	//   which costs J(Delta) plus J(e_i xor Delta) for all i.
	//
	//   Since J is quadratic, the columns are also given by the polar bilinear form
	//       col_i(M_Delta)
	//         = J(e_i) xor J(e_i xor Delta) xor J(0) xor J(Delta)
	//         = B_J(e_i, Delta)
	//         = xor_{j : Delta_j = 1} B_J(e_i, e_j),
	//   where
	//       B_J(e_i,e_j) = J(e_i) xor J(e_j) xor J(e_i xor e_j) xor J(0).
	//
	//   We precompute B_J(e_i,e_j), compress it into byte tables, and construct all
	//   32 columns using table lookups.  Each cache miss now needs one exact map
	//   evaluation J(Delta), not thirty-three exact map evaluations.
	//
	//   If the injection layer is changed to include cubic or higher-degree terms,
	//   this polar-form fast path is invalid.  The dedicated injection-affine
	//   validation check compares the fast polar model against the slow derivative
	//   sampler and must be rerun after any injection-layer change.
	// ============================================================================

	static int gf2_pivot_bit( std::uint32_t x )
	{
		if ( x == 0u )
			throw std::runtime_error( "gf2_pivot_bit called with zero" );
		return 31 - static_cast<int>( std::countl_zero( x ) );
	}

	static int gf2_pivot_bit_64( std::uint64_t x )
	{
		if ( x == 0u )
			throw std::runtime_error( "gf2_pivot_bit_64 called with zero" );
		return 63 - static_cast<int>( std::countl_zero( x ) );
	}

	static std::pair<std::vector<std::uint32_t>, int> gf2_basis( const std::vector<std::uint32_t>& columns )
	{
		std::vector<std::uint32_t> basis( 32, 0 );
		int						   rank = 0;
		for ( std::uint32_t v : columns )
		{
			std::uint32_t x = v;
			while ( x != 0 )
			{
				const int pivot = gf2_pivot_bit( x );
				if ( basis[ pivot ] != 0 )
				{
					x ^= basis[ pivot ];
				}
				else
				{
					basis[ pivot ] = x;
					++rank;
					break;
				}
			}
		}
		return { basis, rank };
	}

	static bool gf2_in_span( const std::vector<std::uint32_t>& basis, std::uint32_t v )
	{
		std::uint32_t x = v;
		while ( x != 0 )
		{
			const int pivot = gf2_pivot_bit( x );
			if ( basis[ pivot ] == 0 )
				return false;
			x ^= basis[ pivot ];
		}
		return true;
	}

	static std::pair<std::vector<std::uint64_t>, int> gf2_basis_64( const std::vector<std::uint64_t>& columns )
	{
		std::vector<std::uint64_t> basis( JOINT_OUTPUT_SIZE, 0 );
		int						   rank = 0;
		for ( std::uint64_t v : columns )
		{
			std::uint64_t x = v;
			while ( x != 0 )
			{
				const int pivot = gf2_pivot_bit_64( x );
				if ( basis[ pivot ] != 0 )
				{
					x ^= basis[ pivot ];
				}
				else
				{
					basis[ pivot ] = x;
					++rank;
					break;
				}
			}
		}
		return { basis, rank };
	}

	static bool gf2_in_span_64( const std::vector<std::uint64_t>& basis, std::uint64_t v )
	{
		std::uint64_t x = v;
		while ( x != 0 )
		{
			const int pivot = gf2_pivot_bit_64( x );
			if ( basis[ pivot ] == 0 )
				return false;
			x ^= basis[ pivot ];
		}
		return true;
	}

	static int gf2_dot_64( std::uint64_t a, std::uint64_t b )
	{
		return static_cast<int>( std::popcount( a & b ) & 1u );
	}

	static std::vector<std::uint64_t> gf2_nullspace_basis_from_rows_64( const std::vector<std::uint64_t>& rows )
	{
		std::array<std::uint64_t, JOINT_OUTPUT_SIZE> pivot_rows {};
		for ( std::uint64_t row : rows )
		{
			std::uint64_t x = row;
			while ( x != 0 )
			{
				const int pivot = gf2_pivot_bit_64( x );
				if ( pivot_rows[ static_cast<std::size_t>( pivot ) ] != 0 )
				{
					x ^= pivot_rows[ static_cast<std::size_t>( pivot ) ];
				}
				else
				{
					pivot_rows[ static_cast<std::size_t>( pivot ) ] = x;
					break;
				}
			}
		}

		std::vector<std::uint64_t> nullspace;
		for ( int free_bit = 0; free_bit < JOINT_OUTPUT_SIZE; ++free_bit )
		{
			if ( pivot_rows[ static_cast<std::size_t>( free_bit ) ] != 0 )
				continue;

			std::uint64_t v = std::uint64_t( 1 ) << free_bit;
			for ( int pivot = 0; pivot < JOINT_OUTPUT_SIZE; ++pivot )
			{
				const std::uint64_t row = pivot_rows[ static_cast<std::size_t>( pivot ) ];
				if ( row == 0 )
					continue;
				const std::uint64_t without_pivot = row & ~( std::uint64_t( 1 ) << pivot );
				if ( gf2_dot_64( without_pivot, v ) )
					v |= ( std::uint64_t( 1 ) << pivot );
			}
			nullspace.push_back( v );
		}
		return nullspace;
	}

	static int gf2_dot( std::uint32_t a, std::uint32_t b )
	{
		return static_cast<int>( std::popcount( a & b ) & 1u );
	}

	static std::vector<std::uint32_t> gf2_nullspace_basis_from_rows( const std::vector<std::uint32_t>& rows )
	{
		std::array<std::uint32_t, WORD_SIZE> pivot_rows {};
		for ( std::uint32_t row : rows )
		{
			std::uint32_t x = row;
			while ( x != 0 )
			{
				const int pivot = gf2_pivot_bit( x );
				if ( pivot_rows[ static_cast<std::size_t>( pivot ) ] != 0 )
				{
					x ^= pivot_rows[ static_cast<std::size_t>( pivot ) ];
				}
				else
				{
					pivot_rows[ static_cast<std::size_t>( pivot ) ] = x;
					break;
				}
			}
		}

		std::vector<std::uint32_t> nullspace;
		for ( int free_bit = 0; free_bit < WORD_SIZE; ++free_bit )
		{
			if ( pivot_rows[ static_cast<std::size_t>( free_bit ) ] != 0 )
				continue;

			std::uint32_t v = 1u << free_bit;
			for ( int pivot = 0; pivot < WORD_SIZE; ++pivot )
			{
				const std::uint32_t row = pivot_rows[ static_cast<std::size_t>( pivot ) ];
				if ( row == 0 )
					continue;
				const std::uint32_t without_pivot = row & ~( 1u << pivot );
				if ( gf2_dot( without_pivot, v ) )
					v |= ( 1u << pivot );
			}
			nullspace.push_back( v );
		}
		return nullspace;
	}

	enum class InjectionKind
	{
		B_TO_A_AFTER_RC4,
		A_TO_B_AFTER_RC9
	};

	struct InjectionTransitionResult
	{
		bool		   valid = false;
		int		   rank = 0;
		std::uint64_t affine_constant = 0;
		std::uint64_t violated_parity_check = 0;
	};

	struct InjectionAffineImage
	{
		int								  rank = 0;
		std::uint64_t					  affine_constant = 0;
		const std::vector<std::uint64_t>* parity_checks = nullptr;
	};

	// ============================================================================
	// Audit section 3: affine derivative rank oracle
	// ============================================================================
	// Oracle-to-MILP bridge for the joint injection layer, differential side.
	// For fixed input difference Delta, the derivative D_Delta J is affine for
	// the current quadratic injection map. Its image is an affine subspace:
	//
	//     delta_out in c_Delta + Im(M_Delta).
	//
	// The MILP witness below proves membership by constructing one x. This
	// oracle supplies the missing global facts that are cheap once Delta is
	// fixed: parity checks for the affine image and the rank(M_Delta) weight.
	class InjectionRankOracle
	{
	public:
		using CacheKey = std::pair<int, std::uint32_t>;
		using PolarBasisTable = std::array<std::array<std::uint64_t, 32>, 32>;
		using PolarByteTable = std::array<std::array<std::array<std::uint64_t, 256>, 4>, 32>;

		InjectionRankOracle() = default;

		InjectionTransitionResult transition( InjectionKind kind, std::uint32_t delta_in, std::uint64_t delta_out )
		{
			// Point check used for integral solutions: subtract the affine offset
			// and test whether the proposed joint output difference lies in the
			// image basis. violated_check records one parity witness for logging
			// and no-good cut diagnosis.
			const auto& model = affine_model( kind, delta_in );
			const std::uint64_t centered = delta_out ^ model.affine_constant;
			const bool		   valid = gf2_in_span_64( model.basis, centered );
			std::uint64_t	   violated_check = 0;
			if ( !valid )
			{
				for ( std::uint64_t check : model.parity_checks )
				{
					if ( gf2_dot_64( check, centered ) != 0 )
					{
						violated_check = check;
						break;
					}
				}
			}
			return { valid, model.rank, model.affine_constant, violated_check };
		}

		int rank( InjectionKind kind, std::uint32_t delta_in )
		{
			return affine_model( kind, delta_in ).rank;
		}

		InjectionAffineImage image( InjectionKind kind, std::uint32_t delta_in )
		{
			const auto& model = affine_model( kind, delta_in );
			return { model.rank, model.affine_constant, &model.parity_checks };
		}

	private:
		struct AffineModel
		{
			std::uint64_t				  affine_constant = 0;
			std::vector<std::uint64_t> columns;
			std::vector<std::uint64_t> basis;
			std::vector<std::uint64_t> parity_checks;
			int						  rank = 0;
		};

		std::map<CacheKey, AffineModel> cache_;
		static constexpr std::uint64_t function( InjectionKind kind, std::uint32_t x )
		{
			switch ( kind )
			{
			case InjectionKind::B_TO_A_AFTER_RC4:
				return joint_injection_from_B_value( x );
			case InjectionKind::A_TO_B_AFTER_RC9:
				return joint_injection_from_A_value( x );
			}
			return 0;
		}

		static constexpr PolarBasisTable make_polar_basis_table( InjectionKind kind )
		{
			PolarBasisTable		table {};
			const std::uint64_t f0 = function( kind, 0u );

			std::array<std::uint64_t, 32> fe {};
			for ( int i = 0; i < 32; ++i )
			{
				fe[ static_cast<std::size_t>( i ) ] = function( kind, 1u << i );
			}

			for ( int i = 0; i < 32; ++i )
			{
				for ( int j = 0; j < 32; ++j )
				{
					const std::uint64_t ei = 1u << i;
					const std::uint64_t ej = 1u << j;
					table[ static_cast<std::size_t>( i ) ][ static_cast<std::size_t>( j ) ] = fe[ static_cast<std::size_t>( i ) ] ^ fe[ static_cast<std::size_t>( j ) ] ^ function( kind, ei ^ ej ) ^ f0;
				}
			}
			return table;
		}

		static constexpr PolarByteTable make_polar_byte_table( InjectionKind kind )
		{
			const PolarBasisTable polar_basis = make_polar_basis_table( kind );
			PolarByteTable		  byte_table {};

			for ( int i = 0; i < 32; ++i )
			{
				for ( int byte_index = 0; byte_index < 4; ++byte_index )
				{
					for ( int byte_value = 0; byte_value < 256; ++byte_value )
					{
						std::uint64_t acc = 0u;
						for ( int k = 0; k < 8; ++k )
						{
							if ( ( byte_value >> k ) & 1 )
							{
								const int bit_index = byte_index * 8 + k;
								acc ^= polar_basis[ static_cast<std::size_t>( i ) ][ static_cast<std::size_t>( bit_index ) ];
							}
						}
						byte_table[ static_cast<std::size_t>( i ) ][ static_cast<std::size_t>( byte_index ) ][ static_cast<std::size_t>( byte_value ) ] = acc;
					}
				}
			}
			return byte_table;
		}

		static std::uint64_t polar_column_from_byte_table( const PolarByteTable& table, int input_basis_bit, std::uint32_t delta )
		{
			return table[ static_cast<std::size_t>( input_basis_bit ) ][ 0 ][ ( delta >> 0 ) & 0xFFu ] ^ table[ static_cast<std::size_t>( input_basis_bit ) ][ 1 ][ ( delta >> 8 ) & 0xFFu ] ^ table[ static_cast<std::size_t>( input_basis_bit ) ][ 2 ][ ( delta >> 16 ) & 0xFFu ] ^ table[ static_cast<std::size_t>( input_basis_bit ) ][ 3 ][ ( delta >> 24 ) & 0xFFu ];
		}

		const PolarByteTable& polar_byte_table( InjectionKind kind ) const
		{
			static constexpr PolarByteTable polar_byte_b = make_polar_byte_table( InjectionKind::B_TO_A_AFTER_RC4 );
			static constexpr PolarByteTable polar_byte_a = make_polar_byte_table( InjectionKind::A_TO_B_AFTER_RC9 );
			switch ( kind )
			{
			case InjectionKind::B_TO_A_AFTER_RC4:
				return polar_byte_b;
			case InjectionKind::A_TO_B_AFTER_RC9:
				return polar_byte_a;
			}
			return polar_byte_b;
		}

		const AffineModel& affine_model( InjectionKind kind, std::uint32_t delta )
		{
			CacheKey key { static_cast<int>( kind ), delta };
			auto	 it = cache_.find( key );
			if ( it != cache_.end() )
				return it->second;

			AffineModel model;
			// Because J is quadratic, D_delta J(x) = J(0) xor J(delta) xor
			// M_delta x. The byte tables evaluate the polar columns of M_delta
			// without rebuilding the full derivative graph for every SCIP query.
			model.affine_constant = function( kind, 0u ) ^ function( kind, delta );
			model.columns.reserve( 32 );

			const PolarByteTable& table = polar_byte_table( kind );
			for ( int bit = 0; bit < 32; ++bit )
			{
				model.columns.push_back( polar_column_from_byte_table( table, bit, delta ) );
			}

			auto [ basis, rank ] = gf2_basis_64( model.columns );
			model.basis = std::move( basis );
			model.parity_checks = gf2_nullspace_basis_from_rows_64( model.columns );
			model.rank = rank;
			auto inserted = cache_.emplace( key, std::move( model ) );
			return inserted.first->second;
		}
	};

	// ============================================================================
	// Audit section 4: SCIP constraint handler for affine-image/rank cuts
	// ============================================================================
	// Per-SCIP-constraint state for the differential joint injection handler.
	// The explicit witness MILP already models H(x) xor H(x xor delta); this
	// handler adds two solver-side reinforcements for fixed or integral deltas:
	// local affine-image parity rows and the rank_weight >= rank(M_delta) bound.
	struct InjectionRankConsData
	{
		InjectionKind						  kind = InjectionKind::B_TO_A_AFTER_RC4;
		std::array<SCIP_VAR*, WORD_SIZE>		  input_bits {};
		std::array<SCIP_VAR*, JOINT_OUTPUT_SIZE> output_bits {};
		SCIP_VAR*							  rank_weight = nullptr;
		std::set<std::tuple<SCIP_Longint, std::uint32_t>> local_affine_image_seen;
		InjectionRankOracle						  oracle;
	};

	static InjectionRankConsData* injection_rank_cons_data( SCIP_CONS* cons )
	{
		return reinterpret_cast<InjectionRankConsData*>( SCIPconsGetData( cons ) );
	}

	static SCIP_RETCODE injection_rank_constraint_data_capture_variables( SCIP* scip, InjectionRankConsData* constraint_data )
	{
		for ( SCIP_VAR* variable : constraint_data->input_bits )
			SCIP_CALL( SCIPcaptureVar( scip, variable ) );
		for ( SCIP_VAR* variable : constraint_data->output_bits )
			SCIP_CALL( SCIPcaptureVar( scip, variable ) );
		SCIP_CALL( SCIPcaptureVar( scip, constraint_data->rank_weight ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE injection_rank_constraint_data_release_variables( SCIP* scip, InjectionRankConsData* constraint_data )
	{
		for ( SCIP_VAR*& variable : constraint_data->input_bits )
		{
			if ( variable != nullptr )
				SCIP_CALL( SCIPreleaseVar( scip, &variable ) );
		}
		for ( SCIP_VAR*& variable : constraint_data->output_bits )
		{
			if ( variable != nullptr )
				SCIP_CALL( SCIPreleaseVar( scip, &variable ) );
		}
		if ( constraint_data->rank_weight != nullptr )
			SCIP_CALL( SCIPreleaseVar( scip, &constraint_data->rank_weight ) );
		return SCIP_OKAY;
	}

	static SCIP_Real injection_rank_constraint_solution_value( SCIP* scip, SCIP_SOL* solution, SCIP_VAR* variable )
	{
		return solution != nullptr ? SCIPgetSolVal( scip, solution, variable ) : SCIPgetVarSol( scip, variable );
	}

	static std::uint32_t injection_rank_constraint_read_input_difference( SCIP* scip, SCIP_SOL* solution, const InjectionRankConsData* constraint_data )
	{
		std::uint32_t input_difference = 0;
		for ( int bit_index = 0; bit_index < WORD_SIZE; ++bit_index )
		{
			if ( injection_rank_constraint_solution_value( scip, solution, constraint_data->input_bits[ static_cast<std::size_t>( bit_index ) ] ) > 0.5 )
				input_difference |= ( 1u << bit_index );
		}
		return input_difference;
	}

	static std::uint64_t injection_rank_constraint_read_output_difference( SCIP* scip, SCIP_SOL* solution, const InjectionRankConsData* constraint_data )
	{
		std::uint64_t output_difference = 0;
		for ( int bit_index = 0; bit_index < JOINT_OUTPUT_SIZE; ++bit_index )
		{
			if ( injection_rank_constraint_solution_value( scip, solution, constraint_data->output_bits[ static_cast<std::size_t>( bit_index ) ] ) > 0.5 )
				output_difference |= ( std::uint64_t( 1 ) << bit_index );
		}
		return output_difference;
	}

	static bool injection_rank_constraint_bits_are_integral( SCIP* scip, SCIP_SOL* solution, const InjectionRankConsData* constraint_data )
	{
		for ( int bit_index = 0; bit_index < WORD_SIZE; ++bit_index )
		{
			const SCIP_Real input_value = injection_rank_constraint_solution_value( scip, solution, constraint_data->input_bits[ static_cast<std::size_t>( bit_index ) ] );
			if ( !SCIPisFeasIntegral( scip, input_value ) )
				return false;
		}
		for ( int bit_index = 0; bit_index < JOINT_OUTPUT_SIZE; ++bit_index )
		{
			const SCIP_Real output_value = injection_rank_constraint_solution_value( scip, solution, constraint_data->output_bits[ static_cast<std::size_t>( bit_index ) ] );
			if ( !SCIPisFeasIntegral( scip, output_value ) )
				return false;
		}
		return true;
	}

	static bool injection_rank_constraint_input_is_locally_fixed( SCIP* scip, const InjectionRankConsData* constraint_data, std::uint32_t& input_difference )
	{
		input_difference = 0;
		for ( int bit_index = 0; bit_index < WORD_SIZE; ++bit_index )
		{
			SCIP_VAR* variable = constraint_data->input_bits[ static_cast<std::size_t>( bit_index ) ];
			const SCIP_Real local_lower_bound = SCIPvarGetLbLocal( variable );
			const SCIP_Real local_upper_bound = SCIPvarGetUbLocal( variable );
			if ( SCIPisFeasGE( scip, local_lower_bound, 0.5 ) )
			{
				input_difference |= ( 1u << bit_index );
			}
			else if ( SCIPisFeasLE( scip, local_upper_bound, 0.5 ) )
			{
				continue;
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	static SCIP_RETCODE injection_rank_constraint_tighten_weight_for_fixed_input( SCIP* scip, InjectionRankConsData* constraint_data, std::uint32_t input_difference, bool& infeasible, bool& reduced_domain );

	static SCIP_RETCODE injection_rank_constraint_add_local_xor_parity( SCIP* scip, SCIP_CONS* cons, const InjectionRankConsData* constraint_data, std::uint32_t input_difference, std::uint64_t affine_constant, std::uint64_t parity_check, bool& added )
	{
		if ( parity_check == 0u )
			return SCIP_OKAY;

		std::array<SCIP_VAR*, JOINT_OUTPUT_SIZE> variables {};
		int								 variable_count = 0;
		for ( int bit_index = 0; bit_index < JOINT_OUTPUT_SIZE; ++bit_index )
		{
			if ( ( parity_check >> bit_index ) & 1u )
				variables[ static_cast<std::size_t>( variable_count++ ) ] = constraint_data->output_bits[ static_cast<std::size_t>( bit_index ) ];
		}
		if ( variable_count == 0 )
			return SCIP_OKAY;

		SCIP_CONS*		   xor_constraint = nullptr;
		const bool		   right_hand_side = gf2_dot_64( parity_check, affine_constant ) != 0;
		const std::string name = std::string( SCIPconsGetName( cons ) ) + "_local_img_delta_" + std::to_string( input_difference ) + "_check_" + std::to_string( parity_check );
		SCIP_CALL( SCIPcreateConsXor( scip,
									   &xor_constraint,
									   name.c_str(),
									   right_hand_side ? TRUE : FALSE,
									   variable_count,
									   variables.data(),
									   TRUE,
									   TRUE,
									   TRUE,
									   TRUE,
									   TRUE,
									   TRUE,
									   FALSE,
									   TRUE,
									   TRUE,
									   TRUE ) );
		SCIP_CALL( SCIPaddConsLocal( scip, xor_constraint, nullptr ) );
		SCIP_CALL( SCIPreleaseCons( scip, &xor_constraint ) );
		added = true;
		return SCIP_OKAY;
	}

	static SCIP_Longint injection_rank_cons_current_node_number( SCIP* scip )
	{
		SCIP_NODE* node = SCIPgetCurrentNode( scip );
		return node != nullptr ? SCIPnodeGetNumber( node ) : -1;
	}

	static std::set<SCIP_Longint> injection_rank_cons_current_path_node_numbers( SCIP* scip )
	{
		std::set<SCIP_Longint> path;
		for ( SCIP_NODE* node = SCIPgetCurrentNode( scip ); node != nullptr; node = SCIPnodeGetParent( node ) )
			path.insert( SCIPnodeGetNumber( node ) );
		path.insert( -1 );
		return path;
	}

	static void injection_rank_constraint_prune_local_affine_image_seen_to_current_path( SCIP* scip, InjectionRankConsData* constraint_data )
	{
		const auto path = injection_rank_cons_current_path_node_numbers( scip );
		for ( auto iterator = constraint_data->local_affine_image_seen.begin(); iterator != constraint_data->local_affine_image_seen.end(); )
		{
			if ( path.find( std::get<0>( *iterator ) ) == path.end() )
				iterator = constraint_data->local_affine_image_seen.erase( iterator );
			else
				++iterator;
		}
	}

	static bool injection_rank_constraint_affine_image_seen_on_node_path( SCIP* scip, const InjectionRankConsData* constraint_data, std::uint32_t input_difference )
	{
		for ( SCIP_NODE* node = SCIPgetCurrentNode( scip ); node != nullptr; node = SCIPnodeGetParent( node ) )
		{
			if ( constraint_data->local_affine_image_seen.find( { SCIPnodeGetNumber( node ), input_difference } ) != constraint_data->local_affine_image_seen.end() )
				return true;
		}
		return constraint_data->local_affine_image_seen.find( { -1, input_difference } ) != constraint_data->local_affine_image_seen.end();
	}

	static SCIP_RETCODE injection_rank_constraint_materialize_local_affine_image( SCIP* scip, SCIP_CONS* cons, InjectionRankConsData* constraint_data, std::uint32_t input_difference, bool& added )
	{
		added = false;
		// Local image rows are tied to the current branch-and-bound path because
		// they are valid only under the local fixing Delta=input_difference.
		// Stale rows from sibling nodes would overconstrain other deltas.
		injection_rank_constraint_prune_local_affine_image_seen_to_current_path( scip, constraint_data );
		if ( injection_rank_constraint_affine_image_seen_on_node_path( scip, constraint_data, input_difference ) )
			return SCIP_OKAY;

		const InjectionAffineImage image = constraint_data->oracle.image( constraint_data->kind, input_difference );
		if ( image.parity_checks != nullptr )
		{
			for ( std::uint64_t parity_check : *image.parity_checks )
			{
				SCIP_CALL( injection_rank_constraint_add_local_xor_parity( scip, cons, constraint_data, input_difference, image.affine_constant, parity_check, added ) );
			}
		}

		const SCIP_Longint node_number = injection_rank_cons_current_node_number( scip );
		constraint_data->local_affine_image_seen.insert( { node_number, input_difference } );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE injection_rank_constraint_add_local_no_good_pair( SCIP* scip, SCIP_CONS* cons, const InjectionRankConsData* constraint_data, std::uint32_t input_difference, std::uint64_t output_difference, bool& added )
	{
		static constexpr int TOTAL_PATTERN_BITS = WORD_SIZE + JOINT_OUTPUT_SIZE;
		std::array<SCIP_VAR*, TOTAL_PATTERN_BITS> variables {};
		std::array<SCIP_Real, TOTAL_PATTERN_BITS> coefficients {};
		int									   variable_count = 0;
		int									   zero_count = 0;

		for ( int bit_index = 0; bit_index < WORD_SIZE; ++bit_index )
		{
			const bool bit_is_one = ( ( input_difference >> bit_index ) & 1u ) != 0;
			variables[ static_cast<std::size_t>( variable_count ) ] = constraint_data->input_bits[ static_cast<std::size_t>( bit_index ) ];
			coefficients[ static_cast<std::size_t>( variable_count ) ] = bit_is_one ? 1.0 : -1.0;
			++variable_count;
			if ( !bit_is_one )
				++zero_count;
		}
		for ( int bit_index = 0; bit_index < JOINT_OUTPUT_SIZE; ++bit_index )
		{
			const bool bit_is_one = ( ( output_difference >> bit_index ) & 1u ) != 0;
			variables[ static_cast<std::size_t>( variable_count ) ] = constraint_data->output_bits[ static_cast<std::size_t>( bit_index ) ];
			coefficients[ static_cast<std::size_t>( variable_count ) ] = bit_is_one ? 1.0 : -1.0;
			++variable_count;
			if ( !bit_is_one )
				++zero_count;
		}

		const SCIP_Real right_hand_side = static_cast<SCIP_Real>( TOTAL_PATTERN_BITS - 1 - zero_count );
		SCIP_CONS*		 no_good_constraint = nullptr;
		const std::string name = std::string( SCIPconsGetName( cons ) ) + "_local_nogood_delta_" + std::to_string( input_difference ) + "_out_" + std::to_string( output_difference );
		SCIP_CALL( SCIPcreateConsBasicLinear( scip,
											  &no_good_constraint,
											  name.c_str(),
											  variable_count,
											  variables.data(),
											  coefficients.data(),
											  -SCIPinfinity( scip ),
											  right_hand_side ) );
		SCIP_CALL( SCIPaddConsLocal( scip, no_good_constraint, nullptr ) );
		SCIP_CALL( SCIPreleaseCons( scip, &no_good_constraint ) );
		added = true;
		return SCIP_OKAY;
	}

	static SCIP_RETCODE injection_rank_constraint_separate_one( SCIP* scip, SCIP_CONS* cons, SCIP_SOL* solution, SCIP_RESULT* result, bool enforcement )
	{
		// Handler flow:
		//   1. if Delta is locally fixed, tighten rank_weight >= rank(M_Delta)
		//      and materialize affine-image parity rows;
		//   2. if all bits are integral, reject output differences outside that
		//      affine image or rank weights below rank(M_Delta).
		// The explicit witness MILP remains the constructive support model; this
		// handler adds the algebraic closure that the witness alone gives weakly.
		InjectionRankConsData* constraint_data = injection_rank_cons_data( cons );
		assert( constraint_data != nullptr );
		std::uint32_t fixed_input_difference = 0;
		const bool	  input_fixed = injection_rank_constraint_input_is_locally_fixed( scip, constraint_data, fixed_input_difference );
		if ( input_fixed )
		{
			bool infeasible = false;
			bool reduced_domain = false;
			SCIP_CALL( injection_rank_constraint_tighten_weight_for_fixed_input( scip, constraint_data, fixed_input_difference, infeasible, reduced_domain ) );
			if ( infeasible )
			{
				*result = SCIP_CUTOFF;
				return SCIP_OKAY;
			}
			if ( reduced_domain )
			{
				*result = SCIP_REDUCEDDOM;
				return SCIP_OKAY;
			}

			bool added = false;
			SCIP_CALL( injection_rank_constraint_materialize_local_affine_image( scip, cons, constraint_data, fixed_input_difference, added ) );
			if ( added )
			{
				*result = SCIP_CONSADDED;
				return SCIP_OKAY;
			}
		}

		if ( !injection_rank_constraint_bits_are_integral( scip, solution, constraint_data ) )
			return SCIP_OKAY;
		const std::uint32_t input_difference = injection_rank_constraint_read_input_difference( scip, solution, constraint_data );
		const std::uint64_t output_difference = injection_rank_constraint_read_output_difference( scip, solution, constraint_data );
		const InjectionTransitionResult transition_result = constraint_data->oracle.transition( constraint_data->kind, input_difference, output_difference );
		if ( !transition_result.valid )
		{
			if ( enforcement )
			{
				*result = SCIP_INFEASIBLE;
			}
			else
			{
				bool added = false;
				SCIP_CALL( injection_rank_constraint_add_local_no_good_pair( scip, cons, constraint_data, input_difference, output_difference, added ) );
				if ( added )
					*result = SCIP_CONSADDED;
			}
			return SCIP_OKAY;
		}
		const int		 rank = transition_result.rank;
		const SCIP_Real rank_weight_value = injection_rank_constraint_solution_value( scip, solution, constraint_data->rank_weight );
		if ( SCIPisFeasLT( scip, rank_weight_value, static_cast<SCIP_Real>( rank ) ) )
		{
			if ( enforcement )
				*result = SCIP_INFEASIBLE;
			return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSENFOLP( injectionRankConsEnfolp )
	{
		( void )conshdlr;
		( void )nusefulconss;
		( void )solinfeasible;
		*result = SCIP_FEASIBLE;
		for ( int constraint_index = 0; constraint_index < nconss; ++constraint_index )
		{
			SCIP_CALL( injection_rank_constraint_separate_one( scip, conss[ constraint_index ], nullptr, result, true ) );
			if ( *result != SCIP_FEASIBLE )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSENFOPS( injectionRankConsEnfops )
	{
		( void )conshdlr;
		( void )nusefulconss;
		( void )solinfeasible;
		( void )objinfeasible;
		*result = SCIP_FEASIBLE;
		for ( int constraint_index = 0; constraint_index < nconss; ++constraint_index )
		{
			SCIP_CALL( injection_rank_constraint_separate_one( scip, conss[ constraint_index ], nullptr, result, true ) );
			if ( *result != SCIP_FEASIBLE )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSSEPALP( injectionRankConsSepalp )
	{
		( void )conshdlr;
		( void )nusefulconss;
		*result = SCIP_DIDNOTFIND;
		for ( int constraint_index = 0; constraint_index < nconss; ++constraint_index )
		{
			SCIP_CALL( injection_rank_constraint_separate_one( scip, conss[ constraint_index ], nullptr, result, false ) );
			if ( *result == SCIP_CONSADDED || *result == SCIP_CUTOFF )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSSEPASOL( injectionRankConsSepasol )
	{
		( void )conshdlr;
		( void )nusefulconss;
		*result = SCIP_DIDNOTFIND;
		for ( int constraint_index = 0; constraint_index < nconss; ++constraint_index )
		{
			SCIP_CALL( injection_rank_constraint_separate_one( scip, conss[ constraint_index ], sol, result, false ) );
			if ( *result == SCIP_CONSADDED || *result == SCIP_CUTOFF )
				return SCIP_OKAY;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSCHECK( injectionRankConsCheck )
	{
		( void )conshdlr;
		( void )checkintegrality;
		( void )checklprows;
		( void )completely;
		*result = SCIP_FEASIBLE;
		for ( int constraint_index = 0; constraint_index < nconss; ++constraint_index )
		{
			InjectionRankConsData* constraint_data = injection_rank_cons_data( conss[ constraint_index ] );
			assert( constraint_data != nullptr );
			if ( !injection_rank_constraint_bits_are_integral( scip, sol, constraint_data ) )
				continue;
			const std::uint32_t input_difference = injection_rank_constraint_read_input_difference( scip, sol, constraint_data );
			const std::uint64_t output_difference = injection_rank_constraint_read_output_difference( scip, sol, constraint_data );
			const InjectionTransitionResult transition_result = constraint_data->oracle.transition( constraint_data->kind, input_difference, output_difference );
			if ( !transition_result.valid )
			{
				if ( printreason )
					SCIPinfoMessage( scip, nullptr, "injection joint support violated: %s delta=0x%08x joint_out=0x%016llx violated_check=0x%016llx\n", SCIPconsGetName( conss[ constraint_index ] ), input_difference, static_cast<unsigned long long>( output_difference ), static_cast<unsigned long long>( transition_result.violated_parity_check ) );
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
			const int			rank = transition_result.rank;
			const SCIP_Real	rank_weight_value = injection_rank_constraint_solution_value( scip, sol, constraint_data->rank_weight );
			if ( SCIPisFeasLT( scip, rank_weight_value, static_cast<SCIP_Real>( rank ) ) )
			{
				if ( printreason )
					SCIPinfoMessage( scip, nullptr, "injection rank epigraph violated: %s delta=0x%08x w=%g rank=%d\n", SCIPconsGetName( conss[ constraint_index ] ), input_difference, rank_weight_value, rank );
				*result = SCIP_INFEASIBLE;
				return SCIP_OKAY;
			}
		}
		return SCIP_OKAY;
	}

	static SCIP_RETCODE injection_rank_constraint_tighten_weight_for_fixed_input( SCIP* scip, InjectionRankConsData* constraint_data, std::uint32_t input_difference, bool& infeasible, bool& reduced_domain )
	{
		const int rank = constraint_data->oracle.rank( constraint_data->kind, input_difference );
		if ( rank <= 0 )
			return SCIP_OKAY;

		SCIP_Bool cutoff = FALSE;
		SCIP_Bool tightened = FALSE;
		SCIP_CALL( SCIPtightenVarLb( scip, constraint_data->rank_weight, static_cast<SCIP_Real>( rank ), FALSE, &cutoff, &tightened ) );
		if ( cutoff )
			infeasible = true;
		if ( tightened )
			reduced_domain = true;
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSPROP( injectionRankConsProp )
	{
		( void )conshdlr;
		( void )nusefulconss;
		( void )nmarkedconss;
		( void )proptiming;
		*result = SCIP_DIDNOTFIND;
		for ( int constraint_index = 0; constraint_index < nconss; ++constraint_index )
		{
			InjectionRankConsData* constraint_data = injection_rank_cons_data( conss[ constraint_index ] );
			assert( constraint_data != nullptr );
			std::uint32_t input_difference = 0;
			if ( !injection_rank_constraint_input_is_locally_fixed( scip, constraint_data, input_difference ) )
				continue;

			bool infeasible = false;
			bool reduced_domain = false;
			SCIP_CALL( injection_rank_constraint_tighten_weight_for_fixed_input( scip, constraint_data, input_difference, infeasible, reduced_domain ) );
			if ( infeasible )
			{
				*result = SCIP_CUTOFF;
				return SCIP_OKAY;
			}
			if ( reduced_domain )
				*result = SCIP_REDUCEDDOM;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSLOCK( injectionRankConsLock )
	{
		( void )conshdlr;
		InjectionRankConsData* constraint_data = injection_rank_cons_data( cons );
		assert( constraint_data != nullptr );
		for ( SCIP_VAR* variable : constraint_data->input_bits )
			SCIP_CALL( SCIPaddVarLocksType( scip, variable, locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		for ( SCIP_VAR* variable : constraint_data->output_bits )
			SCIP_CALL( SCIPaddVarLocksType( scip, variable, locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		SCIP_CALL( SCIPaddVarLocksType( scip, constraint_data->rank_weight, locktype, nlockspos + nlocksneg, nlockspos + nlocksneg ) );
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSDELETE( injectionRankConsDelete )
	{
		( void )conshdlr;
		if ( consdata != nullptr && *consdata != nullptr )
		{
			auto* constraint_data = reinterpret_cast<InjectionRankConsData*>( *consdata );
			SCIP_CALL( injection_rank_constraint_data_release_variables( scip, constraint_data ) );
			delete constraint_data;
			*consdata = nullptr;
		}
		return SCIP_OKAY;
	}

	static SCIP_DECL_CONSTRANS( injectionRankConsTrans )
	{
		( void )conshdlr;
		auto* source_constraint_data = reinterpret_cast<InjectionRankConsData*>( SCIPconsGetData( sourcecons ) );
		assert( source_constraint_data != nullptr );

		auto* target_constraint_data = new InjectionRankConsData;
		target_constraint_data->kind = source_constraint_data->kind;
		SCIP_CALL( SCIPgetTransformedVars( scip, WORD_SIZE, source_constraint_data->input_bits.data(), target_constraint_data->input_bits.data() ) );
		SCIP_CALL( SCIPgetTransformedVars( scip, JOINT_OUTPUT_SIZE, source_constraint_data->output_bits.data(), target_constraint_data->output_bits.data() ) );
		SCIP_CALL( SCIPgetTransformedVar( scip, source_constraint_data->rank_weight, &target_constraint_data->rank_weight ) );
		SCIP_CALL( injection_rank_constraint_data_capture_variables( scip, target_constraint_data ) );

		SCIP_CALL( SCIPcreateCons( scip,
								   targetcons,
								   SCIPconsGetName( sourcecons ),
								   SCIPfindConshdlr( scip, "injection_rank" ),
								   reinterpret_cast<SCIP_CONSDATA*>( target_constraint_data ),
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

	static SCIP_RETCODE include_injection_rank_const_handler( SCIP* scip )
	{
		if ( SCIPfindConshdlr( scip, "injection_rank" ) != nullptr )
			return SCIP_OKAY;

		SCIP_CONSHDLR* conshdlr = nullptr;
		SCIP_CALL( SCIPincludeConshdlrBasic( scip,
											 &conshdlr,
											 "injection_rank",
											 "NeoAlzette joint injection rank epigraph",
											 1000000,
											 1000000,
											 1,
											 TRUE,
											 injectionRankConsEnfolp,
											 injectionRankConsEnfops,
											 injectionRankConsCheck,
											 injectionRankConsLock,
											 nullptr ) );
		assert( conshdlr != nullptr );
		SCIP_CALL( SCIPsetConshdlrDelete( scip, conshdlr, injectionRankConsDelete ) );
		SCIP_CALL( SCIPsetConshdlrTrans( scip, conshdlr, injectionRankConsTrans ) );
		SCIP_CALL( SCIPsetConshdlrSepa( scip, conshdlr, injectionRankConsSepalp, injectionRankConsSepasol, 1, -100000, TRUE ) );
		SCIP_CALL( SCIPsetConshdlrProp( scip, conshdlr, injectionRankConsProp, 1, FALSE, SCIP_PROPTIMING_ALWAYS ) );
		return SCIP_OKAY;
	}

	static SCIP_RETCODE add_injection_rank_constraint( SCIP* scip, const std::string& name, InjectionKind kind, const BitVector& input_bits, const BitVector& output_bits, const ScipVariable& rank_weight )
	{
		SCIP_CONSHDLR* conshdlr = SCIPfindConshdlr( scip, "injection_rank" );
		if ( conshdlr == nullptr )
		{
			SCIP_CALL( include_injection_rank_const_handler( scip ) );
			conshdlr = SCIPfindConshdlr( scip, "injection_rank" );
		}
		assert( conshdlr != nullptr );
		if ( input_bits.size() != WORD_SIZE )
			return SCIP_INVALIDDATA;
		if ( output_bits.size() != JOINT_OUTPUT_SIZE )
			return SCIP_INVALIDDATA;

		auto* constraint_data = new InjectionRankConsData;
		constraint_data->kind = kind;
		for ( int bit_index = 0; bit_index < WORD_SIZE; ++bit_index )
		{
			constraint_data->input_bits[ static_cast<std::size_t>( bit_index ) ] = input_bits[ static_cast<std::size_t>( bit_index ) ].var;
		}
		for ( int bit_index = 0; bit_index < JOINT_OUTPUT_SIZE; ++bit_index )
		{
			constraint_data->output_bits[ static_cast<std::size_t>( bit_index ) ] = output_bits[ static_cast<std::size_t>( bit_index ) ].var;
		}
		constraint_data->rank_weight = rank_weight.var;
		SCIP_CALL( injection_rank_constraint_data_capture_variables( scip, constraint_data ) );

		SCIP_CONS* cons = nullptr;
		SCIP_CALL( SCIPcreateCons( scip,
								   &cons,
								   name.c_str(),
								   conshdlr,
								   reinterpret_cast<SCIP_CONSDATA*>( constraint_data ),
								   FALSE,
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
		return SCIP_OKAY;
	}

	struct InjectionInstance
	{
		std::string	  name;
		InjectionKind kind;
		BitVector	  input_bits;
		BitVector	  xor_output_bits;
		BitVector	  add_operand_bits;
		BitVector	  joint_output_bits;
		ScipVariable rank_weight;
		std::string	  support_source = "explicit_milp_witness";
	};

	// ========================================================================
	// Audit section 5: explicit witness MILP for joint injection support
	// ========================================================================
	// This block builds H(x) and H(x xor Delta) symbolically. The handler above
	// adds affine-image parity rows and rank bounds once Delta is fixed.
	namespace injection_support_milp
	{
		[[nodiscard]] inline BitVector make_joint_output_bit_vector( const BitVector& xor_output_bits, const BitVector& add_operand_bits, const std::string& prefix )
		{
			if ( xor_output_bits.size() != WORD_SIZE || add_operand_bits.size() != WORD_SIZE )
				throw std::runtime_error( "joint injection outputs must both be 32 bits for " + prefix );
			BitVector joint_output_bits;
			joint_output_bits.reserve( JOINT_OUTPUT_SIZE );
			joint_output_bits.insert( joint_output_bits.end(), xor_output_bits.begin(), xor_output_bits.end() );
			joint_output_bits.insert( joint_output_bits.end(), add_operand_bits.begin(), add_operand_bits.end() );
			return joint_output_bits;
		}

		template <class Builder>
		[[nodiscard]] inline std::pair<BitVector, BitVector> cd_injection_from_B_bits( Builder& model_builder, const BitVector& source_bits, const std::string& prefix )
		{
			const BitVector companion0 = Builder::rotate_right( source_bits, 24 );
			const BitVector mask = [&]() {
				BitVector stage1 = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_left( source_bits, 2 ), prefix + "_mask0_stage1" );
				BitVector stage2 = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_left( stage1, 17 ), prefix + "_mask0_stage2" );
				BitVector stage3 = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_left( stage2, 4 ), prefix + "_mask0_stage3" );
				BitVector stage4 = model_builder.create_xor_bit_vector( stage3, Builder::rotate_left( stage3, 24 ), prefix + "_mask0_stage4" );
				return model_builder.create_xor_bit_vector( stage2, Builder::rotate_left( stage4, 7 ), prefix + "_mask0_mask" );
			}();
			const BitVector companion_mask = model_builder.create_constant_xor_bit_vector( Builder::rotate_right( mask, 24 ), DIFFUSION_MASK0_FROM_ROUND_CONSTANT_7, prefix + "_companion_mask" );
			const BitVector mask_r1 = Builder::rotate_right( mask, 5 );
			const BitVector x0 = model_builder.create_xor_bit_vector( companion0, mask, prefix + "_x0" );
			const BitVector x1 = model_builder.create_xor_bit_vector( source_bits, mask, prefix + "_x1" );
			const BitVector view = model_builder.create_xor_bit_vector( companion0, companion_mask, prefix + "_view" );
			const BitVector bridge_state = model_builder.create_xor_bit_vector( Builder::rotate_right( source_bits, 19 ), model_builder.shift_left_bit_vector( source_bits, 9, prefix + "_bridge_shift_left9" ), prefix + "_bridge_state" );
			const BitVector q_state_na = model_builder.create_constant_xor_bit_vector( model_builder.create_negated_bit_vector( model_builder.create_and_bit_vector( source_bits, mask, prefix + "_state_and_mask" ), prefix + "_state_not_and_mask" ), ROUND_CONSTANT_7_ROTATED_RIGHT_24, prefix + "_q_state_na" );
			const BitVector q_comp_no = model_builder.create_multiple_xor_bit_vector( { companion0, source_bits, model_builder.create_constant_xor_bit_vector( model_builder.create_negated_bit_vector( model_builder.create_or_bit_vector( companion0, mask_r1, prefix + "_comp_or_mask_r1" ), prefix + "_comp_not_or_mask_r1" ), ROUND_CONSTANT_8_ROTATED_RIGHT_24, prefix + "_comp_const_not_or" ) }, prefix + "_q_comp_no" );
			const BitVector q_bridge = model_builder.create_multiple_xor_bit_vector( { bridge_state, source_bits, model_builder.create_constant_xor_bit_vector( model_builder.create_negated_bit_vector( model_builder.create_and_bit_vector( bridge_state, companion_mask, prefix + "_bridge_and_companion_mask" ), prefix + "_bridge_not_and_companion_mask" ), ROUND_CONSTANT_13_ROTATED_RIGHT_24, prefix + "_bridge_const_not_and" ) }, prefix + "_q_bridge" );
			const BitVector q_shared = model_builder.create_xor_bit_vector( q_state_na, q_comp_no, prefix + "_q_shared" );
			const BitVector cross_q = model_builder.create_and_bit_vector( model_builder.create_xor_bit_vector( source_bits, mask_r1, prefix + "_cross_left" ), Builder::rotate_right( model_builder.create_xor_bit_vector( mask, companion_mask, prefix + "_cross_mask_mix" ), 7 ), prefix + "_cross_q" );
			const BitVector anti_q_left = model_builder.create_multiple_xor_bit_vector( { model_builder.shift_right_bit_vector( x1, 3, prefix + "_anti_x1_shr3" ), model_builder.shift_right_bit_vector( view, 5, prefix + "_anti_view_shr5" ), mask_r1 }, prefix + "_anti_left" );
			const BitVector anti_q_right = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_right( x0, 11 ), prefix + "_anti_right" );
			const BitVector anti_q = model_builder.create_and_bit_vector( anti_q_left, anti_q_right, prefix + "_anti_q" );
			BitVector c = model_builder.create_multiple_xor_bit_vector( { q_shared, Builder::rotate_right( q_comp_no, 5 ), Builder::rotate_right( q_comp_no, 11 ), anti_q }, prefix + "_C0" );
			BitVector d = model_builder.create_multiple_xor_bit_vector( { q_shared, Builder::rotate_right( q_state_na, 5 ), Builder::rotate_right( q_bridge, 13 ), cross_q, anti_q }, prefix + "_D0" );
			return { c, d };
		}

		template <class Builder>
		[[nodiscard]] inline std::pair<BitVector, BitVector> cd_injection_from_A_bits( Builder& model_builder, const BitVector& source_bits, const std::string& prefix )
		{
			const BitVector companion0 = Builder::rotate_left( source_bits, 8 );
			const BitVector mask = [&]() {
				BitVector stage1 = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_right( source_bits, 2 ), prefix + "_mask1_stage1" );
				BitVector stage2 = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_right( stage1, 17 ), prefix + "_mask1_stage2" );
				BitVector stage3 = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_right( stage2, 4 ), prefix + "_mask1_stage3" );
				BitVector stage4 = model_builder.create_xor_bit_vector( stage3, Builder::rotate_right( stage3, 24 ), prefix + "_mask1_stage4" );
				return model_builder.create_xor_bit_vector( stage2, Builder::rotate_right( stage4, 7 ), prefix + "_mask1_mask" );
			}();
			const BitVector companion_mask = model_builder.create_constant_xor_bit_vector( Builder::rotate_left( mask, 8 ), DIFFUSION_MASK1_FROM_ROUND_CONSTANT_2, prefix + "_companion_mask" );
			const BitVector mask_r1 = Builder::rotate_right( mask, 5 );
			const BitVector x0 = model_builder.create_xor_bit_vector( companion0, mask, prefix + "_x0" );
			const BitVector x1 = model_builder.create_xor_bit_vector( source_bits, mask, prefix + "_x1" );
			const BitVector view = model_builder.create_xor_bit_vector( companion0, companion_mask, prefix + "_view" );
			const BitVector bridge_state = model_builder.create_xor_bit_vector( Builder::rotate_left( source_bits, 19 ), model_builder.shift_right_bit_vector( source_bits, 9, prefix + "_bridge_shift_right9" ), prefix + "_bridge_state" );
			const BitVector q_state_no = model_builder.create_constant_xor_bit_vector( model_builder.create_negated_bit_vector( model_builder.create_or_bit_vector( source_bits, mask, prefix + "_state_or_mask" ), prefix + "_state_not_or_mask" ), ROUND_CONSTANT_2_ROTATED_LEFT_8, prefix + "_q_state_no" );
			const BitVector q_comp_na = model_builder.create_multiple_xor_bit_vector( { companion0, source_bits, model_builder.create_constant_xor_bit_vector( model_builder.create_negated_bit_vector( model_builder.create_and_bit_vector( companion0, mask_r1, prefix + "_comp_and_mask_r1" ), prefix + "_comp_not_and_mask_r1" ), ROUND_CONSTANT_3_ROTATED_LEFT_8, prefix + "_comp_const_not_and" ) }, prefix + "_q_comp_na" );
			const BitVector q_bridge = model_builder.create_multiple_xor_bit_vector( { bridge_state, source_bits, model_builder.create_constant_xor_bit_vector( model_builder.create_negated_bit_vector( model_builder.create_or_bit_vector( bridge_state, companion_mask, prefix + "_bridge_or_companion_mask" ), prefix + "_bridge_not_or_companion_mask" ), ROUND_CONSTANT_12_ROTATED_LEFT_8, prefix + "_bridge_const_not_or" ) }, prefix + "_q_bridge" );
			const BitVector q_shared = model_builder.create_xor_bit_vector( q_state_no, q_comp_na, prefix + "_q_shared" );
			const BitVector cross_q = model_builder.create_and_bit_vector( model_builder.create_xor_bit_vector( source_bits, mask_r1, prefix + "_cross_left" ), Builder::rotate_left( model_builder.create_xor_bit_vector( mask, companion_mask, prefix + "_cross_mask_mix" ), 13 ), prefix + "_cross_q" );
			const BitVector anti_q_left = model_builder.create_multiple_xor_bit_vector( { model_builder.shift_left_bit_vector( x1, 3, prefix + "_anti_x1_shl3" ), model_builder.shift_left_bit_vector( view, 5, prefix + "_anti_view_shl5" ), mask_r1 }, prefix + "_anti_left" );
			const BitVector anti_q_right = model_builder.create_xor_bit_vector( source_bits, Builder::rotate_left( x0, 11 ), prefix + "_anti_right" );
			const BitVector anti_q = model_builder.create_or_bit_vector( anti_q_left, anti_q_right, prefix + "_anti_q" );
			BitVector c = model_builder.create_multiple_xor_bit_vector( { q_shared, Builder::rotate_left( q_comp_na, 5 ), Builder::rotate_left( q_comp_na, 11 ), anti_q }, prefix + "_C1" );
			BitVector d = model_builder.create_multiple_xor_bit_vector( { q_shared, Builder::rotate_left( q_state_no, 5 ), Builder::rotate_left( q_bridge, 13 ), cross_q, anti_q }, prefix + "_D1" );
			return { c, d };
		}

		template <class Builder>
		[[nodiscard]] inline std::pair<BitVector, BitVector> joint_injection_outputs_from_witness_bits( Builder& model_builder, const BitVector& source_bits, const std::string& prefix, InjectionKind kind )
		{
			switch ( kind )
			{
			case InjectionKind::B_TO_A_AFTER_RC4:
			{
				auto [ c0, d0 ] = cd_injection_from_B_bits( model_builder, source_bits, prefix + "_from_B" );
				BitVector xor_output = model_builder.create_multiple_xor_bit_vector( { Builder::rotate_left( source_bits, 24 ), Builder::rotate_left( c0, 16 ), Builder::rotate_left( source_bits, 8 ) }, prefix + "_xor_to_A" );
				BitVector cd0 = model_builder.create_xor_bit_vector( model_builder.shift_left_bit_vector( c0, 2, prefix + "_CD0_C0_shl2" ), model_builder.shift_right_bit_vector( d0, 2, prefix + "_CD0_D0_shr2" ), prefix + "_CD0" );
				BitVector cd1 = model_builder.create_xor_bit_vector( model_builder.shift_right_bit_vector( c0, 5, prefix + "_CD1_C0_shr5" ), model_builder.shift_left_bit_vector( d0, 5, prefix + "_CD1_D0_shl5" ), prefix + "_CD1" );
				BitVector add_operand = model_builder.create_xor_bit_vector( Builder::rotate_left( cd0, 31 ), Builder::rotate_left( cd1, 17 ), prefix + "_add0_operand" );
				return { xor_output, add_operand };
			}
			case InjectionKind::A_TO_B_AFTER_RC9:
			{
				auto [ c1, d1 ] = cd_injection_from_A_bits( model_builder, source_bits, prefix + "_from_A" );
				BitVector xor_output = model_builder.create_multiple_xor_bit_vector( { Builder::rotate_right( source_bits, 24 ), Builder::rotate_right( d1, 16 ), Builder::rotate_right( source_bits, 8 ) }, prefix + "_xor_to_B" );
				BitVector cd2 = model_builder.create_xor_bit_vector( model_builder.shift_right_bit_vector( c1, 3, prefix + "_CD2_C1_shr3" ), model_builder.shift_left_bit_vector( d1, 3, prefix + "_CD2_D1_shl3" ), prefix + "_CD2" );
				BitVector cd3 = model_builder.create_xor_bit_vector( model_builder.shift_left_bit_vector( c1, 1, prefix + "_CD3_C1_shl1" ), model_builder.shift_right_bit_vector( d1, 1, prefix + "_CD3_D1_shr1" ), prefix + "_CD3" );
				BitVector add_operand = model_builder.create_xor_bit_vector( cd2, cd3, prefix + "_add1_operand" );
				return { xor_output, add_operand };
			}
			}
			throw std::runtime_error( "unknown injection kind in joint_injection_outputs_from_witness_bits" );
		}

		template <class Builder>
		[[nodiscard]] inline std::pair<BitVector, BitVector> create_joint_injection_differences( Builder& model_builder, std::vector<InjectionInstance>& injections, const BitVector& controlling_difference, const std::string& prefix, InjectionKind kind )
		{
			BitVector xor_output_difference = model_builder.create_bit_vector( prefix + "_xor_delta", WORD_SIZE );
			BitVector add_operand_difference = model_builder.create_bit_vector( prefix + "_add_operand_delta", WORD_SIZE );
			ScipVariable rank_weight = model_builder.create_continuous_variable( prefix + "_rank_weight", 0.0, 32.0, 1.0 );

			BitVector witness_source = model_builder.create_bit_vector( prefix + "_support_witness_x", WORD_SIZE );
			BitVector witness_source_prime = model_builder.create_xor_bit_vector( witness_source, controlling_difference, prefix + "_support_witness_x_prime" );
			auto [ witness_xor_output, witness_add_operand ] = joint_injection_outputs_from_witness_bits( model_builder, witness_source, prefix + "_support_H_x", kind );
			auto [ witness_prime_xor_output, witness_prime_add_operand ] = joint_injection_outputs_from_witness_bits( model_builder, witness_source_prime, prefix + "_support_H_x_prime", kind );
			BitVector witness_xor_difference = model_builder.create_xor_bit_vector( witness_xor_output, witness_prime_xor_output, prefix + "_support_xor_output_delta_from_witness" );
			BitVector witness_add_operand_difference = model_builder.create_xor_bit_vector( witness_add_operand, witness_prime_add_operand, prefix + "_support_add_operand_delta_from_witness" );
			for ( int bit_index = 0; bit_index < WORD_SIZE; ++bit_index )
			{
				model_builder.add_equality_to_zero_constraint( prefix + "_bind_xor_output_delta_" + std::to_string( bit_index ), { { xor_output_difference[ bit_index ], 1 }, { witness_xor_difference[ bit_index ], -1 } } );
				model_builder.add_equality_to_zero_constraint( prefix + "_bind_add_operand_delta_" + std::to_string( bit_index ), { { add_operand_difference[ bit_index ], 1 }, { witness_add_operand_difference[ bit_index ], -1 } } );
			}

			model_builder.set_branch_priority( controlling_difference, 200000 );
			model_builder.set_branch_priority( xor_output_difference, 100000 );
			model_builder.set_branch_priority( add_operand_difference, 100000 );
			BitVector joint_output_bits = make_joint_output_bit_vector( xor_output_difference, add_operand_difference, prefix + "_joint_delta" );
			SCIP_CALL_THROW( add_injection_rank_constraint( model_builder.scip, prefix + "_joint_rank", kind, controlling_difference, joint_output_bits, rank_weight ) );
			injections.push_back( { prefix, kind, controlling_difference, xor_output_difference, add_operand_difference, joint_output_bits, rank_weight, "explicit_milp_witness_plus_local_oracle_constraint" } );
			return { xor_output_difference, add_operand_difference };
		}
	}  // namespace injection_support_milp

}  // namespace neoalzette_diff_milp
