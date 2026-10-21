// Purpose: Declares a reasoner's belief about the world, kept
// separate from the true world state.
// Introduced in: Stage 2a (rebuild)
// Architecture: WorldModel predicts from a Belief instead of
// the true WorldState, so "what I think" can never be passed
// where "what is" is expected without the compiler noticing.

#ifndef AI_HOLOGRAM_SRC_MODEL_BELIEF_H
#define AI_HOLOGRAM_SRC_MODEL_BELIEF_H

#include "world/world_state.h"

namespace aih {

// A belief is a separate type that only wraps a WorldState.
// Because Belief and WorldState are different types, code
// that expects the true world cannot silently be given a
// belief, and code that predicts cannot silently be given
// the truth: the compiler stops the mix-up.
struct Belief {
    WorldState state{};
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_MODEL_BELIEF_H
