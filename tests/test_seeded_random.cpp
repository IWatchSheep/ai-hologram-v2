// Purpose: Tests for the seeded random generator (RN1 to
// RN8, plan Section 6.1), with the exact values of plan 3.1.
// Introduced in: Stage 4a (rebuild)
// Architecture: Every check builds its own SeededRandom, so
// no check can change the sequence another check sees. The
// same seed must give the same sequence, or an experiment
// could not be repeated.

#include <vector>

#include "experiment/seeded_random.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {
namespace {

void checkNextValues(TestCounter& counter) {
    SeededRandom seedZero(0);
    const bool rn1 = seedZero.next() == 16294208416658607535ULL &&
                     seedZero.next() == 7960286522194355700ULL &&
                     seedZero.next() == 487617019471545679ULL;
    check(counter, rn1, "RN1: seed 0 gives the three exact next values");
    SeededRandom seeded(1234567);
    const bool rn2 = seeded.next() == 6457827717110365317ULL &&
                     seeded.next() == 3203168211198807973ULL &&
                     seeded.next() == 9817491932198370423ULL;
    check(counter, rn2, "RN2: seed 1234567 gives the three exact values");
}

void checkBelowAndShuffle(TestCounter& counter) {
    SeededRandom seeded(42);
    std::vector<int> drawn;
    for (int i = 0; i < 10; i = i + 1) {
        drawn.push_back(seeded.below(10));
    }
    const std::vector<int> expected{3, 1, 8, 4, 0, 2, 5, 8, 5, 4};
    check(counter, drawn == expected, "RN3: seed 42 gives ten exact below(10)");
    SeededRandom shuffler(42);
    std::vector<int> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    shuffler.shuffle(values);
    const std::vector<int> shuffled{1, 10, 6, 9, 7, 5, 8, 3, 2, 4};
    check(counter, values == shuffled, "RN4: seed 42 shuffles 1..10 exactly");
    SeededRandom picker(7);
    const std::vector<int> picked =
        picker.pickDistinct(std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 3);
    const std::vector<int> expectedPicked{8, 1, 5};
    check(counter, picked == expectedPicked,
          "RN5: seed 7 pickDistinct(1..10, 3) gives 8, 1, 5");
}

void checkBelowOne(TestCounter& counter) {
    SeededRandom first(5);
    const int value = first.below(1);
    SeededRandom second(5);
    second.next();
    const bool passed = value == 0 && first.next() == second.next();
    check(counter, passed, "RN6: below(1) is 0 and uses one next value");
}

void checkRepeatable(TestCounter& counter) {
    SeededRandom first(99);
    SeededRandom second(99);
    bool same = true;
    for (int i = 0; i < 1000; i = i + 1) {
        if (first.next() != second.next()) {
            same = false;
        }
    }
    SeededRandom seedOne(1);
    SeededRandom seedTwo(2);
    const bool differ = seedOne.next() != seedTwo.next();
    check(counter, same && differ,
          "RN7: one seed repeats 1000 values; seeds 1 and 2 differ at once");
}

// True when 1000 draws below n (seed 2026) stay in range, and
// records which values below 7 were seen.
bool drawsInRange(int n, std::vector<bool>& seenSeven) {
    SeededRandom seeded(2026);
    bool inRange = true;
    for (int i = 0; i < 1000; i = i + 1) {
        const int value = seeded.below(n);
        if (value < 0 || value >= n) {
            inRange = false;
        }
        if (n == 7 && value >= 0 && value < 7) {
            seenSeven[static_cast<std::size_t>(value)] = true;
        }
    }
    return inRange;
}

void checkRanges(TestCounter& counter) {
    std::vector<bool> seenSeven(7, false);
    bool inRange = true;
    const std::vector<int> sizes{1, 2, 3, 7, 100, 1000};
    for (const int n : sizes) {
        if (!drawsInRange(n, seenSeven)) {
            inRange = false;
        }
    }
    bool allSeen = true;
    for (const bool seen : seenSeven) {
        if (!seen) {
            allSeen = false;
        }
    }
    check(counter, inRange && allSeen,
          "RN8: below(n) in range; every value of below(7) seen");
}

}  // namespace

void runSeededRandomTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Seeded random ---\n";
    }
    checkNextValues(counter);
    checkBelowAndShuffle(counter);
    checkBelowOne(counter);
    checkRepeatable(counter);
    checkRanges(counter);
}

}  // namespace aih
