#include <numeric>
#include <set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdint>
#include <boost/dynamic_bitset.hpp>

#include "timer_asm.h"

template<class Key = uint64_t, class Hash = std::hash<Key> >
class bloom_filter{
public:
    bloom_filter(uint64_t size = 1024):db(boost::dynamic_bitset<>(1024)){}

    inline void insert(const Key& k) {
        auto hashValues = Hash()(k);
        db[hashValues%db.size()] = true;
    }

    inline  bool try_contain(const Key& k ) const {
        auto hashValues = Hash()(k);

        if (!db[hashValues%db.size()])
            return false;

        return true;
    }

private:
    boost::dynamic_bitset<> db;
};

int main(){
    bloom_filter<> bf;

    std::vector<uint64_t> v(160384,0); // say 16384 neurons GID
    std::set<uint64_t> s; // the key=value and no duplication so set
    std::iota(v.begin(), v.end(), 0);

    //shuffle
    std::random_shuffle(v.begin(),v.end());
    //take only 1024
    for(int i=0; i < 1024; ++i){
        bf.insert(v[i]);
        s.insert(v[i]);
    }

    //do something to avoid agressive optimization
    auto sum(0);
    //timing
    unsigned long long int t1(0),t2(0);

    t1 = rdtsc();
    // cneuron checks everything coming from mpi_all_gather
    for(uint64_t i=0; i < v.size(); ++i){
        if(bf.try_contain(i)){ // I may exist, but not garanty
            auto search = s.find(i);
            if(search != s.end())
                sum += *search;
        }
    }
    t2 = rdtsc();

    std::cout << sum << " using prefilter time: " << t2 -t1 << " [cycles] " << std::endl;

    //reset sum
    sum = 0;
    t1 = rdtsc();
    // cneuron checks everything coming from mpi_all_gather
    for(uint64_t i=0; i < v.size(); ++i){
          auto search = s.find(i);
          if(search != s.end())
             sum += *search;
    }
    t2 = rdtsc();

    std::cout << sum << " old way time: " << t2 -t1 << " [cycles] " << std::endl;


}