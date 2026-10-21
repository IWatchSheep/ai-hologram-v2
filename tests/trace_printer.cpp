// Purpose: Implements the trace printer: the watch-only
// counting observer and the lines one traced step writes.
// Introduced in: Stage 3g (rebuild)
// Architecture: These are the Tally, Observer and C1Printer
// of ref_trace.py, moved here whole from trace_writer.cpp
// (plan v1.5). The printer reads the agents' public state
// only (rule R6), and a correction line is printed when
// the record is new or its text changed since it was last
// printed, so a change is traced at the step it happens.

#include "trace_printer.h"

#include <string>
#include <vector>

#include "agent/heuristic.h"
#include "trace_text.h"
#include "world/action.h"
#include "world/rules.h"

namespace aih {
namespace {

// The observer line for one tally, in the golden text.
std::string tallyLine(const Tally& tally) {
    return "observer evaluations " + std::to_string(tally.own) + " variant " +
           std::to_string(tally.variant) + " dead reads " +
           std::to_string(tally.deadReads) + " true " +
           std::to_string(tally.deadTrue) + " heuristics " +
           std::to_string(tally.heuristics) + " infinite " +
           std::to_string(tally.infinite) + " sum " + std::to_string(tally.sum);
}

// The plan kind word, in PlanKind's declaration order.
std::string planKindText(PlanKind kind) {
    const char* words[] = {"none", "goal", "fallback", "test"};
    return words[static_cast<int>(kind)];
}

}  // namespace

ComputationObserver makeObserver(Tally& tally) {
    ComputationObserver observer;
    observer.onEvaluation = [&tally](const WorldState&, Action,
                                     const ActionModel* model,
                                     const Prediction&) {
        tally.variant = tally.variant + (model != nullptr ? 1 : 0);
        tally.own = tally.own + (model != nullptr ? 0 : 1);
    };
    observer.onDeadRead = [&tally](Position, bool value) {
        tally.deadReads = tally.deadReads + 1;
        tally.deadTrue = tally.deadTrue + (value ? 1 : 0);
    };
    observer.onHeuristic = [&tally](const WorldState&, int value) {
        tally.heuristics = tally.heuristics + 1;
        if (value >= kInfiniteHeuristic) {
            tally.infinite = tally.infinite + 1;
        } else {
            tally.sum = tally.sum + value;
        }
    };
    return observer;
}

void Printer::stepHead(const std::string& head) {
    out.push_back(head);
    if (observers) {
        out.push_back(tallyLine(tally));
        tally = Tally{};
    }
}

void Printer::appendRecords(const SingleAgent& agent) {
    while (logShown < agent.state.log.size()) {
        out.push_back("log " + agent.state.log[logShown]);
        logShown = logShown + 1;
    }
    while (mismatchesShown < agent.state.mismatches.size()) {
        const std::vector<std::string> lines =
            mismatchLines(agent.state.mismatches[mismatchesShown]);
        out.insert(out.end(), lines.begin(), lines.end());
        mismatchesShown = mismatchesShown + 1;
    }
    for (std::size_t i = 0; i < agent.state.corrections.size(); i = i + 1) {
        const std::string line = correctionLine(agent.state.corrections[i]);
        if (i >= correctionsShown.size()) {
            correctionsShown.push_back("");
        }
        if (line != correctionsShown[i]) {
            out.push_back(line);
            correctionsShown[i] = line;
        }
    }
}

void Printer::appendCommon(const SingleAgent& agent) {
    const SingleAgentState& state = agent.state;
    out.push_back("belief " + stateText(state.belief));
    out.push_back("plan " + planKindText(state.planKind) + " " +
                  std::to_string(state.plan.size()));
    std::string work = "work v1 ";
    if (state.pendingV1.has_value()) {
        work += actionName(state.pendingV1.value().action);
        for (const int k : state.pendingV1.value().pendingIndices) {
            work += " " + std::to_string(k);
        }
    } else {
        work += "none";
    }
    work += " | deferred ";
    work += state.deferred.has ? "yes" : "no";
    work += " | trap ";
    work += state.waitingTrap.has
                ? std::to_string(state.waitingTrap.correctionId)
                : "none";
    out.push_back(work);
    out.push_back("evaluations " +
                  std::to_string(state.predictor.usedThisStep()) + " total " +
                  std::to_string(state.predictor.usedTotal()) + " safe stops " +
                  std::to_string(state.safeStops) + " reserve " +
                  std::to_string(reserveFor(state.corrections)));
    appendRecords(agent);
}

void Printer::c1Step(const SingleAgent& agent, const Observation& given,
                     const Decision& decision) {
    stepHead("step " + std::to_string(agent.state.decisionStep));
    if (observers) {
        return;
    }
    out.push_back(observationLine(given));
    if (decision.isAction) {
        out.push_back("decision " + actionName(decision.action));
    } else {
        out.push_back("decision stop " + decision.stopReason);
    }
    if (agent.state.committed.has) {
        const Prediction& p = agent.state.committed.prediction;
        std::string line = "committed " + actionName(p.action) + " " +
                           outcomeName(p.outcome) + " " + stateText(p.state) +
                           " tests";
        if (agent.state.committed.testedCorrectionIds.empty()) {
            line += " none";
        }
        for (const int id : agent.state.committed.testedCorrectionIds) {
            line += " " + std::to_string(id);
        }
        out.push_back(line);
    } else {
        out.push_back("committed none");
    }
    appendCommon(agent);
}

void Printer::c1Final(const SingleAgent& agent, const Observation& given) {
    stepHead("final");
    if (observers) {
        return;
    }
    out.push_back(observationLine(given));
    appendCommon(agent);
}

void Printer::c0Step(const NaiveAgent& agent, const Observation& given,
                     const Decision& decision) {
    stepHead("step " + std::to_string(agent.state.decisionStep));
    if (observers) {
        return;
    }
    out.push_back(observationLine(given));
    out.push_back("decision " + actionName(decision.action));
    out.push_back("belief " + stateText(agent.state.belief));
    out.push_back("plan " + std::to_string(agent.state.plan.size()) +
                  " searches " + std::to_string(agent.state.searches));
    out.push_back(
        "evaluations " + std::to_string(agent.state.predictor.usedThisStep()) +
        " total " + std::to_string(agent.state.predictor.usedTotal()));
}

}  // namespace aih
