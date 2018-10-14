#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <seqan/sequence.h>
#include <seqan/seq_io.h>
#include <seqan/basic.h>

#include "BBHashKmerContainer.h"
#include "cmdline.h"
#include "thread_util.h"
#include "FastaRecord.h"
#include "SharedQueue.h"


using Record = FastaRecord;
using Queue = SharedQueue<Record>;


using namespace seqan;



//Thread interface declaration
void filter(Queue&, Queue&) {}

/*Program to output the */
void output(Queue &q, char **argv){

	seqan::CharString seqFileName = "data/chrY-output.fa"; //TODO: replace with argument
	seqan::SeqFileOut seqFileOut(toCString(seqFileName));

	// while the boolean is 1 and the Queue is non-empty
	/*while(!done){
		if(!q.empty()){
			seqan::writeRecord(seqFileOut, q.front().id, q.front().seq); //TODO: add call for fastq sequences
			q.pop(); // remove the front entry from the dequeue
		}
	}*/

	return;
}


int filter_test(BBHashKmerContainer<KMerIterator<Dna5>,Dna5> &table){
    CharString seqFileName = "data/reads.fa";

    SeqFileIn seqfile;

    if( !open(seqfile, toCString(seqFileName))){
        std::cerr << "ERROR\n" << std::endl;
        return -1;

    }

    StringSet<CharString> ids;
    StringSet<Dna5String> seqs;


    try{
        readRecords(ids,seqs,seqfile);
    }catch(Exception const & e){
        std::cerr << "ERROR" << e.what() << std::endl;
        return 1;
    }

    for(unsigned i = 0; i < length(ids) ; ++i){
        std::cout << table.contains(toCString(seqs[i]));

    }
    return 0;


}

int reuse_build(int argc, char **argv){

    std::cout << "Building reference......"<< std::endl;
    ParametersBuild p_build;
    if( 0!= parse_command_line_build( argc, argv, p_build)){
        return -1;
    }
    std::cout << "number of thread "<<p_build.threads<< std::endl;
//    std::cout <<"input " << p_build.seq_filename<< std::endl;

    CharString seqFileName = "data/chrT.fa";

    SeqFileIn seqfile;

    if( !open(seqfile, toCString(seqFileName))){
        std::cerr << "ERROR\n" << std::endl;
        return -1;

    }

    StringSet<CharString> ids;
    StringSet<Dna5String> seqs;


    try{
        readRecords(ids,seqs,seqfile);
    }catch(Exception const & e){
        std::cerr << "ERROR" << e.what() << std::endl;
        return 1;
    }


    BBHashKmerContainer<KMerIterator<Dna5>,Dna5> table(1,2,length(seqs[0])/10-21,21);
    std::cout << "TEST" << std::endl;
    for(unsigned i = 0; i < length(ids) ; ++i){

        KMerIterator<Dna5> _begin = get_begin(toCString(seqs[i]),21);
        KMerIterator<Dna5> _end = get_end(toCString(seqs[i]),length(seqs[i]),21);

        std::cout << "Length : " << length(seqs[i]) << std::endl;
        table.addRange(_begin,_end);

    }

    filter_test(table);
    return 0;
    // build hash table; binary encoding
}


int reuse_filter(int argc, char **argv){
    std::cout << "Filtering sequence......"<< std::endl;
    ParametersFilter p_filter;
    parse_command_line_filter( argc, argv, p_filter);
    std::cout << "number of thread "<<p_filter.threads<< std::endl;
    std::cout <<"input " << p_filter.seq_filename_1<< std::endl;
    std::cout <<"paired? " << p_filter.paired<< std::endl;
    //TODO replace with input parameters
    unsigned int max_threads = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 1; //If return 0, set to 1
    unsigned int queue_limit = 10; //Default soft limit for queue before thread pool increase

    //Parse and validate parameters



    //Init thread pool
    Queue pending_records, output_records;
    bool done = false;
    std::vector<std::thread> thread_pool;
    auto t = thread_pool.emplace(thread_pool.end(), filter, std::ref(pending_records), std::ref(output_records));
    increment_priority(*t, -1); //Lower priority of filter workers so not to interfere with IO
    thread_pool.emplace(thread_pool.end(), output, std::ref(output_records), argv); //Pass params

    //Read in records to queue
    seqan::CharString id;
    seqan::Dna5QString seq;
    seqan::CharString qual;

    //Call sequence stream function of seqan to read from the file
    const char* seqFileName = "data/chrY.fa"; //TODO: replace with param
    seqan::SeqFileIn seqFileIn(seqFileName);

    //Push record into queue
    while (!atEnd(seqFileIn)) { // TODO: readRecord(id, seq, qual, seqStream) for fastq files
        try {
            // if(norc in args) TODO
            reverseComplement(seq);
            readRecord(id, seq, qual, seqFileIn);
            // else
            // readRecord(id, seq, qual, seqFileIn);
        } catch (std::exception const & e) {
            std::cout << "ERROR: " << e.what() << std::endl;
            return 1;
        }

        //construct a fasta/fastq object
        FastaRecord fa = FastaRecord(id, seq);

        //push to the pending queue
        pending_records.push(fa);

        //Check queue size and increase thread pool to desaturate
        if (pending_records.size() > queue_limit) {
            if (thread_pool.size() < max_threads)
                //Increase thread pool by 1
                t = thread_pool.emplace(thread_pool.end(), filter, std::ref(pending_records), std::ref(output_records));
            increment_priority(*t, -1); //Lower priority of filter workers so not to interfere with IO

            //Wait for pending records to desaturate (Non-blocking size check)
            while (pending_records.size(false) > queue_limit);
        }
    }

    //Join thread pool
    //Signal threads to exit
    pending_records.signal_done();
    output_records.signal_done();
    for (auto& thread : thread_pool) thread.join();

    //Output statistics

    return 0;
}

int main( int argc, char **argv) {
    std::cerr << "Hello, World!" << std::endl;
    //Parse and validate parameters

    if(argc == 1){
        std::cerr << "Please have \"build\" or \"filter\" as argument" << std::endl;
        print_help();
        reuse_exit(ExitSignal::IllegalArgumentError);
    }
    if(strcmp(argv[1], "build")==0) {
        reuse_build(argc-1, argv+1);
    }else if(strcmp(argv[1], "filter")==0){
        reuse_filter(argc-1, argv+1);
    }else{
        print_help();
        reuse_exit(ExitSignal::IllegalArgumentError);
    }
    return 0;
}
