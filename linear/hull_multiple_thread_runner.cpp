// NeoAlzette linear HULL hybrid multi-process/multi-thread campaign runner.
//
// The parent runner is a C++ scheduler: std::thread workers create OS child
// processes with posix_spawn or CreateProcess. Each child process is the same
// runner binary in an internal worker mode and calls the existing HULL
// implementation as a C++ API. This is not system(), not a shell script, and
// not a wrapper around the standalone HULL executable.
//
// Scheme A: independent Forest seeds, same optional external fixed input.
// Scheme B: deterministic fixed-input sector representatives; each worker then lets
//           the existing Forest Layer feed Q1-valid outputs back as later inputs.
//
// Build:
//   Link this translation unit with the same SCIP include/library flags as
//   neoalzette_scip_round_hull_search.cpp; see hull_multiple_thread_runner.md.
//   The runner embeds the HULL implementation in library mode, so a plain
//   g++ command without SCIP libraries is intentionally insufficient.
//
// Runner examples:
//   ./hull_multiple_thread_runner --mode A --workers 16 --jobs 64 --job-time-limit 43200 --output runs/seed_campaign -- --rounds 1
//
//   ./hull_multiple_thread_runner --mode B --workers 16 --jobs 4096 --job-time-limit 300 --output runs/sector_campaign --prefix-bits 8 --sector-start 0 --samples-per-sector 1 -- --rounds 1
//
// The arguments after "--" are passed unchanged to the existing HULL option parser.
// Options owned by this runner (seed/time/output and Scheme-B fixed inputs) cannot
// be repeated there. The parent creates child processes by OS API; every child
// calls run_forest_hull_search() directly inside the child process.

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cctype>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <spawn.h>
#  include <signal.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  ifdef __linux__
#    include <sched.h>
#  endif
#endif

#ifndef _WIN32
extern char** environ;
#endif

#define NEOALZETTE_HULL_LIBRARY_MODE
#include "neoalzette_scip_round_hull_search.cpp"

namespace neoalzette_runner
{
    namespace fs = std::filesystem;

    constexpr const char* kAnalysis = "linear";
    constexpr const char* kTimeOption = "--time-limit";
    constexpr const char* kInputAOption = "--fix-input-ma";
    constexpr const char* kInputBOption = "--fix-input-mb";
    constexpr const char* kInputAField = "mA_in";
    constexpr const char* kInputBField = "mB_in";
    constexpr const char* kOutputAField = "mA_out";
    constexpr const char* kOutputBField = "mB_out";
    constexpr const char* kCharacteristicsOption = "--hull-characteristics-jsonl";

    enum class CampaignMode { MultiSeed, Sector };

    struct Config
    {
        CampaignMode mode = CampaignMode::MultiSeed;
        unsigned workers = std::max( 1u, std::thread::hardware_concurrency() );
        std::uint64_t jobs = 0;
        double job_time_limit_seconds = 0.0;
        fs::path output_directory;
        fs::path self_executable;
        std::uint64_t seed_start = 0;
        unsigned prefix_bits = 8;
        std::uint64_t sector_start = 0;
        unsigned samples_per_sector = 1;
        std::uint64_t sample_seed = 0;
        bool pin_workers = true;
        std::vector<std::string> hull_arguments;
    };

    struct ScheduledJob
    {
        std::uint64_t job_id = 0;
        std::uint64_t forest_seed = 0;
        std::optional<std::uint64_t> sector;
        unsigned sector_sample = 0;
        std::optional<std::uint32_t> fixed_a;
        std::optional<std::uint32_t> fixed_b;
        fs::path directory;
        std::vector<std::string> arguments;
        int exit_code = -1;
        bool result_json_exists = false;
        std::string failure;
    };

    struct AttemptRecord
    {
        std::uint64_t job_id = 0;
        std::uint64_t forest_seed = 0;
        std::optional<std::uint64_t> sector;
        unsigned sector_sample = 0;
        int attempt_id = -1;
        int tree_id = -1;
        int layer = -1;
        std::uint32_t input_a = 0;
        std::uint32_t input_b = 0;
        bool has_output = false;
        std::uint32_t output_a = 0;
        std::uint32_t output_b = 0;
        double local_weight = std::numeric_limits<double>::quiet_NaN();
        double cumulative_weight = std::numeric_limits<double>::quiet_NaN();
        double hull_primary_measure = std::numeric_limits<double>::quiet_NaN();
        double hull_absolute_measure = std::numeric_limits<double>::quiet_NaN();
        double hull_effective_weight = std::numeric_limits<double>::quiet_NaN();
        bool cycle_detected = false;
        bool completed = false;
        std::string stop_reason;
    };

    static std::atomic<bool> gStopRequested { false };
    static std::mutex gPrintMutex;

#ifndef _WIN32
    static volatile std::sig_atomic_t gSignalStopRequested = 0;

    static void signal_handler( int )
    {
        gSignalStopRequested = 1;
    }

    static bool stop_requested()
    {
        return gSignalStopRequested != 0 || gStopRequested.load( std::memory_order_relaxed );
    }
#else
    static BOOL WINAPI console_control_handler( DWORD control_type )
    {
        switch ( control_type )
        {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            gStopRequested.store( true, std::memory_order_relaxed );
            return TRUE;
        default:
            return FALSE;
        }
    }

    static bool stop_requested()
    {
        return gStopRequested.load( std::memory_order_relaxed );
    }
#endif

    [[noreturn]] static void fail( const std::string& message )
    {
        throw std::runtime_error( message );
    }

    static std::string lower_copy( std::string value )
    {
        std::transform( value.begin(), value.end(), value.begin(), []( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
        return value;
    }

    static std::uint64_t parse_u64( const std::string& text, const std::string& option )
    {
        std::size_t consumed = 0;
        int base = 10;
        if ( text.size() > 2 && text[ 0 ] == '0' && ( text[ 1 ] == 'x' || text[ 1 ] == 'X' ) )
            base = 16;
        try
        {
            const auto value = std::stoull( text, &consumed, base );
            if ( consumed != text.size() )
                fail( "invalid integer for " + option + ": " + text );
            return value;
        }
        catch ( const std::exception& )
        {
            fail( "invalid integer for " + option + ": " + text );
        }
    }

    static unsigned parse_unsigned( const std::string& text, const std::string& option )
    {
        const auto value = parse_u64( text, option );
        if ( value > std::numeric_limits<unsigned>::max() )
            fail( "value is too large for " + option + ": " + text );
        return static_cast<unsigned>( value );
    }

    static double parse_positive_double( const std::string& text, const std::string& option )
    {
        std::size_t consumed = 0;
        double value = 0.0;
        try
        {
            value = std::stod( text, &consumed );
        }
        catch ( const std::exception& )
        {
            fail( "invalid number for " + option + ": " + text );
        }
        if ( consumed != text.size() || !std::isfinite( value ) || value <= 0.0 )
            fail( option + " must be a finite positive number" );
        return value;
    }

    static std::string require_value( int& index, int argc, char** argv, const std::string& option )
    {
        if ( index + 1 >= argc )
            fail( "missing value for " + option );
        return argv[ ++index ];
    }

    static void print_help( const char* argv0 )
    {
        std::cout
            << "Usage:\n"
            << "  " << argv0 << " --mode A --workers N --jobs J --job-time-limit S --output DIR [runner options] [-- HULL_ARGS...]\n"
            << "  " << argv0 << " --mode B --workers N --jobs J --job-time-limit S --output DIR [runner options] [-- HULL_ARGS...]\n\n"
            << "Required runner options:\n"
            << "  --mode A|B                    A: independent Forest seeds; B: fixed-input sector representatives\n"
            << "  --jobs J                      total child HULL API worker processes to schedule\n"
            << "  --job-time-limit S            wall-clock budget passed to each child HULL API job\n"
            << "  --output DIR                  campaign result directory\n\n"
            << "Common runner options:\n"
            << "  --workers N                   scheduler threads / concurrent child processes, default hardware concurrency\n"
            << "  --no-affinity                 do not pin Linux/Windows child processes to available CPUs\n"
            << "  --help                        print this help\n\n"
            << "Scheme A options:\n"
            << "  --seed-start X                first Forest seed, default 0; job i uses X+i\n\n"
            << "Scheme B options:\n"
            << "  --prefix-bits P               high-prefix bits per 32-bit branch, 1..16, default 8\n"
            << "  --sector-start X              first pair-prefix sector, default 0\n"
            << "  --samples-per-sector N        deterministic representatives per sector, default 1\n"
            << "  --sample-seed X               deterministic low-bit/Forest-seed generator, default 0\n\n"
            << "Arguments after -- are forwarded unchanged to the existing HULL option parser inside each child process.\n"
            << "The runner owns " << kTimeOption << ", --forest-seed, --hull-output-json"
            << ( std::string( kCharacteristicsOption ).empty() ? "" : std::string( ", " ) + kCharacteristicsOption )
            << ". Scheme B also owns " << kInputAOption << " and " << kInputBOption << ".\n\n"
            << "Cross-worker HULL probabilities/correlations are intentionally not summed.\n"
            << "The merged artifacts report exact observed input/output coverage and candidate statistics only.\n";
    }

    static Config parse_config( int argc, char** argv )
    {
        Config config;
        bool mode_set = false;
        bool passthrough = false;
        for ( int index = 1; index < argc; ++index )
        {
            const std::string arg = argv[ index ];
            if ( passthrough )
            {
                config.hull_arguments.push_back( arg );
                continue;
            }
            if ( arg == "--" )
            {
                passthrough = true;
                continue;
            }
            if ( arg == "--help" || arg == "-h" )
            {
                print_help( argv[ 0 ] );
                std::exit( 0 );
            }
            if ( arg == "--mode" )
            {
                const auto value = lower_copy( require_value( index, argc, argv, arg ) );
                if ( value == "a" || value == "seed" || value == "multi-seed" )
                    config.mode = CampaignMode::MultiSeed;
                else if ( value == "b" || value == "sector" || value == "fan" )
                    config.mode = CampaignMode::Sector;
                else
                    fail( "--mode must be A or B" );
                mode_set = true;
            }
            else if ( arg == "--workers" )
                config.workers = parse_unsigned( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--jobs" )
                config.jobs = parse_u64( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--job-time-limit" )
                config.job_time_limit_seconds = parse_positive_double( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--output" )
                config.output_directory = require_value( index, argc, argv, arg );
            else if ( arg == "--seed-start" )
                config.seed_start = parse_u64( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--prefix-bits" )
                config.prefix_bits = parse_unsigned( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--sector-start" )
                config.sector_start = parse_u64( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--samples-per-sector" )
                config.samples_per_sector = parse_unsigned( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--sample-seed" )
                config.sample_seed = parse_u64( require_value( index, argc, argv, arg ), arg );
            else if ( arg == "--no-affinity" )
                config.pin_workers = false;
            else
                fail( "unknown runner option before --: " + arg );
        }

        if ( !mode_set ) fail( "missing --mode A|B" );
        if ( config.workers == 0 ) fail( "--workers must be >= 1" );
        if ( config.jobs == 0 ) fail( "--jobs must be >= 1" );
        if ( config.jobs > static_cast<std::uint64_t>( std::numeric_limits<std::size_t>::max() ) )
            fail( "--jobs exceeds this platform's addressable container size" );
        if ( config.mode == CampaignMode::MultiSeed && config.jobs - 1 > std::numeric_limits<std::uint64_t>::max() - config.seed_start )
            fail( "Scheme-A seed range overflows uint64_t" );
        if ( config.job_time_limit_seconds <= 0.0 ) fail( "missing --job-time-limit" );
        if ( config.output_directory.empty() ) fail( "missing --output" );
        if ( config.mode == CampaignMode::Sector )
        {
            if ( config.prefix_bits < 1 || config.prefix_bits > 16 )
                fail( "--prefix-bits must be in 1..16" );
            if ( config.samples_per_sector == 0 )
                fail( "--samples-per-sector must be >= 1" );
            const std::uint64_t total_sectors = std::uint64_t( 1 ) << ( 2 * config.prefix_bits );
            const std::uint64_t sectors_needed = config.jobs / config.samples_per_sector +
                                                       ( config.jobs % config.samples_per_sector != 0 ? 1u : 0u );
            if ( config.sector_start >= total_sectors || sectors_needed > total_sectors - config.sector_start )
                fail( "requested Scheme-B sector interval exceeds the prefix-sector space" );
        }
        return config;
    }

    static bool option_matches( const std::string& token, const std::string& option )
    {
        return token == option || token.rfind( option + "=", 0 ) == 0;
    }

    static void validate_passthrough( const Config& config )
    {
        std::vector<std::string> reserved { kTimeOption, "--forest-seed", "--hull-output-json", "--help", "-h" };
        if ( std::strlen( kCharacteristicsOption ) != 0 ) reserved.emplace_back( kCharacteristicsOption );
        if ( config.mode == CampaignMode::Sector )
        {
            reserved.emplace_back( kInputAOption );
            reserved.emplace_back( kInputBOption );
        }
        for ( const auto& token : config.hull_arguments )
            for ( const auto& option : reserved )
                if ( option_matches( token, option ) )
                    fail( "HULL passthrough repeats runner-owned option: " + option );
    }

    static std::uint64_t splitmix64_next( std::uint64_t& state )
    {
        std::uint64_t z = ( state += 0x9E3779B97F4A7C15ull );
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ull;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBull;
        return z ^ ( z >> 31 );
    }

    static std::string hex_u64( std::uint64_t value, unsigned width = 16 )
    {
        std::ostringstream stream;
        stream << "0x" << std::hex << std::setw( static_cast<int>( width ) ) << std::setfill( '0' ) << value;
        return stream.str();
    }

    static std::string decimal_double( double value )
    {
        std::ostringstream stream;
        stream << std::setprecision( 17 ) << value;
        return stream.str();
    }

    static std::pair<std::uint32_t, std::uint32_t> sector_representative(
        unsigned prefix_bits, std::uint64_t sector, unsigned sample_index, std::uint64_t sample_seed )
    {
        const std::uint64_t prefix_mask = ( std::uint64_t( 1 ) << prefix_bits ) - 1;
        const std::uint32_t prefix_a = static_cast<std::uint32_t>( ( sector >> prefix_bits ) & prefix_mask );
        const std::uint32_t prefix_b = static_cast<std::uint32_t>( sector & prefix_mask );
        const unsigned low_bits = 32 - prefix_bits;
        const std::uint64_t low_mask = low_bits == 32 ? 0xFFFFFFFFull : ( ( std::uint64_t( 1 ) << low_bits ) - 1 );
        std::uint64_t state = sample_seed ^ ( sector * 0xD1B54A32D192ED03ull ) ^
                              ( std::uint64_t( sample_index + 1 ) * 0x94D049BB133111EBull );
        const std::uint32_t low_a = sample_index == 0 ? 0u : static_cast<std::uint32_t>( splitmix64_next( state ) & low_mask );
        const std::uint32_t low_b = sample_index == 0 ? 0u : static_cast<std::uint32_t>( splitmix64_next( state ) & low_mask );
        std::uint32_t a = static_cast<std::uint32_t>( ( std::uint64_t( prefix_a ) << low_bits ) | low_a );
        std::uint32_t b = static_cast<std::uint32_t>( ( std::uint64_t( prefix_b ) << low_bits ) | low_b );
        if ( ( a | b ) == 0u ) b = 1u;
        return { a, b };
    }


    static std::string quote_manifest_argument( const std::string& value )
    {
        if ( value.find_first_of( " \t\"'" ) == std::string::npos ) return value;
        std::string out = "\"";
        for ( char c : value )
        {
            if ( c == '\\' || c == '\"' ) out.push_back( '\\' );
            out.push_back( c );
        }
        out.push_back( '\"' );
        return out;
    }

    static void write_command_file( const ScheduledJob& job )
    {
        std::ofstream out( job.directory / "worker_invocation.txt" );
        out << "internal_worker_api:";
        for ( const auto& argument : job.arguments ) out << ' ' << quote_manifest_argument( argument );
        out << '\n';
    }

#ifdef __linux__
    static std::vector<int> allowed_cpu_ids()
    {
        cpu_set_t set;
        CPU_ZERO( &set );
        std::vector<int> cpus;
        if ( sched_getaffinity( 0, sizeof( set ), &set ) == 0 )
            for ( int cpu = 0; cpu < CPU_SETSIZE; ++cpu )
                if ( CPU_ISSET( cpu, &set ) ) cpus.push_back( cpu );
        return cpus;
    }
#elif defined(_WIN32)
    static std::vector<int> allowed_cpu_ids()
    {
        DWORD_PTR process_mask = 0;
        DWORD_PTR system_mask = 0;
        std::vector<int> cpus;
        if ( GetProcessAffinityMask( GetCurrentProcess(), &process_mask, &system_mask ) )
            for ( int cpu = 0; cpu < static_cast<int>( sizeof( DWORD_PTR ) * 8 ); ++cpu )
                if ( ( process_mask & ( DWORD_PTR( 1 ) << cpu ) ) != 0 ) cpus.push_back( cpu );
        return cpus;
    }
#else
    static std::vector<int> allowed_cpu_ids() { return {}; }
#endif

#ifdef __linux__
    static void apply_process_affinity( std::optional<int> cpu_id )
    {
        if ( !cpu_id.has_value() ) return;
        cpu_set_t cpu_set;
        CPU_ZERO( &cpu_set );
        CPU_SET( *cpu_id, &cpu_set );
        (void)sched_setaffinity( 0, sizeof( cpu_set ), &cpu_set );
    }
#elif defined(_WIN32)
    static void apply_process_affinity( std::optional<int> cpu_id )
    {
        if ( !cpu_id.has_value() ) return;
        if ( *cpu_id >= 0 && *cpu_id < static_cast<int>( sizeof( DWORD_PTR ) * 8 ) )
            (void)SetProcessAffinityMask( GetCurrentProcess(), DWORD_PTR( 1 ) << *cpu_id );
    }
#else
    static void apply_process_affinity( std::optional<int> ) {}
#endif

    static int run_hull_api_job( const ScheduledJob& job, std::optional<int> cpu_id )
    {
        try
        {
            apply_process_affinity( cpu_id );
            std::vector<std::string> argv_values;
            argv_values.reserve( job.arguments.size() + 1 );
            argv_values.emplace_back( "hull_api_worker" );
            argv_values.insert( argv_values.end(), job.arguments.begin(), job.arguments.end() );
            std::vector<char*> argv_ptrs;
            argv_ptrs.reserve( argv_values.size() + 1 );
            for ( auto& value : argv_values ) argv_ptrs.push_back( value.data() );
            argv_ptrs.push_back( nullptr );
            neoalzette_linear_milp::run_forest_hull_search_from_argv( static_cast<int>( argv_values.size() ), argv_ptrs.data() );
            return 0;
        }
        catch ( const std::exception& exception )
        {
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << exception.what() << '\n';
            return 1;
        }
    }


    struct InternalWorkerConfig
    {
        fs::path job_directory;
        std::optional<int> cpu_id;
        std::vector<std::string> hull_arguments;
    };

    static bool is_internal_worker_invocation( int argc, char** argv )
    {
        return argc > 1 && std::string( argv[ 1 ] ) == "--internal-worker";
    }

    static InternalWorkerConfig parse_internal_worker_config( int argc, char** argv )
    {
        InternalWorkerConfig config;
        bool passthrough = false;
        for ( int index = 2; index < argc; ++index )
        {
            const std::string arg = argv[ index ];
            if ( passthrough )
            {
                config.hull_arguments.push_back( arg );
                continue;
            }
            if ( arg == "--" )
            {
                passthrough = true;
                continue;
            }
            if ( arg == "--job-dir" )
                config.job_directory = require_value( index, argc, argv, arg );
            else if ( arg == "--cpu-id" )
                config.cpu_id = static_cast<int>( parse_unsigned( require_value( index, argc, argv, arg ), arg ) );
            else
                fail( "unknown internal worker option: " + arg );
        }
        if ( config.job_directory.empty() ) fail( "internal worker missing --job-dir" );
        if ( !passthrough ) fail( "internal worker missing -- before HULL arguments" );
        return config;
    }

    static int run_internal_worker_process( int argc, char** argv )
    {
        const auto config = parse_internal_worker_config( argc, argv );
        fs::create_directories( config.job_directory );

        ScheduledJob job;
        job.directory = config.job_directory;
        job.arguments = config.hull_arguments;
        write_command_file( job );

        const int code = run_hull_api_job( job, config.cpu_id );
        std::cout.flush();
        std::cerr.flush();
        std::fflush( nullptr );
        return code;
    }

    static bool is_usable_executable( const fs::path& path )
    {
        std::error_code error;
        if ( path.empty() || !fs::exists( path, error ) || error || !fs::is_regular_file( path, error ) || error )
            return false;
#ifndef _WIN32
        return ::access( path.c_str(), X_OK ) == 0;
#else
        return true;
#endif
    }

    static fs::path search_path_for_executable( const fs::path& name )
    {
        const char* path_env = std::getenv( "PATH" );
        if ( path_env == nullptr ) return name;
#ifdef _WIN32
        constexpr char separator = ';';
#else
        constexpr char separator = ':';
#endif
        std::string paths = path_env;
        std::size_t start = 0;
        while ( start <= paths.size() )
        {
            const std::size_t end = paths.find( separator, start );
            const std::string item = paths.substr( start, end == std::string::npos ? std::string::npos : end - start );
            if ( !item.empty() )
            {
                const fs::path candidate = fs::path( item ) / name;
                if ( is_usable_executable( candidate ) ) return fs::absolute( candidate );
#ifdef _WIN32
                const fs::path candidate_exe = fs::path( item ) / ( name.string() + ".exe" );
                if ( is_usable_executable( candidate_exe ) ) return fs::absolute( candidate_exe );
#endif
            }
            if ( end == std::string::npos ) break;
            start = end + 1;
        }
        return name;
    }

    static fs::path resolve_self_executable( const char* argv0 )
    {
#ifdef __linux__
        std::vector<char> buffer( 4096 );
        const ssize_t length = ::readlink( "/proc/self/exe", buffer.data(), buffer.size() - 1 );
        if ( length > 0 )
        {
            buffer[ static_cast<std::size_t>( length ) ] = '\0';
            return fs::path( buffer.data() );
        }
#endif
        fs::path path = argv0 == nullptr ? fs::path() : fs::path( argv0 );
        if ( path.empty() ) return path;
        if ( path.has_parent_path() ) return fs::absolute( path );
        if ( is_usable_executable( path ) ) return fs::absolute( path );
        return search_path_for_executable( path );
    }

    static std::vector<std::string> make_internal_worker_arguments(
        const Config& config, const ScheduledJob& job, std::optional<int> cpu_id )
    {
        std::vector<std::string> args;
        args.emplace_back( config.self_executable.string() );
        args.emplace_back( "--internal-worker" );
        args.emplace_back( "--job-dir" );
        args.emplace_back( job.directory.string() );
        if ( cpu_id.has_value() )
        {
            args.emplace_back( "--cpu-id" );
            args.emplace_back( std::to_string( *cpu_id ) );
        }
        args.emplace_back( "--" );
        args.insert( args.end(), job.arguments.begin(), job.arguments.end() );
        return args;
    }

#ifdef _WIN32
    static std::string quote_windows_argument( const std::string& argument )
    {
        if ( argument.empty() || argument.find_first_of( " \t\"" ) != std::string::npos )
        {
            std::string out = "\"";
            unsigned backslashes = 0;
            for ( char c : argument )
            {
                if ( c == '\\' )
                {
                    ++backslashes;
                }
                else if ( c == '"' )
                {
                    out.append( backslashes * 2 + 1, '\\' );
                    out.push_back( '"' );
                    backslashes = 0;
                }
                else
                {
                    out.append( backslashes, '\\' );
                    backslashes = 0;
                    out.push_back( c );
                }
            }
            out.append( backslashes * 2, '\\' );
            out.push_back( '"' );
            return out;
        }
        return argument;
    }

    static std::string build_windows_command_line( const std::vector<std::string>& args )
    {
        std::string command;
        for ( std::size_t i = 0; i < args.size(); ++i )
        {
            if ( i != 0 ) command.push_back( ' ' );
            command += quote_windows_argument( args[ i ] );
        }
        return command;
    }
#endif

    static int launch_internal_worker_process( const Config& config, const ScheduledJob& job, std::optional<int> cpu_id )
    {
        const auto args = make_internal_worker_arguments( config, job, cpu_id );
        const fs::path stdout_path = job.directory / "stdout.log";
        const fs::path stderr_path = job.directory / "stderr.log";
#ifndef _WIN32
        std::vector<char*> argv_ptrs;
        argv_ptrs.reserve( args.size() + 1 );
        for ( const auto& arg : args ) argv_ptrs.push_back( const_cast<char*>( arg.c_str() ) );
        argv_ptrs.push_back( nullptr );

        posix_spawn_file_actions_t file_actions;
        const int init_error = ::posix_spawn_file_actions_init( &file_actions );
        if ( init_error != 0 )
        {
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "posix_spawn file-action initialization failed: " << std::strerror( init_error ) << '\n';
            return 125;
        }
        int action_error = ::posix_spawn_file_actions_addopen(
            &file_actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0 );
        if ( action_error == 0 )
            action_error = ::posix_spawn_file_actions_addopen(
                &file_actions, STDOUT_FILENO, stdout_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666 );
        if ( action_error == 0 )
            action_error = ::posix_spawn_file_actions_addopen(
                &file_actions, STDERR_FILENO, stderr_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666 );
        if ( action_error != 0 )
        {
            (void)::posix_spawn_file_actions_destroy( &file_actions );
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "posix_spawn file-action setup failed: " << std::strerror( action_error ) << '\n';
            return 125;
        }

        pid_t pid = -1;
        const int spawn_error = ::posix_spawn(
            &pid, args.front().c_str(), &file_actions, nullptr, argv_ptrs.data(), environ );
        (void)::posix_spawn_file_actions_destroy( &file_actions );
        if ( spawn_error != 0 )
        {
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "posix_spawn failed: " << std::strerror( spawn_error ) << '\n';
            return 125;
        }

        int status = 0;
        bool termination_sent = false;
        bool kill_sent = false;
        std::chrono::steady_clock::time_point termination_time{};
        for ( ;; )
        {
            const pid_t wait_result = ::waitpid( pid, &status, WNOHANG );
            if ( wait_result == pid ) break;
            if ( wait_result < 0 )
            {
                if ( errno == EINTR ) continue;
                const int wait_error = errno;
                (void)::kill( pid, SIGKILL );
                int reap_status = 0;
                while ( ::waitpid( pid, &reap_status, 0 ) < 0 && errno == EINTR ) {}
                std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
                error_out << "waitpid failed: " << std::strerror( wait_error ) << '\n';
                return 126;
            }
            if ( stop_requested() && !termination_sent )
            {
                (void)::kill( pid, SIGTERM );
                termination_sent = true;
                termination_time = std::chrono::steady_clock::now();
            }
            if ( termination_sent && !kill_sent &&
                 std::chrono::steady_clock::now() - termination_time >= std::chrono::seconds( 5 ) )
            {
                (void)::kill( pid, SIGKILL );
                kill_sent = true;
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        }
        if ( WIFEXITED( status ) ) return WEXITSTATUS( status );
        if ( WIFSIGNALED( status ) ) return 128 + WTERMSIG( status );
        return 126;
#else
        SECURITY_ATTRIBUTES security{};
        security.nLength = sizeof( security );
        security.bInheritHandle = TRUE;

        HANDLE child_stdout = CreateFileA(
            stdout_path.string().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
        HANDLE child_stderr = CreateFileA(
            stderr_path.string().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
        HANDLE child_stdin = CreateFileA(
            "NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
        if ( child_stdout == INVALID_HANDLE_VALUE || child_stderr == INVALID_HANDLE_VALUE || child_stdin == INVALID_HANDLE_VALUE )
        {
            const DWORD error = GetLastError();
            if ( child_stdout != INVALID_HANDLE_VALUE ) CloseHandle( child_stdout );
            if ( child_stderr != INVALID_HANDLE_VALUE ) CloseHandle( child_stderr );
            if ( child_stdin != INVALID_HANDLE_VALUE ) CloseHandle( child_stdin );
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "cannot create child standard-stream handles: " << error << '\n';
            return 125;
        }

        SIZE_T attribute_bytes = 0;
        (void)InitializeProcThreadAttributeList( nullptr, 1, 0, &attribute_bytes );
        std::vector<unsigned char> attribute_storage( attribute_bytes );
        auto* attribute_list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>( attribute_storage.data() );
        if ( attribute_bytes == 0 || !InitializeProcThreadAttributeList( attribute_list, 1, 0, &attribute_bytes ) )
        {
            const DWORD error = GetLastError();
            CloseHandle( child_stdin );
            CloseHandle( child_stdout );
            CloseHandle( child_stderr );
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "InitializeProcThreadAttributeList failed: " << error << '\n';
            return 125;
        }
        HANDLE inherited_handles[] = { child_stdin, child_stdout, child_stderr };
        if ( !UpdateProcThreadAttribute(
                attribute_list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                inherited_handles, sizeof( inherited_handles ), nullptr, nullptr ) )
        {
            const DWORD error = GetLastError();
            DeleteProcThreadAttributeList( attribute_list );
            CloseHandle( child_stdin );
            CloseHandle( child_stdout );
            CloseHandle( child_stderr );
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "UpdateProcThreadAttribute failed: " << error << '\n';
            return 125;
        }

        std::string command_line = build_windows_command_line( args );
        STARTUPINFOEXA startup{};
        startup.StartupInfo.cb = sizeof( startup );
        startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        startup.StartupInfo.hStdInput = child_stdin;
        startup.StartupInfo.hStdOutput = child_stdout;
        startup.StartupInfo.hStdError = child_stderr;
        startup.lpAttributeList = attribute_list;
        PROCESS_INFORMATION process{};
        std::vector<char> mutable_command( command_line.begin(), command_line.end() );
        mutable_command.push_back( '\0' );
        const std::string application_name = config.self_executable.string();
        const BOOL ok = CreateProcessA(
            application_name.c_str(), mutable_command.data(), nullptr, nullptr, TRUE,
            EXTENDED_STARTUPINFO_PRESENT | CREATE_NO_WINDOW,
            nullptr, nullptr, &startup.StartupInfo, &process );
        const DWORD create_error = ok ? ERROR_SUCCESS : GetLastError();
        DeleteProcThreadAttributeList( attribute_list );
        CloseHandle( child_stdin );
        CloseHandle( child_stdout );
        CloseHandle( child_stderr );
        if ( !ok )
        {
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "CreateProcessA failed: " << create_error << '\n';
            return 125;
        }

        bool termination_sent = false;
        DWORD wait_result = WAIT_TIMEOUT;
        while ( wait_result == WAIT_TIMEOUT )
        {
            wait_result = WaitForSingleObject( process.hProcess, 100 );
            if ( stop_requested() && !termination_sent )
            {
                (void)TerminateProcess( process.hProcess, 130 );
                termination_sent = true;
            }
        }

        DWORD exit_code = 126;
        if ( wait_result != WAIT_OBJECT_0 )
        {
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "WaitForSingleObject failed: " << GetLastError() << '\n';
        }
        else if ( !GetExitCodeProcess( process.hProcess, &exit_code ) )
        {
            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
            error_out << "GetExitCodeProcess failed: " << GetLastError() << '\n';
            exit_code = 126;
        }
        CloseHandle( process.hThread );
        CloseHandle( process.hProcess );
        return static_cast<int>( exit_code );
#endif
    }

    // Minimal strict JSON parser used only for the JSON generated by the HULL API.
    struct Json
    {
        using Array = std::vector<Json>;
        using Object = std::map<std::string, Json>;
        std::variant<std::nullptr_t, bool, double, std::string, Array, Object> value = nullptr;

        const Object* object() const { return std::get_if<Object>( &value ); }
        const Array* array() const { return std::get_if<Array>( &value ); }
        const std::string* string() const { return std::get_if<std::string>( &value ); }
        const double* number() const { return std::get_if<double>( &value ); }
        const bool* boolean() const { return std::get_if<bool>( &value ); }
    };

    class JsonParser
    {
    public:
        explicit JsonParser( std::string text ) : text_( std::move( text ) ) {}
        Json parse()
        {
            skip_ws();
            Json value = parse_value();
            skip_ws();
            if ( position_ != text_.size() ) fail( "trailing bytes in JSON" );
            return value;
        }
    private:
        std::string text_;
        std::size_t position_ = 0;

        void skip_ws()
        {
            while ( position_ < text_.size() && std::isspace( static_cast<unsigned char>( text_[ position_ ] ) ) ) ++position_;
        }
        char peek() const { return position_ < text_.size() ? text_[ position_ ] : '\0'; }
        char take()
        {
            if ( position_ >= text_.size() ) fail( "unexpected end of JSON" );
            return text_[ position_++ ];
        }
        void expect( char expected )
        {
            if ( take() != expected ) fail( std::string( "expected JSON character: " ) + expected );
        }
        bool consume_literal( const char* literal )
        {
            const std::size_t length = std::strlen( literal );
            if ( text_.compare( position_, length, literal ) != 0 ) return false;
            position_ += length;
            return true;
        }
        Json parse_value()
        {
            skip_ws();
            const char c = peek();
            if ( c == '{' ) return Json { parse_object() };
            if ( c == '[' ) return Json { parse_array() };
            if ( c == '\"' ) return Json { parse_string() };
            if ( c == 't' && consume_literal( "true" ) ) return Json { true };
            if ( c == 'f' && consume_literal( "false" ) ) return Json { false };
            if ( c == 'n' && consume_literal( "null" ) ) return Json { nullptr };
            if ( c == '-' || std::isdigit( static_cast<unsigned char>( c ) ) ) return Json { parse_number() };
            fail( "invalid JSON value" );
        }
        Json::Object parse_object()
        {
            Json::Object object;
            expect( '{' );
            skip_ws();
            if ( peek() == '}' ) { take(); return object; }
            for ( ;; )
            {
                skip_ws();
                const std::string key = parse_string();
                skip_ws(); expect( ':' );
                skip_ws(); object.emplace( key, parse_value() );
                skip_ws();
                const char separator = take();
                if ( separator == '}' ) break;
                if ( separator != ',' ) fail( "invalid JSON object separator" );
            }
            return object;
        }
        Json::Array parse_array()
        {
            Json::Array array;
            expect( '[' );
            skip_ws();
            if ( peek() == ']' ) { take(); return array; }
            for ( ;; )
            {
                array.push_back( parse_value() );
                skip_ws();
                const char separator = take();
                if ( separator == ']' ) break;
                if ( separator != ',' ) fail( "invalid JSON array separator" );
                skip_ws();
            }
            return array;
        }
        static unsigned hex_digit( char c )
        {
            if ( c >= '0' && c <= '9' ) return c - '0';
            if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
            if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
            fail( "invalid JSON unicode escape" );
        }
        std::string parse_string()
        {
            expect( '\"' );
            std::string value;
            while ( true )
            {
                const char c = take();
                if ( c == '\"' ) break;
                if ( c != '\\' ) { value.push_back( c ); continue; }
                const char escape = take();
                switch ( escape )
                {
                    case '\"': value.push_back( '\"' ); break;
                    case '\\': value.push_back( '\\' ); break;
                    case '/': value.push_back( '/' ); break;
                    case 'b': value.push_back( '\b' ); break;
                    case 'f': value.push_back( '\f' ); break;
                    case 'n': value.push_back( '\n' ); break;
                    case 'r': value.push_back( '\r' ); break;
                    case 't': value.push_back( '\t' ); break;
                    case 'u':
                    {
                        unsigned code = 0;
                        for ( int i = 0; i < 4; ++i ) code = ( code << 4 ) | hex_digit( take() );
                        if ( code <= 0x7F ) value.push_back( static_cast<char>( code ) );
                        else value.push_back( '?' );
                        break;
                    }
                    default: fail( "invalid JSON escape" );
                }
            }
            return value;
        }
        double parse_number()
        {
            const std::size_t begin = position_;
            if ( peek() == '-' ) ++position_;
            while ( std::isdigit( static_cast<unsigned char>( peek() ) ) ) ++position_;
            if ( peek() == '.' )
            {
                ++position_;
                while ( std::isdigit( static_cast<unsigned char>( peek() ) ) ) ++position_;
            }
            if ( peek() == 'e' || peek() == 'E' )
            {
                ++position_;
                if ( peek() == '+' || peek() == '-' ) ++position_;
                while ( std::isdigit( static_cast<unsigned char>( peek() ) ) ) ++position_;
            }
            const std::string token = text_.substr( begin, position_ - begin );
            char* end = nullptr;
            errno = 0;
            const double value = std::strtod( token.c_str(), &end );
            if ( errno != 0 || end != token.c_str() + token.size() ) fail( "invalid JSON number" );
            return value;
        }
    };

    static const Json* member( const Json& value, const std::string& name )
    {
        const auto* object = value.object();
        if ( object == nullptr ) return nullptr;
        const auto iterator = object->find( name );
        return iterator == object->end() ? nullptr : &iterator->second;
    }

    static std::optional<double> number_member( const Json& value, const std::string& name )
    {
        const Json* child = member( value, name );
        if ( child == nullptr || child->number() == nullptr ) return std::nullopt;
        return *child->number();
    }

    static std::optional<bool> bool_member( const Json& value, const std::string& name )
    {
        const Json* child = member( value, name );
        if ( child == nullptr || child->boolean() == nullptr ) return std::nullopt;
        return *child->boolean();
    }

    static std::string string_member( const Json& value, const std::string& name )
    {
        const Json* child = member( value, name );
        return child != nullptr && child->string() != nullptr ? *child->string() : std::string();
    }

    [[maybe_unused]] static std::optional<double> numeric_or_string_member( const Json& value, const std::string& name )
    {
        const Json* child = member( value, name );
        if ( child == nullptr ) return std::nullopt;
        if ( const double* number = child->number() ) return *number;
        if ( const std::string* string = child->string() )
        {
            char* end = nullptr;
            errno = 0;
            const double parsed = std::strtod( string->c_str(), &end );
            if ( errno == 0 && end == string->c_str() + string->size() ) return parsed;
        }
        return std::nullopt;
    }

    static std::uint32_t parse_hex32_field( const Json& object, const char* name )
    {
        const std::string text = string_member( object, name );
        if ( text.empty() ) fail( std::string( "missing JSON field " ) + name );
        const auto value = parse_u64( text, name );
        if ( value > 0xFFFFFFFFull ) fail( std::string( "JSON field exceeds 32 bits: " ) + name );
        return static_cast<std::uint32_t>( value );
    }

    static Json read_json_file( const fs::path& path )
    {
        std::ifstream input( path, std::ios::binary );
        if ( !input ) fail( "cannot open JSON file: " + path.string() );
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return JsonParser( buffer.str() ).parse();
    }

    static std::vector<AttemptRecord> extract_attempts( const ScheduledJob& job )
    {
        std::vector<AttemptRecord> records;
        const Json root = read_json_file( job.directory / "result.json" );
        const Json* attempts_value = member( root, "attempts" );
        if ( attempts_value == nullptr || attempts_value->array() == nullptr )
            fail( "result JSON has no attempts array" );
        for ( const Json& attempt : *attempts_value->array() )
        {
            const Json* input = member( attempt, "new_input_source" );
            if ( input == nullptr || input->object() == nullptr ) continue;
            AttemptRecord record;
            record.job_id = job.job_id;
            record.forest_seed = job.forest_seed;
            record.sector = job.sector;
            record.sector_sample = job.sector_sample;
            if ( auto value = number_member( attempt, "attempt_id" ) ) record.attempt_id = static_cast<int>( *value );
            if ( auto value = number_member( attempt, "tree_id" ) ) record.tree_id = static_cast<int>( *value );
            if ( auto value = number_member( attempt, "layer" ) ) record.layer = static_cast<int>( *value );
            record.input_a = parse_hex32_field( *input, kInputAField );
            record.input_b = parse_hex32_field( *input, kInputBField );
            if ( const Json* endpoint = member( attempt, "endpoint" ); endpoint != nullptr && endpoint->object() != nullptr )
            {
                record.has_output = true;
                record.output_a = parse_hex32_field( *endpoint, kOutputAField );
                record.output_b = parse_hex32_field( *endpoint, kOutputBField );
            }
            if ( auto value = number_member( attempt, "local_best_weight" ) ) record.local_weight = *value;
            if ( auto value = number_member( attempt, "cumulative_weight_after" ) ) record.cumulative_weight = *value;
            if ( auto value = number_member( attempt, "signed_correlation_sum" ) ) record.hull_primary_measure = *value;
            if ( auto value = number_member( attempt, "abs_correlation_sum" ) ) record.hull_absolute_measure = *value;
            if ( auto value = number_member( attempt, "effective_weight_signed_abs" ) ) record.hull_effective_weight = *value;
            if ( auto value = bool_member( attempt, "cycle_detected" ) ) record.cycle_detected = *value;
            if ( auto value = bool_member( attempt, "completed" ) ) record.completed = *value;
            record.stop_reason = string_member( attempt, "stop_reason" );
            records.push_back( std::move( record ) );
        }
        return records;
    }

    static std::string json_escape( const std::string& text )
    {
        std::ostringstream out;
        out << '\"';
        for ( unsigned char c : text )
        {
            switch ( c )
            {
                case '\"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\b': out << "\\b"; break;
                case '\f': out << "\\f"; break;
                case '\n': out << "\\n"; break;
                case '\r': out << "\\r"; break;
                case '\t': out << "\\t"; break;
                default:
                    if ( c < 0x20 ) out << "\\u" << std::hex << std::setw( 4 ) << std::setfill( '0' ) << unsigned( c ) << std::dec;
                    else out << static_cast<char>( c );
            }
        }
        out << '\"';
        return out.str();
    }

    static std::uint64_t pack_pair( std::uint32_t a, std::uint32_t b )
    {
        return ( std::uint64_t( a ) << 32 ) | b;
    }

    static unsigned popcount32( std::uint32_t value )
    {
#if defined(__GNUC__) || defined(__clang__)
        return static_cast<unsigned>( __builtin_popcount( value ) );
#else
        unsigned count = 0;
        while ( value != 0 ) { value &= value - 1; ++count; }
        return count;
#endif
    }

    static std::string scientific_fraction( std::uint64_t numerator, long double denominator )
    {
        std::ostringstream out;
        out << std::scientific << std::setprecision( 12 ) << ( static_cast<long double>( numerator ) / denominator );
        return out.str();
    }

    static void write_manifest_jsonl( const fs::path& output, const std::vector<ScheduledJob>& jobs )
    {
        std::ofstream out( output / "job_manifest.jsonl" );
        for ( const auto& job : jobs )
        {
            out << "{\"job_id\":" << job.job_id
                << ",\"forest_seed\":" << json_escape( hex_u64( job.forest_seed ) )
                << ",\"sector\":" << ( job.sector ? std::to_string( *job.sector ) : "null" )
                << ",\"sector_sample\":" << job.sector_sample
                << ",\"fixed_a\":" << ( job.fixed_a ? json_escape( hex_u64( *job.fixed_a, 8 ) ) : "null" )
                << ",\"fixed_b\":" << ( job.fixed_b ? json_escape( hex_u64( *job.fixed_b, 8 ) ) : "null" )
                << ",\"exit_code\":" << job.exit_code
                << ",\"result_json_exists\":" << ( job.result_json_exists ? "true" : "false" )
                << ",\"directory\":" << json_escape( job.directory.string() )
                << ",\"failure\":" << json_escape( job.failure ) << "}\n";
        }
    }

    static void write_candidates_jsonl( const fs::path& output, const std::vector<AttemptRecord>& records )
    {
        std::ofstream out( output / "forest_candidates.jsonl" );
        for ( const auto& record : records )
        {
            out << "{\"analysis\":" << json_escape( kAnalysis )
                << ",\"job_id\":" << record.job_id
                << ",\"forest_seed\":" << json_escape( hex_u64( record.forest_seed ) )
                << ",\"sector\":" << ( record.sector ? std::to_string( *record.sector ) : "null" )
                << ",\"sector_sample\":" << record.sector_sample
                << ",\"attempt_id\":" << record.attempt_id
                << ",\"tree_id\":" << record.tree_id
                << ",\"layer\":" << record.layer
                << ",\"input_a\":" << json_escape( hex_u64( record.input_a, 8 ) )
                << ",\"input_b\":" << json_escape( hex_u64( record.input_b, 8 ) )
                << ",\"output_a\":" << ( record.has_output ? json_escape( hex_u64( record.output_a, 8 ) ) : "null" )
                << ",\"output_b\":" << ( record.has_output ? json_escape( hex_u64( record.output_b, 8 ) ) : "null" )
                << ",\"local_weight\":" << ( std::isfinite( record.local_weight ) ? decimal_double( record.local_weight ) : "null" )
                << ",\"cumulative_weight\":" << ( std::isfinite( record.cumulative_weight ) ? decimal_double( record.cumulative_weight ) : "null" )
                << ",\"hull_primary_measure\":" << ( std::isfinite( record.hull_primary_measure ) ? decimal_double( record.hull_primary_measure ) : "null" )
                << ",\"hull_absolute_measure\":" << ( std::isfinite( record.hull_absolute_measure ) ? decimal_double( record.hull_absolute_measure ) : "null" )
                << ",\"hull_effective_weight\":" << ( std::isfinite( record.hull_effective_weight ) ? decimal_double( record.hull_effective_weight ) : "null" )
                << ",\"cycle_detected\":" << ( record.cycle_detected ? "true" : "false" )
                << ",\"completed\":" << ( record.completed ? "true" : "false" )
                << ",\"stop_reason\":" << json_escape( record.stop_reason ) << "}\n";
        }
    }

    static void write_campaign_summary( const Config& config, const std::vector<ScheduledJob>& jobs, const std::vector<AttemptRecord>& records )
    {
        std::unordered_set<std::uint32_t> unique_a;
        std::unordered_set<std::uint32_t> unique_b;
        std::unordered_set<std::uint64_t> unique_pairs;
        std::unordered_set<std::uint64_t> unique_outputs;
        std::unordered_set<std::uint32_t> prefix8_cells;
        std::unordered_set<std::uint32_t> prefix12_cells;
        std::set<std::tuple<unsigned, unsigned, unsigned>> hamming_cells;
        std::set<std::pair<std::uint64_t, int>> trees;
        std::map<std::uint64_t, std::uint64_t> output_frequency;
        std::uint64_t duplicate_pair_observations = 0;
        std::uint64_t cycle_count = 0;
        std::uint64_t completed_attempts = 0;
        int max_layer = -1;
        double best_local = std::numeric_limits<double>::infinity();
        double best_cumulative = std::numeric_limits<double>::infinity();
        double best_hull_effective = std::numeric_limits<double>::infinity();
        double maximum_hull_absolute = 0.0;

        for ( const auto& record : records )
        {
            unique_a.insert( record.input_a );
            unique_b.insert( record.input_b );
            const std::uint64_t pair = pack_pair( record.input_a, record.input_b );
            if ( !unique_pairs.insert( pair ).second ) ++duplicate_pair_observations;
            prefix8_cells.insert( ( ( record.input_a >> 24 ) << 8 ) | ( record.input_b >> 24 ) );
            prefix12_cells.insert( ( ( record.input_a >> 20 ) << 12 ) | ( record.input_b >> 20 ) );
            hamming_cells.emplace( popcount32( record.input_a ), popcount32( record.input_b ), popcount32( record.input_a ^ record.input_b ) );
            trees.emplace( record.job_id, record.tree_id );
            max_layer = std::max( max_layer, record.layer );
            if ( record.cycle_detected ) ++cycle_count;
            if ( record.completed ) ++completed_attempts;
            if ( std::isfinite( record.local_weight ) ) best_local = std::min( best_local, record.local_weight );
            if ( std::isfinite( record.cumulative_weight ) ) best_cumulative = std::min( best_cumulative, record.cumulative_weight );
            if ( std::isfinite( record.hull_effective_weight ) ) best_hull_effective = std::min( best_hull_effective, record.hull_effective_weight );
            if ( std::isfinite( record.hull_absolute_measure ) ) maximum_hull_absolute = std::max( maximum_hull_absolute, record.hull_absolute_measure );
            if ( record.has_output )
            {
                const std::uint64_t output_pair = pack_pair( record.output_a, record.output_b );
                unique_outputs.insert( output_pair );
                ++output_frequency[ output_pair ];
            }
        }

        std::vector<std::pair<std::uint64_t, std::uint64_t>> top_outputs( output_frequency.begin(), output_frequency.end() );
        std::sort( top_outputs.begin(), top_outputs.end(), []( const auto& left, const auto& right )
        {
            if ( left.second != right.second ) return left.second > right.second;
            return left.first < right.first;
        } );
        if ( top_outputs.size() > 32 ) top_outputs.resize( 32 );

        std::uint64_t successful_jobs = 0;
        std::uint64_t failed_jobs = 0;
        std::set<std::uint64_t> scheduled_sectors;
        for ( const auto& job : jobs )
        {
            if ( job.exit_code == 0 && job.result_json_exists && job.failure.empty() ) ++successful_jobs;
            else ++failed_jobs;
            if ( job.sector ) scheduled_sectors.insert( *job.sector );
        }

        std::ofstream out( config.output_directory / "campaign_summary.json" );
        out << "{\n"
            << "  \"analysis\": " << json_escape( kAnalysis ) << ",\n"
            << "  \"runner\": \"hull_multiple_thread_runner\",\n"
            << "  \"mode\": " << json_escape( config.mode == CampaignMode::MultiSeed ? "A_multi_seed_forest" : "B_fixed_input_sector_forest" ) << ",\n"
            << "  \"workers\": " << config.workers << ",\n"
            << "  \"scheduled_jobs\": " << jobs.size() << ",\n"
            << "  \"successful_jobs\": " << successful_jobs << ",\n"
            << "  \"failed_jobs\": " << failed_jobs << ",\n"
            << "  \"job_time_limit_seconds\": " << decimal_double( config.job_time_limit_seconds ) << ",\n"
            << "  \"cross_worker_hull_numeric_merge\": \"not_performed_to_avoid_duplicate_characteristic_counting\",\n"
            << "  \"coverage_semantics\": \"exact_unique_states_observed_in_worker_forest_attempt_records\",\n"
            << "  \"total_attempt_records\": " << records.size() << ",\n"
            << "  \"completed_attempt_records\": " << completed_attempts << ",\n"
            << "  \"tree_count\": " << trees.size() << ",\n"
            << "  \"cycle_count\": " << cycle_count << ",\n"
            << "  \"max_layer\": " << max_layer << ",\n"
            << "  \"unique_a_count\": " << unique_a.size() << ",\n"
            << "  \"unique_b_count\": " << unique_b.size() << ",\n"
            << "  \"unique_input_pair_count\": " << unique_pairs.size() << ",\n"
            << "  \"duplicate_input_pair_observations\": " << duplicate_pair_observations << ",\n"
            << "  \"unique_output_pair_count\": " << unique_outputs.size() << ",\n"
            << "  \"a_space_coverage_fraction\": " << json_escape( scientific_fraction( unique_a.size(), std::ldexp( 1.0L, 32 ) ) ) << ",\n"
            << "  \"b_space_coverage_fraction\": " << json_escape( scientific_fraction( unique_b.size(), std::ldexp( 1.0L, 32 ) ) ) << ",\n"
            << "  \"pair_space_coverage_fraction\": " << json_escape( scientific_fraction( unique_pairs.size(), std::ldexp( 1.0L, 64 ) ) ) << ",\n"
            << "  \"prefix_cell_8_count\": " << prefix8_cells.size() << ",\n"
            << "  \"prefix_cell_8_coverage_fraction\": " << json_escape( scientific_fraction( prefix8_cells.size(), std::ldexp( 1.0L, 16 ) ) ) << ",\n"
            << "  \"prefix_cell_12_count\": " << prefix12_cells.size() << ",\n"
            << "  \"prefix_cell_12_coverage_fraction\": " << json_escape( scientific_fraction( prefix12_cells.size(), std::ldexp( 1.0L, 24 ) ) ) << ",\n"
            << "  \"hamming_cell_count\": " << hamming_cells.size() << ",\n"
            << "  \"best_local_weight\": " << ( std::isfinite( best_local ) ? decimal_double( best_local ) : "null" ) << ",\n"
            << "  \"best_cumulative_weight\": " << ( std::isfinite( best_cumulative ) ? decimal_double( best_cumulative ) : "null" ) << ",\n"
            << "  \"best_per_worker_hull_effective_weight\": " << ( std::isfinite( best_hull_effective ) ? decimal_double( best_hull_effective ) : "null" ) << ",\n"
            << "  \"maximum_per_worker_hull_absolute_measure\": " << decimal_double( maximum_hull_absolute ) << ",\n"
            << "  \"per_worker_hull_measure_note\": \"reported_without_cross_worker_summation\",\n"
            << "  \"scheme_b_prefix_bits\": " << ( config.mode == CampaignMode::Sector ? std::to_string( config.prefix_bits ) : "null" ) << ",\n"
            << "  \"scheme_b_scheduled_sector_count\": " << scheduled_sectors.size() << ",\n"
            << "  \"scheme_b_note\": " << json_escape( config.mode == CampaignMode::Sector
                ? "each scheduled sector contributes deterministic representative inputs; this is not exhaustive enumeration of every 64-bit pair in the sector"
                : "not_applicable" ) << ",\n"
            << "  \"top_output_pairs\": [";
        if ( !top_outputs.empty() ) out << '\n';
        for ( std::size_t i = 0; i < top_outputs.size(); ++i )
        {
            const auto pair = top_outputs[ i ].first;
            out << "    {\"a\":" << json_escape( hex_u64( pair >> 32, 8 ) )
                << ",\"b\":" << json_escape( hex_u64( pair & 0xFFFFFFFFull, 8 ) )
                << ",\"observations\":" << top_outputs[ i ].second << "}";
            if ( i + 1 != top_outputs.size() ) out << ',';
            out << '\n';
        }
        out << "  ]\n}\n";
    }

    static std::vector<ScheduledJob> build_jobs( const Config& config )
    {
        std::vector<ScheduledJob> jobs;
        jobs.reserve( static_cast<std::size_t>( config.jobs ) );
        const fs::path output_absolute = fs::absolute( config.output_directory );
        for ( std::uint64_t job_id = 0; job_id < config.jobs; ++job_id )
        {
            ScheduledJob job;
            job.job_id = job_id;
            job.directory = output_absolute / ( "job_" + [&]
            {
                std::ostringstream name;
                name << std::setw( 6 ) << std::setfill( '0' ) << job_id;
                return name.str();
            }() );
            if ( config.mode == CampaignMode::MultiSeed )
                job.forest_seed = config.seed_start + job_id;
            else
            {
                const std::uint64_t sector = config.sector_start + job_id / config.samples_per_sector;
                const unsigned sample = static_cast<unsigned>( job_id % config.samples_per_sector );
                const auto [ a, b ] = sector_representative( config.prefix_bits, sector, sample, config.sample_seed );
                job.sector = sector;
                job.sector_sample = sample;
                job.fixed_a = a;
                job.fixed_b = b;
                job.forest_seed = config.sample_seed;  // Scheme B intentionally holds the Forest seed constant.
            }
            job.arguments = config.hull_arguments;
            job.arguments.emplace_back( kTimeOption );
            job.arguments.push_back( decimal_double( config.job_time_limit_seconds ) );
            job.arguments.emplace_back( "--forest-seed" );
            job.arguments.push_back( hex_u64( job.forest_seed ) );
            job.arguments.emplace_back( "--hull-output-json" );
            job.arguments.emplace_back( ( job.directory / "result.json" ).string() );
            if ( std::strlen( kCharacteristicsOption ) != 0 )
            {
                job.arguments.emplace_back( kCharacteristicsOption );
                job.arguments.emplace_back( ( job.directory / "characteristics.jsonl" ).string() );
            }
            if ( config.mode == CampaignMode::Sector )
            {
                job.arguments.emplace_back( kInputAOption );
                job.arguments.push_back( hex_u64( *job.fixed_a, 8 ) );
                job.arguments.emplace_back( kInputBOption );
                job.arguments.push_back( hex_u64( *job.fixed_b, 8 ) );
            }
            jobs.push_back( std::move( job ) );
        }
        return jobs;
    }

    static int run( int argc, char** argv )
    {
#ifndef _WIN32
        struct sigaction action {};
        action.sa_handler = signal_handler;
        ::sigemptyset( &action.sa_mask );
        action.sa_flags = 0;
        if ( ::sigaction( SIGINT, &action, nullptr ) != 0 || ::sigaction( SIGTERM, &action, nullptr ) != 0 )
            fail( std::string( "cannot install signal handlers: " ) + std::strerror( errno ) );
#else
        if ( !SetConsoleCtrlHandler( console_control_handler, TRUE ) )
            fail( "cannot install Windows console control handler" );
#endif
        Config config = parse_config( argc, argv );
        config.self_executable = resolve_self_executable( argv[ 0 ] );
        if ( !is_usable_executable( config.self_executable ) )
            fail( "cannot resolve the runner executable for OS worker creation: " + config.self_executable.string() );
        validate_passthrough( config );
        config.output_directory = fs::absolute( config.output_directory );
        if ( fs::exists( config.output_directory ) )
        {
            if ( !fs::is_directory( config.output_directory ) )
                fail( "campaign output path exists and is not a directory: " + config.output_directory.string() );
            if ( !fs::is_empty( config.output_directory ) )
                fail( "campaign output directory already exists and is not empty: " + config.output_directory.string() );
        }
        fs::create_directories( config.output_directory );

        auto jobs = build_jobs( config );
        for ( auto& job : jobs )
        {
            fs::create_directories( job.directory );
            write_command_file( job );
        }

        const std::vector<int> cpus = config.pin_workers ? allowed_cpu_ids() : std::vector<int> {};
        std::atomic<std::uint64_t> next_job { 0 };
        std::vector<std::thread> workers;
        workers.reserve( config.workers );

        {
            std::lock_guard<std::mutex> lock( gPrintMutex );
            std::cout << "NeoAlzette " << kAnalysis << " HULL campaign\n"
                      << "mode=" << ( config.mode == CampaignMode::MultiSeed ? "A_multi_seed" : "B_fixed_input_sector" )
                      << " workers=" << config.workers << " jobs=" << config.jobs
                      << " job_time_limit=" << config.job_time_limit_seconds << "s\n"
                      << "output=" << config.output_directory << "\n"
                      << "execution=thread_scheduled_os_process_workers_with_internal_hull_api\n";
        }

        try
        {
            for ( unsigned worker_index = 0; worker_index < config.workers; ++worker_index )
            {
                workers.emplace_back( [&, worker_index]
                {
                    const std::optional<int> cpu = cpus.empty() ? std::nullopt : std::optional<int>( cpus[ worker_index % cpus.size() ] );
                    while ( !stop_requested() )
                    {
                        const std::uint64_t job_index = next_job.fetch_add( 1 );
                        if ( job_index >= jobs.size() ) break;
                        if ( stop_requested() ) break;
                        auto& job = jobs[ static_cast<std::size_t>( job_index ) ];
                        try
                        {
                            {
                                std::lock_guard<std::mutex> lock( gPrintMutex );
                                std::cout << "[worker " << worker_index << "] start job " << job.job_id
                                          << " seed=" << hex_u64( job.forest_seed );
                                if ( job.sector ) std::cout << " sector=" << *job.sector << " sample=" << job.sector_sample;
                                std::cout << '\n';
                            }
                            job.exit_code = launch_internal_worker_process( config, job, cpu );
                            job.result_json_exists = fs::exists( job.directory / "result.json" );
                            if ( job.exit_code != 0 ) job.failure = "child_exit_code_" + std::to_string( job.exit_code );
                            else if ( !job.result_json_exists ) job.failure = "missing_result_json";
                        }
                        catch ( const std::exception& exception )
                        {
                            job.exit_code = 1;
                            job.result_json_exists = fs::exists( job.directory / "result.json" );
                            job.failure = std::string( "scheduler_exception:" ) + exception.what();
                            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
                            error_out << job.failure << '\n';
                        }
                        catch ( ... )
                        {
                            job.exit_code = 1;
                            job.result_json_exists = fs::exists( job.directory / "result.json" );
                            job.failure = "scheduler_exception:unknown";
                            std::ofstream error_out( job.directory / "api_error.txt", std::ios::app );
                            error_out << job.failure << '\n';
                        }
                        {
                            std::lock_guard<std::mutex> lock( gPrintMutex );
                            std::cout << "[worker " << worker_index << "] finish job " << job.job_id
                                      << " exit=" << job.exit_code
                                      << " result_json=" << ( job.result_json_exists ? "yes" : "no" ) << '\n';
                        }
                    }
                } );
            }
        }
        catch ( ... )
        {
            gStopRequested.store( true, std::memory_order_relaxed );
            for ( auto& worker : workers )
                if ( worker.joinable() ) worker.join();
            throw;
        }
        for ( auto& worker : workers )
            if ( worker.joinable() ) worker.join();

        for ( auto& job : jobs )
            if ( job.exit_code < 0 && job.failure.empty() )
                job.failure = stop_requested() ? "not_started_due_to_stop_request" : "not_started";

        std::vector<AttemptRecord> records;
        for ( auto& job : jobs )
        {
            if ( !job.result_json_exists ) continue;
            try
            {
                auto job_records = extract_attempts( job );
                records.insert( records.end(), std::make_move_iterator( job_records.begin() ), std::make_move_iterator( job_records.end() ) );
            }
            catch ( const std::exception& exception )
            {
                if ( !job.failure.empty() ) job.failure += ";";
                job.failure += std::string( "result_parse_error:" ) + exception.what();
            }
        }

        write_manifest_jsonl( config.output_directory, jobs );
        write_candidates_jsonl( config.output_directory, records );
        write_campaign_summary( config, jobs, records );

        std::uint64_t failed = 0;
        for ( const auto& job : jobs ) if ( job.exit_code != 0 || !job.result_json_exists || !job.failure.empty() ) ++failed;
        std::cout << "campaign complete: jobs=" << jobs.size() << " failed=" << failed
                  << " attempt_records=" << records.size() << '\n'
                  << "summary=" << ( config.output_directory / "campaign_summary.json" ) << '\n';
        if ( stop_requested() ) return 130;
        return failed == 0 ? 0 : 2;
    }
}

int main( int argc, char** argv )
{
    try
    {
        if ( neoalzette_runner::is_internal_worker_invocation( argc, argv ) )
            return neoalzette_runner::run_internal_worker_process( argc, argv );
        return neoalzette_runner::run( argc, argv );
    }
    catch ( const std::exception& exception )
    {
        std::cerr << "hull_multiple_thread_runner error: " << exception.what() << '\n';
        return 1;
    }
}
