// Purpose: Declares the F3 faulty action model: which
// actions are faulted, how each is rotated, and the model
// built from that table (plan 3.4).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. No agent file may include this header.
// An F3 fault does not rewrite the world rules in the
// world; it gives one reasoner a different action model,
// so only that reasoner's predictions change.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_MODEL_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_MODEL_H

#include <array>

#include "experiment/fault_event.h"
#include "model/world_model.h"
#include "world/action.h"

namespace aih {

// Which actions are faulted (F3), and how each one is
// rotated. Indexed by the shared actionIndex: North 0,
// East 1, South 2, West 3 (D-076).
struct FaultedActions {
    std::array<bool, 4> faulted{};
    std::array<Rotation, 4> rotation{};
};

// Turns an action one step in the rotation's direction:
// left is N->W->S->E->N, right is N->E->S->W->N.
Action rotatedAction(Action action, Rotation rotation);

// Builds the faulty action model for a table of F3 faults.
// The table is taken by value and copied now, so later
// faults do not change a model already handed out. For an
// action not in the table, or a standard result that is
// not Pushed, the standard result is returned; otherwise
// the agent moves onto the object's cell and the object is
// predicted one cell further in the rotated direction,
// Blocked with the state unchanged if that cell is a wall.
ActionModel makeFaultyModel(FaultedActions table);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_MODEL_H
