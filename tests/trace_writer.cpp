// Purpose: Builds an agent from a trace case, runs it with
// the real run loop and writes its trace or observer lines.
// Introduced in: Stage 3g (rebuild)
// Architecture: The agent is built by its constructor, then
// seeded through the shared seeding helpers of
// agent_test_fixtures (rule R4), in the order walls, V1,
// plan, committed. The run loop's watchers hand each step
// to the trace printer, which only watches (rule R6).

#include "trace_writer.h"

#include <string>
#include <vector>

#include "agent/naive_agent.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "trace_cases.h"
#include "trace_models.h"
#include "trace_printer.h"
#include "trace_text.h"
#include "world/environment.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;

// Seeds a built C1 agent from its case, in the order of
// plan 3.14, through the shared seeding helpers (R4).
void applySeeds(SingleAgent& agent, const TraceCase& traceCase) {
    if (!traceCase.walls.empty()) {
        std::vector<WallSeed> seeds;
        for (const TraceSeedWall& wall : traceCase.walls) {
            seeds.push_back(
                WallSeed{wall.cell, wall.newBlocked, wall.oldBlocked});
        }
        seedWallCorrections(agent, seeds);
    }
    if (traceCase.v1.has_value()) {
        seedModelV1(agent, traceCase.world, traceCase.v1.value().action,
                    traceCase.v1.value().count);
    }
    if (traceCase.plan.has_value()) {
        seedPlan(agent, traceCase.plan.value());
    }
    if (traceCase.committed.has_value()) {
        seedCommitted(agent, traceCase.committed.value());
    }
}

// Runs a built C1 agent: from the start, or after its
// seeded committed action through continueRun (plan 3.14).
RunResult runC1Case(SingleAgent& agent, const TraceCase& tc,
                    const RunWatchers& watchers) {
    const bool seeded =
        tc.committed.has_value() && tc.world.object != tc.world.destination;
    if (!seeded) {
        return runSingleAgent(tc.world, agent, tc.settings, watchers);
    }
    Environment environment(tc.world);
    const ActionResult stepped = environment.step(tc.committed.value());
    Observation current = makeObservation(2, environment.state());
    current.lastAction = tc.committed.value();
    current.lastOutcome = stepped.outcome;
    return continueRun(environment, current, 1, agent, tc.settings, watchers);
}

// Runs one C1 case and writes its trace or observer block.
Lines traceC1(const TraceCase& tc, bool observers) {
    SingleAgent agent(makeObservation(1, tc.world), tc.settings, tc.belief,
                      traceBaseModel(tc.modelSpec), tc.notDead);
    applySeeds(agent, tc);
    Printer printer;
    printer.observers = observers;
    printer.out.push_back("case " + tc.name);
    ComputationObserver observer;
    if (observers) {
        observer = makeObserver(printer.tally);
        agent.state.predictor.setObserver(&observer);
        agent.state.deadCache.observer = &observer;
    }
    RunWatchers watchers;
    watchers.afterDecision = [&printer](const SingleAgent& agent,
                                        const Observation& given,
                                        const Decision& decision) {
        printer.c1Step(agent, given, decision);
    };
    watchers.afterFinalCheck = [&printer](const SingleAgent& agent,
                                          const Observation& given) {
        printer.c1Final(agent, given);
    };
    const RunResult result = runC1Case(agent, tc, watchers);
    printer.out.push_back(traceResultLine(
        result.outcome, result.worldSteps, agent.state.decisionStep,
        agent.state.safeStops, static_cast<int>(agent.state.mismatches.size()),
        static_cast<int>(agent.state.corrections.size()), agent.verifiedCount(),
        agent.totalEvaluations()));
    printer.out.push_back("end");
    return printer.out;
}

// Runs one C0 case and writes its trace or observer block.
Lines traceC0(const TraceCase& tc, bool observers) {
    NaiveAgent agent(makeObservation(1, tc.world), tc.settings, tc.belief,
                     traceBaseModel(tc.modelSpec));
    Printer printer;
    printer.observers = observers;
    printer.out.push_back("case " + tc.name);
    ComputationObserver observer;
    if (observers) {
        observer = makeObserver(printer.tally);
        agent.state.predictor.setObserver(&observer);
        agent.state.deadCache.observer = &observer;
    }
    NaiveRunWatchers watchers;
    watchers.afterDecision = [&printer](const NaiveAgent& agent,
                                        const Observation& given,
                                        const Decision& decision) {
        printer.c0Step(agent, given, decision);
    };
    const RunResult result =
        runSingleAgent(tc.world, agent, tc.settings, watchers);
    printer.out.push_back(traceResultLine(result.outcome, result.worldSteps,
                                          agent.state.decisionStep, 0, 0, 0, 0,
                                          agent.totalEvaluations()));
    printer.out.push_back("end");
    return printer.out;
}
}  // namespace

std::vector<std::string> writeTrace(const TraceCase& traceCase) {
    if (traceCase.agent == "C0") {
        return traceC0(traceCase, false);
    }
    return traceC1(traceCase, false);
}

std::vector<std::string> writeObserverTrace(const TraceCase& traceCase) {
    if (traceCase.agent == "C0") {
        return traceC0(traceCase, true);
    }
    return traceC1(traceCase, true);
}

}  // namespace aih
