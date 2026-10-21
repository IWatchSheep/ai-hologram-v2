// Purpose: Tests for the model catalogue (MC1 to MC5), the
// trace text of the demo world (TW1), and the first part of
// the M1.4 equivalence gate itself (G4, G5, plan 6.7).
// Introduced in: Stage 3g (rebuild)
// Architecture: The gate replays all 789 golden cases through
// the rebuilt agents and compares every traced line. It runs
// in every test run (R20), and it fails on the first
// difference, printing the case, the step and the field.

#include <string>
#include <vector>

#include "agent_test_support.h"
#include "golden_files.h"
#include "test_suites.h"
#include "test_support.h"
#include "trace_blocks.h"
#include "trace_cases.h"
#include "trace_models.h"
#include "trace_text.h"
#include "trace_writer.h"
#include "world/rules.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;

// Calls one catalogue model of a spec on a state and action.
ActionResult callModel(const std::vector<std::string>& spec,
                       const WorldState& state, Action action) {
    return traceBaseModel(spec).value()(state, action);
}

// One gate check (G4 or G5): every golden block, in order,
// must equal the trace the rebuilt agents write for its
// case. The first difference, or a reading problem, fails.
void checkGate(TestCounter& counter, const std::string& id, bool observers,
               const std::string& goldenName) {
    const std::string folder =
        std::string(AIH_SOURCE_DIR) + "/tools/reference/stage3/golden/";
    const Lines caseLines = readLines(folder + "c1_cases.txt");
    const Lines goldenLines = readLines(folder + goldenName);
    std::string failure;
    if (caseLines.empty() || goldenLines.empty()) {
        failure = "a golden file is missing or empty";
    }
    const TraceCaseRead cases = readTraceCases(caseLines);
    if (failure.empty() && !cases.ok) {
        failure = cases.problem;
    }
    const TraceBlockSplit blocks = splitTraceBlocks(goldenLines);
    if (failure.empty() && !blocks.ok) {
        failure = blocks.problem;
    }
    if (failure.empty() && blocks.blocks.size() != cases.cases.size()) {
        failure = "trace file has " + std::to_string(blocks.blocks.size()) +
                  " cases, the case file " + std::to_string(cases.cases.size());
    }
    for (std::size_t i = 0; failure.empty() && i < cases.cases.size();
         i = i + 1) {
        const TraceCase& traceCase = cases.cases[i];
        const TraceBlock& block = blocks.blocks[i];
        if (block.name != traceCase.name) {
            failure =
                "trace " + block.name + ": expected case " + traceCase.name;
            break;
        }
        const Lines actual =
            observers ? writeObserverTrace(traceCase) : writeTrace(traceCase);
        failure = firstTraceDifference(traceCase.name, block.lines, actual);
    }
    std::string description = id + ": all 789 golden blocks are identical";
    if (!failure.empty()) {
        description += " (" + failure + ")";
    }
    check(counter, failure.empty(), description);
}

// The catalogue checks MC1 to MC3 (the blocking models).
void checkBlockingModels(TestCounter& counter, const WorldState& demo,
                         const WorldState& fixture) {
    const ActionResult stuckEast =
        callModel({"stuck", "East"}, fixture, Action::East);
    const ActionResult stuckNorth =
        callModel({"stuck", "East"}, fixture, Action::North);
    check(counter,
          stuckEast.outcome == MoveOutcome::Blocked &&
              stuckEast.state == fixture &&
              stuckNorth.outcome == MoveOutcome::Moved,
          "MC1: stuck East blocks East and leaves North moved");
    const ActionResult noPushEast =
        callModel({"nopush", "East"}, fixture, Action::East);
    const ActionResult noPushMoved =
        callModel({"nopush", "East"}, demo, Action::East);
    check(counter,
          noPushEast.outcome == MoveOutcome::Blocked &&
              noPushMoved.outcome == MoveOutcome::Moved,
          "MC2: nopush East blocks a push and leaves a move");
    const WorldState agent42 = withAgentAt(demo, Position{4, 2});
    const ActionResult slipNorth =
        callModel({"slip", "North"}, fixture, Action::North);
    const ActionResult slipWall =
        callModel({"slip", "North"}, agent42, Action::North);
    check(counter,
          slipNorth.state.agent == Position{4, 1} &&
              slipWall.state.agent == Position{4, 1},
          "MC3: slip North goes one cell further when it can");
}

// The catalogue checks MC4 and MC5 (the turning models).
void checkTurningModels(TestCounter& counter, const WorldState& demo,
                        const WorldState& fixture) {
    const ActionResult swapEast =
        callModel({"swap", "East", "South"}, fixture, Action::East);
    const ActionResult swapWest =
        callModel({"swap", "East", "South"}, fixture, Action::West);
    check(counter,
          swapEast.state.agent == Position{4, 4} &&
              swapWest.state.agent == Position{3, 3},
          "MC4: swap East South predicts East as South");
    const WorldState turnFixture =
        withObjectAt(withAgentAt(demo, Position{4, 1}), Position{5, 1});
    const ActionResult turn =
        callModel({"pushturn", "East", "North"}, fixture, Action::East);
    const ActionResult turnBlocked =
        callModel({"pushturn", "East", "North"}, turnFixture, Action::East);
    check(counter,
          turn.state.object == Position{5, 2} &&
              turn.state.agent == Position{5, 3} &&
              turn.outcome == MoveOutcome::Pushed &&
              turnBlocked.outcome == MoveOutcome::Blocked,
          "MC5: pushturn East North turns a push, or blocks it");
}

}  // namespace

void runTraceGateTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Trace gate ---\n";
    }
    const WorldState demo = demoWorld();
    const WorldState fixture = withAgentAt(demo, Position{4, 3});
    checkBlockingModels(counter, demo, fixture);
    checkTurningModels(counter, demo, fixture);
    check(counter,
          hexWalls(demo) == "ffe0180601806018c601807ff" &&
              stateText(demo) ==
                  "agent (3, 3) object (5, 3) destination (8, 3) walls "
                  "ffe0180601806018c601807ff",
          "TW1: demo walls hex and state text are exact");
    checkGate(counter, "G4", false, "c1_traces.txt");
    checkGate(counter, "G5", true, "c1_observers.txt");
}

}  // namespace aih
