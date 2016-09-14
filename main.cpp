#include <array>
#include <boost/dynamic_bitset.hpp>

#include "murmur_hash3.hpp"

std::array<uint64_t, 2> hash(const uint8_t *data, std::size_t len) {
    std::array<uint64_t, 2> hashValue;
    MurmurHash3_x64_128(data, len, 0, hashValue.data());
    return hashValue;
}

inline uint64_t nthHash(uint8_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) {
    return (hashA + n * hashB) % filterSize;
}

//template<class Key, class Hash = std::hash<Key> >
class bloom_filter{
public:
    bloom_filter(uint64_t size = 1024, uint8_t num = 1):num_hashes(num),db(boost::dynamic_bitset<>(1024)){}

    void add(const uint8_t *data, std::size_t len) {
        auto hashValues = hash(data, len);

        for (int n = 0; n < num_hashes; n++)
            db[nthHash(n, hashValues[0], hashValues[1], db.size())] = true;
    }

    bool possiblyContains(const uint8_t *data, std::size_t len) const {
        auto hashValues = hash(data, len);

        for (int n = 0; n < num_hashes; n++)
            if (!db[nthHash(n, hashValues[0], hashValues[1], db.size())])
                return false;

        return true;
    }

private:
    uint8_t num_hashes;
    boost::dynamic_bitset<> db;
};

int main(){


    bloom_filter bf;

    

}