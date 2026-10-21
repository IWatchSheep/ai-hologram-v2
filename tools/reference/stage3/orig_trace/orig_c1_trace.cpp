// Golden-trace tool for the Stage 3 rebuild (ai-hologram-v2 plan STAGE_03,
// Migration Addendum M1.4). Built ONLY against the ORIGINAL sources of
// IWatchSheep/ai-hologram: src/world, src/model, src/compare (kept for
// Stages 1 and 2) and src/agent at commit 7587f38 (tag stage-3), or, with
// -DWITH_OBSERVER, src/agent at commit 89ea9b6 (stage-3 plus IE-1 H1-H4).
// Reference material, not project code (tools/reference/ is exempt from R1).
//
// Usage: orig_c1_trace traces <cases>     the decision-step traces (plan 3.13)
//        orig_c1_trace observers <cases>  the IE-1 observer counts (needs WITH_OBSERVER)
//
// The tool runs each case in its own copy of the original run loop
// (runSingleAgent, src/agent/run_loop.cpp), so that it can print the agent's
// state after every decision. It then runs the same case again with the
// original runSingleAgent itself and stops with an error unless the outcome,
// the counts and every result line are identical.
#include "agent/agent_log.h"
#include "agent/heuristic.h"
#include "agent/naive_agent.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "world/environment.h"
#include "world/rules.h"
#include "world/world_text.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace ai_hologram;

namespace {

Action actionFrom(const std::string& name) {
    if (name == "North") return Action::North;
    if (name == "East") return Action::East;
    if (name == "South") return Action::South;
    return Action::West;
}

struct SeedWall {
    Position cell{0, 0};
    bool newBlocked = false;
    bool oldBlocked = false;
};

struct Case {
    std::string name;
    std::string agent;
    AgentSettings settings{};
    WorldState world{};
    bool hasBelief = false;
    WorldState belief{};
    std::vector<std::string> model;
    std::vector<Position> notDead;
    std::vector<SeedWall> walls;
    bool hasV1 = false;
    Action v1Action = Action::East;
    int v1Count = 0;
    bool hasCommitted = false;
    Action committedAction = Action::East;
    bool hasPlan = false;
    Action planAction = Action::East;
};

// ---------------- the hand-written model catalogue (plan 3.13) ----------------
ActionModel buildModel(const std::vector<std::string>& spec) {
    const std::string kind = spec.empty() ? "standard" : spec[0];
    if (kind == "standard") return ActionModel{&applyAction};
    const Action a = actionFrom(spec[1]);
    if (kind == "stuck") {
        return [a](const WorldState& s, Action act) -> ActionResult {
            if (act != a) return applyAction(s, act);
            const ActionResult standard = applyAction(s, act);
            if (standard.outcome == MoveOutcome::InvalidState) return standard;
            return {s, MoveOutcome::Blocked};
        };
    }
    if (kind == "nopush") {
        return [a](const WorldState& s, Action act) -> ActionResult {
            const ActionResult standard = applyAction(s, act);
            if (act != a || standard.outcome != MoveOutcome::Pushed) return standard;
            return {s, MoveOutcome::Blocked};
        };
    }
    if (kind == "slip") {
        return [a](const WorldState& s, Action act) -> ActionResult {
            const ActionResult standard = applyAction(s, act);
            if (act != a || standard.outcome != MoveOutcome::Moved) return standard;
            const Position beyond = moved(standard.state.agent, act);
            if (isWall(s, beyond) || beyond == s.object) return standard;
            WorldState wrong = standard.state;
            wrong.agent = beyond;
            return {wrong, MoveOutcome::Moved};
        };
    }
    if (kind == "swap") {
        const Action b = actionFrom(spec[2]);
        return [a, b](const WorldState& s, Action act) -> ActionResult {
            return applyAction(s, act == a ? b : act);
        };
    }
    // pushturn A D: a push in direction A sends the object one cell in direction D.
    const Action d = actionFrom(spec[2]);
    return [a, d](const WorldState& s, Action act) -> ActionResult {
        const ActionResult standard = applyAction(s, act);
        if (act != a || standard.outcome != MoveOutcome::Pushed) return standard;
        const Position target = moved(s.object, d);
        if (isWall(s, target)) return {s, MoveOutcome::Blocked};
        WorldState wrong = s;
        wrong.agent = s.object;
        wrong.object = target;
        return {wrong, MoveOutcome::Pushed};
    };
}

// ---------------- reading the case file ----------------
std::vector<std::string> readRows(std::istream& in) {
    std::vector<std::string> rows;
    for (int i = 0; i < 10; ++i) {
        std::string r;
        std::getline(in, r);
        rows.push_back(r);
    }
    return rows;
}

std::vector<std::string> words(const std::string& line) {
    std::istringstream ss(line);
    std::vector<std::string> out;
    std::string w;
    while (ss >> w) out.push_back(w);
    return out;
}

bool readCases(const char* path, std::vector<Case>& cases) {
    std::ifstream in(path);
    std::string line;
    Case current;
    while (std::getline(in, line)) {
        const std::vector<std::string> w = words(line);
        if (w.empty()) continue;
        if (w[0] == "case") { current = Case{}; current.name = w[1]; }
        else if (w[0] == "agent") current.agent = w[1];
        else if (w[0] == "settings") {
            current.settings = AgentSettings{std::stoi(w[1]), std::stoi(w[2]), std::stoi(w[3]), std::stoi(w[4])};
        } else if (w[0] == "world") {
            const ParseResult p = parseWorld(readRows(in));
            if (!p.world) { std::cerr << current.name << ": invalid world\n"; return false; }
            current.world = *p.world;
        } else if (w[0] == "belief") {
            if (w[1] == "given") {
                const ParseResult p = parseWorld(readRows(in));
                if (!p.world) { std::cerr << current.name << ": invalid belief\n"; return false; }
                current.hasBelief = true;
                current.belief = *p.world;
            }
        } else if (w[0] == "model") {
            current.model.assign(w.begin() + 1, w.end());
        } else if (w[0] == "notdead") {
            for (std::size_t i = 2; i + 1 < w.size(); i += 2) {
                current.notDead.push_back(Position{std::stoi(w[i]), std::stoi(w[i + 1])});
            }
        } else if (w[0] == "seed" && w[1] == "wall") {
            current.walls.push_back(SeedWall{{std::stoi(w[2]), std::stoi(w[3])}, w[4] == "blocked", w[5] == "blocked"});
        } else if (w[0] == "seed" && w[1] == "v1") {
            current.hasV1 = true; current.v1Action = actionFrom(w[2]); current.v1Count = std::stoi(w[3]);
        } else if (w[0] == "seed" && w[1] == "committed") {
            current.hasCommitted = true; current.committedAction = actionFrom(w[2]);
        } else if (w[0] == "seed" && w[1] == "plan") {
            current.hasPlan = true; current.planAction = actionFrom(w[2]);
        } else if (w[0] == "end") cases.push_back(current);
    }
    return true;
}

// ---------------- trace text (plan 3.13) ----------------
std::string hexWalls(const WorldState& s) {
    const char* digits = "0123456789abcdef";
    std::string out;
    for (int j = 0; j < 25; ++j) {
        int v = 0;
        for (int k = 0; k < 4; ++k) {
            const int i = 4 * j + k;
            v = v * 2 + (s.walls[static_cast<std::size_t>(i / 10)][static_cast<std::size_t>(i % 10)] ? 1 : 0);
        }
        out += digits[v];
    }
    return out;
}

std::string stateText(const WorldState& s) {
    return "agent " + positionText(s.agent) + " object " + positionText(s.object) + " destination " +
           positionText(s.destination) + " walls " + hexWalls(s);
}

std::string yesNo(bool b) { return b ? "yes" : "no"; }

std::string kindName(ElementKind k) {
    switch (k) {
        case ElementKind::Wall: return "wall";
        case ElementKind::Object: return "object";
        case ElementKind::ActionModel: return "model";
        case ElementKind::Trap: return "trap";
    }
    return "?";
}

std::string statusName(CorrectionStatus s) {
    switch (s) {
        case CorrectionStatus::Consistent: return "CONSISTENT";
        case CorrectionStatus::Verified: return "VERIFIED";
        case CorrectionStatus::Rejected: return "REJECTED";
    }
    return "?";
}

std::string sourceName(SourceLabel s) {
    switch (s) {
        case SourceLabel::S1: return "S1";
        case SourceLabel::S2: return "S2";
        case SourceLabel::S3: return "S3";
        case SourceLabel::S4: return "S4";
        case SourceLabel::S5: return "S5";
    }
    return "?";
}

std::string elementText(const Element& e) {
    if (e.kind == ElementKind::ActionModel) return "model " + actionName(e.action);
    if (e.kind == ElementKind::Object) return "object";
    return kindName(e.kind) + " " + positionText(e.cell);
}

std::string planKindName(PlanKind k) {
    switch (k) {
        case PlanKind::None: return "none";
        case PlanKind::Goal: return "goal";
        case PlanKind::Fallback: return "fallback";
        case PlanKind::Test: return "test";
    }
    return "?";
}

std::string bitText(bool b) { return b ? "1" : "0"; }

void printObservation(std::ostream& out, const Observation& o) {
    out << "observation " << o.decisionStep << " " << stateText(o.state);
    if (o.lastAction.has_value()) {
        out << " last " << actionName(*o.lastAction) << " " << outcomeName(o.lastOutcome.value_or(MoveOutcome::Blocked));
    } else {
        out << " last none";
    }
    out << "\n";
}

void printMismatch(std::ostream& out, const MismatchRecord& m) {
    out << "mismatch step " << m.decisionStep << " action " << actionName(m.action) << " outcomes "
        << (m.differences.outcomesDiffer ? "differ" : "same") << " suspected " << sourceName(m.suspected)
        << " anomaly " << yesNo(m.anomaly) << "\n";
    for (const FootprintCellDifference& d : m.differences.cells) {
        out << "  cell " << positionText(d.cell) << " wall " << bitText(d.predictedWall) << bitText(d.observedWall)
            << " agent " << bitText(d.predictedAgentHere) << bitText(d.observedAgentHere) << " object "
            << bitText(d.predictedObjectHere) << bitText(d.observedObjectHere) << "\n";
    }
    for (const Element& e : m.candidates) {
        out << "  candidate " << elementText(e) << "\n";
    }
}

void printCorrection(std::ostream& out, const CorrectionRecord& c) {
    out << "correction " << c.id << " " << elementText(c.element);
    if (c.element.kind == ElementKind::Wall) {
        out << " new " << (c.newBlocked ? "blocked" : "free") << " old " << (c.oldBlocked ? "blocked" : "free");
    } else if (c.element.kind == ElementKind::Object) {
        out << " new " << positionText(c.newPosition) << " old " << positionText(c.oldPosition);
    }
    out << " " << statusName(c.status) << " since " << c.stepConsistent << " tested " << yesNo(c.tested) << "\n";
}

int reserveOf(const SingleAgentState& s) {
    int reserve = 1;
    for (const CorrectionRecord& r : s.corrections) {
        if (r.status == CorrectionStatus::Consistent && r.element.kind != ElementKind::Trap) ++reserve;
    }
    return reserve;
}

struct C1Printer {
    std::size_t logShown = 0;
    std::size_t mismatchesShown = 0;
    std::vector<std::string> correctionsShown;  // the last printed line of each record

    void common(std::ostream& out, const SingleAgent& agent) {
        const SingleAgentState& s = agent.state;
        out << "belief " << stateText(s.belief) << "\n";
        out << "plan " << planKindName(s.planKind) << " " << s.plan.size() << "\n";
        out << "work v1 ";
        if (s.pendingV1.has_value()) {
            out << actionName(s.pendingV1->action);
            for (const int k : s.pendingV1->pendingIndices) out << " " << k;
        } else {
            out << "none";
        }
        out << " | deferred " << yesNo(s.deferred.has) << " | trap ";
        if (s.waitingTrap.has) out << s.waitingTrap.correctionId; else out << "none";
        out << "\n";
        out << "evaluations " << s.predictor.usedThisStep() << " total " << s.predictor.usedTotal() << " safe stops "
            << s.safeStops << " reserve " << reserveOf(s) << "\n";
        for (; logShown < s.log.size(); ++logShown) out << "log " << s.log[logShown] << "\n";
        for (; mismatchesShown < s.mismatches.size(); ++mismatchesShown) printMismatch(out, s.mismatches[mismatchesShown]);
        for (std::size_t i = 0; i < s.corrections.size(); ++i) {
            std::ostringstream line;
            printCorrection(line, s.corrections[i]);
            if (i >= correctionsShown.size()) correctionsShown.push_back("");
            if (line.str() != correctionsShown[i]) {
                out << line.str();
                correctionsShown[i] = line.str();
            }
        }
    }

    void step(std::ostream& out, const SingleAgent& agent, const Observation& given, const Decision& d) {
        const SingleAgentState& s = agent.state;
        out << "step " << s.decisionStep << "\n";
        printObservation(out, given);
        if (d.isAction) out << "decision " << actionName(d.action) << "\n";
        else out << "decision stop " << d.stopReason << "\n";
        if (s.committed.has) {
            out << "committed " << actionName(s.committed.prediction.action) << " "
                << outcomeName(s.committed.prediction.outcome) << " " << stateText(s.committed.prediction.state)
                << " tests";
            if (s.committed.testedCorrectionIds.empty()) out << " none";
            for (const int id : s.committed.testedCorrectionIds) out << " " << id;
            out << "\n";
        } else {
            out << "committed none\n";
        }
        common(out, agent);
    }

    void finalCheck(std::ostream& out, const SingleAgent& agent, const Observation& given) {
        out << "final\n";
        printObservation(out, given);
        common(out, agent);
    }
};

// ---------------- observers (IE-1 H3, H4; 89ea9b6 only) ----------------
#ifdef WITH_OBSERVER
struct Tally {
    int own = 0;
    int variant = 0;
    int deadReads = 0;
    int deadTrue = 0;
    int heuristics = 0;
    int infinite = 0;
    long long sum = 0;
};

void printTally(std::ostream& out, const Tally& t) {
    out << "observer evaluations " << t.own << " variant " << t.variant << " dead reads " << t.deadReads << " true "
        << t.deadTrue << " heuristics " << t.heuristics << " infinite " << t.infinite << " sum " << t.sum << "\n";
}

ComputationObserver makeObserver(Tally& t) {
    ComputationObserver o;
    o.onEvaluation = [&t](const WorldState&, Action, const ActionModel* m, const Prediction&) {
        if (m == nullptr) ++t.own; else ++t.variant;
    };
    o.onDeadRead = [&t](Position, bool v) { ++t.deadReads; if (v) ++t.deadTrue; };
    o.onHeuristic = [&t](const WorldState&, int v) {
        ++t.heuristics;
        if (v >= kInfiniteHeuristic) ++t.infinite; else t.sum += v;
    };
    return o;
}
#endif

// ---------------- building the agents ----------------
std::unique_ptr<SingleAgent> makeC1(const Case& c) {
    const Observation first{1, c.world, std::nullopt, std::nullopt};
    std::optional<WorldState> belief;
    if (c.hasBelief) belief = c.belief;
    std::optional<ActionModel> model;
    if (!c.model.empty() && c.model[0] != "standard") model = buildModel(c.model);
    auto agent = std::make_unique<SingleAgent>(first, c.settings, belief, model, c.notDead);
    int id = 1;
    for (const SeedWall& w : c.walls) {
        CorrectionRecord r;
        r.id = id++;
        r.element.kind = ElementKind::Wall;
        r.element.cell = w.cell;
        r.newBlocked = w.newBlocked;
        r.oldBlocked = w.oldBlocked;
        r.status = CorrectionStatus::Consistent;
        r.stepConsistent = 0;
        agent->state.corrections.push_back(r);
    }
    agent->state.nextCorrectionId = id;
    if (c.hasV1) {
        WorldState s = c.world;
        ModelV1Work work;
        work.action = c.v1Action;
        for (int i = 1; i <= c.v1Count; ++i) {
            const ActionResult r = applyAction(s, c.v1Action);
            s = r.state;
            agent->state.history.push_back(Observation{i + 1, s, c.v1Action, r.outcome});
            work.pendingIndices.push_back(i);
        }
        agent->state.pendingV1 = work;
    }
    if (c.hasPlan) {
        agent->state.plan = {PlanStep{c.planAction, agent->state.belief}};
        agent->state.planKind = PlanKind::Goal;
    }
    if (c.hasCommitted) {
        const WorldState b = agent->state.belief;
        agent->state.decisionStep = 1;
        agent->state.committed.has = true;
        agent->state.committed.prediction =
            Prediction{c.committedAction, applyAction(b, c.committedAction).outcome, applyAction(b, c.committedAction).state};
        agent->state.committed.beliefBefore = b;
        agent->state.committed.decisionStep = 1;
    }
    return agent;
}

std::unique_ptr<NaiveAgent> makeC0(const Case& c) {
    const Observation first{1, c.world, std::nullopt, std::nullopt};
    std::optional<WorldState> belief;
    if (c.hasBelief) belief = c.belief;
    std::optional<ActionModel> model;
    if (!c.model.empty() && c.model[0] != "standard") model = buildModel(c.model);
    return std::make_unique<NaiveAgent>(first, c.settings, belief, model);
}

struct Ending {
    std::string outcome;
    int worldSteps = 0;
};

void printResult(std::ostream& out, const std::string& outcome, int worldSteps, int decisionSteps, int safeStops,
                 int mismatches, int corrections, int verified, int evaluations) {
    out << "result " << outcome << " | world steps " << worldSteps << " | decision steps " << decisionSteps
        << " | safe stops " << safeStops << " | mismatches " << mismatches << " | corrections " << corrections
        << " | verified " << verified << " | evaluations " << evaluations << "\n";
}

// The original runSingleAgent loop (run_loop.cpp), with a print after each decision.
template <typename OnStep, typename OnFinal>
Ending runC1Loop(const Case& c, SingleAgent& agent, OnStep onStep, OnFinal onFinal) {
    Environment environment(c.world);
    if (c.world.object == c.world.destination) return {"Success", 0};
    Observation current{1, c.world, std::nullopt, std::nullopt};
    int worldSteps = 0;
    if (c.hasCommitted) {
        const ActionResult stepped = environment.step(c.committedAction);
        ++worldSteps;
        current = Observation{2, environment.state(), c.committedAction, stepped.outcome};
    }
    while (true) {
        const Decision decision = agent.decide(current);
        onStep(current, decision);
        if (!decision.isAction) {
            if (agent.safeStopCount() >= c.settings.safeStopLimit) return {"Stopped -- unresolved", worldSteps};
            current = Observation{agent.state.decisionStep + 1, current.state, std::nullopt, std::nullopt};
            continue;
        }
        const ActionResult stepped = environment.step(decision.action);
        ++worldSteps;
        Observation next{agent.state.decisionStep + 1, environment.state(), decision.action, stepped.outcome};
        if (environment.state().object == environment.state().destination) {
            agent.finalCheck(next);
            onFinal(next);
            return {"Success", worldSteps};
        }
        if (worldSteps >= c.settings.maxWorldSteps) {
            agent.finalCheck(next);
            onFinal(next);
            return {"Step limit", worldSteps};
        }
        current = next;
    }
}

template <typename OnStep>
Ending runC0Loop(const Case& c, NaiveAgent& agent, OnStep onStep) {
    Environment environment(c.world);
    if (c.world.object == c.world.destination) return {"Success", 0};
    Observation current{1, c.world, std::nullopt, std::nullopt};
    int worldSteps = 0;
    while (true) {
        const Decision decision = agent.decide(current);
        onStep(current, decision);
        const ActionResult stepped = environment.step(decision.action);
        ++worldSteps;
        Observation next{agent.state.decisionStep + 1, environment.state(), decision.action, stepped.outcome};
        if (environment.state().object == environment.state().destination) return {"Success", worldSteps};
        if (worldSteps >= c.settings.maxWorldSteps) return {"Step limit", worldSteps};
        current = next;
    }
}

bool crossCheckC1(const Case& c, const Ending& e, const SingleAgent& traced) {
    if (c.hasCommitted) return true;  // runSingleAgent cannot start after a seeded action
    std::unique_ptr<SingleAgent> again = makeC1(c);
    const RunResult r = runSingleAgent(c.world, *again, c.settings);
    std::vector<std::string> lines = traced.logLines();
    return r.outcome == e.outcome && r.worldSteps == e.worldSteps && r.lines.size() == lines.size() + 4 &&
           std::equal(lines.begin(), lines.end(), r.lines.begin()) && r.evaluations == traced.totalEvaluations();
}

int traceC1(std::ostream& out, const Case& c, bool observers) {
    std::unique_ptr<SingleAgent> agent = makeC1(c);
    C1Printer printer;
#ifdef WITH_OBSERVER
    Tally tally;
    const ComputationObserver observer = makeObserver(tally);
    if (observers) {
        agent->state.predictor.setObserver(&observer);
        agent->state.deadCache.observer = &observer;
    }
#endif
    std::ostringstream body;
    const Ending e = runC1Loop(
        c, *agent,
        [&](const Observation& given, const Decision& d) {
            if (!observers) { printer.step(body, *agent, given, d); return; }
#ifdef WITH_OBSERVER
            body << "step " << agent->state.decisionStep << "\n";
            printTally(body, tally);
            tally = Tally{};
#endif
            (void)given; (void)d;
        },
        [&](const Observation& given) {
            if (!observers) { printer.finalCheck(body, *agent, given); return; }
#ifdef WITH_OBSERVER
            body << "final\n";
            printTally(body, tally);
            tally = Tally{};
#endif
            (void)given;
        });
    if (!crossCheckC1(c, e, *agent)) { std::cerr << c.name << ": loop differs from runSingleAgent\n"; return 1; }
    out << "case " << c.name << "\n" << body.str();
    printResult(out, e.outcome, e.worldSteps, agent->state.decisionStep, agent->safeStopCount(),
                agent->mismatchCount(), agent->correctionCount(), agent->verifiedCount(), agent->totalEvaluations());
    out << "end\n";
    return 0;
}

int traceC0(std::ostream& out, const Case& c, bool observers) {
    std::unique_ptr<NaiveAgent> agent = makeC0(c);
#ifdef WITH_OBSERVER
    Tally tally;
    const ComputationObserver observer = makeObserver(tally);
    if (observers) {
        agent->state.predictor.setObserver(&observer);
        agent->state.deadCache.observer = &observer;
    }
#endif
    std::ostringstream body;
    const Ending e = runC0Loop(c, *agent, [&](const Observation& given, const Decision& d) {
        const NaiveAgentState& s = agent->state;
        body << "step " << s.decisionStep << "\n";
        if (!observers) {
            printObservation(body, given);
            body << "decision " << actionName(d.action) << "\n";
            body << "belief " << stateText(s.belief) << "\n";
            body << "plan " << s.plan.size() << " searches " << s.searches << "\n";
            body << "evaluations " << s.predictor.usedThisStep() << " total " << s.predictor.usedTotal() << "\n";
            return;
        }
#ifdef WITH_OBSERVER
        printTally(body, tally);
        tally = Tally{};
#endif
    });
    std::unique_ptr<NaiveAgent> again = makeC0(c);
    const RunResult r = runSingleAgent(c.world, *again, c.settings);
    if (r.outcome != e.outcome || r.worldSteps != e.worldSteps || r.evaluations != agent->totalEvaluations()) {
        std::cerr << c.name << ": C0 loop differs from runSingleAgent\n";
        return 1;
    }
    out << "case " << c.name << "\n" << body.str();
    printResult(out, e.outcome, e.worldSteps, agent->state.decisionStep, 0, 0, 0, 0, agent->totalEvaluations());
    out << "end\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string mode = argc > 2 ? argv[1] : "";
    if (mode != "traces" && mode != "observers") {
        std::cerr << "usage: orig_c1_trace traces|observers <cases>\n";
        return 1;
    }
#ifndef WITH_OBSERVER
    if (mode == "observers") { std::cerr << "observers needs the 89ea9b6 build\n"; return 1; }
#endif
    std::vector<Case> cases;
    if (!readCases(argv[2], cases)) return 2;
    for (const Case& c : cases) {
        const bool observers = mode == "observers";
        const int status = c.agent == "C0" ? traceC0(std::cout, c, observers) : traceC1(std::cout, c, observers);
        if (status != 0) return status;
    }
    return 0;
}
