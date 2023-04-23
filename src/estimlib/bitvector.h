#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace estimlib {
// Block must be unsigned integer
template <typename Block = size_t, typename BlockAddress = uint32_t>
    requires(std::is_unsigned_v<Block> && std::is_unsigned_v<BlockAddress>)
class BitVector {
public:
    using BlockType = Block;
    using WordType = size_t;

private:
    class BitReference {
    public:
        BitReference(Block& block, Block mask) : _block(block), _mask(mask) {}

        const BitReference& operator=(bool val) {
            if (val) {
                _block |= _mask;
            } else {
                _block &= ~_mask;
            }

            return *this;
        }

        const BitReference& operator=(const BitReference& br) {
            return this->operator=(bool(br));
        }

        operator bool() const {
            return (_block & _mask) != 0;
        }

    private:
        Block& _block;
        Block _mask;
    };

    using BitAddress = uint16_t;

    struct Index {
        BlockAddress block;
        BitAddress bit;
    };

    static constexpr BlockType Ones = ~static_cast<Block>(0);
    static constexpr size_t BitsPerBlock = sizeof(BlockType) * 8;
    static constexpr size_t BitsPerWord = sizeof(WordType) * 8;

public:
    explicit BitVector(size_t size) : _size(size), _bitBlocks(calculateNumberOfBlocks(size), 0) {}

    bool operator[](size_t bitIndex) const {
        const auto index = getIndex(bitIndex);
        const BlockType mask = static_cast<BlockType>(1) << index.bit;
        return (_bitBlocks[index.block] & mask) != 0;
    }

    BitReference operator[](::std::size_t bitIndex) {
        const auto index = getIndex(bitIndex);
        const BlockType mask = static_cast<BlockType>(1) << index.bit;
        return BitReference{_bitBlocks[index.block], mask};
    }

    WordType getValue(size_t bitIndex, size_t length) const {
        // bits indexes grow from right to left
        length = std::min(length, BitsPerWord);
        const Index lowIndex = getIndex(bitIndex);
        const Index highIndex = getIndex(bitIndex + length - 1);

        // value within one block
        if (lowIndex.block == highIndex.block) {
            const auto mask = ~(Ones << highIndex.bit + 1) & (Ones << lowIndex.bit);
            return toWord(_bitBlocks[lowIndex.block] & mask) >> lowIndex.bit;
        }

        // Multiple block case.
        // 1. Read low block.
        WordType result =
            toWord(_bitBlocks[lowIndex.block] & (Ones << lowIndex.bit)) >> lowIndex.bit;

        // Position of the next bit to write in the return value.
        size_t bitPosition = BitsPerBlock - lowIndex.bit;

        // 2. Read central blocks.
        for (size_t blockIndex = lowIndex.block + 1; blockIndex < highIndex.block; ++blockIndex) {
            result |= toWord(_bitBlocks[blockIndex]) << bitPosition;
            bitPosition += BitsPerBlock;
        }

        // 3. Read high block.
        result |= toWord(_bitBlocks[highIndex.block] & ~(Ones << highIndex.bit + 1)) << bitPosition;

        return result;
    }

    void setValue(size_t bitIndex, size_t length, WordType value) {
        length = std::min(length, BitsPerWord);
        const auto lowIndex = getIndex(bitIndex);
        const auto highIndex = getIndex(bitIndex + length - 1);

        // Value within one block.
        if (lowIndex.block == highIndex.block) {
            const auto mask = (Ones << highIndex.bit + 1) | ~(Ones << lowIndex.bit);
            auto blockValue = _bitBlocks[highIndex.block] & mask;
            blockValue |= (toBlock(value) << lowIndex.bit) & ~mask;
            _bitBlocks[highIndex.block] = blockValue;

            return;
        }

        // Multiple blocks case.
        // 1. Write low block
        {
            const auto mask = Ones << lowIndex.bit;  // 11100000
            BlockType blockValue = _bitBlocks[lowIndex.block] & ~mask;
            blockValue |= toBlock(value << lowIndex.bit);
            _bitBlocks[lowIndex.block] = blockValue;
        }

        // Position of the next bit to read from the value.
        size_t bitPosition = BitsPerBlock - lowIndex.bit;

        // 2. Write central blocks.
        for (size_t blockIndex = lowIndex.block + 1; blockIndex < highIndex.block; ++blockIndex) {
            _bitBlocks[blockIndex] = toBlock(value >> bitPosition);
            bitPosition += BitsPerBlock;
        }

        // 3. Write high block.
        {
            const auto mask = Ones << (highIndex.bit + 1);  // 11111000
            BlockType blockValue = _bitBlocks[highIndex.block] & mask;
            blockValue |= toBlock(value >> bitPosition) & ~mask;
            _bitBlocks[highIndex.block] = blockValue;
        }
    }

    void set(bool bitValue) {
        const Block value = bitValue ? Ones : toBlock(0);
        for (size_t blockIndex = 0; blockIndex < _bitBlocks.size(); ++blockIndex) {
            _bitBlocks[blockIndex] = value;
        }
    }

private:
    static inline constexpr size_t calculateNumberOfBlocks(size_t size) {
        return (size / BitsPerBlock) * BitsPerBlock + (size % BitsPerBlock == 0 ? 0 : 1);
    }

    static inline constexpr BlockType toBlock(WordType word) {
        if constexpr (std::is_same_v<BlockType, WordType>) {
            return word;
        } else {
            return static_cast<BlockType>(word);
        }
    }

    static inline constexpr WordType toWord(BlockType block) {
        if constexpr (std::is_same_v<BlockType, WordType>) {
            return block;
        } else {
            return static_cast<WordType>(block);
        }
    }

    inline constexpr Index getIndex(size_t bitIndex) const {
        return Index{static_cast<BlockAddress>(bitIndex / BitsPerBlock),
                     static_cast<BitAddress>(bitIndex % BitsPerBlock)};
    }

    size_t _size;
    std::vector<Block> _bitBlocks;
};

}  // namespace estimlib