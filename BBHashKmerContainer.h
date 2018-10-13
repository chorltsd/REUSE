#ifndef BBHashKmerContainer
#define BBHashKmerContainer

#include "AbstractKmerContainer.h"
#include "./lib/BBHash/BooPHF.h"

// from BBhash example
#include <time.h>
#include <sys/types.h>
#include <random>
#include <algorithm>

//tells bbhash to use included hash function working on u_int64_t input keys :
typedef boomphf::SingleHashFunctor<u_int64_t>  hasher_t;
typedef boomphf::mphf<  u_int64_t, hasher_t  > boophf_t;

template <typename Iter, typename strType>
class BBHashKmerContainer {
public:
	// default constructor
    BBHashKmerContainer();

    // variable constructor
    BBHashKmerContainer(int numThreads, double gammaFactor, int numElements);

    bool contains(strType&); //TODO change to seqan type
    void add(strType&); //TODO change to seqan type

    void addRange(Iter&);
};


template <typename Iter, typename strType>
BBHashKmerContainer<Iter, strType>::BBHashKmerContainer() {
	 
	std::vector<u_int64_t> input_keys;
	//
	... fill the input_keys vector
	//build the mphf  
	boophf_t * bphf = new boomphf::mphf<u_int64_t,hasher_t>(input_keys.size(),input_keys,nthreads);
	 
	//query the mphf :
	uint64_t  idx = bphf->lookup(input_keys[0]);
}

template <typename Iter, typename strType>
BBHashKmerContainer<Iter, strType>::BBHashKmerContainer(int numThreads, double gammaFactor, int numElements) {
	 
	std::vector<u_int64_t> input_keys;
	//
	... fill the input_keys vector
	//build the mphf  
	boophf_t * bphf = new boomphf::mphf<u_int64_t,hasher_t>(input_keys.size(),input_keys,nthreads);
	 
	//query the mphf :
	uint64_t  idx = bphf->lookup(input_keys[0]);
}

template <typename Iter, typename strType>
bool BBHashKmerContainer<Iter, strType>::contains(strType&) {
//TODO change to seqan type
	
}

template <typename Iter, typename strType>
void BBHashKmerContainer<Iter, strType>::add(strType&) {
	
//TODO change to seqan type
} 

template <typename Iter, typename strType>
void BBHashKmerContainer<Iter, strType>::addRange(Iter&) {

}


#endif