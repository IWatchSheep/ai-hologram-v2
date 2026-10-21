// Purpose: Writes the exact one-line text of one fault
// event (plan 3.3).
// Introduced in: Stage 4b (rebuild)
// Architecture: This is the one text form the tests compare.
// Cells are written with positionText and directions with
// actionName, the one copy of each (rule R4), and the fault
// words come from fault_event.cpp, so no spelling is copied.

#include <cstddef>
#include <string>

#include "core/text_words.h"
#include "experiment/fault_event.h"
#include "experiment/fault_events.h"
#include "world/action.h"

namespace aih {
namespace {

// The details after the "|": a cell and a direction for F1,
// the four directions for F2, an action and a rotation for
// F3, a cell or "none" for F4, and "-" for F5.
std::string detailText(const FaultEvent& event) {
    if (event.type == FaultType::F1) {
        const std::string cell =
            event.hasCell ? positionText(event.cell) : "none";
        return cell + " " + f1DirectionName(event.direction);
    }
    if (event.type == FaultType::F2) {
        std::string text = actionName(event.order[0]);
        for (std::size_t i = 1; i < event.order.size(); i = i + 1) {
            text = text + " " + actionName(event.order[i]);
        }
        return text;
    }
    if (event.type == FaultType::F3) {
        return actionName(event.action) + " " + rotationName(event.rotation);
    }
    if (event.type == FaultType::F4) {
        return event.hasCell ? positionText(event.cell) : "none";
    }
    return "-";
}

}  // namespace

std::string faultEventText(const FaultEvent& event) {
    return event.id + " step " + std::to_string(event.step) + " " +
           faultTypeName(event.type) + " target " +
           faultTargetName(event.target) + " | " + detailText(event);
}

}  // namespace aih
