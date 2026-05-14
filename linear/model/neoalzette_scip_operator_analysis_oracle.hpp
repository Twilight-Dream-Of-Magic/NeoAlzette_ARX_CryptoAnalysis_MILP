#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

typedef struct SCIP_Var SCIP_VAR;

// ============================================================================
// NeoAlzette LINEAR modular add/sub models and reference oracles
// ============================================================================
//
// Scope:
//   * Linear masks over GF(2), little-endian bit order.
//   * Word size n is in [1, 32].
//   * Arithmetic is modulo 2^n.
//
// Bibliographic lineage and code mapping:
//   * Two-variable modular addition/subtraction linear correlations:
//       Johan Wallen,
//       "Linear Approximations of Addition Modulo 2^n",
//       FSE 2003, LNCS 2887, pp. 261-273,
//       DOI 10.1007/978-3-540-39887-5_20.
//     The oracle below implements the same common-prefix/carry-mask condition
//     used by Wallen's logarithmic-depth treatment.
//       Kai Fu, Meiqin Wang, Yinghua Guo, Siwei Sun, Lei Hu,
//       "MILP-Based Automatic Search Algorithms for Differential and Linear
//       Trails for Speck", FSE 2016, LNCS 9783, pp. 268-288.
//     The two-variable MILP box is Fu-Wang-Guo's 8-inequality transition model
//     for (s_{i+1}, Gamma_i, A_i, B_i, s_i), not an embedded cpm oracle.
//
//   * Fixed public addend / one variable plus one constant:
//       Hiroshi Miyano,
//       "Addend dependency of differential/linear probability of addition",
//       IEICE Transactions on Fundamentals E81-A(1), pp. 106-109, Jan. 1998.
//     The exact oracle is the 2x2 carry-state transfer matrix form of Miyano's
//     fixed-addend LAP recurrence.  The strict MILP layer encodes the same
//     two-state signed transfer recurrence as static threshold constraints.
//
// Important distinction:
//   * The oracles compute exact local signed correlations.
//   * The two-variable MILP box searches a Wallen/Fu-Wang-Guo linear
//     characteristic and minimizes -log2(abs(correlation contribution)).
//   * The active fixed-addend MILP layer must encode the exact 2-state transfer
//     relation for the visible fixed-addend LAP threshold.
//   * Linear hull aggregation is not encoded in this local MILP.
// ============================================================================

namespace neoalzette_linear_milp
{
	inline constexpr double INF = 1e20;

	// ------------------------------------------------------------------------
	// Audit section 0: shared symbolic MILP value types
	// ------------------------------------------------------------------------
	// These are lightweight handles only. The arithmetic theory starts in the
	// linear_oracle namespace below.
	struct SVar
	{
		SCIP_VAR*	var = nullptr;
		std::string name;
	};

	using BitVec = std::vector<SVar>;

	struct LinearTerm
	{
		SVar   v;
		double c;
	};
}  // namespace neoalzette_linear_milp

namespace neoalzette_linear_milp::linear_oracle
{
	// ------------------------------------------------------------------------
	// Audit section 1: Wallen-style two-variable linear add/sub Q1 oracle
	// ------------------------------------------------------------------------
	// Purpose: offline validation and trace audit for the local add/sub boxes.
	// Solver-facing MILP inequalities live in
	// neoalzette_scip_operator_analysis_milp_constraint.hpp.
	struct LinearOracleResult
	{
		bool		  possible = false;
		int			  sign = 0;
		std::uint32_t weight = std::numeric_limits<std::uint32_t>::max();
		long double	  correlation = 0.0L;
		long double	  abs_correlation = 0.0L;
	};

	struct FixedConstLinearOracleResult
	{
		bool		possible = false;
		int			sign = 0;
		long double weight = std::numeric_limits<long double>::infinity();
		long double correlation = 0.0L;
		long double abs_correlation = 0.0L;
	};

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

	[[nodiscard]] inline int word_bit64( std::uint64_t x, int i )
	{
		if ( i < 0 || i >= 64 )
			return 0;
		return static_cast<int>( ( x >> i ) & 1ull );
	}

	[[nodiscard]] inline int popcount32( std::uint32_t x )
	{
#if defined( __GNUC__ ) || defined( __clang__ )
		return __builtin_popcount( x );
#else
		int n = 0;
		while ( x != 0 )
		{
			x &= ( x - 1 );
			++n;
		}
		return n;
#endif
	}

	[[nodiscard]] inline int parity32( std::uint32_t x )
	{
		return popcount32( x ) & 1;
	}

	[[nodiscard]] inline int highest_active_bit( std::uint32_t x, int bits )
	{
		if ( bits <= 0 )
			return -1;
		if ( bits > 32 )
			throw std::invalid_argument( "highest_active_bit: bit width must be <= 32" );
		x &= mask_for_bits( bits );
		for ( int i = bits - 1; i >= 0; --i )
			if ( ( x >> i ) & 1u )
				return i;
		return -1;
	}

	[[nodiscard]] inline long double pow2_neg( long double weight )
	{
		return std::ldexp( 1.0L, -static_cast<int>( weight ) );
	}

	[[nodiscard]] inline std::uint32_t suffix_parity_mask( std::uint32_t x )
	{
		x ^= x >> 1;
		x ^= x >> 2;
		x ^= x >> 4;
		x ^= x >> 8;
		x ^= x >> 16;
		return x;
	}

	// Wallen/Schulte-Geers common-prefix weight mask, expressed as the log-time
	// parallel prefix scan used by the two-variable linear correlation test.  With
	// little-endian word bits this is the free/weight mask from
	//   m[n-1]=0, m[j]=xor_{k=j+1..n-1}(u[k] xor v[k] xor w[k]).
	// The nonzero test is therefore (p & ~m)==0 and (q & ~m)==0, matching the
	// z_i >= u_i xor v_i, z_i >= u_i xor w_i characteristic MILP box below.
	[[nodiscard]] inline std::uint32_t cpm( std::uint32_t x, std::uint32_t y, int bits )
	{
		if ( bits <= 0 || bits > 32 )
			throw std::invalid_argument( "cpm: bit width must be in [1, 32]" );
		const std::uint32_t mask = mask_for_bits( bits );
		const std::uint32_t eq = ~( x ^ y ) & mask;
		return ( suffix_parity_mask( eq ) >> 1 ) & mask;
	}

	[[nodiscard]] inline LinearOracleResult oracle_add2( std::uint32_t u, std::uint32_t v, std::uint32_t w, int bits )
	{
		if ( bits <= 0 || bits > 32 )
			return {};
		const std::uint32_t mask = mask_for_bits( bits );
		u &= mask;
		v &= mask;
		w &= mask;

		const std::uint32_t p = ( v ^ u ) & mask;
		const std::uint32_t q = ( w ^ u ) & mask;
		const std::uint32_t m = cpm( u, ~( p ^ q ) & mask, bits );
		if ( ( p & ~m ) != 0u || ( q & ~m ) != 0u )
			return {};

		const std::uint32_t weight = static_cast<std::uint32_t>( popcount32( m ) );
		const int			sign = parity32( p & q ) ? -1 : 1;
		const long double	abs_c = pow2_neg( static_cast<long double>( weight ) );
		return { true, sign, weight, static_cast<long double>( sign ) * abs_c, abs_c };
	}

	[[nodiscard]] inline LinearOracleResult oracle_sub2( std::uint32_t u, std::uint32_t v, std::uint32_t w, int bits )
	{
		return oracle_add2( v, u, w, bits );
	}

	// ------------------------------------------------------------------------
	// Audit section 2: Miyano fixed-addend signed-correlation Q1 oracle
	// ------------------------------------------------------------------------
	// Purpose: exact offline check for X -> X+K and trace metadata. The active
	// solver model compiles the same two-state recurrence into static MILP plus
	// the exact log-weight epigraph handler.
	[[nodiscard]] inline FixedConstLinearOracleResult oracle_add_const( std::uint32_t constant, std::uint32_t a, std::uint32_t b, int bits, std::uint32_t g = 0 )
	{
		if ( bits <= 0 || bits > 32 )
			return {};
		const std::uint32_t mask = mask_for_bits( bits );
		constant &= mask;
		a &= mask;
		b &= mask;
		g &= mask;

		if ( highest_active_bit( a, bits ) != highest_active_bit( b, bits ) )
			return {};

		long double cur[ 2 ] = { 1.0L, 0.0L };
		for ( int i = 0; i < bits; ++i )
		{
			long double next[ 2 ] = { 0.0L, 0.0L };
			const int	ki = word_bit( constant, i );
			const int	ai = word_bit( a, i );
			const int	bi = word_bit( b, i );
			const int	si = ( word_bit( g, i ) & ki ) ? -1 : 1;
			for ( int c = 0; c <= 1; ++c )
			{
				if ( cur[ c ] == 0.0L )
					continue;
				if ( c == ki )
				{
					if ( ai == bi )
						next[ ki ] += cur[ c ] * static_cast<long double>( si );
				}
				else
				{
					next[ 0 ] += cur[ c ] * static_cast<long double>( si ) * 0.5L * ( bi ? -1.0L : 1.0L );
					next[ 1 ] += cur[ c ] * static_cast<long double>( si ) * 0.5L * ( ai ? -1.0L : 1.0L );
				}
			}
			cur[ 0 ] = next[ 0 ];
			cur[ 1 ] = next[ 1 ];
		}

		const long double corr = cur[ 0 ] + cur[ 1 ];
		const long double abs_c = std::fabs( corr );
		if ( abs_c == 0.0L )
			return {};
		const int sign = corr < 0.0L ? -1 : 1;
		return { true, sign, -std::log2( abs_c ), corr, abs_c };
	}

	[[nodiscard]] inline FixedConstLinearOracleResult oracle_sub_const( std::uint32_t constant, std::uint32_t a, std::uint32_t b, int bits, std::uint32_t g = 0 )
	{
		const std::uint32_t mask = mask_for_bits( bits );
		const std::uint32_t neg_constant = ( std::uint32_t( 0 ) - ( constant & mask ) ) & mask;
		return oracle_add_const( neg_constant, a, b, bits, g );
	}

	[[nodiscard]] inline int fixed_const_characteristic_sign( std::uint32_t constant, std::uint32_t a, std::uint32_t b, std::uint64_t carry_path_mask, std::uint32_t bad_mask, int bits, std::uint32_t g = 0 )
	{
		int phase = 0;
		for ( int i = 0; i < bits; ++i )
		{
			const int ki = word_bit( constant, i );
			const int qi = word_bit( bad_mask, i );
			phase ^= ( word_bit( g, i ) & ki );
			if ( qi != 0 )
			{
				const int cn = word_bit64( carry_path_mask, i + 1 );
				phase ^= ( cn == 0 ) ? word_bit( b, i ) : word_bit( a, i );
			}
		}
		return phase ? -1 : 1;
	}
}  // namespace neoalzette_linear_milp::linear_oracle

// ============================================================================
// NeoAlzette joint injection LINEAR Walsh oracle
// ============================================================================

namespace neoalzette_linear_milp
{
	// ------------------------------------------------------------------------
	// Audit section 3: NeoAlzette joint injection value functions
	// ------------------------------------------------------------------------
	// These constexpr functions are the source of truth for J(x). The linear
	// Walsh oracle below only derives polar/quad data from this value-level map.
	static constexpr int		   WORD_SIZE = 32;
	static constexpr int		   JOINT_INJECTION_BETA_SIZE = 64;
	static constexpr std::uint32_t WORD_MASK = 0xFFFFFFFFu;
	static constexpr int		   FIRST_BRIDGE_ROTATE0 = 22;
	static constexpr int		   FIRST_BRIDGE_ROTATE1 = 13;
	static constexpr int		   SECOND_BRIDGE_ROT_B_TO_A = 5;
	static constexpr int		   SECOND_BRIDGE_ROT_A_TO_B = 25;

	static constexpr std::uint32_t ROUND_CONSTANTS[ 16 ] = { 0x16B2C40B, 0xC117176A, 0x0F9A2598, 0xA1563ACA, 0x243F6A88, 0x85A308D3, 0x13198A2E, 0x03707344, 0x9E3779B9, 0x7F4A7C15, 0xF39CC060, 0x5CEDC834, 0xB7E15162, 0x8AED2A6A, 0xBF715880, 0x9CF4F3C7 };

#define SCIP_CALL_THROW( x )                                                                                 \
	do                                                                                                         \
	{                                                                                                          \
		SCIP_RETCODE _retcode = ( x );                                                                        \
		if ( _retcode != SCIP_OKAY )                                                                          \
		{                                                                                                      \
			std::ostringstream _oss;                                                                           \
			_oss << "SCIP call failed at " << __FILE__ << ":" << __LINE__ << " code=" << _retcode;          \
			throw std::runtime_error( _oss.str() );                                                           \
		}                                                                                                      \
	} while ( false )

	static constexpr std::uint32_t rotl32( std::uint32_t x, int r )
	{
		r &= 31;
		return r == 0 ? x : static_cast<std::uint32_t>( ( x << r ) | ( x >> ( 32 - r ) ) );
	}

	static constexpr std::uint32_t rotr32( std::uint32_t x, int r )
	{
		r &= 31;
		return r == 0 ? x : static_cast<std::uint32_t>( ( x >> r ) | ( x << ( 32 - r ) ) );
	}

	static constexpr std::uint32_t u32_not( std::uint32_t x )
	{
		return ~x;
	}

	static constexpr std::uint32_t generate_dynamic_diffusion_mask0_value( std::uint32_t x )
	{
		const std::uint32_t v0 = x;
		const std::uint32_t v1 = v0 ^ rotl32( v0, 2 );
		const std::uint32_t v2 = v0 ^ rotl32( v1, 17 );
		const std::uint32_t v3 = v0 ^ rotl32( v2, 4 );
		const std::uint32_t v4 = v3 ^ rotl32( v3, 24 );
		return v2 ^ rotl32( v4, 7 );
	}

	static constexpr std::uint32_t generate_dynamic_diffusion_mask1_value( std::uint32_t x )
	{
		const std::uint32_t v0 = x;
		const std::uint32_t v1 = v0 ^ rotr32( v0, 2 );
		const std::uint32_t v2 = v0 ^ rotr32( v1, 17 );
		const std::uint32_t v3 = v0 ^ rotr32( v2, 4 );
		const std::uint32_t v4 = v3 ^ rotr32( v3, 24 );
		return v2 ^ rotr32( v4, 7 );
	}

	static const std::uint32_t RC7_R24 = rotr32( ROUND_CONSTANTS[ 7 ], 24 );
	static const std::uint32_t RC8_R24 = rotr32( ROUND_CONSTANTS[ 8 ], 24 );
	static const std::uint32_t RC13_R24 = rotr32( ROUND_CONSTANTS[ 13 ], 24 );
	static const std::uint32_t RC2_L8 = rotl32( ROUND_CONSTANTS[ 2 ], 8 );
	static const std::uint32_t RC3_L8 = rotl32( ROUND_CONSTANTS[ 3 ], 8 );
	static const std::uint32_t RC12_L8 = rotl32( ROUND_CONSTANTS[ 12 ], 8 );
	static const std::uint32_t MASK0_RC7 = generate_dynamic_diffusion_mask0_value( ROUND_CONSTANTS[ 7 ] );
	static const std::uint32_t MASK1_RC2 = generate_dynamic_diffusion_mask1_value( ROUND_CONSTANTS[ 2 ] );

	static constexpr std::pair<std::uint32_t, std::uint32_t> cd_injection_from_B_value( std::uint32_t B )
	{
		const std::uint32_t companion0 = rotr32( B, 24 );
		const std::uint32_t mask = generate_dynamic_diffusion_mask0_value( B );
		const std::uint32_t companion_mask = rotr32( mask, 24 ) ^ MASK0_RC7;
		const std::uint32_t mask_r1 = rotr32( mask, 5 );
		const std::uint32_t x0 = companion0 ^ mask;
		const std::uint32_t x1 = B ^ mask;
		const std::uint32_t view = companion0 ^ companion_mask;
		const std::uint32_t bridge_state = rotr32( B, 19 ) ^ ( B << 9 );
		const std::uint32_t q_state_na = RC7_R24 ^ u32_not( B & mask );
		const std::uint32_t q_comp_no = companion0 ^ B ^ RC8_R24 ^ u32_not( companion0 | mask_r1 );
		const std::uint32_t q_bridge = bridge_state ^ B ^ RC13_R24 ^ u32_not( bridge_state & companion_mask );
		const std::uint32_t q_shared = q_state_na ^ q_comp_no;
		const std::uint32_t cross_q = ( B ^ mask_r1 ) & rotr32( mask ^ companion_mask, 7 );
		const std::uint32_t anti_q = ( ( x1 >> 3 ) ^ ( view >> 5 ) ^ mask_r1 ) & ( B ^ rotr32( x0, 11 ) );
		const std::uint32_t c = q_shared ^ rotr32( q_comp_no, 5 ) ^ rotr32( q_comp_no, 11 ) ^ anti_q;
		const std::uint32_t d = q_shared ^ rotr32( q_state_na, 5 ) ^ rotr32( q_bridge, 13 ) ^ cross_q ^ anti_q;
		return { c, d };
	}

	static constexpr std::pair<std::uint32_t, std::uint32_t> cd_injection_from_A_value( std::uint32_t A )
	{
		const std::uint32_t companion0 = rotl32( A, 8 );
		const std::uint32_t mask = generate_dynamic_diffusion_mask1_value( A );
		const std::uint32_t companion_mask = rotl32( mask, 8 ) ^ MASK1_RC2;
		const std::uint32_t mask_r1 = rotr32( mask, 5 );
		const std::uint32_t x0 = companion0 ^ mask;
		const std::uint32_t x1 = A ^ mask;
		const std::uint32_t view = companion0 ^ companion_mask;
		const std::uint32_t bridge_state = rotl32( A, 19 ) ^ ( A >> 9 );
		const std::uint32_t q_state_no = RC2_L8 ^ u32_not( A | mask );
		const std::uint32_t q_comp_na = companion0 ^ A ^ RC3_L8 ^ u32_not( companion0 & mask_r1 );
		const std::uint32_t q_bridge = bridge_state ^ A ^ RC12_L8 ^ u32_not( bridge_state | companion_mask );
		const std::uint32_t q_shared = q_state_no ^ q_comp_na;
		const std::uint32_t cross_q = ( A ^ mask_r1 ) & rotl32( mask ^ companion_mask, 13 );
		const std::uint32_t anti_q = ( ( x1 << 3 ) ^ ( view << 5 ) ^ mask_r1 ) | ( A ^ rotl32( x0, 11 ) );
		const std::uint32_t c = q_shared ^ rotl32( q_comp_na, 5 ) ^ rotl32( q_comp_na, 11 ) ^ anti_q;
		const std::uint32_t d = q_shared ^ rotl32( q_state_no, 5 ) ^ rotl32( q_bridge, 13 ) ^ cross_q ^ anti_q;
		return { c, d };
	}

	static constexpr std::uint32_t b_to_a_xor_side_value( std::uint32_t B )
	{
		const auto [ c, d ] = cd_injection_from_B_value( B );
		(void)d;
		return rotl32( B, 24 ) ^ rotl32( c, 16 ) ^ rotl32( B, 8 );
	}

	static constexpr std::uint32_t b_to_a_addend_side_value( std::uint32_t B )
	{
		const auto [ c, d ] = cd_injection_from_B_value( B );
		const std::uint32_t cd0 = ( c << 2 ) ^ ( d >> 2 );
		const std::uint32_t cd1 = ( c >> 5 ) ^ ( d << 5 );
		return rotl32( cd0, 31 ) ^ rotl32( cd1, 17 );
	}

	static constexpr std::uint32_t a_to_b_xor_side_value( std::uint32_t A )
	{
		const auto [ c, d ] = cd_injection_from_A_value( A );
		(void)c;
		return rotr32( A, 24 ) ^ rotr32( d, 16 ) ^ rotr32( A, 8 );
	}

	static constexpr std::uint32_t a_to_b_addend_side_value( std::uint32_t A )
	{
		const auto [ c, d ] = cd_injection_from_A_value( A );
		const std::uint32_t cd2 = ( c >> 3 ) ^ ( d << 3 );
		const std::uint32_t cd3 = ( c << 1 ) ^ ( d >> 1 );
		return cd2 ^ cd3;
	}

	static constexpr std::uint64_t pack_joint_injection_output( std::uint32_t beta_xor_side, std::uint32_t beta_addend_side )
	{
		return static_cast<std::uint64_t>( beta_xor_side ) | ( static_cast<std::uint64_t>( beta_addend_side ) << 32 );
	}

	static int popcount32( std::uint32_t x )
	{
		return linear_oracle::popcount32( x );
	}

	static int parity32( std::uint32_t x )
	{
		return linear_oracle::parity32( x );
	}

	static int popcount64( std::uint64_t x )
	{
#if defined( __GNUC__ ) || defined( __clang__ )
		return __builtin_popcountll( x );
#else
		int n = 0;
		while ( x != 0 )
		{
			x &= ( x - 1 );
			++n;
		}
		return n;
#endif
	}

	static int parity64( std::uint64_t x )
	{
		return popcount64( x ) & 1;
	}

	enum class InjectionKind
	{
		B_TO_A_JOINT,
		A_TO_B_JOINT
	};

	struct InjectionLinearOracleResult
	{
		bool		  valid = false;
		int			  sign = 0;
		int			  rank = 0;
		double		  weight = std::numeric_limits<double>::infinity();
		std::uint32_t alpha = 0;
		std::uint64_t beta = 0;
		std::uint32_t beta_xor = 0;
		std::uint32_t beta_addend = 0;
		std::uint32_t support_offset = 0;
		std::vector<std::pair<std::uint32_t, int>> support_parities;
	};

	// ------------------------------------------------------------------------
	// Audit section 4: quadratic Walsh support/rank oracle for joint injection
	// ------------------------------------------------------------------------
	// Oracle-to-MILP bridge for the joint injection layer, linear side.
	// For a fixed packed output mask beta, the scalar component beta . J(x) is a
	// quadratic Boolean function of the 32-bit source word. Its Walsh transform
	// is either zero or has magnitude 2^{-rank/2}; the nonzero support is an
	// affine set of admissible source masks alpha. This is why the MILP handler
	// below talks about support parities and a rank/2 weight instead of trying to
	// enumerate a 2^32 table.
	//
	// beta is intentionally packed as (xor-side mask, addend-side mask). The
	// injection output feeds both an XOR edge and the following fixed-addend
	// modular addition, so detaching the addend mask would lose the actual local
	// linear approximation being charged.
	class LinearInjectionOracle
	{
	public:
		using PolarTable = std::array<std::array<std::uint64_t, 32>, 32>;

		struct SupportDescriptor
		{
			bool possible = false;
			int	 rank = 0;
			std::uint32_t offset = 0;
			std::vector<std::pair<std::uint32_t, int>> parities;
		};

		InjectionLinearOracleResult transition( InjectionKind kind, std::uint32_t alpha, std::uint64_t beta )
		{
			auto key = std::make_tuple( static_cast<int>( kind ), alpha, beta );
			auto it = cache_.find( key );
			if ( it != cache_.end() )
				return it->second;

			// Build the quadratic form of alpha . x xor beta . J(x). The linear
			// term absorbs alpha and the first-order polar columns of beta . J;
			// the upper-triangular quad array stores the second-order polar terms.
			const std::uint64_t f0 = function( kind, 0u );
			std::uint32_t		linear = alpha;
			for ( int i = 0; i < 32; ++i )
			{
				if ( parity64( beta & ( function( kind, std::uint32_t( 1 ) << i ) ^ f0 ) ) )
					linear ^= ( std::uint32_t( 1 ) << i );
			}

			std::array<std::uint32_t, 32> quad {};
			const PolarTable&			  polar = polar_table( kind );
			for ( int i = 0; i < 32; ++i )
			{
				for ( int j = i + 1; j < 32; ++j )
				{
					if ( parity64( beta & polar[ i ][ j ] ) )
						quad[ i ] ^= ( std::uint32_t( 1 ) << j );
				}
			}

			const auto			support = support_descriptor( kind, beta );
			auto				wr = quadratic_walsh( parity64( beta & f0 ), linear, quad );
			InjectionLinearOracleResult out;
			out.alpha = alpha;
			out.beta = beta;
			out.beta_xor = static_cast<std::uint32_t>( beta & WORD_MASK );
			out.beta_addend = static_cast<std::uint32_t>( beta >> 32 );
			out.support_offset = support.offset;
			out.support_parities = support.parities;
			out.rank = support.rank;
			out.weight = support.possible ? static_cast<double>( support.rank ) / 2.0 : std::numeric_limits<double>::infinity();
			out.valid = support.possible && support_contains( support, alpha ) && wr.valid;
			out.sign = out.valid ? wr.sign : 0;
			cache_.emplace( key, out );
			return out;
		}

		int rank_for_output_mask( InjectionKind kind, std::uint64_t beta )
		{
			auto key = std::make_pair( static_cast<int>( kind ), beta );
			auto it = rank_cache_.find( key );
			if ( it != rank_cache_.end() )
				return it->second;
			auto res = transition( kind, 0u, beta );
			rank_cache_[ key ] = res.rank;
			return res.rank;
		}

		SupportDescriptor support( InjectionKind kind, std::uint64_t beta )
		{
			return support_descriptor( kind, beta );
		}

	private:
		struct WalshReductionResult
		{
			bool valid = false;
			int	 sign = 0;
			int	 rank = 0;
			std::vector<std::pair<std::uint32_t, int>> support_parities;
		};

		std::map<std::tuple<int, std::uint32_t, std::uint64_t>, InjectionLinearOracleResult> cache_;
		std::map<std::pair<int, std::uint64_t>, int>										 rank_cache_;
		std::map<std::pair<int, std::uint64_t>, SupportDescriptor>							 support_cache_;

		static std::uint64_t function( InjectionKind kind, std::uint32_t x )
		{
			switch ( kind )
			{
			case InjectionKind::B_TO_A_JOINT:
				return pack_joint_injection_output( b_to_a_xor_side_value( x ), b_to_a_addend_side_value( x ) );
			case InjectionKind::A_TO_B_JOINT:
				return pack_joint_injection_output( a_to_b_xor_side_value( x ), a_to_b_addend_side_value( x ) );
			}
			return 0;
		}

		static PolarTable make_polar_table( InjectionKind kind )
		{
			PolarTable table {};
			const std::uint64_t f0 = function( kind, 0u );
			std::array<std::uint64_t, 32> fe {};
			for ( int i = 0; i < 32; ++i )
				fe[ i ] = function( kind, std::uint32_t( 1 ) << i );
			for ( int i = 0; i < 32; ++i )
			{
				for ( int j = 0; j < 32; ++j )
				{
					const std::uint32_t ei = std::uint32_t( 1 ) << i;
					const std::uint32_t ej = std::uint32_t( 1 ) << j;
					table[ i ][ j ] = fe[ i ] ^ fe[ j ] ^ function( kind, ei ^ ej ) ^ f0;
				}
			}
			return table;
		}

		static const PolarTable& polar_table( InjectionKind kind )
		{
			static const PolarTable b = make_polar_table( InjectionKind::B_TO_A_JOINT );
			static const PolarTable a = make_polar_table( InjectionKind::A_TO_B_JOINT );
			return kind == InjectionKind::B_TO_A_JOINT ? b : a;
		}

		static bool quad_get( const std::array<std::uint32_t, 32>& q, int a, int b )
		{
			if ( a == b )
				return false;
			if ( a > b )
				std::swap( a, b );
			return ( q[ a ] >> b ) & 1u;
		}

		static void quad_toggle( std::array<std::uint32_t, 32>& q, int a, int b )
		{
			if ( a == b )
				return;
			if ( a > b )
				std::swap( a, b );
			q[ a ] ^= ( std::uint32_t( 1 ) << b );
		}

		static void quad_clear_incident( std::array<std::uint32_t, 32>& q, int v )
		{
			q[ v ] = 0u;
			for ( int i = 0; i < v; ++i )
				q[ i ] &= ~( std::uint32_t( 1 ) << v );
		}

		static int xor_basis_add_32( std::array<std::uint32_t, 32>& basis_by_msb, std::uint32_t v )
		{
			while ( v != 0u )
			{
				const unsigned		bit = 31u - std::countl_zero( v );
				const std::uint32_t basis = basis_by_msb[ bit ];
				if ( basis != 0u )
					v ^= basis;
				else
				{
					basis_by_msb[ bit ] = v;
					return 1;
				}
			}
			return 0;
		}

		static int compute_nullspace_basis_32( const std::array<std::uint32_t, 32>& rows_in, std::array<std::uint32_t, 32>& nullspace_basis )
		{
			std::array<std::uint32_t, 32> rows = rows_in;
			std::array<int, 32>			  pivot_cols {};
			pivot_cols.fill( -1 );

			int rank = 0;
			for ( int col = 0; col < 32 && rank < 32; ++col )
			{
				int selected = -1;
				for ( int row = rank; row < 32; ++row )
				{
					if ( ( ( rows[ row ] >> col ) & 1u ) != 0u )
					{
						selected = row;
						break;
					}
				}
				if ( selected < 0 )
					continue;
				if ( selected != rank )
					std::swap( rows[ selected ], rows[ rank ] );
				for ( int row = 0; row < 32; ++row )
				{
					if ( row != rank && ( ( rows[ row ] >> col ) & 1u ) != 0u )
						rows[ row ] ^= rows[ rank ];
				}
				pivot_cols[ rank ] = col;
				++rank;
			}

			nullspace_basis.fill( 0u );
			int nullity = 0;
			for ( int free_col = 0; free_col < 32; ++free_col )
			{
				bool is_pivot = false;
				for ( int row = 0; row < rank; ++row )
				{
					if ( pivot_cols[ row ] == free_col )
					{
						is_pivot = true;
						break;
					}
				}
				if ( is_pivot )
					continue;

				std::uint32_t vector = ( 1u << free_col );
				for ( int row = rank - 1; row >= 0; --row )
				{
					const int pivot_col = pivot_cols[ row ];
					if ( pivot_col >= 0 && parity32( rows[ row ] & vector ) != 0 )
						vector ^= ( 1u << pivot_col );
				}
				nullspace_basis[ nullity++ ] = vector;
			}
			return nullity;
		}

		static bool solve_linear_system_rows_32( const std::array<std::uint32_t, 32>& rows_in, std::uint32_t rhs_bits, std::uint32_t& solution )
		{
			std::array<std::uint32_t, 32> rows = rows_in;
			std::array<std::uint8_t, 32>  rhs {};
			std::array<int, 32>			  pivot_col_for_row {};
			for ( int i = 0; i < 32; ++i )
			{
				rhs[ i ] = static_cast<std::uint8_t>( ( rhs_bits >> i ) & 1u );
				pivot_col_for_row[ i ] = -1;
			}

			int pivot_row = 0;
			for ( int col = 0; col < 32 && pivot_row < 32; ++col )
			{
				int selected = -1;
				for ( int row = pivot_row; row < 32; ++row )
				{
					if ( ( ( rows[ row ] >> col ) & 1u ) != 0u )
					{
						selected = row;
						break;
					}
				}
				if ( selected < 0 )
					continue;
				if ( selected != pivot_row )
				{
					std::swap( rows[ selected ], rows[ pivot_row ] );
					std::swap( rhs[ selected ], rhs[ pivot_row ] );
				}
				for ( int row = 0; row < 32; ++row )
				{
					if ( row != pivot_row && ( ( rows[ row ] >> col ) & 1u ) != 0u )
					{
						rows[ row ] ^= rows[ pivot_row ];
						rhs[ row ] ^= rhs[ pivot_row ];
					}
				}
				pivot_col_for_row[ pivot_row ] = col;
				++pivot_row;
			}

			for ( int row = pivot_row; row < 32; ++row )
				if ( rhs[ row ] != 0u )
					return false;

			solution = 0u;
			for ( int row = 0; row < pivot_row; ++row )
			{
				if ( rhs[ row ] == 0u )
					continue;
				const int pivot_col = pivot_col_for_row[ row ];
				if ( pivot_col < 0 )
					return false;
				solution ^= ( 1u << pivot_col );
			}
			return true;
		}

		SupportDescriptor support_descriptor( InjectionKind kind, std::uint64_t beta )
		{
			auto key = std::make_pair( static_cast<int>( kind ), beta );
			auto it = support_cache_.find( key );
			if ( it != support_cache_.end() )
				return it->second;

			SupportDescriptor out;
			// The Walsh support of a quadratic Boolean function is an affine
			// coset constrained by the kernel of its associated alternating
			// bilinear form. rows is that bilinear matrix; kernel parities become
			// SCIP XOR rows when beta is fixed.
			const std::uint64_t f0 = function( kind, 0u );
			const int			q0 = parity64( beta & f0 );
			std::array<std::uint32_t, 32> rows {};
			const PolarTable& polar = polar_table( kind );
			for ( int i = 0; i < 32; ++i )
			{
				for ( int j = i + 1; j < 32; ++j )
				{
					if ( parity64( beta & polar[ i ][ j ] ) )
					{
						rows[ i ] |= ( 1u << j );
						rows[ j ] |= ( 1u << i );
					}
				}
			}

			std::array<std::uint32_t, 32> row_basis {};
			for ( const std::uint32_t row : rows )
				out.rank += xor_basis_add_32( row_basis, row );

			std::array<std::uint32_t, 32> kernel_basis {};
			const int kernel_dim = compute_nullspace_basis_32( rows, kernel_basis );
			std::array<std::uint32_t, 32> kernel_rows {};
			std::uint32_t				 kernel_rhs_bits = 0u;
			for ( int i = 0; i < kernel_dim; ++i )
			{
				kernel_rows[ i ] = kernel_basis[ i ];
				const int qz = parity64( beta & function( kind, kernel_basis[ i ] ) );
				if ( ( qz ^ q0 ) != 0 )
					kernel_rhs_bits |= ( 1u << i );
			}

			std::uint32_t offset = 0u;
			if ( !solve_linear_system_rows_32( kernel_rows, kernel_rhs_bits, offset ) )
			{
				out.possible = false;
				support_cache_.emplace( key, out );
				return out;
			}
			out.offset = offset;
			for ( int i = 0; i < kernel_dim; ++i )
			{
				const std::uint32_t mask = kernel_basis[ i ];
				if ( mask != 0u )
					out.parities.push_back( { mask, parity32( mask & offset ) } );
			}
			out.possible = true;
			support_cache_.emplace( key, out );
			return out;
		}

		static bool support_contains( const SupportDescriptor& support, std::uint32_t alpha )
		{
			if ( !support.possible )
				return false;
			for ( const auto& [ mask, rhs ] : support.parities )
				if ( parity32( mask & alpha ) != rhs )
					return false;
			return true;
		}

		static WalshReductionResult quadratic_walsh( int constant, std::uint32_t linear, std::array<std::uint32_t, 32> quad )
		{
			// Symplectic elimination for quadratic Boolean Walsh transforms:
			// each nonzero quadratic pair contributes rank 2 and halves the
			// Walsh magnitude by one bit, hence final weight rank/2. Variables
			// left active after all pairs are removed are kernel directions; a
			// remaining linear term on such a direction makes the coefficient zero.
			std::uint32_t active = 0xFFFFFFFFu;
			int			  pairs = 0;
			std::array<std::uint32_t, 32> linear_masks {};
			for ( int i = 0; i < 32; ++i )
				linear_masks[ i ] = std::uint32_t( 1 ) << i;
			std::vector<std::pair<std::uint32_t, int>> support_parities;
			while ( true )
			{
				int pi = -1, pj = -1;
				for ( int i = 0; i < 32 && pi < 0; ++i )
				{
					if ( ( active & ( std::uint32_t( 1 ) << i ) ) == 0u )
						continue;
					for ( int j = i + 1; j < 32; ++j )
					{
						if ( ( active & ( std::uint32_t( 1 ) << j ) ) != 0u && quad_get( quad, i, j ) )
						{
							pi = i;
							pj = j;
							break;
						}
					}
				}
				if ( pi < 0 )
					break;

				++pairs;
				const int ca = ( linear >> pi ) & 1u;
				const int cb = ( linear >> pj ) & 1u;
				std::uint32_t avec = 0u;
				std::uint32_t bvec = 0u;
				for ( int k = 0; k < 32; ++k )
				{
					if ( k == pi || k == pj || ( active & ( std::uint32_t( 1 ) << k ) ) == 0u )
						continue;
					if ( quad_get( quad, pi, k ) )
						avec |= ( std::uint32_t( 1 ) << k );
					if ( quad_get( quad, pj, k ) )
						bvec |= ( std::uint32_t( 1 ) << k );
				}

				quad_clear_incident( quad, pi );
				quad_clear_incident( quad, pj );
				active &= ~( std::uint32_t( 1 ) << pi );
				active &= ~( std::uint32_t( 1 ) << pj );
				linear &= ~( std::uint32_t( 1 ) << pi );
				linear &= ~( std::uint32_t( 1 ) << pj );

				if ( ca && cb )
					constant ^= 1;
				if ( ca )
				{
					linear ^= bvec;
					for ( int k = 0; k < 32; ++k )
						if ( ( bvec >> k ) & 1u )
							linear_masks[ k ] ^= linear_masks[ pi ];
				}
				if ( cb )
				{
					linear ^= avec;
					for ( int k = 0; k < 32; ++k )
						if ( ( avec >> k ) & 1u )
							linear_masks[ k ] ^= linear_masks[ pj ];
				}
				for ( int a = 0; a < 32; ++a )
				{
					if ( ( avec & ( std::uint32_t( 1 ) << a ) ) == 0u )
						continue;
					for ( int b = 0; b < 32; ++b )
					{
						if ( ( bvec & ( std::uint32_t( 1 ) << b ) ) == 0u )
							continue;
						if ( a == b )
							linear ^= ( std::uint32_t( 1 ) << a );
						else
							quad_toggle( quad, a, b );
					}
				}
			}

			for ( int i = 0; i < 32; ++i )
				if ( ( active >> i ) & 1u )
					support_parities.push_back( { linear_masks[ i ], 0 } );
			if ( ( linear & active ) != 0u )
			{
				WalshReductionResult out;
				out.valid = false;
				out.sign = 0;
				out.rank = 2 * pairs;
				out.support_parities = std::move( support_parities );
				return out;
			}
			return { true, constant ? -1 : 1, 2 * pairs, std::move( support_parities ) };
		}
	};

	static LinearInjectionOracle& global_injection_oracle()
	{
		static LinearInjectionOracle oracle;
		return oracle;
	}


}  // namespace neoalzette_linear_milp
