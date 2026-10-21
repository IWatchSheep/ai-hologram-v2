// Purpose: Declares the project's own seeded random number
// generator (SplitMix64), so an experiment can be repeated
// exactly from its seed.
// Introduced in: Stage 4a (rebuild)
// Architecture: This is experimenter code in src/experiment/.
// The project has its own generator because the standard
// library's random distributions are not guaranteed to give
// the same values on every compiler, so a standard generator
// could give a different experiment on another machine. A
// seed is the starting number of the sequence: the same seed
// must always give the same sequence, so the same seed gives
// the same layouts and the same fault lists everywhere.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_SEEDED_RANDOM_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_SEEDED_RANDOM_H

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace aih {

// SplitMix64: one 64-bit state, advanced by a fixed odd
// constant and mixed with shifts and multiplications. All
// arithmetic is in std::uint64_t, so it wraps modulo 2^64
// exactly the same way on every machine.
class SeededRandom {
public:
    // The seed is the whole starting state.
    explicit SeededRandom(std::uint64_t seed);

    // Returns the next 64-bit value in the sequence.
    std::uint64_t next();

    // Returns a value from 0 to n - 1 (n >= 1), without bias:
    // values below a small threshold are drawn again, so every
    // remainder modulo n is equally likely. below(1) is always
    // 0 and still uses one next() value.
    int below(int n);

    // Shuffles the list in place, from the back: for i from
    // size - 1 down to 1, swap element i with a chosen element
    // from 0 to i. A template, so any element type works.
    template <typename T>
    void shuffle(std::vector<T>& values) {
        if (values.size() < 2) {
            return;
        }
        for (std::size_t i = values.size() - 1; i > 0; i = i - 1) {
            const int j = below(static_cast<int>(i + 1));
            const std::size_t chosen = static_cast<std::size_t>(j);
            std::swap(values[i], values[chosen]);
        }
    }

    // Returns k distinct elements of the list, in the order
    // they were picked: for i from 0 to k - 1, swap element i
    // with a chosen element from i onwards, then take the
    // first k. The list is taken by value, so the caller's
    // list is never changed.
    template <typename T>
    std::vector<T> pickDistinct(std::vector<T> values, int k) {
        for (int i = 0; i < k; i = i + 1) {
            const int rest = static_cast<int>(values.size()) - i;
            const int j = i + below(rest);
            const std::size_t left = static_cast<std::size_t>(i);
            const std::size_t right = static_cast<std::size_t>(j);
            std::swap(values[left], values[right]);
        }
        std::vector<T> picked;
        for (int i = 0; i < k; i = i + 1) {
            picked.push_back(values[static_cast<std::size_t>(i)]);
        }
        return picked;
    }

private:
    std::uint64_t state_;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_SEEDED_RANDOM_H
