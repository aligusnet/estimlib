#include "bitvector.h"
#include <Catch2/catch_amalgamated.hpp>

namespace estimlib {
TEST_CASE("bits access", "bitvector") {
    BitVector<size_t> bitVector(64);

    bitVector[30] = true;
    bitVector[0] = true;

    REQUIRE(bitVector[0] == true);
    REQUIRE(bitVector[1] == false);
    REQUIRE(bitVector[29] == false);
    REQUIRE(bitVector[30] == true);
    REQUIRE(bitVector[31] == false);
}

TEST_CASE("read value within single block", "bitvector") {
    BitVector bitVector(32);

    bitVector[0] = 1;
    bitVector[1] = 0;
    bitVector[2] = 1;
    bitVector[5] = 1;

    REQUIRE(bitVector.getValue(0, 3) == 5);
    REQUIRE(bitVector.getValue(1, 2) == 2);
    REQUIRE(bitVector.getValue(2, 1) == 1);
    REQUIRE(bitVector.getValue(5, 1) == 1);
    REQUIRE(bitVector.getValue(0, 5) == 5);
    REQUIRE(bitVector.getValue(0, 6) == 37);
    REQUIRE(bitVector.getValue(0, 7) == 37);
    REQUIRE(bitVector.getValue(1, 6) == 18);
    REQUIRE(bitVector.getValue(2, 5) == 9);
    REQUIRE(bitVector.getValue(3, 4) == 4);
}

TEST_CASE("read value within across 2 blocks", "bitvector") {
    BitVector<uint8_t> bitVector(32);

    bitVector[5] = 1;

    REQUIRE(bitVector.getValue(5, 5) == 1);
    REQUIRE(bitVector.getValue(5, 7) == 1);
    REQUIRE(bitVector.getValue(4, 5) == 2);

    bitVector[9] = 1;
    REQUIRE(bitVector.getValue(5, 5) == 17);
    REQUIRE(bitVector.getValue(5, 6) == 17);
}

TEST_CASE("read value within across multiple blocks", "bitvector") {
    BitVector<uint8_t> bitVector(128);

    // bits set: 5
    bitVector[5] = 1;
    // 2**0
    REQUIRE(bitVector.getValue(5, 60) == 1);
    // 2**1
    REQUIRE(bitVector.getValue(4, 60) == 2);

    // bits set: 5, 9
    bitVector[9] = 1;
    // 2**0 + 2**4
    REQUIRE(bitVector.getValue(5, 60) == 17);

    // bits set: 5, 9, 19
    bitVector[19] = 1;
    // 2**0 + 2**4 + 2**14
    REQUIRE(bitVector.getValue(5, 60) == 16401);
    // 2**9
    REQUIRE(bitVector.getValue(10, 60) == 512);

    // bits set: 5, 9, 29
    bitVector[29] = 1;
    // 2**0 + 2**4 + 2**14 + 2**24
    REQUIRE(bitVector.getValue(5, 60) == 16793617);

    // bits set: 5, 9, 28, 29
    bitVector[28] = 1;
    // 2**0 + 2**4 + 2**14 + 2**23 + 2**24
    REQUIRE(bitVector.getValue(5, 60) == 25182225);
    // 2**1 + 2**5 + 2**15 + 2**24 + 2**25
    REQUIRE(bitVector.getValue(4, 60) == 50364450);

    // bits set: 5, 9, 28, 29, 60
    bitVector[60] = 1;
    // 2**0 + 2**4 + 2**14 + 2**23 + 2**24 + 2**55
    REQUIRE(bitVector.getValue(5, 60) == 36028797044146193);
    // 2**10
    REQUIRE(bitVector.getValue(50, 70) == 1024);
}

TEST_CASE("write value within single block", "bitvector") {
    BitVector bitVector(128);

    bitVector.setValue(2, 6, 47);
    REQUIRE(bitVector.getValue(2, 6) == 47);
    bitVector.setValue(0, 32, 0);

    bitVector.setValue(2, 5, 47);
    REQUIRE(bitVector.getValue(2, 6) == 15);
    bitVector.setValue(0, 32, 0);

    bitVector.setValue(5, 15, 4729);
    REQUIRE(bitVector.getValue(5, 15) == 4729);

    bitVector.setValue(70, 10, static_cast<size_t>(-1));
    REQUIRE(bitVector[69] == 0);
    for (size_t index = 70; index < 70 + 10; ++index) {
        REQUIRE(bitVector[index] == 1);
    }
    REQUIRE(bitVector[80] == 0);
}

TEST_CASE("write value across multiple blocks", "bitvector") {
    BitVector<uint8_t> bitVector(256);
    bitVector.setValue(5, 60, 36028797044146193);
    REQUIRE(bitVector.getValue(5, 60) == 36028797044146193);

    bitVector.setValue(100, 55, static_cast<size_t>(-1));
    REQUIRE(bitVector[99] == 0);
    for (size_t index = 100; index < 100 + 55; ++index) {
        REQUIRE(bitVector[index] == 1);
    }
    REQUIRE(bitVector[155] == 0);
}

TEST_CASE("read and write", "bitvector") {
    BitVector<uint8_t> bitVector(32);

    for (size_t value = 0; value < 1024; ++value) {
        bitVector.set(true);
        for (size_t bitIndex = 0; bitIndex < 18; ++bitIndex) {
            bitVector.setValue(bitIndex, 12, value + 8192);
            REQUIRE(bitVector.getValue(bitIndex, 12) == value);
        }
    }
}
}  // namespace estimlib