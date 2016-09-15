#include <numeric>
#include <set>
#include <list>
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdint>
#include <boost/dynamic_bitset.hpp>

#include "timer_asm.h"

template<class Key = uint64_t>
class bloom_filter{
public:
    bloom_filter(uint64_t size = 16384):bs(boost::dynamic_bitset<>(size)){}


    inline void insert(const Key& k) {
        bs[k%bs.size()] = true;
    }

    inline  bool may_contain(const Key& k ) const {
        if (!bs[k%bs.size()])
            return false;

        return true;
    }

private:
    boost::dynamic_bitset<> bs; //default constructor 0
};

std::string benchmark(uint64_t size){
    bloom_filter<> bf(size);

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
    uint64_t sum1(0),sum2(0),counter1(0),counter2(0);
    //timing
    unsigned long long int t1(0),t2(0),time1(0),time2(0);

    t1 = rdtsc();
    // cneuron checks everything coming from mpi_all_gather
    for(uint64_t i=0; i < v.size(); ++i){
        if(bf.may_contain(i)){ // I may exist, but not garantee
            counter1++;
            auto search = s.find(i);
            if(search != s.end())
                sum1 += *search;
        }
    }
    t2 = rdtsc();
    time1 = t2-t1;

    t1 = rdtsc();
    // cneuron checks everything coming from mpi_all_gather
    for(uint64_t i=0; i < v.size(); ++i){
          counter2++;
          auto search = s.find(i);
          if(search != s.end())
             sum2 += *search;
    }
    t2 = rdtsc();
    time2 = t2-t1;
    std::string result =  std::to_string(size) + ","
                        + std::to_string(time2) + ","
                        + std::to_string(time1) + ","
                        + std::to_string(counter1/(double)v.size()*100) + ", "
                        + std::to_string(time2/(double)time1) + "\n";
    return result;
}


int main(){
    int size=1;
    std::string filename = std::string("benchamrk_bloom") + ".csv";
    std::list<std::string> res(1,"#size,old,bommer,collision rate,speed up\n");
    for(int i=0; i<15;++i){
        res.push_back(benchmark(size));
        size <<=1;
    }
    //start IO
    std::fstream out;
    out.open(filename,std::fstream::out);
    std::copy(res.begin(),res.end(), std::ostream_iterator<std::string>(out));
}
