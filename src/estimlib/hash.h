#pragma once

#include <functional>
#include <mx3/mx3.h>

namespace estimlib {
// Boost 1.82.0 hash_combine

inline void hashCombine(size_t& seed, size_t hash) {
    seed = mx3::mix(seed + 0x9e3779b9 + hash);
}

template <typename T>
inline void hashCombine(size_t& seed, const T& val) {
    using std::hash;
    hashCombine(seed, hash<T>{}(val));
}
}  // namespace estimlib
