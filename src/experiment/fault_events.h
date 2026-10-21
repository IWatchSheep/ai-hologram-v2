// Purpose: Declares generation of the fault-event list for
// a whole experiment and the exact text form of one fault
// event (plan 3.3).
// Introduced in: Stage 4b (rebuild)
// Architecture: This is experimenter code in src/experiment/.
// No agent file may include this header, and no agent ever
// calls these functions. The list is generated once per
// experiment and is identical for every condition.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_EVENTS_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_EVENTS_H

#include <cstdint>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "world/world_state.h"

namespace aih {

// One run of an experiment, identified by its layout seed
// and run seed. Runs are given in the order layout seed
// ascending, then run seed ascending (plan 3.3).
struct RunKey {
    std::uint64_t layoutSeed = 0;
    int runSeed = 0;
};

// The number of common-mode events for a type with this many
// events: round-half-up of count / 5 (20% of events, per
// type, Section 14), computed as (2 * count + 5) / 10 in
// integer arithmetic. Common-mode means both reasoners are
// hit at once.
int commonModeCount(int count);

// The seed of one run's own random stream. There are two
// random streams on purpose: an experiment-level stream
// fixes the balance over the whole experiment, and each run
// then has its own stream, so changing one run never changes
// another run's events.
std::uint64_t runStreamSeed(std::uint64_t masterSeed, std::uint64_t layoutSeed,
                            int runSeed);

// Generates the fault-event list once for a whole experiment.
// The list is made once for the whole experiment, not run by
// run, because the type mix, the 20% common-mode shares and
// the halves for targets and F1 directions are exact only
// over the whole experiment. layouts[r] is the TRUE layout
// of runs[r]; the two lists must have the same length, and
// an empty list is returned if they do not. The result has
// one vector of exactly 3 events per run, in run order.
std::vector<std::vector<FaultEvent>> generateFaultEvents(
    const std::vector<RunKey>& runs, const std::vector<WorldState>& layouts,
    std::uint64_t masterSeed);

// The exact one-line text of one event:
// "<id> step <s> <type> target <target> | <details>".
std::string faultEventText(const FaultEvent& event);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_EVENTS_H
