// Purpose: Defines the plain fault-event types used by the
// fault-event list (plan 3.3), and the one copy of each of
// their names.
// Introduced in: Stage 4b (rebuild)
// Architecture: This is experimenter code in src/experiment/.
// No agent file may include this header, and no agent ever
// uses these types. A fault event is one scheduled fault: at
// a chosen decision step, one fault of a known type is aimed
// at one reasoner (or both). The list is generated once per
// experiment and is identical for every condition, so any
// difference between conditions cannot come from different
// faults.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_EVENT_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_EVENT_H

#include <array>
#include <cstdint>
#include <string>

#include "world/action.h"
#include "world/position.h"

namespace aih {

// Value faults (F1, F2 and F5) corrupt one stored belief
// value once. Process faults (F3 and F4) install an ongoing
// cause in the action model or the trap analysis, so they
// affect every later use until a correction ends them.

// The five fault types of protocol Section 14 and plan 3.3.
enum class FaultType { F1, F2, F3, F4, F5 };

// Which reasoner an event is aimed at in two-reasoner
// conditions. Both means a common-mode fault: both reasoners
// are hit at once, so agreement between them cannot be
// trusted as independent agreement for that event.
enum class FaultTarget { R1, R2, Both };

// The direction of an F1 wall flip.
enum class F1Direction { FreeToBlocked, BlockedToFree };

// The direction of an F3 model rotation.
enum class Rotation { Left, Right };

// One scheduled fault event. Fields that do not apply to an
// event's type keep their default values.
struct FaultEvent {
    std::string id;
    std::uint64_t layoutSeed = 0;
    int runSeed = 0;
    int index = 0;
    int step = 0;
    FaultType type = FaultType::F1;
    FaultTarget target = FaultTarget::R1;
    bool commonMode = false;
    bool hasCell = false;
    Position cell{0, 0};
    F1Direction direction = F1Direction::FreeToBlocked;
    std::array<Action, 4> order{};
    Action action = Action::North;
    Rotation rotation = Rotation::Left;
};

// The one copy of each name (rule R4). Later sub-stages use
// these, and nothing else may spell these words.
std::string faultTypeName(FaultType type);
std::string faultTargetName(FaultTarget target);
std::string f1DirectionName(F1Direction direction);
std::string rotationName(Rotation rotation);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_EVENT_H
