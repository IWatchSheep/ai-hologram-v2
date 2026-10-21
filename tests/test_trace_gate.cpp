// Purpose: Tests for the model catalogue (MC1 to MC5), the
// trace text of the demo world (TW1), and the first part of
// the M1.4 equivalence gate itself (G4 to G7, plan 6.7).
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
#include "trace_gate.h"
#include "trace_models.h"
#include "trace_text.h"
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
    const std::string failure =
        runTraceGate(caseLines, goldenLines, observers, kGoldenCaseCount);
    std::string description = id + ": all 789 golden blocks are identical";
    if (!failure.empty()) {
        description += " (" + failure + ")";
    }
    check(counter, failure.empty(), description);
}

// The lines up to and including the third "end" line: the
// first three cases, or blocks, of a golden file.
Lines firstThreeBlocks(const Lines& lines) {
    Lines result;
    int ends = 0;
    for (const std::string& line : lines) {
        result.push_back(line);
        if (line == "end") {
            ends = ends + 1;
        }
        if (ends == 3) {
            break;
        }
    }
    return result;
}

// G6 and G7: three matching cases must fail only because
// 789 are required, and must pass when 3 are required.
void checkTruncatedGate(TestCounter& counter) {
    const std::string folder =
        std::string(AIH_SOURCE_DIR) + "/tools/reference/stage3/golden/";
    const Lines caseLines =
        firstThreeBlocks(readLines(folder + "c1_cases.txt"));
    const Lines goldenLines =
        firstThreeBlocks(readLines(folder + "c1_traces.txt"));
    const std::string failure =
        runTraceGate(caseLines, goldenLines, false, kGoldenCaseCount);
    check(counter, failure == "case file has 3 cases, required 789",
          "G6: three matching cases fail with the required count");
    const std::string control = runTraceGate(caseLines, goldenLines, false, 3);
    check(counter, control.empty(),
          "G7: the same three cases pass when 3 are required");
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
    checkTruncatedGate(counter);
}

}  // namespace aih
