// Purpose: Implements the six model specs of the gate's
// catalogue, exactly as build_model in ref_trace.py.
// Introduced in: Stage 3g (rebuild)
// Architecture: Each wrong model changes only the one named
// action and follows applyAction for everything else, which
// is what makes it a believable wrong model for a case to
// carry. The names and action counts live in one table, so
// the word check and the builder can never disagree.

#include "trace_models.h"

#include <optional>
#include <string>
#include <vector>

#include "golden_files.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// stuck a: action a never moves anything (unless invalid).
ActionModel makeStuck(Action named, Action) {
    return [named](const WorldState& state, Action action) {
        const ActionResult standard = applyAction(state, action);
        if (action != named || standard.outcome == MoveOutcome::InvalidState) {
            return standard;
        }
        return ActionResult{state, MoveOutcome::Blocked};
    };
}

// nopush a: a push by action a is blocked instead.
ActionModel makeNoPush(Action named, Action) {
    return [named](const WorldState& state, Action action) {
        const ActionResult standard = applyAction(state, action);
        if (action != named || standard.outcome != MoveOutcome::Pushed) {
            return standard;
        }
        return ActionResult{state, MoveOutcome::Blocked};
    };
}

// slip a: a plain move by a goes one cell further, if it can.
ActionModel makeSlip(Action named, Action) {
    return [named](const WorldState& state, Action action) {
        const ActionResult standard = applyAction(state, action);
        if (action != named || standard.outcome != MoveOutcome::Moved) {
            return standard;
        }
        const Position beyond = moved(standard.state.agent, action);
        if (isWall(state, beyond) || beyond == state.object) {
            return standard;
        }
        ActionResult wrong = standard;
        wrong.state.agent = beyond;
        return wrong;
    };
}

// swap a b: action a is predicted as action b.
ActionModel makeSwap(Action named, Action other) {
    return [named, other](const WorldState& state, Action action) {
        return applyAction(state, action == named ? other : action);
    };
}

// pushturn a b: a push by a sends the object in direction b.
ActionModel makePushTurn(Action named, Action other) {
    return [named, other](const WorldState& state, Action action) {
        const ActionResult standard = applyAction(state, action);
        if (action != named || standard.outcome != MoveOutcome::Pushed) {
            return standard;
        }
        const Position target = moved(state.object, other);
        if (isWall(state, target)) {
            return ActionResult{state, MoveOutcome::Blocked};
        }
        ActionResult wrong;
        wrong.state = state;
        wrong.state.agent = state.object;
        wrong.state.object = target;
        wrong.outcome = MoveOutcome::Pushed;
        return wrong;
    };
}

using ModelMaker = ActionModel (*)(Action, Action);

// One catalogue entry: its name, how many actions follow
// it in a spec, and the function that builds its model.
struct ModelEntry {
    const char* name;
    int actionCount;
    ModelMaker maker;
};

const ModelEntry kCatalogue[] = {{"stuck", 1, makeStuck},
                                 {"nopush", 1, makeNoPush},
                                 {"slip", 1, makeSlip},
                                 {"swap", 2, makeSwap},
                                 {"pushturn", 2, makePushTurn}};

// Finds a spec's catalogue entry, or nullptr when unknown.
const ModelEntry* findEntry(const std::vector<std::string>& spec) {
    if (spec.empty()) {
        return nullptr;
    }
    for (const ModelEntry& entry : kCatalogue) {
        if (spec[0] == entry.name) {
            return &entry;
        }
    }
    return nullptr;
}

}  // namespace

bool traceModelSpecOk(const std::vector<std::string>& spec) {
    if (spec.size() == 1 && spec[0] == "standard") {
        return true;
    }
    const ModelEntry* entry = findEntry(spec);
    if (entry == nullptr) {
        return false;
    }
    if (spec.size() != 1 + static_cast<std::size_t>(entry->actionCount)) {
        return false;
    }
    Action action;
    for (int k = 1; k <= entry->actionCount; k = k + 1) {
        if (!actionFromName(spec[static_cast<std::size_t>(k)], action)) {
            return false;
        }
    }
    return true;
}

std::optional<ActionModel> traceBaseModel(
    const std::vector<std::string>& spec) {
    const ModelEntry* entry = findEntry(spec);
    if (entry == nullptr) {
        return std::nullopt;
    }
    Action first = Action::North;
    Action second = Action::North;
    actionFromName(spec[1], first);
    if (entry->actionCount == 2) {
        actionFromName(spec[2], second);
    }
    return entry->maker(first, second);
}

}  // namespace aih
