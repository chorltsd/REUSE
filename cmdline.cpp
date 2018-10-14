#include <getopt.h>

#include <iostream>

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "cmdline.h"

ParametersCommon::ParametersCommon(){
	output_folder_name = NULL;
	is_stdin =true;
	threads = 8;
	ram_limit=4000;
}

ParametersBuild::ParametersBuild(){
	ParametersCommon();
	seq_filename=NULL;
	kmer_length=21;
	mask = NULL;
}
ParametersFilter::ParametersFilter(){
	ParametersCommon();
	seq_filename_1=NULL;
	seq_filename_2=NULL;
	index_filename=NULL;
	log_filename=NULL;
	is_stdout=true;
	paired = false;
}

CompressType parse_compress_type(char *text){
	if(strcmp(text,"gzip")==0){
		return CompressType::gzip;
	}
	else if(strcmp(text,"bzip2")==0){
		return CompressType::bzip2;
	}
	else{
		std::cerr << "Illegal compression type, Use gzip or bzip2!" << std::endl;
		reuse_exit(ExitSignal::IllegalArgumentError);
	}

}
int parse_command_line_build( int argc, char** argv, ParametersBuild& params)
{
	static struct option long_options[] = 
	{
			{"in"    				, required_argument, 0, 'i'},
			{"out"    				, required_argument, 0, 'o'},
			{"threads"				, required_argument, 0, 'p'},
			{"ram_limit"			, required_argument, 0, 'r'},
			{"k-mer_length"			, required_argument, 0, 'k'},
			{"mask"					, required_argument, 0, 'm'},
			{"compress"				, no_argument,		 0, 'g'},
			{"help"   				, no_argument,       0, 'h'},
			{"version"				, no_argument,       0, 'v'},
			{0       			 	, 0	,                0,  0 }
	};

	if( argc == 1)
	{
		print_help();
		return -1;
	}
 	int o;
	int index;
	while( ( o = getopt_long( argc, argv, "i:o:p:r:k:m:ghv", long_options, &index)) != -1)
	{
		switch( o)
		{
			case 'i':
				set_str( &( params.seq_filename), optarg);
				params.is_stdin = false;
				break;
			case 'o':
				set_str( &( params.output_folder_name), optarg);
				break;
			case 'p':
				params.threads = atoi(optarg);
				break;
			case 'r':
				params.ram_limit = atoi(optarg);
				break;
			case 'k':
				params.kmer_length = atoi(optarg);
				break;
			case 'm':
				set_str( &( params.mask), optarg);
				break;
			case 'g':
				params.zip = parse_compress_type(optarg);
				break;
			case 't':
				params.threads = atoi( optarg);
			break;
			case 'h':
				print_help();

				reuse_exit(ExitSignal::Success);
			break;
			case 'v':
				std::cerr <<  "\nREUSE: Rapid Elimination of Useless Sequences." << std::endl;
				reuse_exit(ExitSignal::Success);
			break; 
		}
	}
	if (params.seq_filename == NULL){
		std::cerr << "Sequence file not specified!" << std::endl;
		reuse_exit(ExitSignal::IllegalArgumentError);
	}
}

int parse_command_line_filter( int argc, char** argv, ParametersFilter& params)
{
	static struct option long_options[] =
			{
					{"index"    			, required_argument, 0, 'x'},
					{"in1"    				, required_argument, 0, '1'},
					{"in2"    				, required_argument, 0, '2'},
					{"out"    				, required_argument, 0, 'o'},
					{"threads"				, required_argument, 0, 'p'},
					{"ram_limit"			, required_argument, 0, 'r'},
					{"min_k-mer_len"        , required_argument, 0, 'k'},
					{"compress"				, required_argument, 0, 'g'},
					{"help"   				, no_argument,       0, 'h'},
					{"version"				, no_argument,       0, 'v'},
					{"log_file"				, required_argument, 0, 'l'},
					{0       			 	, 0	,                0,  0 }
			};

	if( argc == 1)
	{
		print_help();
		return -1;
	}
	int o;
	int index;
	while( ( o = getopt_long( argc, argv, "x:1:2:o:p:r:k:ghvl:", long_options, &index)) != -1)
	{
		switch( o)
		{
			case 'x':
				set_str( &( params.index_filename), optarg);
				break;
			case '1':
				set_str( &( params.seq_filename_1), optarg);
				params.is_stdin = false;
				break;
			case '2':
				set_str( &( params.seq_filename_2), optarg);
				params.paired = true;
				break;
			case 'o':
				set_str( &( params.output_folder_name), optarg);
				params.is_stdout = false;
				break;
			case 'p':
				params.threads = atoi(optarg);
				break;
			case 'r':
				params.ram_limit = atoi(optarg);
				break;
			case 'k':
				params.min_kmer_threshhold = atoi(optarg);
				break;
			case 'g':
				params.zip = parse_compress_type(optarg);
				break;
			case 'h':
				print_help();
				reuse_exit(ExitSignal::Success);
				break;
			case 'v':
				std::cerr <<  "\nREUSE: Rapid Elimination of Useless Sequences." << std::endl;
				reuse_exit(ExitSignal::Success);
				break;
			case 'l':
				set_str( &( params.log_filename), optarg);
				break;
		}
	}
	if (params.seq_filename_1 == NULL){
		if(params.seq_filename_2 == NULL){
			params.is_stdin=true;
		}else {
			std::cout << "you need to have filename for both -1 and -2" << std::endl;
			reuse_exit(ExitSignal::IllegalArgumentError);
		}
	}
}



void print_help(){
	std::cout << "Help" << std::endl;
	std::cout << "Usage: reuse <command> [options]"<< std::endl;
    std::cout << " "<< std::endl;
    std::cout << "Command"<< std::endl;
    std::cout << "      build	Identify all k-mers within a reference dataset and store that library to disk"<< std::endl;
    std::cout << "              options: reuse build [options] "<< std::endl;
	std::cout << "              -i = reference_in. A comma-separated list of FASTA files containing the reference sequences to be aligned to. (default: read from STDIN ) "<< std::endl;
	std::cout << "              -o = Location to save index k-mer dataset to disk "<< std::endl;
	std::cout << "              -p/--threads =Number of threads used (default: 8)"<< std::endl;
	std::cout << "              -r = Maximum RAM usage in MB (default: 400)"<< std::endl;
	std::cout << "              -k = k-mer length (default: 21)"<< std::endl;
	std::cout << "              -m = Mask k-mers found in this fasta file from the reference database. This option is used to minimize false positive filtering of related species or species of interest."<< std::endl;
	std::cout << "              -g = Compress index when saving to disk. May take longer to generate the index and load when searching."<< std::endl;
	std::cout << "              -h/--help = Print usage information and quit"<< std::endl;
	std::cout << "              -v/--version = Print version information and quit"<< std::endl;
	std::cout << "      filter"<< std::endl;
	std::cout << "              options: reuse filter [options] -x <index> -1 <m1> -2 <m2>"<< std::endl;
	std::cout << "              -x <index> The basename of the index for the reference dataset. This can either be generated with reuse-build (.db.gz) or with an alternative program for k-mer counting, such as Jellyfish, KAnalyze or others. K-mer count files in Jellyfish dump format/KAnalyze default output format (two column text file for both) may be compressed with gzip."<< std::endl;
	std::cout << "              -1 <m1> Comma-separated list of files containing mate 1s (filename usually includes _1), e.g. -1 flyA_1.fq,flyB_1.fq. Sequences specified with this option must correspond file-for-file and read-for-read with those specified in . Reads may be a mix of different lengths. If - is specified, reuse will read the mate 1s from the “standard in” or “stdin” filehandle. Reads may be in FASTQ or FASTA format."<< std::endl;
	std::cout << "              -2 <m2> Comma-separated list of files containing mate 2s (filename usually includes _2), e.g. -2 flyA_2.fq,flyB_2.fq. Sequences specified with this option must correspond file-for-file and read-for-read with those specified in . Reads may be a mix of different lengths. If - is specified, resuse will read the mate 2s from the “standard in” or “stdin” filehandle. Reads may be in FASTQ or FASTA format."<< std::endl;
	std::cout << "              -o = Save reads not matching the k-mer filter to .fast(q/a) for single-end reads, or _1.fast(q/a) and _2.fast(q/a) for paired-end reads. By default, reads are output to STDOUT."<< std::endl;
	std::cout << "              -f = Save reads matching the k-mer filter to .fast(q/a) for single-end reads, or _1.fast(q/a) and _2.fast(q/a) for paired-end reads. By default, reads are discarded."<< std::endl;
	std::cout << "              -g = Compress outputted reads"<< std::endl;
	std::cout << "              -r = Maximum RAM usage (default: all available RAM)" << std::endl;
	std::cout << "              -p/--threads = Threads to use (default: available number of threads)"<< std::endl;
	std::cout << "              -l = Log file"<< std::endl;
	std::cout << "              -k = Minimum number of k-mers per read to filter it (default: 1)"<< std::endl;




}

