#pragma once
#include "estimlib/bitvector.h"
#include <iosfwd>

namespace estimlib {

class BloomFilter {
public:
    /**
     * Write out expected filetr size and number of hash functions for the given bloom filter
     * characteristics.
     */
    static void writeCharacteristics(std::ostream& out, size_t populationSize, double error);


    /**
     * populationSize is expected number of distinc elements
     * error - expected error rate (0.0, 1.0)
     */
    explicit BloomFilter(size_t populationSize, double error = 0.02);

    void insertHashed(size_t hash);
    bool lookupHashed(size_t hash) const;

    template <typename T>
    void insert(const T& value) {
        using std::hash;
        insertHashed(hash<T>{}(value));
    }

    template <typename T>
    bool lookup(const T& value) {
        using std::hash;
        return lookupHashed(hash<T>{}(value));
    }

private:
    static constexpr size_t SeedBase = 0x735a2d97;
    BitVector<> _filter;
    size_t _numberOfHashFunctions;
};

}  // namespace estimlib
