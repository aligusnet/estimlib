#include "estimlib/bloom_filter.h"
#include <Catch2/catch_amalgamated.hpp>

namespace estimlib {
TEST_CASE("smoke", "BloomFilter") {
    BloomFilter bf{10, 0.01};
    bf.insert(std::string("hello"));
    bf.insert(3465364534);

    REQUIRE(bf.lookup(std::string("hello")) == true);
    REQUIRE(bf.lookup(3465364534) == true);

    REQUIRE(bf.lookup(std::string("hello world")) == false);
    REQUIRE(bf.lookup(3465364533) == false);
}
}  // namespace estimlib
