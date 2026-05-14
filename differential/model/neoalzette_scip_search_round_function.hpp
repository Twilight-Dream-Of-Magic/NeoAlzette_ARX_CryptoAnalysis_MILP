#pragma once

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/pub_tree.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "neoalzette_scip_operator_analysis_milp_constraint.hpp"

// ============================================================================
// NeoAlzette XOR-Differential round-search model builder and search functions
// ============================================================================
//
// This header contains shared best-characteristic and round-table orchestration.
//
// Intentional split:
//   * operator_analysis_oracle.hpp: injection Oracle + support MILP.
//   * operator_analysis_milp_constraint.hpp: pure differential MILP constraints.
//   * search_round_function.hpp: round-level build()/build_one_round(), search
//     options, solve helpers, JSON output helpers.
//
// Audit map:
//   1. result/trace data structures and JSON helpers;
//   2. NeoAlzetteScipEngineeringSearch: round-level MILP construction;
//   3. solution finalization and Q1/injection trace audit;
//   4. CLI parsing and run modes.
//
// Entry points call the shared functions here. No .cpp text inclusion and no
// search-mode macro dispatch are used.
// ============================================================================

namespace neoalzette_diff_milp
{
	// ------------------------------------------------------------------------
	// Audit section 1: result/trace data structures and JSON helpers
	// ------------------------------------------------------------------------
	// This first block contains solution readers, trace/result records, and JSON
	// serialization helpers. It does not add SCIP constraints.
	static std::uint32_t value_from_solution( SCIP* scip, SCIP_SOL* solution, const BitVector& bits )
	{
		if ( bits.size() > WORD_SIZE )
			throw std::runtime_error( "value_from_solution only supports up to 32 bits" );
		std::uint32_t word_value = 0;
		for ( int bit_index = 0; bit_index < static_cast<int>( bits.size() ); ++bit_index )
			if ( SCIPgetSolVal( scip, solution, bits[ bit_index ].var ) > 0.5 )
				word_value |= ( 1u << bit_index );
		return word_value;
	}

	static std::uint64_t value64_from_solution( SCIP* scip, SCIP_SOL* solution, const BitVector& bits )
	{
		if ( bits.size() > JOINT_OUTPUT_SIZE )
			throw std::runtime_error( "value64_from_solution only supports up to 64 bits" );
		std::uint64_t word_value = 0;
		for ( int bit_index = 0; bit_index < static_cast<int>( bits.size() ); ++bit_index )
			if ( SCIPgetSolVal( scip, solution, bits[ bit_index ].var ) > 0.5 )
				word_value |= ( std::uint64_t( 1 ) << bit_index );
		return word_value;
	}

	struct NoGoodCut
	{
		std::vector<std::pair<std::string, int>> assignment;
	};

	struct EnumerationState
	{
		std::vector<NoGoodCut>											no_good_cuts;
	};

	enum class ConstantModel
	{
		FIXED_PUBLIC_EXACT,
		ZERO_DIFFERENCE_AVERAGE
	};

	struct TraceNode
	{
		int			round = 0;
		std::string stage;
		BitVector		A;
		BitVector		B;
	};

	struct WeightStepSpec
	{
		int			round = 0;
		int			step = 0;
		std::string stage;
		std::string operation;
		std::string prefix;
		std::string weight_model;
		BitVector		A_before;
		BitVector		B_before;
		BitVector		A_after;
		BitVector		B_after;
		bool		has_local_input0 = false;
		bool		has_local_input1 = false;
		bool		has_local_output = false;
		std::string local_input0_label;
		std::string local_input1_label;
		std::string local_output_label;
		BitVector		local_input0;
		BitVector		local_input1;
		BitVector		local_output;
		bool		has_public_constant = false;
		std::uint32_t public_constant = 0;
		std::string injection_name;
		std::size_t objective_begin = 0;
		std::size_t objective_end = 0;
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
		std::uint32_t A_before = 0;
		std::uint32_t B_before = 0;
		std::uint32_t A_after = 0;
		std::uint32_t B_after = 0;
		bool		has_local_input0 = false;
		bool		has_local_input1 = false;
		bool		has_local_output = false;
		std::string local_input0_label;
		std::string local_input1_label;
		std::string local_output_label;
		std::uint32_t local_input0 = 0;
		std::uint32_t local_input1 = 0;
		std::uint32_t local_output = 0;
		bool		has_public_constant = false;
		std::uint32_t public_constant = 0;
		bool		has_injection_audit = false;
		std::string injection_name;
		std::string injection_support_source;
		bool		injection_support_valid = false;
		int			injection_joint_rank = 0;
		double		model_rank_weight = 0.0;
		std::uint32_t injection_din = 0;
		std::uint32_t injection_dout_xor = 0;
		std::uint32_t injection_dout_add = 0;
		std::uint64_t injection_joint_dout = 0;
		std::uint64_t injection_affine_constant = 0;
		double		local_weight = 0.0;
		double		cumulative_weight = 0.0;
		int			objective_term_count = 0;
		int			selected_term_count = 0;
		std::vector<WeightTraceTerm> selected_terms;
	};

	struct SearchOptions
	{
		int							 rounds = 1;
		ConstantModel				 constant_model = ConstantModel::FIXED_PUBLIC_EXACT;
		bool						 quiet = false;
		bool						 objective_window_enabled = false;
		double						 objective_window_from = std::numeric_limits<double>::quiet_NaN();
		double						 objective_window_to = std::numeric_limits<double>::quiet_NaN();
		double						 time_limit_seconds = std::numeric_limits<double>::quiet_NaN();
		std::string					 output_result_json;
		std::string					 output_weight_trace_json;
		std::string					 output_round_table_json;
		std::optional<std::uint32_t> fix_input_da;
		std::optional<std::uint32_t> fix_input_db;
		std::optional<std::uint32_t> fix_output_da;
		std::optional<std::uint32_t> fix_output_db;
	};

	struct SolutionSnapshot
	{
		double		  objective = 0.0;
		std::uint32_t dA_in = 0;
		std::uint32_t dB_in = 0;
		std::uint32_t dA_out = 0;
		std::uint32_t dB_out = 0;
	};

	struct ScipSolveResult
	{
		bool			 feasible = false;
		bool			 complete = false;
		bool			 hit_time_limit = false;
		bool			 hit_memory_limit = false;
		bool			 hit_solution_limit = false;
		SCIP_STATUS		 scip_status = SCIP_STATUS_UNKNOWN;
		std::string		 error_message;
		SolutionSnapshot snapshot;
		NoGoodCut		 no_good;
		std::vector<WeightTraceEntry> weight_trace;
		double						 weight_trace_total = 0.0;
		double						 weight_trace_objective_delta = 0.0;
		bool						 weight_trace_available = false;
		bool						 weight_trace_matches_objective = false;
	};

	struct RoundTableRow
	{
		int			 rounds = 0;
		ScipSolveResult best_trail;
	};

	static std::string hex32( std::uint32_t x )
	{
		std::ostringstream oss;
		oss << std::hex << std::setw( 8 ) << std::setfill( '0' ) << x;
		return oss.str();
	}

	static std::string hex32_json( std::uint32_t x )
	{
		return "0x" + hex32( x );
	}
	static std::string hex64( std::uint64_t x )
	{
		std::ostringstream oss;
		oss << std::hex << std::setfill( '0' ) << std::setw( 16 ) << x << std::dec;
		return oss.str();
	}
	static std::string hex64_json( std::uint64_t x )
	{
		return "0x" + hex64( x );
	}

	static const char* constant_model_name( ConstantModel mode )
	{
		return mode == ConstantModel::FIXED_PUBLIC_EXACT ? "fixed-public-exact" : "zero-difference-average";
	}

	static std::string scip_status_name( SCIP_STATUS status )
	{
		switch ( status )
		{
		case SCIP_STATUS_UNKNOWN:
			return "unknown";
		case SCIP_STATUS_OPTIMAL:
			return "optimal";
		case SCIP_STATUS_INFEASIBLE:
			return "infeasible";
		case SCIP_STATUS_UNBOUNDED:
			return "unbounded";
		case SCIP_STATUS_INFORUNBD:
			return "infeasible_or_unbounded";
		case SCIP_STATUS_USERINTERRUPT:
			return "user_interrupt";
		case SCIP_STATUS_TERMINATE:
			return "terminate";
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
		case SCIP_STATUS_PRIMALLIMIT:
			return "primal_limit";
		case SCIP_STATUS_DUALLIMIT:
			return "dual_limit";
		case SCIP_STATUS_SOLLIMIT:
			return "solution_limit";
		case SCIP_STATUS_BESTSOLLIMIT:
			return "best_solution_limit";
		case SCIP_STATUS_RESTARTLIMIT:
			return "restart_limit";
		default:
			return "status_" + std::to_string( static_cast<int>( status ) );
		}
	}

	static std::uint32_t parse_u32_hex_or_dec( const std::string& text )
	{
		std::size_t	  pos = 0;
		unsigned long value = 0;
		if ( text.size() > 2 && text[ 0 ] == '0' && ( text[ 1 ] == 'x' || text[ 1 ] == 'X' ) )
			value = std::stoul( text, &pos, 16 );
		else
			value = std::stoul( text, &pos, 0 );
		if ( pos != text.size() )
			throw std::runtime_error( "invalid integer: " + text );
		return static_cast<std::uint32_t>( value & 0xFFFFFFFFul );
	}

	static std::string weight_key( double weight )
	{
		std::ostringstream output_stream;
		output_stream << std::fixed << std::setprecision( 9 ) << weight;
		return output_stream.str();
	}

	static std::string json_escape( const std::string& text )
	{
		std::ostringstream output_stream;
		for ( unsigned char character : text )
		{
			switch ( character )
			{
			case '\\':
				output_stream << "\\\\";
				break;
			case '"':
				output_stream << "\\\"";
				break;
			case '\b':
				output_stream << "\\b";
				break;
			case '\f':
				output_stream << "\\f";
				break;
			case '\n':
				output_stream << "\\n";
				break;
			case '\r':
				output_stream << "\\r";
				break;
			case '\t':
				output_stream << "\\t";
				break;
			default:
				if ( character < 0x20 )
					output_stream << "\\u" << std::hex << std::setw( 4 ) << std::setfill( '0' ) << static_cast<int>( character ) << std::dec << std::setfill( ' ' );
				else
					output_stream << static_cast<char>( character );
				break;
			}
		}
		return output_stream.str();
	}

	static std::string json_string( const std::string& text )
	{
		return "\"" + json_escape( text ) + "\"";
	}

	static std::string json_number( double value )
	{
		if ( !std::isfinite( value ) )
			return "null";
		std::ostringstream output_stream;
		output_stream << std::setprecision( 17 ) << value;
		return output_stream.str();
	}

	static std::string json_number( long double value )
	{
		if ( !std::isfinite( static_cast<double>( value ) ) )
			return "null";
		std::ostringstream output_stream;
		output_stream << std::setprecision( 17 ) << static_cast<double>( value );
		return output_stream.str();
	}

	static std::string probability_string_from_weight( double weight )
	{
		return "2^-" + weight_key( weight );
	}

	static std::string scientific_string( long double value )
	{
		if ( value == 0.0L )
			return "0";
		if ( !std::isfinite( static_cast<double>( value ) ) )
			return "null";
		std::ostringstream output_stream;
		output_stream << std::scientific << std::setprecision( 17 ) << static_cast<double>( value );
		return output_stream.str();
	}

	static void write_weight_trace_terms_json( std::ostream& output_stream, const std::vector<WeightTraceTerm>& terms, const std::string& indent )
	{
		output_stream << "[";
		if ( !terms.empty() )
			output_stream << "\n";
		for ( std::size_t term_index = 0; term_index < terms.size(); ++term_index )
		{
			const auto& term = terms[ term_index ];
			output_stream << indent << "  {"
						  << "\"variable\": " << json_string( term.variable )
						  << ", \"coefficient\": " << json_number( term.coefficient )
						  << ", \"value\": " << json_number( term.value )
						  << ", \"contribution\": " << json_number( term.contribution )
						  << "}";
			if ( term_index + 1 != terms.size() )
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
			const auto& trace_entry = trace[ trace_index ];
			output_stream << indent << "  {\n"
						  << indent << "    \"round\": " << trace_entry.round << ",\n"
						  << indent << "    \"step\": " << trace_entry.step << ",\n"
						  << indent << "    \"stage\": " << json_string( trace_entry.stage ) << ",\n"
						  << indent << "    \"operation\": " << json_string( trace_entry.operation ) << ",\n"
						  << indent << "    \"prefix\": " << json_string( trace_entry.prefix ) << ",\n"
						  << indent << "    \"weight_model\": " << json_string( trace_entry.weight_model ) << ",\n"
						  << indent << "    \"state_before\": {\"A\": " << json_string( hex32_json( trace_entry.A_before ) ) << ", \"B\": " << json_string( hex32_json( trace_entry.B_before ) ) << "},\n"
						  << indent << "    \"state_after\": {\"A\": " << json_string( hex32_json( trace_entry.A_after ) ) << ", \"B\": " << json_string( hex32_json( trace_entry.B_after ) ) << "},\n"
						  << indent << "    \"public_constant\": " << ( trace_entry.has_public_constant ? json_string( hex32_json( trace_entry.public_constant ) ) : "null" ) << ",\n"
						  << indent << "    \"local_input0\": " << ( trace_entry.has_local_input0 ? json_string( hex32_json( trace_entry.local_input0 ) ) : "null" ) << ",\n"
						  << indent << "    \"local_input0_label\": " << ( trace_entry.has_local_input0 ? json_string( trace_entry.local_input0_label ) : "null" ) << ",\n"
						  << indent << "    \"local_input1\": " << ( trace_entry.has_local_input1 ? json_string( hex32_json( trace_entry.local_input1 ) ) : "null" ) << ",\n"
						  << indent << "    \"local_input1_label\": " << ( trace_entry.has_local_input1 ? json_string( trace_entry.local_input1_label ) : "null" ) << ",\n"
						  << indent << "    \"local_output\": " << ( trace_entry.has_local_output ? json_string( hex32_json( trace_entry.local_output ) ) : "null" ) << ",\n"
						  << indent << "    \"local_output_label\": " << ( trace_entry.has_local_output ? json_string( trace_entry.local_output_label ) : "null" ) << ",\n"
						  << indent << "    \"joint_injection\": ";
			if ( trace_entry.has_injection_audit )
			{
				output_stream << "{"
							  << "\"name\": " << json_string( trace_entry.injection_name )
							  << ", \"joint_input_delta\": " << json_string( hex32_json( trace_entry.injection_din ) )
							  << ", \"xor_output_delta\": " << json_string( hex32_json( trace_entry.injection_dout_xor ) )
							  << ", \"add_operand_delta\": " << json_string( hex32_json( trace_entry.injection_dout_add ) )
							  << ", \"joint_output_delta\": " << json_string( hex64_json( trace_entry.injection_joint_dout ) )
							  << ", \"support_source\": " << json_string( trace_entry.injection_support_source )
							  << ", \"support_audit_valid\": " << ( trace_entry.injection_support_valid ? "true" : "false" )
							  << ", \"joint_rank\": " << trace_entry.injection_joint_rank
							  << ", \"rank_weight\": " << json_number( trace_entry.model_rank_weight )
							  << ", \"affine_constant\": " << json_string( hex64_json( trace_entry.injection_affine_constant ) )
							  << "}";
			}
			else
			{
				output_stream << "null";
			}
			output_stream << ",\n"
						  << indent << "    \"local_weight\": " << json_number( trace_entry.local_weight ) << ",\n"
						  << indent << "    \"cumulative_weight\": " << json_number( trace_entry.cumulative_weight ) << ",\n"
						  << indent << "    \"objective_term_count\": " << trace_entry.objective_term_count << ",\n"
						  << indent << "    \"selected_term_count\": " << trace_entry.selected_term_count << ",\n"
						  << indent << "    \"selected_objective_terms\": ";
			write_weight_trace_terms_json( output_stream, trace_entry.selected_terms, indent + "    " );
			output_stream << "\n" << indent << "  }";
			if ( trace_index + 1 != trace.size() )
				output_stream << ",";
			output_stream << "\n";
		}
		output_stream << indent << "]";
	}

	static void write_best_result_json_object( std::ostream& output_stream, const SearchOptions& options, const ScipSolveResult& result, const std::string& indent, bool include_weight_trace = true )
	{
		const bool	   has_weight_trace = result.feasible && result.weight_trace_available;
		output_stream << indent << "{\n"
					  << indent << "  \"analysis\": \"xor_differential_single_characteristic\",\n"
					  << indent << "  \"cipher\": \"NeoAlzette\",\n"
					  << indent << "  \"backend\": \"SCIP_C_API\",\n"
					  << indent << "  \"rounds\": " << options.rounds << ",\n"
					  << indent << "  \"constant_model\": " << json_string( constant_model_name( options.constant_model ) ) << ",\n"
					  << indent << "  \"injection_model\": \"explicit-milp-witness-support-plus-joint-affine-rank\",\n"
					  << indent << "  \"solver_status\": " << json_string( scip_status_name( result.scip_status ) ) << ",\n"
					  << indent << "  \"hit_time_limit\": " << ( result.hit_time_limit ? "true" : "false" ) << ",\n"
					  << indent << "  \"hit_memory_limit\": " << ( result.hit_memory_limit ? "true" : "false" ) << ",\n"
					  << indent << "  \"hit_solution_limit\": " << ( result.hit_solution_limit ? "true" : "false" ) << ",\n"
					  << indent << "  \"error_message\": " << ( result.error_message.empty() ? "null" : json_string( result.error_message ) ) << ",\n"
					  << indent << "  \"objective_weight\": " << ( result.feasible ? json_number( result.snapshot.objective ) : "null" ) << ",\n"
					  << indent << "  \"probability\": " << ( result.feasible ? json_string( probability_string_from_weight( result.snapshot.objective ) ) : "null" ) << ",\n"
					  << indent << "  \"weight_trace_available\": " << ( has_weight_trace ? "true" : "false" ) << ",\n"
					  << indent << "  \"weight_trace_total\": " << ( has_weight_trace ? json_number( result.weight_trace_total ) : "null" ) << ",\n"
					  << indent << "  \"weight_trace_objective_delta\": " << ( has_weight_trace ? json_number( result.weight_trace_objective_delta ) : "null" ) << ",\n"
					  << indent << "  \"weight_trace_matches_objective\": " << ( has_weight_trace && result.weight_trace_matches_objective ? "true" : "false" ) << ",\n";
		if ( include_weight_trace )
		{
			output_stream << indent << "  \"weight_trace\": ";
			write_weight_trace_json_array( output_stream, result.weight_trace, indent + "  " );
			output_stream << ",\n";
		}
		output_stream << indent << "  \"external_difference\": ";
		if ( result.feasible )
		{
			output_stream << indent << "{\n"
						  << indent << "    \"dA_in\": " << json_string( hex32_json( result.snapshot.dA_in ) ) << ",\n"
						  << indent << "    \"dB_in\": " << json_string( hex32_json( result.snapshot.dB_in ) ) << ",\n"
						  << indent << "    \"dA_out\": " << json_string( hex32_json( result.snapshot.dA_out ) ) << ",\n"
						  << indent << "    \"dB_out\": " << json_string( hex32_json( result.snapshot.dB_out ) ) << "\n"
						  << indent << "  }";
		}
		else
		{
			output_stream << "null";
		}
		output_stream << ",\n"
					  << indent << "  \"complete\": " << ( result.complete ? "true" : "false" ) << "\n"
					  << indent << "}";
	}

	static void write_best_result_json_file( const std::string& path, const SearchOptions& options, const ScipSolveResult& result )
	{
		if ( path.empty() )
			return;
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
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open weight-trace JSON output file: " + path );
		out << "{\n"
			<< "  \"analysis\": \"xor_differential_single_characteristic_weight_trace\",\n"
			<< "  \"cipher\": \"NeoAlzette\",\n"
			<< "  \"backend\": \"SCIP_C_API\",\n"
			<< "  \"rounds\": " << options.rounds << ",\n"
			<< "  \"constant_model\": " << json_string( constant_model_name( options.constant_model ) ) << ",\n"
			<< "  \"injection_model\": \"explicit-milp-witness-support-plus-joint-affine-rank\",\n"
			<< "  \"solver_status\": " << json_string( scip_status_name( result.scip_status ) ) << ",\n"
			<< "  \"hit_time_limit\": " << ( result.hit_time_limit ? "true" : "false" ) << ",\n"
			<< "  \"hit_memory_limit\": " << ( result.hit_memory_limit ? "true" : "false" ) << ",\n"
			<< "  \"hit_solution_limit\": " << ( result.hit_solution_limit ? "true" : "false" ) << ",\n"
			<< "  \"error_message\": " << ( result.error_message.empty() ? "null" : json_string( result.error_message ) ) << ",\n"
			<< "  \"objective_weight\": " << ( result.feasible ? json_number( result.snapshot.objective ) : "null" ) << ",\n"
			<< "  \"weight_trace_available\": " << ( result.feasible && result.weight_trace_available ? "true" : "false" ) << ",\n"
			<< "  \"weight_trace_total\": " << ( result.feasible && result.weight_trace_available ? json_number( result.weight_trace_total ) : "null" ) << ",\n"
			<< "  \"weight_trace_objective_delta\": " << ( result.feasible && result.weight_trace_available ? json_number( result.weight_trace_objective_delta ) : "null" ) << ",\n"
			<< "  \"weight_trace_matches_objective\": " << ( result.feasible && result.weight_trace_available && result.weight_trace_matches_objective ? "true" : "false" ) << ",\n"
			<< "  \"complete\": " << ( result.complete ? "true" : "false" ) << ",\n"
			<< "  \"weight_trace\": ";
		write_weight_trace_json_array( out, result.weight_trace, "  " );
		out << "\n}\n";
		std::cout << "WEIGHT_TRACE_JSON_FILE=" << path << "\n";
	}

	static void ensure_default_best_trail_json_paths( SearchOptions& options )
	{
		if ( !options.output_result_json.empty() && !options.output_weight_trace_json.empty() )
			return;

		const std::string stem = "scip_best_round" + std::to_string( options.rounds );
		if ( options.output_result_json.empty() )
			options.output_result_json = stem + "_result.json";
		if ( options.output_weight_trace_json.empty() )
			options.output_weight_trace_json = stem + "_weight_trace.json";
	}

	static void write_current_incumbent_artifacts( const SearchOptions& options, const ScipSolveResult& result )
	{
		if ( !result.feasible )
			return;
		if ( options.output_result_json.empty() && options.output_weight_trace_json.empty() )
			return;
		write_best_result_json_file( options.output_result_json, options, result );
		write_weight_trace_json_file( options.output_weight_trace_json, options, result );
	}

	static void write_round_table_json_file( const std::string& path, const SearchOptions& base_options, const std::vector<RoundTableRow>& rows )
	{
		if ( path.empty() )
			return;
		std::ofstream out( path );
		if ( !out )
			throw std::runtime_error( "failed to open round-table JSON output file: " + path );
		out << "{\n"
			<< "  \"analysis\": \"xor_differential_round_table\",\n"
			<< "  \"cipher\": \"NeoAlzette\",\n"
			<< "  \"backend\": \"SCIP_C_API\",\n"
			<< "  \"constant_model\": " << json_string( constant_model_name( base_options.constant_model ) ) << ",\n"
			<< "  \"injection_model\": \"explicit-milp-witness-support-plus-joint-affine-rank\",\n"
			<< "  \"table_kind\": \"prefix_best_single_characteristics\",\n"
			<< "  \"notes\": \"rows contain only MILP best single characteristics for rounds 1..R.\",\n"
			<< "  \"rows\": [";
		if ( !rows.empty() )
			out << "\n";
		for ( std::size_t i = 0; i < rows.size(); ++i )
		{
			const auto& row = rows[ i ];
			SearchOptions row_options = base_options;
			row_options.rounds = row.rounds;
			out << "    {\n"
				<< "      \"rounds\": " << row.rounds << ",\n"
				<< "      \"best_trail\": ";
			write_best_result_json_object( out, row_options, row.best_trail, "      ", true );
			out << "\n"
				<< "    }";
			if ( i + 1 != rows.size() )
				out << ",";
			out << "\n";
		}
		out << "  ]\n}\n";
		std::cout << "ROUND_TABLE_JSON_FILE=" << path << "\n";
	}

	struct NeoAlzetteScipEngineeringSearch
	{
		// --------------------------------------------------------------------
		// Audit section 2: round-level differential MILP construction
		// --------------------------------------------------------------------
		// The helper methods below are grouped by semantic operator: fixed public
		// constants, joint injection witness/rank bridge, modular add/sub boxes,
		// and finally build_one_round() as the schedule.
		SearchOptions				   options;
		std::vector<InjectionInstance> injections;
		std::vector<TraceNode>		   trace;
		std::vector<WeightStepSpec>	   weight_steps;
		std::vector<BitVector>			   canonical_bit_groups;
		BitVector						   dA_in, dB_in, dA_out, dB_out;

		explicit NeoAlzetteScipEngineeringSearch( SearchOptions opt ) : options( std::move( opt ) )
		{
			if ( options.rounds < 1 )
				throw std::runtime_error( "--rounds must be >= 1" );
		}

		BitVector create_two_rotation_xor( ScipModelBuilder& model_builder, const BitVector& input_bits, int first_rotation, int second_rotation, const std::string& prefix )
		{
			return model_builder.create_xor_bit_vector( ScipModelBuilder::rotate_left( input_bits, first_rotation ), ScipModelBuilder::rotate_left( input_bits, second_rotation ), prefix );
		}

		std::pair<BitVector, BitVector> create_joint_injection_differences( ScipModelBuilder& model_builder, const BitVector& controlling_difference, const std::string& prefix, InjectionKind kind )
		{
			return injection_support_milp::create_joint_injection_differences( model_builder, injections, controlling_difference, prefix, kind );
		}

		void add_fixed_constant_addition_operation( ScipModelBuilder& model_builder, std::uint32_t constant, const BitVector& input_difference, const BitVector& output_difference, const std::string& prefix )
		{
			if ( options.constant_model == ConstantModel::FIXED_PUBLIC_EXACT )
				arithmetic_model::add_fixed_public_constant_exact( model_builder, constant, input_difference, output_difference, prefix );
			else
				arithmetic_model::add_zero_diff_operand_average( model_builder, input_difference, output_difference, prefix );
		}

		void add_fixed_constant_subtraction_operation( ScipModelBuilder& model_builder, std::uint32_t constant, const BitVector& input_difference, const BitVector& output_difference, const std::string& prefix )
		{
			if ( options.constant_model == ConstantModel::FIXED_PUBLIC_EXACT )
				arithmetic_model::add_fixed_public_constant_sub_exact( model_builder, constant, input_difference, output_difference, prefix );
			else
				arithmetic_model::add_zero_diff_operand_average( model_builder, input_difference, output_difference, prefix );
		}

		void add_trace( int round, const std::string& stage, const BitVector& A, const BitVector& B )
		{
			trace.push_back( { round, stage, A, B } );
			canonical_bit_groups.push_back( A );
			canonical_bit_groups.push_back( B );
		}

		void add_weight_step( WeightStepSpec spec )
		{
			spec.step = static_cast<int>( weight_steps.size() );
			weight_steps.push_back( std::move( spec ) );
		}

		void add_public_xor_constant_step( ScipModelBuilder& model_builder, int round, const std::string& stage, const std::string& prefix, std::uint32_t constant, const BitVector& A_difference, const BitVector& B_difference )
		{
			WeightStepSpec spec;
			spec.round = round;
			spec.stage = stage;
			spec.operation = "public_xor_constant";
			spec.prefix = prefix;
			spec.weight_model = "zero_difference_public_xor";
			spec.A_before = A_difference;
			spec.B_before = B_difference;
			spec.A_after = A_difference;
			spec.B_after = B_difference;
			spec.has_public_constant = true;
			spec.public_constant = constant;
			spec.objective_begin = model_builder.objective_terms.size();
			spec.objective_end = model_builder.objective_terms.size();
			add_weight_step( std::move( spec ) );
		}
		// Build one XOR-differential round in value-domain order.  The local
		// names intentionally mirror the checkpoint names in the trace: fixed
		// public constants, joint injection witnesses, modular additions, and
		// zero-weight XOR/rotation bridges are all added in the order the core
		// round applies them.
		std::pair<BitVector, BitVector> build_one_round( ScipModelBuilder& model_builder, int round, const BitVector& input_A_difference, const BitVector& input_B_difference )
		{
			const std::string round_prefix = "r" + std::to_string( round ) + "_";
			add_trace( round, "start", input_A_difference, input_B_difference );

			BitVector after_first_constant_subtraction_B = model_builder.create_bit_vector( round_prefix + "B1_after_sub_RC1" );
			std::size_t objective_begin = model_builder.objective_terms.size();
			add_fixed_constant_subtraction_operation( model_builder, ROUND_CONSTANTS[ 1 ], input_B_difference, after_first_constant_subtraction_B, round_prefix + "CONST_SUB_RC1" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "CONST_SUB_RC1",
											  "fixed_public_constant_subtraction",
											  round_prefix + "CONST_SUB_RC1",
											  constant_model_name( options.constant_model ),
											  input_A_difference,
											  input_B_difference,
											  input_A_difference,
											  after_first_constant_subtraction_B,
											  true,
											  false,
											  true,
											  "dB_before",
											  "",
											  "dB_after",
											  input_B_difference,
											  BitVector {},
											  after_first_constant_subtraction_B,
											  true,
											  ROUND_CONSTANTS[ 1 ],
											  "",
											  objective_begin,
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_CONST_SUB_RC1", input_A_difference, after_first_constant_subtraction_B );

			objective_begin = model_builder.objective_terms.size();
			auto [ first_A_injection_difference, first_add_operand_difference ] = create_joint_injection_differences( model_builder, after_first_constant_subtraction_B, round_prefix + "J0_joint_injection_from_B", InjectionKind::B_TO_A_AFTER_RC4 );
			BitVector after_first_injection_A = model_builder.create_xor_bit_vector( input_A_difference, first_A_injection_difference, round_prefix + "A1_after_B_to_A_injection" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "JOINT_INJECTION_B_to_A",
											  "joint_injection_derivative",
											  round_prefix + "J0_joint_injection_from_B",
											  "explicit_milp_witness_plus_joint_affine_rank",
											  after_first_constant_subtraction_B,
											  input_A_difference,
											  after_first_constant_subtraction_B,
											  after_first_injection_A,
											  true,
											  true,
											  true,
											  "joint_input_delta_B",
											  "add_operand_delta",
											  "xor_output_delta_to_A",
											  after_first_constant_subtraction_B,
											  first_add_operand_difference,
											  first_A_injection_difference,
											  false,
											  0,
											  round_prefix + "J0_joint_injection_from_B",
											  objective_begin,
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_JOINT_INJECTION_B_to_A", after_first_injection_A, after_first_constant_subtraction_B );

			BitVector after_first_modular_addition_A = model_builder.create_bit_vector( round_prefix + "A2_after_add0" );
			objective_begin = model_builder.objective_terms.size();
			arithmetic_model::add_two_input_add_diff( model_builder, after_first_injection_A, first_add_operand_difference, after_first_modular_addition_A, round_prefix + "ADD0_A_plus_joint_operand" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "ADD0_A_plus_joint_operand",
											  "modular_addition",
											  round_prefix + "ADD0_A_plus_joint_operand",
											  "two_input_add_xor_difference",
											  after_first_injection_A,
											  after_first_constant_subtraction_B,
											  after_first_modular_addition_A,
											  after_first_constant_subtraction_B,
											  true,
											  true,
											  true,
											  "dA_after_xor_injection",
											  "joint_add_operand_delta",
											  "dA_after_add0",
											  after_first_injection_A,
											  first_add_operand_difference,
											  after_first_modular_addition_A,
											  false,
											  0,
											  "",
											  objective_begin,
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_ADD0_A_plus_joint_operand", after_first_modular_addition_A, after_first_constant_subtraction_B );

			BitVector after_first_A_to_B_bridge_B = model_builder.create_xor_bit_vector( after_first_constant_subtraction_B, ScipModelBuilder::rotate_left( after_first_modular_addition_A, FIRST_BRIDGE_ROTATE0 ), round_prefix + "B2_after_bridge0_A_to_B" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "BRIDGE0_A_to_B_linear_xor",
											  "linear_xor_bridge",
											  round_prefix + "BRIDGE0_A_to_B",
											  "zero_weight_linear_xor",
											  after_first_modular_addition_A,
											  after_first_constant_subtraction_B,
											  after_first_modular_addition_A,
											  after_first_A_to_B_bridge_B,
											  false,
											  false,
											  false,
											  "",
											  "",
											  "",
											  BitVector {},
											  BitVector {},
											  BitVector {},
											  false,
											  0,
											  "",
											  model_builder.objective_terms.size(),
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_BRIDGE0_A_to_B", after_first_modular_addition_A, after_first_A_to_B_bridge_B );
			add_public_xor_constant_step( model_builder, round, "XOR_RC4_B", round_prefix + "XOR_RC4_B", ROUND_CONSTANTS[ 4 ], after_first_modular_addition_A, after_first_A_to_B_bridge_B );
			add_trace( round, "after_XOR_RC4", after_first_modular_addition_A, after_first_A_to_B_bridge_B );

			BitVector after_first_feedback_A = model_builder.create_xor_bit_vector( after_first_modular_addition_A, ScipModelBuilder::rotate_left( after_first_A_to_B_bridge_B, FIRST_BRIDGE_ROTATE1 ), round_prefix + "A3_after_bridge1_B_to_A" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "BRIDGE1_B_to_A_linear_xor",
											  "linear_xor_bridge",
											  round_prefix + "BRIDGE1_B_to_A",
											  "zero_weight_linear_xor",
											  after_first_modular_addition_A,
											  after_first_A_to_B_bridge_B,
											  after_first_feedback_A,
											  after_first_A_to_B_bridge_B,
											  false,
											  false,
											  false,
											  "",
											  "",
											  "",
											  BitVector {},
											  BitVector {},
											  BitVector {},
											  false,
											  0,
											  "",
											  model_builder.objective_terms.size(),
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_BRIDGE1_B_to_A", after_first_feedback_A, after_first_A_to_B_bridge_B );

			BitVector after_second_constant_subtraction_A = model_builder.create_bit_vector( round_prefix + "A4_after_sub_RC6" );
			objective_begin = model_builder.objective_terms.size();
			add_fixed_constant_subtraction_operation( model_builder, ROUND_CONSTANTS[ 6 ], after_first_feedback_A, after_second_constant_subtraction_A, round_prefix + "CONST_SUB_RC6" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "CONST_SUB_RC6",
											  "fixed_public_constant_subtraction",
											  round_prefix + "CONST_SUB_RC6",
											  constant_model_name( options.constant_model ),
											  after_first_feedback_A,
											  after_first_A_to_B_bridge_B,
											  after_second_constant_subtraction_A,
											  after_first_A_to_B_bridge_B,
											  true,
											  false,
											  true,
											  "dA_before",
											  "",
											  "dA_after",
											  after_first_feedback_A,
											  BitVector {},
											  after_second_constant_subtraction_A,
											  true,
											  ROUND_CONSTANTS[ 6 ],
											  "",
											  objective_begin,
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_CONST_SUB_RC6", after_second_constant_subtraction_A, after_first_A_to_B_bridge_B );

			objective_begin = model_builder.objective_terms.size();
			auto [ second_B_injection_difference, second_add_operand_difference ] = create_joint_injection_differences( model_builder, after_second_constant_subtraction_A, round_prefix + "J1_joint_injection_from_A", InjectionKind::A_TO_B_AFTER_RC9 );
			BitVector after_second_injection_B = model_builder.create_xor_bit_vector( after_first_A_to_B_bridge_B, second_B_injection_difference, round_prefix + "B3_after_A_to_B_injection" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "JOINT_INJECTION_A_to_B",
											  "joint_injection_derivative",
											  round_prefix + "J1_joint_injection_from_A",
											  "explicit_milp_witness_plus_joint_affine_rank",
											  after_second_constant_subtraction_A,
											  after_first_A_to_B_bridge_B,
											  after_second_constant_subtraction_A,
											  after_second_injection_B,
											  true,
											  true,
											  true,
											  "joint_input_delta_A",
											  "add_operand_delta",
											  "xor_output_delta_to_B",
											  after_second_constant_subtraction_A,
											  second_add_operand_difference,
											  second_B_injection_difference,
											  false,
											  0,
											  round_prefix + "J1_joint_injection_from_A",
											  objective_begin,
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_JOINT_INJECTION_A_to_B", after_second_constant_subtraction_A, after_second_injection_B );

			BitVector after_second_modular_addition_B = model_builder.create_bit_vector( round_prefix + "B4_after_add1" );
			objective_begin = model_builder.objective_terms.size();
			arithmetic_model::add_two_input_add_diff( model_builder, after_second_injection_B, second_add_operand_difference, after_second_modular_addition_B, round_prefix + "ADD1_B_plus_joint_operand" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "ADD1_B_plus_joint_operand",
											  "modular_addition",
											  round_prefix + "ADD1_B_plus_joint_operand",
											  "two_input_add_xor_difference",
											  after_second_constant_subtraction_A,
											  after_second_injection_B,
											  after_second_constant_subtraction_A,
											  after_second_modular_addition_B,
											  true,
											  true,
											  true,
											  "dB_after_xor_injection",
											  "joint_add_operand_delta",
											  "dB_after_add1",
											  after_second_injection_B,
											  second_add_operand_difference,
											  after_second_modular_addition_B,
											  false,
											  0,
											  "",
											  objective_begin,
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_ADD1_B_plus_joint_operand", after_second_constant_subtraction_A, after_second_modular_addition_B );

			BitVector output_A_difference = model_builder.create_xor_bit_vector( after_second_constant_subtraction_A, ScipModelBuilder::rotate_left( after_second_modular_addition_B, SECOND_BRIDGE_ROTATE0 ), round_prefix + "A5_after_linear_B_to_A_rot5" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "LINEAR_B_to_A_ROT5",
											  "linear_xor_bridge",
											  round_prefix + "LINEAR_B_to_A_ROT5",
											  "zero_weight_linear_xor",
											  after_second_constant_subtraction_A,
											  after_second_modular_addition_B,
											  output_A_difference,
											  after_second_modular_addition_B,
											  false,
											  false,
											  false,
											  "",
											  "",
											  "",
											  BitVector {},
											  BitVector {},
											  BitVector {},
											  false,
											  0,
											  "",
											  model_builder.objective_terms.size(),
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_LINEAR_B_to_A_ROT5", output_A_difference, after_second_modular_addition_B );
			add_public_xor_constant_step( model_builder, round, "XOR_RC9_A", round_prefix + "XOR_RC9_A", ROUND_CONSTANTS[ 9 ], output_A_difference, after_second_modular_addition_B );
			add_trace( round, "after_XOR_RC9", output_A_difference, after_second_modular_addition_B );

			BitVector output_B_difference = model_builder.create_xor_bit_vector( after_second_modular_addition_B, ScipModelBuilder::rotate_left( output_A_difference, SECOND_BRIDGE_ROTATE1 ), round_prefix + "B5_after_final_bridge_A_to_B_rot25" );
			add_weight_step( WeightStepSpec { round,
											  0,
											  "FINAL_BRIDGE_A_to_B_ROT25",
											  "linear_xor_bridge",
											  round_prefix + "FINAL_BRIDGE_A_to_B_ROT25",
											  "zero_weight_linear_xor",
											  output_A_difference,
											  after_second_modular_addition_B,
											  output_A_difference,
											  output_B_difference,
											  false,
											  false,
											  false,
											  "",
											  "",
											  "",
											  BitVector {},
											  BitVector {},
											  BitVector {},
											  false,
											  0,
											  "",
											  model_builder.objective_terms.size(),
											  model_builder.objective_terms.size() } );
			add_trace( round, "after_FINAL_BRIDGE_A_to_B_ROT25", output_A_difference, output_B_difference );

			// Final public whitening is differential no-op here, but we keep the
			// checkpoint so the MILP trace matches the construction sketch.
			add_public_xor_constant_step( model_builder, round, "FINAL_XOR_RC10_A", round_prefix + "FINAL_XOR_RC10_A", ROUND_CONSTANTS[ 10 ], output_A_difference, output_B_difference );
			add_trace( round, "after_XOR_RC10", output_A_difference, output_B_difference );
			add_public_xor_constant_step( model_builder, round, "FINAL_XOR_RC11_B", round_prefix + "FINAL_XOR_RC11_B", ROUND_CONSTANTS[ 11 ], output_A_difference, output_B_difference );
			add_trace( round, "after_XOR_RC11", output_A_difference, output_B_difference );
			add_trace( round, "after_FINAL_XOR_RC10_RC11", output_A_difference, output_B_difference );
			add_trace( round, "end", output_A_difference, output_B_difference );
			return { output_A_difference, output_B_difference };
		}

		void fix_bit_vector_to_value( ScipModelBuilder& model_builder, const BitVector& bits, std::uint32_t value, const std::string& name )
		{
			if ( bits.size() > WORD_SIZE )
				throw std::runtime_error( "fix_bit_vector_to_value only supports up to 32 bits for " + name );
			for ( int bit_index = 0; bit_index < static_cast<int>( bits.size() ); ++bit_index )
				model_builder.add_equality_to_constant_constraint( name + "_bit_" + std::to_string( bit_index ), { { bits[ bit_index ], 1 } }, static_cast<double>( ( value >> bit_index ) & 1u ) );
		}

		void build( ScipModelBuilder& model_builder, const EnumerationState& enumeration )
		{
			injections.clear();
			trace.clear();
			weight_steps.clear();
			canonical_bit_groups.clear();
			dA_in = model_builder.create_bit_vector( "dA_in" );
			dB_in = model_builder.create_bit_vector( "dB_in" );
			BitVector current_A_difference = dA_in;
			BitVector current_B_difference = dB_in;
			for ( int round = 0; round < options.rounds; ++round )
			{
				auto next_round_difference = build_one_round( model_builder, round, current_A_difference, current_B_difference );
				current_A_difference = next_round_difference.first;
				current_B_difference = next_round_difference.second;
			}
			dA_out = current_A_difference;
			dB_out = current_B_difference;
			// Best-trail search always excludes the trivial external all-zero
			// input difference.  Internal zero windows remain legal whenever the
			// explicit MILP transition constraints allow them.
			std::vector<LinearTerm> nonzero;
			for ( auto& input_A_bit : dA_in )
				nonzero.push_back( { input_A_bit, 1 } );
			for ( auto& input_B_bit : dB_in )
				nonzero.push_back( { input_B_bit, 1 } );
			model_builder.add_linear( "nonzero_input_difference", nonzero, 1.0, INF );
			if ( options.fix_input_da )
				fix_bit_vector_to_value( model_builder, dA_in, *options.fix_input_da, "fix_input_da" );
			if ( options.fix_input_db )
				fix_bit_vector_to_value( model_builder, dB_in, *options.fix_input_db, "fix_input_db" );
			if ( options.fix_output_da )
				fix_bit_vector_to_value( model_builder, dA_out, *options.fix_output_da, "fix_output_da" );
			if ( options.fix_output_db )
				fix_bit_vector_to_value( model_builder, dB_out, *options.fix_output_db, "fix_output_db" );
			apply_no_good_cuts( model_builder, enumeration );
			if ( options.objective_window_enabled )
			{
				const double lower_bound = std::isnan( options.objective_window_from ) ? -INF : options.objective_window_from - 1e-8;
				const double upper_bound = std::isnan( options.objective_window_to ) ? INF : options.objective_window_to + 1e-8;
				model_builder.add_objective_bound( "objective_weight_window", lower_bound, upper_bound );
			}
		}

		void apply_no_good_cuts( ScipModelBuilder& model_builder, const EnumerationState& enumeration )
		{
			int index = 0;
			for ( const auto& no_good_cut : enumeration.no_good_cuts )
			{
				int						one_count = 0;
				std::vector<LinearTerm> terms;
				for ( const auto& assignment_item : no_good_cut.assignment )
				{
					ScipVariable assigned_variable = model_builder.find_var_or_throw( assignment_item.first );
					if ( assignment_item.second )
					{
						terms.push_back( { assigned_variable, -1.0 } );
						++one_count;
					}
					else
						terms.push_back( { assigned_variable, 1.0 } );
				}
				model_builder.add_linear( "nogood_characteristic_" + std::to_string( index++ ), terms, 1.0 - one_count, INF );
			}
		}
	};

	static void append_no_good_group( NoGoodCut& no_good_cut, std::set<std::string>& seen_names, SCIP* scip, SCIP_SOL* solution, const BitVector& group )
	{
		for ( const auto& bit : group )
		{
			if ( !seen_names.insert( bit.name ).second )
				continue;
			const int value = SCIPgetSolVal( scip, solution, bit.var ) > 0.5 ? 1 : 0;
			no_good_cut.assignment.push_back( { bit.name, value } );
		}
	}

	static NoGoodCut capture_no_good( SCIP* scip, SCIP_SOL* solution, const NeoAlzetteScipEngineeringSearch& search )
	{
		NoGoodCut no_good_cut;
		std::set<std::string> seen_names;

		// Keep the existing state checkpoints, then add every semantic local
		// input/output difference used by a weighted operator.  In particular,
		// a joint injection has two output differences (XOR side and addend
		// side); both belong to the differential characteristic.  Deliberately
		// do not include the internal value-domain witness bits.
		for ( const auto& group : search.canonical_bit_groups )
			append_no_good_group( no_good_cut, seen_names, scip, solution, group );
		for ( const auto& spec : search.weight_steps )
		{
			if ( spec.has_local_input0 )
				append_no_good_group( no_good_cut, seen_names, scip, solution, spec.local_input0 );
			if ( spec.has_local_input1 )
				append_no_good_group( no_good_cut, seen_names, scip, solution, spec.local_input1 );
			if ( spec.has_local_output )
				append_no_good_group( no_good_cut, seen_names, scip, solution, spec.local_output );
		}
		for ( const auto& injection : search.injections )
		{
			append_no_good_group( no_good_cut, seen_names, scip, solution, injection.input_bits );
			append_no_good_group( no_good_cut, seen_names, scip, solution, injection.xor_output_bits );
			append_no_good_group( no_good_cut, seen_names, scip, solution, injection.add_operand_bits );
		}
		return no_good_cut;
	}

	static SolutionSnapshot make_snapshot( ScipModelBuilder& model_builder, NeoAlzetteScipEngineeringSearch& search )
	{
		SCIP_SOL* solution = SCIPgetBestSol( model_builder.scip );
		if ( !solution )
			throw std::runtime_error( "SCIP produced no incumbent solution" );
		SolutionSnapshot snapshot;
		snapshot.objective = SCIPgetPrimalbound( model_builder.scip );
		snapshot.dA_in = value_from_solution( model_builder.scip, solution, search.dA_in );
		snapshot.dB_in = value_from_solution( model_builder.scip, solution, search.dB_in );
		snapshot.dA_out = value_from_solution( model_builder.scip, solution, search.dA_out );
		snapshot.dB_out = value_from_solution( model_builder.scip, solution, search.dB_out );
		return snapshot;
	}

	static void print_solution_summary( const SolutionSnapshot& snapshot, const std::string& label )
	{
		long double probability = std::pow( 2.0L, -static_cast<long double>( snapshot.objective ) );
		std::cout << label << " objective_weight=" << std::setprecision( 12 ) << snapshot.objective << " probability≈2^{-" << std::setprecision( 12 ) << snapshot.objective << "}"
				  << " decimal≈" << std::scientific << static_cast<double>( probability ) << std::defaultfloat << "\n";
		std::cout << "ΔA_in=0x" << hex32( snapshot.dA_in ) << " ΔB_in=0x" << hex32( snapshot.dB_in ) << " ΔA_out=0x" << hex32( snapshot.dA_out ) << " ΔB_out=0x" << hex32( snapshot.dB_out ) << "\n";
	}

	static void print_trace( ScipModelBuilder& model_builder, NeoAlzetteScipEngineeringSearch& search )
	{
		SCIP_SOL*			solution = SCIPgetBestSol( model_builder.scip );
		InjectionRankOracle oracle;
		std::cout << "\nΔ = Differential Perturbation\n";
		std::cout << "\n=== CHARACTERISTIC CHECKPOINT TRACE ===\n";
		for ( const auto& node : search.trace )
		{
			std::cout << "round " << node.round << " / " << std::left << std::setw( 28 ) << node.stage << " ΔA=0x" << hex32( value_from_solution( model_builder.scip, solution, node.A ) ) << " ΔB=0x" << hex32( value_from_solution( model_builder.scip, solution, node.B ) ) << "\n";
		}
		if ( !search.injections.empty() )
		{
			std::cout << "\n=== JOINT INJECTION LOCAL TRANSITIONS ===\n";
			for ( const auto& injection_instance : search.injections )
			{
				auto input_difference = value_from_solution( model_builder.scip, solution, injection_instance.input_bits );
				auto xor_output_difference = value_from_solution( model_builder.scip, solution, injection_instance.xor_output_bits );
				auto add_operand_difference = value_from_solution( model_builder.scip, solution, injection_instance.add_operand_bits );
				auto joint_output_difference = value64_from_solution( model_builder.scip, solution, injection_instance.joint_output_bits );
				auto transition_result = oracle.transition( injection_instance.kind, input_difference, joint_output_difference );
				std::cout << injection_instance.name << " din=0x" << hex32( input_difference )
						  << " xor_dout=0x" << hex32( xor_output_difference )
						  << " add_dout=0x" << hex32( add_operand_difference )
						  << " joint_dout=0x" << hex64( joint_output_difference )
						  << " support_source=" << injection_instance.support_source
						  << " support_audit=" << ( transition_result.valid ? "yes" : "NO" )
						  << " joint_rank=" << transition_result.rank
						  << " rank_weight=" << SCIPgetSolVal( model_builder.scip, solution, injection_instance.rank_weight.var ) << "\n";
			}
		}
	}

	static const InjectionInstance* find_injection_instance( const NeoAlzetteScipEngineeringSearch& search, const std::string& name )
	{
		for ( const auto& injection_instance : search.injections )
			if ( injection_instance.name == name )
				return &injection_instance;
		return nullptr;
	}

	// ------------------------------------------------------------------------
	// Audit section 3: incumbent finalization and trace/oracle audit
	// ------------------------------------------------------------------------
	// These functions read the SCIP incumbent back into semantic checkpoints and
	// verify local model choices with the exact Q1 or injection-rank oracle.
	static std::vector<WeightTraceEntry> collect_weight_trace( ScipModelBuilder& model_builder, const NeoAlzetteScipEngineeringSearch& search, double& total )
	{
		SCIP_SOL* solution = SCIPgetBestSol( model_builder.scip );
		if ( !solution )
			throw std::runtime_error( "SCIP produced no incumbent solution" );
		InjectionRankOracle				  oracle;
		std::vector<WeightTraceEntry> weight_trace;
		weight_trace.reserve( search.weight_steps.size() );
		total = 0.0;
		for ( const auto& spec : search.weight_steps )
		{
			WeightTraceEntry entry;
			entry.round = spec.round;
			entry.step = spec.step;
			entry.stage = spec.stage;
			entry.operation = spec.operation;
			entry.prefix = spec.prefix;
			entry.weight_model = spec.weight_model;
			entry.A_before = value_from_solution( model_builder.scip, solution, spec.A_before );
			entry.B_before = value_from_solution( model_builder.scip, solution, spec.B_before );
			entry.A_after = value_from_solution( model_builder.scip, solution, spec.A_after );
			entry.B_after = value_from_solution( model_builder.scip, solution, spec.B_after );
			entry.has_local_input0 = spec.has_local_input0;
			entry.has_local_input1 = spec.has_local_input1;
			entry.has_local_output = spec.has_local_output;
			entry.local_input0_label = spec.local_input0_label;
			entry.local_input1_label = spec.local_input1_label;
			entry.local_output_label = spec.local_output_label;
			if ( spec.has_local_input0 )
				entry.local_input0 = value_from_solution( model_builder.scip, solution, spec.local_input0 );
			if ( spec.has_local_input1 )
				entry.local_input1 = value_from_solution( model_builder.scip, solution, spec.local_input1 );
			if ( spec.has_local_output )
				entry.local_output = value_from_solution( model_builder.scip, solution, spec.local_output );
			entry.has_public_constant = spec.has_public_constant;
			entry.public_constant = spec.public_constant;
			entry.injection_name = spec.injection_name;
			for ( std::size_t objective_index = spec.objective_begin; objective_index < spec.objective_end && objective_index < model_builder.objective_terms.size(); ++objective_index )
			{
				const auto& term = model_builder.objective_terms[ objective_index ];
				const double value = SCIPgetSolVal( model_builder.scip, solution, term.variable.var );
				const double contribution = term.coefficient * value;
				entry.local_weight += contribution;
				++entry.objective_term_count;
				if ( std::fabs( value ) > 1e-9 || std::fabs( contribution ) > 1e-9 )
				{
					entry.selected_terms.push_back( { term.variable.name, term.coefficient, value, contribution } );
					++entry.selected_term_count;
				}
			}
			if ( !spec.injection_name.empty() )
			{
				const InjectionInstance* injection_instance = find_injection_instance( search, spec.injection_name );
				if ( injection_instance )
				{
					bool rank_weight_in_step = false;
					for ( std::size_t objective_index = spec.objective_begin; objective_index < spec.objective_end && objective_index < model_builder.objective_terms.size(); ++objective_index )
					{
						if ( model_builder.objective_terms[ objective_index ].variable.var == injection_instance->rank_weight.var )
						{
							rank_weight_in_step = true;
							break;
						}
					}
					if ( !rank_weight_in_step )
						throw std::runtime_error( "injection weight trace is missing rank objective term for " + spec.injection_name );
					entry.has_injection_audit = true;
					entry.injection_din = value_from_solution( model_builder.scip, solution, injection_instance->input_bits );
					entry.injection_dout_xor = value_from_solution( model_builder.scip, solution, injection_instance->xor_output_bits );
					entry.injection_dout_add = value_from_solution( model_builder.scip, solution, injection_instance->add_operand_bits );
					entry.injection_joint_dout = value64_from_solution( model_builder.scip, solution, injection_instance->joint_output_bits );
					entry.injection_support_source = injection_instance->support_source;
					const auto transition_result = oracle.transition( injection_instance->kind, entry.injection_din, entry.injection_joint_dout );
					entry.injection_support_valid = transition_result.valid;
					entry.injection_joint_rank = transition_result.rank;
					entry.injection_affine_constant = transition_result.affine_constant;
					entry.model_rank_weight = SCIPgetSolVal( model_builder.scip, solution, injection_instance->rank_weight.var );
				}
			}
			total += entry.local_weight;
			entry.cumulative_weight = total;
			weight_trace.push_back( std::move( entry ) );
		}
		return weight_trace;
	}

	static void attach_weight_trace( ScipSolveResult& solve_result, ScipModelBuilder& model_builder, const NeoAlzetteScipEngineeringSearch& search )
	{
		if ( !solve_result.feasible )
			return;
		solve_result.weight_trace = collect_weight_trace( model_builder, search, solve_result.weight_trace_total );
		solve_result.weight_trace_objective_delta = solve_result.weight_trace_total - solve_result.snapshot.objective;
		solve_result.weight_trace_available = true;
		solve_result.weight_trace_matches_objective = std::fabs( solve_result.weight_trace_objective_delta ) <= 1e-6;
	}

	static void print_weight_trace( const ScipSolveResult& result )
	{
		if ( result.weight_trace.empty() )
			return;
		std::cout << "\n=== CHARACTERISTIC WEIGHT TRACE ===\n";
		for ( const auto& trace_entry : result.weight_trace )
		{
			std::cout << "round " << trace_entry.round << " step " << trace_entry.step << " " << trace_entry.stage << " op=" << trace_entry.operation
					  << " model=" << trace_entry.weight_model
					  << " prefix=" << trace_entry.prefix
					  << " local_weight=" << std::setprecision( 12 ) << trace_entry.local_weight
					  << " cumulative=" << trace_entry.cumulative_weight << "\n";
			std::cout << "  state: ΔA 0x" << hex32( trace_entry.A_before ) << " -> 0x" << hex32( trace_entry.A_after )
					  << "  ΔB 0x" << hex32( trace_entry.B_before ) << " -> 0x" << hex32( trace_entry.B_after ) << "\n";
			if ( trace_entry.has_public_constant )
				std::cout << "  public_constant=0x" << hex32( trace_entry.public_constant ) << "\n";
			if ( trace_entry.has_local_input0 )
				std::cout << "  " << trace_entry.local_input0_label << "=0x" << hex32( trace_entry.local_input0 ) << "\n";
			if ( trace_entry.has_local_input1 )
				std::cout << "  " << trace_entry.local_input1_label << "=0x" << hex32( trace_entry.local_input1 ) << "\n";
			if ( trace_entry.has_local_output )
				std::cout << "  " << trace_entry.local_output_label << "=0x" << hex32( trace_entry.local_output ) << "\n";
			if ( trace_entry.has_injection_audit )
				std::cout << "  joint_injection: " << trace_entry.injection_name << " din=0x" << hex32( trace_entry.injection_din )
						  << " xor_dout=0x" << hex32( trace_entry.injection_dout_xor )
						  << " add_dout=0x" << hex32( trace_entry.injection_dout_add )
						  << " joint_dout=0x" << hex64( trace_entry.injection_joint_dout )
						  << " support_source=" << trace_entry.injection_support_source
						  << " support_audit=" << ( trace_entry.injection_support_valid ? "yes" : "NO" )
						  << " joint_rank=" << trace_entry.injection_joint_rank
						  << " rank_weight=" << std::setprecision( 12 ) << trace_entry.model_rank_weight << "\n";
			std::cout << "  selected_objective_terms=" << trace_entry.selected_term_count << "/" << trace_entry.objective_term_count << "\n";
			for ( const auto& selected_term : trace_entry.selected_terms )
			{
				std::cout << "    " << selected_term.variable
						  << " coeff=" << std::setprecision( 12 ) << selected_term.coefficient
						  << " value=" << selected_term.value
						  << " contribution=" << selected_term.contribution << "\n";
			}
		}
		std::cout << "TRACE_WEIGHT_TOTAL=" << std::setprecision( 12 ) << result.weight_trace_total
				  << " objective_weight=" << result.snapshot.objective
				  << " delta=" << result.weight_trace_objective_delta
				  << " matches_objective=" << ( result.weight_trace_matches_objective ? "true" : "false" ) << "\n";
	}

	static void fill_common_result_status( ScipSolveResult& solve_result )
	{
		solve_result.hit_time_limit = solve_result.scip_status == SCIP_STATUS_TIMELIMIT;
		solve_result.hit_memory_limit = solve_result.scip_status == SCIP_STATUS_MEMLIMIT;
		solve_result.hit_solution_limit = solve_result.scip_status == SCIP_STATUS_SOLLIMIT || solve_result.scip_status == SCIP_STATUS_BESTSOLLIMIT;
	}

	static void finalize_incumbent_result( ScipSolveResult& solve_result, ScipModelBuilder& model_builder, NeoAlzetteScipEngineeringSearch& search )
	{
		solve_result.snapshot = make_snapshot( model_builder, search );
		solve_result.no_good = capture_no_good( model_builder.scip, SCIPgetBestSol( model_builder.scip ), search );
		attach_weight_trace( solve_result, model_builder, search );
	}

	static void print_result_artifact_trace( ScipModelBuilder& model_builder, NeoAlzetteScipEngineeringSearch& search, const ScipSolveResult& solve_result, const std::string& label )
	{
		print_solution_summary( solve_result.snapshot, label );
		print_trace( model_builder, search );
		print_weight_trace( solve_result );
	}

	static ScipSolveResult solve_in_model( const SearchOptions& options, EnumerationState& enumeration, bool print_final, bool collect_final_weight_trace = false )
	{
		( void )collect_final_weight_trace;
		ScipModelBuilder				builder( options.quiet, options.time_limit_seconds );
		NeoAlzetteScipEngineeringSearch search( options );
		search.build( builder, enumeration );
		SCIP_CALL_THROW( SCIPsolve( builder.scip ) );
		SCIP_STATUS status = SCIPgetStatus( builder.scip );
		std::cout << "[SCIP API model solve] status=" << static_cast<int>( status ) << " obj=" << SCIPgetPrimalbound( builder.scip ) << "\n";
		if ( status == SCIP_STATUS_INFEASIBLE )
		{
			ScipSolveResult solve_result;
			solve_result.feasible = false;
			solve_result.complete = true;
			solve_result.scip_status = status;
			fill_common_result_status( solve_result );
			return solve_result;
		}
		if ( status != SCIP_STATUS_OPTIMAL )
		{
			ScipSolveResult solve_result;
			solve_result.feasible = SCIPgetBestSol( builder.scip ) != nullptr;
			solve_result.complete = false;
			solve_result.scip_status = status;
			solve_result.error_message = solve_result.feasible ? "SCIP did not prove optimality; reporting current incumbent characteristic" : "SCIP did not prove optimality and produced no incumbent solution";
			fill_common_result_status( solve_result );
			if ( solve_result.feasible )
			{
				finalize_incumbent_result( solve_result, builder, search );
				if ( print_final )
					print_result_artifact_trace( builder, search, solve_result, "SCIP CURRENT INCUMBENT TRAIL" );
			}
			return solve_result;
		}
		ScipSolveResult solve_result;
		solve_result.feasible = true;
		solve_result.complete = true;
		solve_result.scip_status = status;
		fill_common_result_status( solve_result );
		finalize_incumbent_result( solve_result, builder, search );
		if ( print_final )
			print_result_artifact_trace( builder, search, solve_result, "SCIP MODEL-CONSTRAINED BEST SINGLE TRAIL" );
		return solve_result;
	}


	// Reuse one exact endpoint model while semantic no-good cuts monotonically
	// restrict its feasible set.  SCIP 10.0.2 reoptimization is designed for
	// this precise pattern; no objective, endpoint, or local transition model is
	// changed between solves.
	class DifferentialHullReoptimizationSession
	{
	public:
		DifferentialHullReoptimizationSession( const SearchOptions& options, const EnumerationState& initial_enumeration )
			: builder_( options.quiet ), search_( options ), next_no_good_index_( initial_enumeration.no_good_cuts.size() )
		{
			SCIP_CALL_THROW( SCIPenableReoptimization( builder_.scip, TRUE ) );
			search_.build( builder_, initial_enumeration );
		}

		ScipSolveResult solve_next( double remaining_time_seconds, bool print_final = false )
		{
			if ( std::isfinite( remaining_time_seconds ) && remaining_time_seconds > 0.0 )
				SCIP_CALL_THROW( SCIPsetRealParam( builder_.scip, "limits/time", remaining_time_seconds ) );
			SCIP_CALL_THROW( SCIPsolve( builder_.scip ) );
			const SCIP_STATUS status = SCIPgetStatus( builder_.scip );

			ScipSolveResult result;
			result.scip_status = status;
			result.feasible = SCIPgetBestSol( builder_.scip ) != nullptr;
			result.complete = status == SCIP_STATUS_OPTIMAL || status == SCIP_STATUS_INFEASIBLE;
			fill_common_result_status( result );
			if ( result.feasible )
			{
				finalize_incumbent_result( result, builder_, search_ );
				if ( status != SCIP_STATUS_OPTIMAL )
					result.error_message = "SCIP reoptimization did not prove optimality; reporting current incumbent characteristic";
				if ( print_final )
					print_result_artifact_trace( builder_, search_, result, status == SCIP_STATUS_OPTIMAL ? "SCIP REOPTIMIZED ENDPOINT TRAIL" : "SCIP CURRENT REOPTIMIZATION INCUMBENT" );
			}
			return result;
		}

		void exclude_characteristic( const NoGoodCut& no_good_cut )
		{
			SCIP_CALL_THROW( SCIPfreeReoptSolve( builder_.scip ) );
			int one_count = 0;
			std::vector<LinearTerm> terms;
			terms.reserve( no_good_cut.assignment.size() );
			for ( const auto& assignment_item : no_good_cut.assignment )
			{
				const ScipVariable variable = builder_.find_var_or_throw( assignment_item.first );
				if ( assignment_item.second )
				{
					terms.push_back( { variable, -1.0 } );
					++one_count;
				}
				else
					terms.push_back( { variable, 1.0 } );
			}
			if ( terms.empty() )
				throw std::runtime_error( "cannot add an empty differential semantic no-good cut" );
			builder_.add_linear( "nogood_characteristic_reopt_" + std::to_string( next_no_good_index_++ ), terms, 1.0 - one_count, INF );
		}

	private:
		ScipModelBuilder builder_;
		NeoAlzetteScipEngineeringSearch search_;
		std::size_t next_no_good_index_ = 0;
	};


	static void run_round_table( SearchOptions options )
	{
		const int max_rounds = options.rounds;
		std::vector<RoundTableRow> rows;
		rows.reserve( max_rounds );
		for ( int round_count = 1; round_count <= max_rounds; ++round_count )
		{
			SearchOptions row_options = options;
			row_options.rounds = round_count;
			row_options.objective_window_enabled = false;
			row_options.objective_window_from = std::numeric_limits<double>::quiet_NaN();
			row_options.objective_window_to = std::numeric_limits<double>::quiet_NaN();
			std::cout << "\n=== ROUND TABLE SEARCH rounds=" << round_count << " ===\n";
			EnumerationState enumeration;
			ScipSolveResult  result = solve_in_model( row_options, enumeration, true, true );
			RoundTableRow row;
			row.rounds = round_count;
			row.best_trail = result;
			if ( !result.feasible )
				std::cout << "rounds=" << round_count << " infeasible_or_incomplete status=" << scip_status_name( result.scip_status ) << "\n";
			rows.push_back( std::move( row ) );
			write_round_table_json_file( options.output_round_table_json, options, rows );
		}
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
	// ------------------------------------------------------------------------
	// Audit section 4: CLI parsing and run modes
	// ------------------------------------------------------------------------
	static void reject_unknown_options( int argc, char** argv )
	{
		static const std::set<std::string> value_options {
			"--rounds",
			"--constant-model",
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

	static void print_help( const char* argv0 )
	{
		std::cout << "Usage: " << argv0 << " [options]\n\n"
				  << "Search:\n"
				  << "  --rounds R                       number of NeoAlzette rounds, default 1\n"
				  << "  --constant-model fixed-public-exact  strict fixed-public constant model (default)\n"
				  << "  --fix-input-da X                 optional input A difference source, default 0x00000001\n"
				  << "  --fix-input-db X                 optional input B difference source, default 0x00000001\n"
				  << "  --fix-output-da X                fix output A difference\n"
				  << "  --fix-output-db X                fix output B difference\n"
				  << "  --time-limit S                   SCIP time limit for one solve / round-table row\n"
				  << "  --quiet                          reduce SCIP display output\n\n"
				  << "Output:\n"
				  << "  --output-result-json FILE        write best-trail summary JSON; default scip_best_roundR_result.json in best-trail mode\n"
				  << "  --output-weight-trace-json FILE  write best-trail per-step weight decomposition JSON; default scip_best_roundR_weight_trace.json in best-trail mode\n"
				  << "  --output-round-table-json FILE   write prefix best-trail table for rounds 1..R and refresh it after each finished prefix\n"
				  << "  --help                           print this help\n";
	}

	static SearchOptions parse_options( int argc, char** argv )
	{
		reject_unknown_options( argc, argv );
		SearchOptions opt;
		opt.rounds = std::stoi( get_arg( argc, argv, "--rounds", "1" ) );
		std::string cm = get_arg( argc, argv, "--constant-model", "fixed-public-exact" );
		if ( cm == "fixed-public-exact" )
			opt.constant_model = ConstantModel::FIXED_PUBLIC_EXACT;
		else
			throw std::runtime_error( "unknown --constant-model: " + cm + " (supported production mode: fixed-public-exact)" );
		opt.quiet = has_arg( argc, argv, "--quiet" );
		if ( has_arg( argc, argv, "--time-limit" ) )
			opt.time_limit_seconds = std::stod( get_arg( argc, argv, "--time-limit", "0" ) );
		opt.output_result_json = get_arg( argc, argv, "--output-result-json", "" );
		opt.output_weight_trace_json = get_arg( argc, argv, "--output-weight-trace-json", "" );
		opt.output_round_table_json = get_arg( argc, argv, "--output-round-table-json", "" );
		opt.fix_input_da = get_optional_u32_arg( argc, argv, "--fix-input-da" ).value_or( 0x00000001u );
		opt.fix_input_db = get_optional_u32_arg( argc, argv, "--fix-input-db" ).value_or( 0x00000001u );
		opt.fix_output_da = get_optional_u32_arg( argc, argv, "--fix-output-da" );
		opt.fix_output_db = get_optional_u32_arg( argc, argv, "--fix-output-db" );
		return opt;
	}


}  // namespace neoalzette_diff_milp
