// I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I

// Local Includes
#include "json_to_csv/version.hpp"

// RapidJSON Includes
#include "rapidjson/document.h"

// Boost Library Includes
#include <boost/program_options.hpp>
#include <boost/optional.hpp>

// Standard Library Includes
#include <fstream>
#include <iostream>
#include <string>

// I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I I


auto options()
{
    namespace po = boost::program_options;
    static auto Options
        = []()
            {
                auto Options = po::options_description( "Allowed Options" );

                Options.add_options()
                    ( "help", "Display this help message" )
                    ( "version", "Display detailed version information" )
                    ( "input", "The StockTwits JSON dump we wish to read" )
                    ( "output,o", po::value<std::string>(), "The name of the output file. Defaults to <input>.csv");

                return Options;
            }();
    return Options;
}


auto positional_options()
{
    auto PositionalOptions = boost::program_options::positional_options_description();
    PositionalOptions.add( "input", -1 );
    return PositionalOptions;
}


auto read_options( int argc, char* argv[] )
{
    boost::program_options::variables_map VariablesMap;
    store
        (   boost::program_options::command_line_parser( argc, argv )
            .options( options() )
            .positional( positional_options() )
            .run(),
            VariablesMap   );

    return VariablesMap;
}


auto validated_options( int argc, char* argv[] )
{
    auto OptionsMap = read_options( argc, argv );

    using optional_options_t = boost::optional<decltype(OptionsMap)>;

    if( OptionsMap.count( "help" ) )
    {
        return optional_options_t();
    }
    else if( OptionsMap.count( "version" ) )
    {
        return optional_options_t( OptionsMap );
    }
    if( !OptionsMap.count( "input" ) )
    {
        std::cout << "Option \"input\" required but not provided" << std::endl;
        return optional_options_t();
    }
    return optional_options_t( OptionsMap );
}


template<class InputStreamT, class OutputStreamT>
void process_json_file( InputStreamT& JsonFile, OutputStreamT& CsvFile )
{
    for( std::string Line; std::getline( JsonFile, Line ); )
    {
        rapidjson::Document Json;
        Json.Parse( Line.c_str() );

        auto Body = Json["body"].GetString();
        auto Sentiment = Json["entities"].FindMember("sentiment");

        auto SentimentValue = "";
        if( !Sentiment->value.IsNull() )
        {
            SentimentValue = Sentiment->value["basic"].GetString();
        }
        CsvFile << "\"" << Body << "\",\"" << SentimentValue << "\"\n";
    }
}


int generate_csv( const std::string& InputPath, const std::string& OutputPath )
{
    auto JsonFile = std::ifstream( InputPath );
    if( !JsonFile )
    {
        std::cout << "Error: Could not open input file [" << InputPath << "]" << std::endl;
        return -1;
    }

    auto CsvFile = std::ofstream( OutputPath );
    if( !CsvFile )
    {
        std::cout << "Error: Could not open output file [" << OutputPath << "]" << std::endl;
        return -1;
    }

    std::cout << "Writing CSV file to: " << OutputPath << " ..." << std::flush;
    process_json_file( JsonFile, CsvFile );
    std::cout << " Done." <<std::endl;

    return 0;
}


int main( int argc, char* argv[] )
{
    std::cout << "json_to_csv: ver." << json_to_csv::build::identity::product_version() << std::endl;

    auto Options = validated_options( argc, argv );
    if( !Options )
    {
        std::cout << options() << std::endl;
        return 1;
    }
    else if( Options->count( "version" ) )
    {
        std::cout << json_to_csv::build::identity::report() << std::endl;
        return 0;
    }

    auto& OptionMap = *Options;

    auto InputPath = OptionMap["input"].as<std::string>();

    auto OutputPath = InputPath + ".csv";
    if( OptionMap.count( "output") )
    {
        OutputPath = OptionMap["output"].as<std::string>();
    }

    return generate_csv( InputPath, OutputPath );
}
