// Purpose: Defines the agent tests' shared inputs and the
// seeding of a C1 state (plan 4 v1.3, plan 3.14).
// Introduced in: Stage 3e (rebuild)
// Architecture: The seeding helpers write the agent's
// public state directly, exactly as the trace writer in 3g
// will: they build records, they do not run the agent, so
// no correction is applied and no line is logged by them.

#include "agent_test_fixtures.h"

#include <vector>

#include "agent/model_v1.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/single_agent.h"
#include "agent_test_support.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

Observation makeObservation(int step, const WorldState& state) {
    Observation observation;
    observation.decisionStep = step;
    observation.state = state;
    return observation;
}

std::vector<Observation> twoObservations(const WorldState& trueBefore,
                                         Action action) {
    const ActionResult after = applyAction(trueBefore, action);
    Observation second = makeObservation(2, after.state);
    second.lastAction = action;
    second.lastOutcome = after.outcome;
    return {makeObservation(1, trueBefore), second};
}

std::vector<Observation> eastHistory() {
    std::vector<Observation> history;
    history.push_back(makeObservation(1, demoWorld()));
    for (int step = 2; step <= 4; step = step + 1) {
        const ActionResult after =
            applyAction(history.back().state, Action::East);
        Observation next = makeObservation(step, after.state);
        next.lastAction = Action::East;
        next.lastOutcome = after.outcome;
        history.push_back(next);
    }
    return history;
}

ActionResult pushNorth(const WorldState& state, Action action) {
    const ActionResult standard = applyAction(state, action);
    if (action != Action::East) {
        return standard;
    }
    if (standard.outcome != MoveOutcome::Pushed) {
        return standard;
    }
    ActionResult wrong = standard;
    wrong.state.object = moved(state.object, Action::North);
    return wrong;
}

ActionResult alwaysBlocked(const WorldState& state, Action action) {
    (void)action;
    return ActionResult{state, MoveOutcome::Blocked};
}

ActionResult alwaysMovedEast(const WorldState& state, Action action) {
    if (action != Action::East) {
        return applyAction(state, action);
    }
    ActionResult result;
    result.state = state;
    result.state.agent = moved(state.agent, Action::East);
    result.outcome = MoveOutcome::Moved;
    return result;
}

void seedWallCorrections(SingleAgent& agent,
                         const std::vector<WallSeed>& seeds) {
    int nextId = 1;
    for (const WallSeed& seed : seeds) {
        CorrectionRecord record;
        record.id = nextId;
        nextId = nextId + 1;
        record.element.kind = ElementKind::Wall;
        record.element.cell = seed.cell;
        record.newBlocked = seed.newBlocked;
        record.oldBlocked = seed.oldBlocked;
        agent.state.corrections.push_back(record);
    }
    agent.state.nextCorrectionId = static_cast<int>(seeds.size()) + 1;
}

void seedWallCorrections(SingleAgent& agent, const std::vector<Position>& cells,
                         bool newBlocked, bool oldBlocked) {
    std::vector<WallSeed> seeds;
    for (const Position& cell : cells) {
        seeds.push_back(WallSeed{cell, newBlocked, oldBlocked});
    }
    seedWallCorrections(agent, seeds);
}

void seedModelV1(SingleAgent& agent, const WorldState& world, Action action,
                 int count) {
    WorldState state = world;
    std::vector<int> indices;
    for (int index = 1; index <= count; index = index + 1) {
        const ActionResult after = applyAction(state, action);
        state = after.state;
        Observation entry = makeObservation(index + 1, state);
        entry.lastAction = action;
        entry.lastOutcome = after.outcome;
        agent.state.history.push_back(entry);
        indices.push_back(index);
    }
    ModelV1Work work;
    work.action = action;
    work.pendingIndices = indices;
    agent.state.pendingV1 = work;
}

void seedModelV1(SingleAgent& agent, int count) {
    seedModelV1(agent, demoWorld(), Action::East, count);
}

void seedPlan(SingleAgent& agent, Action action) {
    agent.state.plan = {PlanStep{action, agent.state.belief}};
    agent.state.planKind = PlanKind::Goal;
}

void seedCommitted(SingleAgent& agent, Action action) {
    const ActionResult predicted = applyAction(agent.state.belief, action);
    agent.state.decisionStep = 1;
    CommittedPrediction committed;
    committed.has = true;
    committed.prediction =
        Prediction{action, predicted.outcome, predicted.state};
    committed.beliefBefore = agent.state.belief;
    committed.decisionStep = 1;
    agent.state.committed = committed;
}

}  // namespace aih
