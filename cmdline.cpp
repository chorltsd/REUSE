#include <iostream>
#include <thread>
#include <iomanip>

#include "cmdline.h"

CompressType operator<<(CompressType& val, const std::string& str) {
    if (str == "gzip") return val = CompressType::gzip;
    if (str == "bzip2") return val = CompressType::bzip2;
    if (str == "uncompressed") return val = CompressType::uncompressed;
    throw std::invalid_argument("Invalid compression type");
}

const std::string help_text{
R"(Reuse: Filter sequences containing kmers common to a reference dataset
Global options:
)"};

const std::string help_build{
R"(build - Generate all k-mers within a reference dataset
Usage: reuse build [options])"};

const std::string help_filter{
R"(filter - Remove matched sequences from a dataset
Usage: reuse filter [options])"};

const std::string help_options("options:");

const std::string err_required{"Required parameter missing or invalid"};
const std::string err_optional{"Invalid value"};

ParametersCommon::ParametersCommon() :
    kmer_length(21),
	threads(std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1),
	ram_limit(4000),
	zip(CompressType::uncompressed),
	verbose(false),
	help(false)
{}

ParametersCommon::ParametersCommon(bindopt::Options &options) {
    get_options(options);
}

void ParametersCommon::get_options(bindopt::Options &options) {
    options.emplace_back(new bindopt::PositionalOption<decltype(seq_filename)>(seq_filename, "sequence file", '\0', true, "Input sequence file path", err_required));
    options.emplace_back(new bindopt::PositionalOption<decltype(output_filename)>(output_filename, "output file", '\0', true, "Output file path", err_required));
    options.emplace_back(new bindopt::BoundOption<decltype(log_filename)>(log_filename, "log", 'l', false, "Output log to file", err_optional));
    options.emplace_back(new bindopt::BoundOption<decltype(kmer_length)>(kmer_length, "kmer_length", 'k', false, "k-mer length [default: " + std::to_string(kmer_length) + "]", err_optional));
    options.emplace_back(new bindopt::BoundOption<decltype(threads)>(threads, "threads", 't', false, "Threads to use [default: " + std::to_string(threads) + "]", err_optional));
    options.emplace_back(new bindopt::BoundOption<decltype(ram_limit)>(ram_limit, "ram", 'r', false, "Maximum RAM usage (megabytes) [default: " + std::to_string(ram_limit) + "]", err_optional));
    options.emplace_back(new bindopt::FlagOption(verbose, "verbose", 'v', false, "Print activity information", err_optional));
    options.emplace_back(new bindopt::FlagOption(help, "help", 'h', false, "Print help", err_optional));
}

ParametersBuild::ParametersBuild() :
    ParametersCommon()
{}

ParametersBuild::ParametersBuild(bindopt::Options &options) : ParametersCommon(options) {
    get_options(options);
}

void ParametersBuild::get_options(bindopt::Options &options) {
   options.emplace_back(new bindopt::BoundOption<decltype(zip)>(zip, "compression", 'g', false, "Output compression to use (gzip, bzip, uncompressed) [default: uncompressed]" , err_optional));
   options.emplace_back(new bindopt::BoundOption<decltype(mask)>(mask, "mask", 'm', false, "Mask k-mers found in this fasta file from the reference database\n\tThis option is used to minimize false positive filtering of related species or species of interest", err_optional));
}

ParametersFilter::ParametersFilter() :
	ParametersCommon()
{}

ParametersFilter::ParametersFilter(bindopt::Options &options) : ParametersCommon(options) {
    get_options(options);
}

void ParametersFilter::get_options(bindopt::Options &options) {
    //Find output_filename option and place mate and index before it
    for (auto it = options.begin(); it != options.end(); ++it) {
        try {
            if (&dynamic_cast<bindopt::BoundOption<decltype(output_filename)>&>(**it).value == &output_filename) {
                it = options.emplace(it, new bindopt::PositionalOption<decltype(seq_mate_filename)>(seq_mate_filename, "sequence mate file", '\0', false, "Path to sequence mate file", err_optional));
                options.emplace(std::next(it), new bindopt::PositionalOption<decltype(index_filename)>(index_filename, "index file", '\0', true, "Path to index file generated by reuse build", err_required));
                break;
            }
        } catch (std::bad_cast& e) {}
    }
    options.emplace_back(new bindopt::BoundOption<decltype(min_kmer_threshhold)>(min_kmer_threshhold, "min_count", 'c', false, "Minimum number of kmer matches in sequence to filter [default: " + std::to_string(min_kmer_threshhold) + "]", err_optional));
}

void print_help(){
    //Compile positionals into single string for each mode
    std::string build_positional;
    std::string filter_positional;
    std::string* current;
    for (unsigned short i = 0;;++i)
    {
        bindopt::Options options;
        switch (i) { //Is this still an antipattern in this specific use case?
            case 0:
                ParametersBuild{options};
                current = &build_positional;
                break;
            case 1:
                ParametersFilter{options};
                current = &filter_positional;
                break;
            default:
                goto BREAK2;
        }
        for (auto &option : options) {
            if (option->is_positional()) {
                if (option->required) *current += " <" + option->name + ">";
                else *current += " [" + option->name + "]";
            }
        }
    }
    BREAK2:
    //Output help information and non-positional arguments
    for (unsigned short i = 0;;++i)
    {
        bindopt::Options options;
        switch (i) {
            case 0:
                std::cerr << help_text;
                ParametersCommon().get_options(options);
                break;
            case 1:
                std::cerr << std::endl << help_build << build_positional << std::endl << help_options << std::endl;
                ParametersBuild().get_options(options);
                break;
            case 2:
                std::cerr << std::endl << help_filter << filter_positional << std::endl << help_options << std::endl;
                ParametersFilter().get_options(options);
                break;
            default:
                return;
        }
        for (auto &option : options) {
            if (not option->is_positional())
                std::cerr << "-" << option->short_name << "\t--" << std::left << std::setw(12) << option->name << "\t" << option->desc << std::endl;
        }
    }
}

