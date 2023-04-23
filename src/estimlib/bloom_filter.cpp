#include "estimlib/bloom_filter.h"
#include "estimlib/hash.h"
#include <cmath>
#include <sstream>

namespace estimlib {
namespace {
size_t calculateFilterSize(size_t populationSize, double error) {
    // 1.0/(log(2.0)**2)
    static constexpr double factor = 2.0813689810056077;
    return static_cast<size_t>(-std::log(error) * populationSize * factor);
}

size_t calculateNumberOfHashFunctions(size_t filterSize, size_t populationSize) {
    // log(2)
    static constexpr double factor = 0.6931471805599453;
    return static_cast<size_t>(filterSize * factor / populationSize);
}
}  // namespace

void BloomFilter::writeCharacteristics(std::ostream& out, size_t populationSize, double error) {
    const auto filterSize = calculateFilterSize(populationSize, error);
    const auto numberOfHashFunctions = calculateNumberOfHashFunctions(filterSize, populationSize);

    out << "Population size: " << populationSize << ", error rate: " << error << ", "
        << "filter size (bits): " << filterSize
        << ", filter size (MB): " << (filterSize / (8 * 1024 * 1024))
        << " # hash functions: " << numberOfHashFunctions;
}

BloomFilter::BloomFilter(size_t populationSize, double error)
    : _filter(calculateFilterSize(populationSize, error)) {
    _numberOfHashFunctions = calculateNumberOfHashFunctions(_filter.size(), populationSize);
}

void BloomFilter::insertHashed(size_t hash) {
    for (size_t hf = 0; hf < _numberOfHashFunctions; ++hf) {
        size_t seed = SeedBase + hf;
        hashCombine(seed, hash);
        _filter[seed % _filter.size()] = true;
    }
}

bool BloomFilter::lookupHashed(size_t hash) const {
    for (size_t hf = 0; hf < _numberOfHashFunctions; ++hf) {
        size_t seed = SeedBase + hf;
        hashCombine(seed, hash);
        if (!_filter[seed % _filter.size()]) {
            return false;
        }
    }
    return true;
}
}  // namespace estimlib
