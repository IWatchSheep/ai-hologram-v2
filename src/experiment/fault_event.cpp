// Purpose: Implements the one copy of each fault-event
// name: the type, target, F1 direction and rotation words.
// Introduced in: Stage 4b (rebuild)
// Architecture: These small functions are the only place
// these words are written (rule R4), so the event text, the
// later fault code and the CSV output can never drift apart
// by spelling the same word in two places.

#include "experiment/fault_event.h"

#include <string>

namespace aih {

std::string faultTypeName(FaultType type) {
    switch (type) {
        case FaultType::F1: {
            return "F1";
        }
        case FaultType::F2: {
            return "F2";
        }
        case FaultType::F3: {
            return "F3";
        }
        case FaultType::F4: {
            return "F4";
        }
        case FaultType::F5: {
            return "F5";
        }
    }
    return "F1";
}

std::string faultTargetName(FaultTarget target) {
    switch (target) {
        case FaultTarget::R1: {
            return "R1";
        }
        case FaultTarget::R2: {
            return "R2";
        }
        case FaultTarget::Both: {
            return "both";
        }
    }
    return "R1";
}

std::string f1DirectionName(F1Direction direction) {
    if (direction == F1Direction::BlockedToFree) {
        return "blocked->free";
    }
    return "free->blocked";
}

std::string rotationName(Rotation rotation) {
    if (rotation == Rotation::Right) {
        return "right";
    }
    return "left";
}

}  // namespace aih
