#include <iostream>
#include <thread>

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
Usage: reuse build [options] <reference file> <output file>
    options:
)"};

const std::string help_filter{
R"(filter - Remove matched sequences from a dataset
Usage: reuse filter [options] <k-mer table> <sequence file> [sequence pair file] <output file>
    options:
)"};

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
    options.emplace_back(new bindopt::BoundOption<decltype(seq_filename)>(seq_filename, "", '\0', true, "Input sequence file path", err_required));
    options.emplace_back(new bindopt::BoundOption<decltype(output_filename)>(output_filename, "", '\0', true, "Output file path", err_required));
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
   options.emplace_back(new bindopt::BoundOption<decltype(mask)>(mask, "mask", 'm', false, "Mask k-mers found in this fasta file from the reference database. This option is used to minimize false positive filtering of related species or species of interest", err_optional));
}

ParametersFilter::ParametersFilter() :
	ParametersCommon()
{}

ParametersFilter::ParametersFilter(bindopt::Options &options) : ParametersCommon(options) {
    get_options(options);
}

void ParametersFilter::get_options(bindopt::Options &options) {
   options.emplace_back(new bindopt::BoundOption<decltype(seq_mate_filename)>(seq_mate_filename, "mate", 'm', false, "Path to sequence mate file", err_optional));
   options.emplace_back(new bindopt::BoundOption<decltype(index_filename)>(index_filename, "index", 'x', true, "Path to index file generated by reuse build", err_required));
   options.emplace_back(new bindopt::BoundOption<decltype(min_kmer_threshhold)>(min_kmer_threshhold, "min_count", 'c', false, "Minimum number of kmer matches in sequence to filter [default: " + std::to_string(min_kmer_threshhold) + "]", err_optional));
}

void print_help(){
    std::cerr << help_text;
    //for (auto& option : params.options) {
    //	std::cerr << "-" << option->short_name << "\t--" << option->name << "\t" << option->desc << std::endl;
    //}
}

