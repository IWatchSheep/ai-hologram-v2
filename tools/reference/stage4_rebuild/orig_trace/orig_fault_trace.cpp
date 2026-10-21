// Golden fault-trace tool for the Stage 4 rebuild (ai-hologram-v2 plan STAGE_04,
// Migration Addendum M1.4, part 2). Built ONLY against the ORIGINAL sources of
// IWatchSheep/ai-hologram at commit 89ea9b6: src/world, src/model, src/compare,
// src/agent (stage-3 plus IE-1 H1-H4) and src/experiment (the Stage 4 code).
// Reference material, not project code (tools/reference/ is exempt from R1).
//
// Usage: orig_fault_trace traces <cases>
//
// It reuses the Stage 3 tool (orig_c1_trace.cpp: the case reader, the agent
// builders and the C1 step printer) and adds, for each case:
//   - the case's run seeds ("run <layoutSeed> <runSeed>") and fault events
//     ("event <id> <layoutSeed> <runSeed> <index> <step> <type> <target>
//      <common> <details>");
//   - its own copy of the original runWithFaults loop (src/experiment/fault_run.cpp),
//     printing after every decision the Stage 3 step lines, every activation record
//     that changed, and every event score that changed;
//   - at the end, the result line, the runs.csv row, one events.csv row per
//     event and the number of evaluations the observer saw.
// It then runs the same case again with the ORIGINAL runWithFaults and stops with
// an error unless the outcome, the counts, the lines, the activation records, the
// event scores, the run score and the observed evaluations are identical.
#define main stage3_trace_main
#include "../../stage3/orig_trace/orig_c1_trace.cpp"
#undef main

#include "agent/computation_observer.h"
#include "experiment/csv_output.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/fault_run.h"
#include "experiment/scorer.h"

namespace {

struct FaultCase {
    Case base;
    std::uint64_t layoutSeed = 0;
    int runSeed = 0;
    std::vector<FaultEvent> events;
};

Action directionFrom(const std::string& name) { return actionFrom(name); }

bool parseEvent(const std::vector<std::string>& w, FaultEvent& e) {
    // event <id> <layoutSeed> <runSeed> <index> <step> <type> <target> <common> <details...>
    if (w.size() < 10) return false;
    e.id = w[1];
    e.layoutSeed = std::stoull(w[2]);
    e.runSeed = std::stoi(w[3]);
    e.index = std::stoi(w[4]);
    e.step = std::stoi(w[5]);
    const std::string& t = w[6];
    e.type = t == "F1" ? FaultType::F1 : t == "F2" ? FaultType::F2 : t == "F3" ? FaultType::F3
           : t == "F4" ? FaultType::F4 : FaultType::F5;
    e.target = w[7] == "both" ? FaultTarget::Both : w[7] == "R1" ? FaultTarget::R1 : FaultTarget::R2;
    e.commonMode = w[8] == "1";
    if (e.type == FaultType::F1 || e.type == FaultType::F4) {
        // cell X Y | cell none ; F1 then free->blocked | blocked->free
        std::size_t k = 10;
        if (w[9] != "cell") return false;
        if (w[10] == "none") {
            e.hasCell = false;
            k = 11;
        } else {
            e.hasCell = true;
            e.cell = Position{std::stoi(w[10]), std::stoi(w[11])};
            k = 12;
        }
        if (e.type == FaultType::F1) {
            if (k >= w.size()) return false;
            e.direction = w[k] == "free->blocked" ? F1Direction::FreeToBlocked : F1Direction::BlockedToFree;
        }
    } else if (e.type == FaultType::F2) {
        if (w.size() < 13) return false;
        e.order = {directionFrom(w[9]), directionFrom(w[10]), directionFrom(w[11]), directionFrom(w[12])};
    } else if (e.type == FaultType::F3) {
        if (w.size() < 11) return false;
        e.action = directionFrom(w[9]);
        e.rotation = w[10] == "left" ? Rotation::Left : Rotation::Right;
    }
    return true;
}

bool readFaultCases(const char* path, std::vector<FaultCase>& cases) {
    std::vector<Case> bases;
    if (!readCases(path, bases)) return false;
    std::ifstream in(path);
    std::string line;
    std::size_t index = 0;
    FaultCase current;
    while (std::getline(in, line)) {
        const std::vector<std::string> w = words(line);
        if (w.empty()) continue;
        if (w[0] == "case") {
            current = FaultCase{};
        } else if (w[0] == "run") {
            current.layoutSeed = std::stoull(w[1]);
            current.runSeed = std::stoi(w[2]);
        } else if (w[0] == "event") {
            FaultEvent e;
            if (!parseEvent(w, e)) {
                std::cerr << "bad event line: " << line << "\n";
                return false;
            }
            current.events.push_back(e);
        } else if (w[0] == "end") {
            if (index >= bases.size()) return false;
            current.base = bases[index++];
            cases.push_back(current);
        }
    }
    return index == bases.size();
}

std::string statusText(ActivationStatus s) {
    switch (s) {
        case ActivationStatus::Scheduled: return "scheduled";
        case ActivationStatus::Applied: return "applied";
        case ActivationStatus::NotApplicable: return "not-applicable";
        case ActivationStatus::Pending: return "pending";
    }
    return "?";
}

std::string activationText(const ActivationRecord& r) {
    std::string out = "activation " + r.eventId + " " + statusText(r.status);
    if (r.status == ActivationStatus::Applied) out += " at " + std::to_string(r.applicationStep);
    if (!r.reason.empty()) out += " | " + r.reason;
    return out;
}

std::string optText(const std::optional<int>& v) { return v.has_value() ? std::to_string(*v) : "-"; }

std::string endText(EndState s) {
    if (s == EndState::Corrected) return "corrected";
    if (s == EndState::Overwritten) return "overwritten";
    return "none";
}

std::string scoreText(const std::string& id, const EventScore& s) {
    std::string out = "score " + id + " activated " + yesNo(s.activated);
    out += " at " + std::to_string(s.activationStep) + " active " + yesNo(s.active);
    out += " end " + endText(s.endState) + " " + std::to_string(s.endStep);
    out += " by " + std::to_string(s.correctingId);
    out += " opportunity " + optText(s.opportunityStep) + " revealing " + yesNo(s.revealing);
    out += " mismatch " + optText(s.mismatchStep) + " strict " + optText(s.strictStep);
    return out;
}

// Prints every activation record and event score whose text changed.
struct FaultPrinter {
    std::vector<std::string> activationsShown;
    std::vector<std::string> scoresShown;

    void changes(std::ostream& out, const FaultCase& c, const FaultInjector& injector, const Scorer& scorer) {
        const std::vector<ActivationRecord>& records = injector.records();
        activationsShown.resize(records.size());
        for (std::size_t i = 0; i < records.size(); ++i) {
            const std::string text = activationText(records[i]);
            if (text != activationsShown[i]) {
                out << text << "\n";
                activationsShown[i] = text;
            }
        }
        const std::vector<EventScore>& scores = scorer.events();
        scoresShown.resize(scores.size());
        for (std::size_t i = 0; i < scores.size(); ++i) {
            const std::string text = scoreText(c.events[i].id, scores[i]);
            if (text != scoresShown[i]) {
                out << text << "\n";
                scoresShown[i] = text;
            }
        }
    }
};

void attachScorer(Scorer& scorer, ComputationObserver& observer, const ScoredView& view, StepPredictor& predictor,
                  DeadSquareCache& cache) {
    scorer.attachView(view);
    observer.onEvaluation = [&scorer](const WorldState& s, Action a, const ActionModel* v, const Prediction& p) {
        scorer.onEvaluation(s, a, v, p);
    };
    observer.onDeadRead = [&scorer](Position cell, bool value) { scorer.onDeadRead(cell, value); };
    observer.onHeuristic = [&scorer](const WorldState& s, int value) { scorer.onHeuristic(s, value); };
    predictor.setObserver(&observer);
    cache.observer = &observer;
}

template <typename Agent>
void detach(Agent& agent) {
    agent.state.activationHook = std::function<void(int)>{};
    agent.state.predictor.setObserver(nullptr);
    agent.state.deadCache.observer = nullptr;
    agent.state.deadCache.map.observer = nullptr;
}

bool sameActivations(const std::vector<ActivationRecord>& a, const std::vector<ActivationRecord>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (activationText(a[i]) != activationText(b[i])) return false;
    }
    return true;
}

bool sameScores(const FaultCase& c, const std::vector<EventScore>& a, const std::vector<EventScore>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (scoreText(c.events[i].id, a[i]) != scoreText(c.events[i].id, b[i])) return false;
        if (a[i].correctionStatus != b[i].correctionStatus) return false;
    }
    return true;
}

bool sameRunScore(const RunScore& a, const RunScore& b) {
    return a.claims == b.claims && a.falseAlarms == b.falseAlarms && a.wrongAccepted == b.wrongAccepted &&
           a.justifiedSafeStops == b.justifiedSafeStops && a.mismatches == b.mismatches &&
           a.mismatchesWithoutCause == b.mismatchesWithoutCause && a.labelsCorrect == b.labelsCorrect &&
           a.labelsScored == b.labelsScored;
}

void printTail(std::ostream& out, const FaultCase& c, const FaultRunResult& r) {
    const std::string condition = c.base.agent;
    out << "csv run " << runCsvRow(condition, c.layoutSeed, c.runSeed, r) << "\n";
    for (std::size_t i = 0; i < c.events.size(); ++i) {
        out << "csv event " << eventCsvRow(condition, c.events[i], r.activations[i], r.eventScores[i]) << "\n";
    }
    out << "observed " << r.observedEvaluations << "\n";
}

bool sameResult(const FaultCase& c, const FaultRunResult& a, const FaultRunResult& b) {
    return a.run.outcome == b.run.outcome && a.run.worldSteps == b.run.worldSteps && a.run.lines == b.run.lines &&
           a.run.evaluations == b.run.evaluations && a.decisionSteps == b.decisionSteps &&
           sameActivations(a.activations, b.activations) && sameScores(c, a.eventScores, b.eventScores) &&
           sameRunScore(a.score, b.score) && a.observedEvaluations == b.observedEvaluations;
}

// Builds the result exactly as fault_run.cpp's finishRun does.
FaultRunResult finishC1(SingleAgent& agent, FaultInjector& injector, Scorer& scorer, const std::string& outcome,
                        int worldSteps) {
    detach(agent);
    scorer.finish(viewOf(agent.state));
    FaultRunResult r;
    r.run.outcome = outcome;
    r.run.worldSteps = worldSteps;
    r.run.safeStops = agent.safeStopCount();
    r.run.mismatches = agent.mismatchCount();
    r.run.corrections = agent.correctionCount();
    r.run.verified = agent.verifiedCount();
    r.run.evaluations = agent.totalEvaluations();
    r.run.lines = agent.logLines();
    r.run.lines.push_back(resultLine(outcome, worldSteps, agent.safeStopCount()));
    r.run.lines.push_back(mismatchesLine(agent.mismatchCount()));
    r.run.lines.push_back(correctionsLine(agent.correctionCount(), agent.verifiedCount()));
    r.run.lines.push_back(evaluationsLine(agent.totalEvaluations()));
    r.decisionSteps = agent.state.decisionStep;
    r.activations = injector.records();
    r.eventScores = scorer.events();
    r.score = scorer.run();
    r.observedEvaluations = scorer.observedEvaluations();
    return r;
}

FaultRunResult finishC0(NaiveAgent& agent, FaultInjector& injector, Scorer& scorer, const std::string& outcome,
                        int worldSteps) {
    detach(agent);
    scorer.finish(viewOf(agent.state));
    FaultRunResult r;
    r.run.outcome = outcome;
    r.run.worldSteps = worldSteps;
    r.run.evaluations = agent.totalEvaluations();
    r.run.lines.push_back(resultLine(outcome, worldSteps, 0));
    r.run.lines.push_back(mismatchesLine(0));
    r.run.lines.push_back(correctionsLine(0, 0));
    r.run.lines.push_back(evaluationsLine(agent.totalEvaluations()));
    r.decisionSteps = agent.state.decisionStep;
    r.activations = injector.records();
    r.eventScores = scorer.events();
    r.score = scorer.run();
    r.observedEvaluations = scorer.observedEvaluations();
    return r;
}

// The original runWithFaults loop for C1, with a print after each decision.
FaultRunResult runC1Faults(const FaultCase& fc, SingleAgent& agent, std::ostream& body) {
    const Case& c = fc.base;
    Environment environment(c.world);
    std::vector<Observation> trueHistory;
    FaultInjector injector(fc.events, &trueHistory);
    Scorer scorer(fc.events, c.world, &trueHistory);
    ComputationObserver observer;
    attachScorer(scorer, observer, viewOf(agent.state), agent.state.predictor, agent.state.deadCache);
    agent.state.activationHook = [&scorer, &injector, &agent](int step) {
        scorer.endPhaseA(viewOf(agent.state), step);
        injector.atActivationPoint(partsOf(agent.state), step);
        scorer.afterActivation(viewOf(agent.state), injector.records(), step);
    };
    C1Printer printer;
    FaultPrinter faults;
    if (c.world.object == c.world.destination) return finishC1(agent, injector, scorer, "Success", 0);
    Observation current{1, c.world, std::nullopt, std::nullopt};
    trueHistory.push_back(current);
    int worldSteps = 0;
    while (true) {
        const Decision decision = agent.decide(current);
        scorer.afterDecide(viewOf(agent.state), decision);
        printer.step(body, agent, current, decision);
        faults.changes(body, fc, injector, scorer);
        if (!decision.isAction) {
            if (agent.safeStopCount() >= c.settings.safeStopLimit) {
                return finishC1(agent, injector, scorer, "Stopped -- unresolved", worldSteps);
            }
            current = Observation{agent.state.decisionStep + 1, current.state, std::nullopt, std::nullopt};
            trueHistory.push_back(current);
            continue;
        }
        const ActionResult stepped = environment.step(decision.action);
        ++worldSteps;
        Observation next{agent.state.decisionStep + 1, environment.state(), decision.action, stepped.outcome};
        trueHistory.push_back(next);
        const bool reached = environment.state().object == environment.state().destination;
        if (reached || worldSteps >= c.settings.maxWorldSteps) {
            agent.finalCheck(next);
            scorer.endFinalCheck(viewOf(agent.state));
            printer.finalCheck(body, agent, next);
            faults.changes(body, fc, injector, scorer);
            return finishC1(agent, injector, scorer, reached ? "Success" : "Step limit", worldSteps);
        }
        current = next;
    }
}

void printC0Step(std::ostream& body, const NaiveAgent& agent, const Observation& given, const Decision& d) {
    const NaiveAgentState& s = agent.state;
    body << "step " << s.decisionStep << "\n";
    printObservation(body, given);
    body << "decision " << actionName(d.action) << "\n";
    body << "belief " << stateText(s.belief) << "\n";
    body << "plan " << s.plan.size() << " searches " << s.searches << "\n";
    body << "evaluations " << s.predictor.usedThisStep() << " total " << s.predictor.usedTotal() << "\n";
}

FaultRunResult runC0Faults(const FaultCase& fc, NaiveAgent& agent, std::ostream& body) {
    const Case& c = fc.base;
    Environment environment(c.world);
    std::vector<Observation> trueHistory;
    FaultInjector injector(fc.events, &trueHistory);
    Scorer scorer(fc.events, c.world, &trueHistory);
    ComputationObserver observer;
    attachScorer(scorer, observer, viewOf(agent.state), agent.state.predictor, agent.state.deadCache);
    agent.state.activationHook = [&scorer, &injector, &agent](int step) {
        scorer.endPhaseA(viewOf(agent.state), step);
        injector.atActivationPoint(partsOf(agent.state), step);
        scorer.afterActivation(viewOf(agent.state), injector.records(), step);
    };
    FaultPrinter faults;
    if (c.world.object == c.world.destination) return finishC0(agent, injector, scorer, "Success", 0);
    Observation current{1, c.world, std::nullopt, std::nullopt};
    trueHistory.push_back(current);
    int worldSteps = 0;
    while (true) {
        const Decision decision = agent.decide(current);
        scorer.afterDecide(viewOf(agent.state), decision);
        printC0Step(body, agent, current, decision);
        faults.changes(body, fc, injector, scorer);
        const ActionResult stepped = environment.step(decision.action);
        ++worldSteps;
        Observation next{agent.state.decisionStep + 1, environment.state(), decision.action, stepped.outcome};
        trueHistory.push_back(next);
        if (environment.state().object == environment.state().destination) {
            return finishC0(agent, injector, scorer, "Success", worldSteps);
        }
        if (worldSteps >= c.settings.maxWorldSteps) return finishC0(agent, injector, scorer, "Step limit", worldSteps);
        current = next;
    }
}

int traceFaultCase(std::ostream& out, const FaultCase& fc) {
    const Case& c = fc.base;
    if (c.hasCommitted) {
        std::cerr << c.name << ": fault cases cannot start after a seeded action\n";
        return 1;
    }
    std::ostringstream body;
    FaultRunResult traced;
    FaultRunResult again;
    int decisionSteps = 0;
    if (c.agent == "C0") {
        std::unique_ptr<NaiveAgent> agent = makeC0(c);
        traced = runC0Faults(fc, *agent, body);
        std::unique_ptr<NaiveAgent> second = makeC0(c);
        again = runWithFaults(c.world, *second, c.settings, fc.events);
        decisionSteps = agent->state.decisionStep;
        out << "case " << c.name << "\n" << body.str();
        printResult(out, traced.run.outcome, traced.run.worldSteps, decisionSteps, 0, 0, 0, 0,
                    traced.run.evaluations);
    } else {
        std::unique_ptr<SingleAgent> agent = makeC1(c);
        traced = runC1Faults(fc, *agent, body);
        std::unique_ptr<SingleAgent> second = makeC1(c);
        again = runWithFaults(c.world, *second, c.settings, fc.events);
        decisionSteps = agent->state.decisionStep;
        out << "case " << c.name << "\n" << body.str();
        printResult(out, traced.run.outcome, traced.run.worldSteps, decisionSteps, agent->safeStopCount(),
                    agent->mismatchCount(), agent->correctionCount(), agent->verifiedCount(),
                    agent->totalEvaluations());
    }
    if (!sameResult(fc, traced, again)) {
        std::cerr << c.name << ": loop differs from runWithFaults\n";
        return 1;
    }
    printTail(out, fc, traced);
    out << "end\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string mode = argc > 2 ? argv[1] : "";
    if (mode != "traces") {
        std::cerr << "usage: orig_fault_trace traces <cases>\n";
        return 1;
    }
    std::vector<FaultCase> cases;
    if (!readFaultCases(argv[2], cases)) {
        std::cerr << "cannot read the case file\n";
        return 2;
    }
    for (const FaultCase& c : cases) {
        const int status = traceFaultCase(std::cout, c);
        if (status != 0) return status;
    }
    return 0;
}
