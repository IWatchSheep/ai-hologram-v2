// Purpose: Implements the fault trace writer: it runs
// one fault case with runWithFaults and writes its trace.
// Introduced in: Stage 4g (rebuild)
// Architecture: This follows ref_fault_trace.trace. The
// agent is built from the case as the Stage 3 trace writer
// builds it, and C1 is seeded by applyTraceSeeds (R4), so
// both writers start their agents in exactly one way.
// The run is the real run with faults (observe true); the
// extra watchers only watch (R6), writing each step's
// Stage 3 lines through the Stage 3 trace printer (for C0
// its step lines are those six lines), then the activation
// and score lines whose text changed. At the end come the
// Stage 3 result line, the CSV rows of Stage 4f (R4), the
// observed evaluation count and "end".

#include "fault_trace_writer.h"

#include <string>
#include <vector>

#include "agent/naive_agent.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "experiment/csv_output.h"
#include "experiment/fault_injector.h"
#include "experiment/fault_run.h"
#include "experiment/scorer.h"
#include "fault_case_reader.h"
#include "fault_trace_text.h"
#include "trace_agents.h"
#include "trace_models.h"
#include "trace_printer.h"
#include "trace_text.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;

// The activation and score lines printed so far, one per
// event. A line is printed again only when its text has
// changed since it was last printed, so the trace shows
// each change at the step where it happens.
struct FaultChanges {
    std::vector<std::string> activationsShown;
    std::vector<std::string> scoresShown;

    // Appends every changed activation line, then every
    // changed score line, in event order.
    void append(Lines& out, const std::vector<FaultEvent>& events,
                const FaultInjector& injector, const Scorer& scorer) {
        const std::vector<ActivationRecord>& records = injector.records();
        activationsShown.resize(records.size());
        for (std::size_t i = 0; i < records.size(); i = i + 1) {
            const std::string text = faultActivationLine(records[i]);
            if (text != activationsShown[i]) {
                out.push_back(text);
                activationsShown[i] = text;
            }
        }
        const std::vector<EventScore>& scores = scorer.eventScores();
        scoresShown.resize(scores.size());
        for (std::size_t i = 0; i < scores.size(); i = i + 1) {
            const std::string text = faultScoreLine(events[i].id, scores[i]);
            if (text != scoresShown[i]) {
                out.push_back(text);
                scoresShown[i] = text;
            }
        }
    }
};

// Appends the lines every fault trace ends with: the
// Stage 3 result line, one CSV row for the run and one
// per event, the observed count and "end".
void appendTail(Lines& out, const FaultCase& faultCase,
                const FaultRunResult& result, int decisionSteps, int safeStops,
                int mismatches, int corrections, int verified,
                int evaluations) {
    const TraceCase& base = faultCase.base;
    out.push_back(traceResultLine(result.run.outcome, result.run.worldSteps,
                                  decisionSteps, safeStops, mismatches,
                                  corrections, verified, evaluations));
    out.push_back("csv run " + runCsvRow(base.agent, faultCase.layoutSeed,
                                         faultCase.runSeed, result));
    for (std::size_t i = 0; i < faultCase.events.size(); i = i + 1) {
        out.push_back("csv event " + eventCsvRow(base.agent,
                                                 faultCase.events[i],
                                                 result.activations[i],
                                                 result.eventScores[i]));
    }
    out.push_back("observed " + std::to_string(result.observedEvaluations));
    out.push_back("end");
}

// Runs one C1 fault case and writes its trace block.
Lines traceC1(const FaultCase& faultCase) {
    const TraceCase& base = faultCase.base;
    SingleAgent agent(makeObservation(1, base.world), base.settings,
                      base.belief, traceBaseModel(base.modelSpec),
                      base.notDead);
    applyTraceSeeds(agent, base);
    Printer printer;
    printer.out.push_back("case " + base.name);
    FaultChanges changes;
    FaultRunWatchers watchers;
    watchers.afterDecision =
        [&printer, &changes, &faultCase](
            const SingleAgent& watched, const Observation& given,
            const Decision& decision, const FaultInjector& injector,
            const Scorer& scorer) {
            printer.c1Step(watched, given, decision);
            changes.append(printer.out, faultCase.events, injector, scorer);
        };
    watchers.afterFinalCheck =
        [&printer, &changes, &faultCase](
            const SingleAgent& watched, const Observation& given,
            const FaultInjector& injector, const Scorer& scorer) {
            printer.c1Final(watched, given);
            changes.append(printer.out, faultCase.events, injector, scorer);
        };
    const FaultRunResult result = runWithFaults(
        base.world, agent, base.settings, faultCase.events, true, watchers);
    appendTail(printer.out, faultCase, result, result.decisionSteps,
               agent.state.safeStops,
               static_cast<int>(agent.state.mismatches.size()),
               static_cast<int>(agent.state.corrections.size()),
               agent.verifiedCount(), agent.totalEvaluations());
    return printer.out;
}

// Runs one C0 fault case and writes its trace block. C0
// has no final check, so there is no final watcher here.
Lines traceC0(const FaultCase& faultCase) {
    const TraceCase& base = faultCase.base;
    NaiveAgent agent(makeObservation(1, base.world), base.settings, base.belief,
                     traceBaseModel(base.modelSpec));
    Printer printer;
    printer.out.push_back("case " + base.name);
    FaultChanges changes;
    NaiveFaultRunWatchers watchers;
    watchers.afterDecision =
        [&printer, &changes, &faultCase](
            const NaiveAgent& watched, const Observation& given,
            const Decision& decision, const FaultInjector& injector,
            const Scorer& scorer) {
            printer.c0Step(watched, given, decision);
            changes.append(printer.out, faultCase.events, injector, scorer);
        };
    const FaultRunResult result = runWithFaults(
        base.world, agent, base.settings, faultCase.events, true, watchers);
    appendTail(printer.out, faultCase, result, result.decisionSteps, 0, 0, 0, 0,
               agent.totalEvaluations());
    return printer.out;
}
}  // namespace

std::vector<std::string> writeFaultTrace(const FaultCase& faultCase) {
    if (faultCase.base.agent == "C0") {
        return traceC0(faultCase);
    }
    return traceC1(faultCase);
}

}  // namespace aih
