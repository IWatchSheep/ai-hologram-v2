// Purpose: Implements the SplitMix64 seeded random number
// generator, exactly as plan Section 3.1.
// Introduced in: Stage 4a (rebuild)
// Architecture: Only next() changes the state. below() draws
// from next() until the value can be shared out fairly, so
// shuffle and pickDistinct (in the header) are built on the
// one sequence and nothing else in the project is random.

#include "experiment/seeded_random.h"

#include <cstdint>

namespace aih {

SeededRandom::SeededRandom(std::uint64_t seed) : state_(seed) {
}

std::uint64_t SeededRandom::next() {
    // Advance by the golden-ratio constant, then mix. Every
    // step is unsigned 64-bit arithmetic, which wraps modulo
    // 2^64 by definition, so no value can overflow or differ
    // between compilers.
    state_ = state_ + 0x9E3779B97F4A7C15ULL;
    std::uint64_t z = state_;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

int SeededRandom::below(int n) {
    // threshold is (0 - n) % n in unsigned 64-bit arithmetic:
    // 2^64 modulo n, written without a value too big to type.
    // Draws below it would make small remainders slightly
    // more likely, so they are drawn again.
    const std::uint64_t wide = static_cast<std::uint64_t>(n);
    const std::uint64_t threshold = (std::uint64_t{0} - wide) % wide;
    while (true) {
        const std::uint64_t drawn = next();
        if (drawn >= threshold) {
            return static_cast<int>(drawn % wide);
        }
    }
}

}  // namespace aih
