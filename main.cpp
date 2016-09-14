#include <numeric>
#include <set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdint>
#include <bitset>

#include "timer_asm.h"

template<int Size = 1024, class Key = uint64_t, class Hash = std::hash<Key> >
class bloom_filter{
public:
    inline void insert(const Key& k) {
        uint64_t hashValues = Hash()(k);
        db[hashValues%db.size()] = true;
    }

    inline  bool may_contain(const Key& k ) const {
        uint64_t hashValues = Hash()(k);

        if (!db[hashValues%db.size()])
            return false;

        return true;
    }

private:
    std::bitset<Size> db; 
};

int main(){
    bloom_filter<> bf;

    std::vector<uint64_t> v(16384,0); // say 16384 neurons GID
    std::set<uint64_t> s; // the key=value and no duplication so no multi-set

    //shuffle the initial buffer
    std::iota(v.begin(), v.end(), 0);
    std::random_shuffle(v.begin(),v.end());

    //take only 1024, both contains but with maybe collision for the filter (here no collision)
    for(int i=0; i < 1024; ++i){
        bf.insert(v[i]);
        s.insert(v[i]);
    }

    //do something to avoid agressive optimization
    uint64_t sum(0);
    //timing
    unsigned long long int t1(0),t2(0);

    t1 = rdtsc();
    // cneuron checks everything coming from mpi_all_gather
    for(uint64_t i=0; i < v.size(); ++i){
        if(bf.may_contain(i)){ // I may exist, but not garantee
            auto search = s.find(i);
            if(search != s.end())
                sum += *search;
        }
    }
    t2 = rdtsc();

    std::cout << sum << " using bloom_filter time: " << t2 -t1 << " [cycles] " << std::endl;

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