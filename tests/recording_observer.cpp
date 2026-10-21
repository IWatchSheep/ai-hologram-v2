// Purpose: Builds the recording observer the tests attach:
// every report is recorded and nothing is returned to the
// agent being watched.
// Introduced in: Stage 4e (rebuild)
// Architecture: The three report functions only append to
// this recorder's lists. The text lines use positionText,
// the one copy of the cell text (rule R4), exactly as the
// local recording observer of the dead-square observer
// tests spelled them before this file existed.

#include "recording_observer.h"

#include <string>

#include "core/text_words.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {

ComputationObserver RecordingObserver::observer() {
    ComputationObserver observer;
    observer.onEvaluation = [this](const WorldState&, Action,
                                   const ActionModel*, const Prediction&) {
        evaluations = evaluations + 1;
    };
    observer.onDeadRead = [this](Position cell, bool value) {
        cells.push_back(cell);
        values.push_back(value);
        std::string text = "dead " + positionText(cell);
        if (value) {
            text = text + " true";
        }
        if (!value) {
            text = text + " false";
        }
        events.push_back(text);
    };
    observer.onHeuristic = [this](const WorldState& state, int value) {
        heuristics.push_back(value);
        const std::string text = "heuristic " + positionText(state.agent) +
                                 " " + positionText(state.object) + " " +
                                 std::to_string(value);
        events.push_back(text);
    };
    return observer;
}

}  // namespace aih
