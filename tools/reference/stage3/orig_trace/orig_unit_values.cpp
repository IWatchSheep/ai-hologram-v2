// Prints, from the ORIGINAL C++ (89ea9b6 agent code), the values of the Stage 3 rebuild
// checks that are new (not taken from the original tests), so that the reference's values
// can be confirmed against the original code.
#include "agent/agent_log.h"
#include "agent/heuristic.h"
#include "agent/investigation.h"
#include "agent/naive_agent.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent/verification.h"
#include "world/rules.h"
#include "world/world_text.h"

#include <iostream>
#include <string>
#include <vector>

using namespace ai_hologram;

static WorldState demo() {
    return parseWorld({"##########", "#........#", "#........#", "#..A.O..D#", "#........#",
                       "#........#", "#...##...#", "#........#", "#........#", "##########"}).world.value();
}
static std::string p(Position c) { return positionText(c); }

int main() {
    // DS2b
    FreeComputationCounts counts;
    DeadSquareMap m = computeDeadSquares(demo(), {}, {}, counts);
    std::cout << "DS2b runs " << counts.analysisRuns << " cellops " << counts.cellOperations << "\n";
    // OB1, OB2
    std::vector<std::string> events;
    ComputationObserver rec;
    rec.onEvaluation = [&](const WorldState& s, Action a, const ActionModel* model, const Prediction& r) {
        events.push_back("evaluation " + p(s.agent) + " " + p(s.object) + " " + actionName(a) + " " +
                         (model ? "variant" : "own") + " " + outcomeName(r.outcome));
    };
    rec.onDeadRead = [&](Position c, bool v) { events.push_back("dead " + p(c) + " " + (v ? "1" : "0")); };
    rec.onHeuristic = [&](const WorldState& s, int v) {
        events.push_back("heuristic " + p(s.agent) + " " + p(s.object) + " " + std::to_string(v));
    };
    std::cout << "OB1 silent " << readDeadStatus(m, {1, 1}) << readDeadStatus(m, {5, 3});
    m.observer = &rec;
    std::cout << " watched " << readDeadStatus(m, {1, 1}) << readDeadStatus(m, {5, 3}) << "\n";
    for (auto& e : events) std::cout << "  " << e << "\n";
    events.clear();
    FreeComputationCounts c2;
    heuristic(demo(), m, c2);
    WorldState onDead = demo();
    onDead.object = {1, 1};
    heuristic(onDead, m, c2);
    localCheckSaysNotDead(demo(), m, {2, 2}, c2);
    std::cout << "OB2\n";
    for (auto& e : events) std::cout << "  " << e << "\n";
    events.clear();
    // OB4
    StepPredictor sp{WorldModel{}, 3};
    sp.startStep();
    sp.setObserver(&rec);
    const ActionModel blocked = [](const WorldState& s, Action) -> ActionResult { return {s, MoveOutcome::Blocked}; };
    sp.predict(demo(), Action::East);
    sp.predict(demo(), Action::East);
    sp.predictVariant(demo(), Action::North, blocked);
    sp.predict(demo(), Action::South);
    sp.predict(demo(), Action::West);
    std::cout << "OB4 total " << sp.usedTotal() << "\n";
    for (auto& e : events) std::cout << "  " << e << "\n";
    events.clear();
    // AM5
    {
        const ActionModel pushNorth = [](const WorldState& state, Action action) -> ActionResult {
            const ActionResult standard = applyAction(state, action);
            if (action != Action::East || standard.outcome != MoveOutcome::Pushed) return standard;
            WorldState wrong = standard.state;
            wrong.object = moved(state.object, Action::North);
            return {wrong, MoveOutcome::Pushed};
        };
        WorldState s = demo();
        s.agent = {4, 3};
        AgentActionModel am(pushNorth);
        const ActionModel snap = am.current();
        am.restore(Action::East);
        std::cout << "AM3 snapshot object " << p(snap(s, Action::East).state.object) << "\n";
        am.setBase(blocked);
        std::cout << "AM5 restored " << am.isRestored(Action::East) << " east " << p(am.current()(s, Action::East).state.object)
                  << " west " << outcomeName(am.current()(s, Action::West).outcome);
        am.setBase(ActionModel{});
        std::cout << " empty west " << outcomeName(am.current()(s, Action::West).outcome) << "\n";
    }
    // IV11
    {
        WorldState belief = demo();
        belief.agent = {4, 3};
        belief.object = {5, 4};
        WorldState rec3 = demo();
        rec3.agent = {4, 3};
        std::vector<Observation> h{Observation{1, rec3, std::nullopt, std::nullopt},
                                   Observation{2, rec3, Action::East, MoveOutcome::Blocked}};
        StepPredictor q{WorldModel{}, 200};
        q.startStep();
        const InvestigationResult r = investigateMismatch(belief, Action::East, h, q, ActionModel{&applyAction}, 1);
        std::cout << "IV11 outcome " << static_cast<int>(r.outcome) << " suspected " << static_cast<int>(r.suspected)
                  << " used " << q.usedThisStep() << " candidates " << r.candidates.size() << "\n";
    }
    // VF7c
    {
        CorrectionRecord r;
        r.id = 3;
        r.element.kind = ElementKind::Object;
        r.newPosition = {5, 3};
        r.oldPosition = {5, 4};
        WorldState b = demo();
        b.agent = {4, 4};
        StepPredictor q{WorldModel{}, 200};
        q.startStep();
        const auto alt = alternativePrediction(r, b, Action::East, q, ActionModel{&applyAction}, ActionModel{&applyAction});
        std::cout << "VF7c " << outcomeName(alt->outcome) << " " << p(alt->state.agent) << " " << p(alt->state.object)
                  << " used " << q.usedThisStep();
        StepPredictor q2{WorldModel{}, 200};
        q2.startStep();
        const auto none = alternativePrediction(r, b, Action::West, q2, ActionModel{&applyAction}, ActionModel{&applyAction});
        std::cout << " west " << none.has_value() << " used " << q2.usedThisStep() << "\n";
    }
    // C1-6a, C1-6b, VF5b
    {
        AgentSettings st{200, 5, 100, 3};
        SingleAgent a(Observation{1, demo(), std::nullopt, std::nullopt}, st);
        a.state.predictor.startStep();
        const auto pick = a.chooseCautiousAction();
        std::cout << "C1-6a " << actionName(pick->action) << " used " << a.state.predictor.usedThisStep() << "\n";
        SingleAgent b(Observation{1, demo(), std::nullopt, std::nullopt}, AgentSettings{2, 5, 100, 3});
        WorldState s1 = applyAction(demo(), Action::East).state;
        WorldState s2 = applyAction(s1, Action::East).state;
        b.state.history.push_back(Observation{2, s1, Action::East, MoveOutcome::Moved});
        b.state.history.push_back(Observation{3, s2, Action::East, MoveOutcome::Pushed});
        b.state.pendingV1 = ModelV1Work{Action::East, {1, 2}};
        const Decision d = b.decide(Observation{1, demo(), std::nullopt, std::nullopt});
        std::cout << "C1-6b " << d.stopReason << " pending";
        for (int k : b.state.pendingV1->pendingIndices) std::cout << " " << k;
        b.decide(Observation{2, s2, std::nullopt, std::nullopt});
        std::cout << " | VF5b pending " << b.state.pendingV1.has_value() << " corrections " << b.correctionCount()
                  << " restored " << b.state.agentModel.isRestored(Action::East) << "\n";
    }
    // OB5, OB6
    {
        WorldState d2 = demo();
        d2.walls[3][4] = true;
        AgentSettings st{200, 5, 100, 3};
        SingleAgent a(Observation{1, d2, std::nullopt, std::nullopt}, st, demo());
        std::vector<std::string> seen;
        a.state.activationHook = [&](int step) {
            seen.push_back(std::to_string(step) + ":" + std::to_string(a.state.log.size()) + ":" +
                           std::to_string(static_cast<int>(a.state.planKind)));
        };
        runSingleAgent(d2, a, st);
        std::cout << "OB5";
        for (auto& x : seen) std::cout << " " << x;
        std::cout << "\n";
        SingleAgent b(Observation{1, d2, std::nullopt, std::nullopt}, AgentSettings{1, 5, 100, 3}, demo());
        b.state.decisionStep = 1;
        b.state.committed.has = true;
        b.state.committed.prediction = Prediction{Action::East, MoveOutcome::Moved, applyAction(demo(), Action::East).state};
        b.state.committed.beliefBefore = demo();
        b.state.committed.decisionStep = 1;
        std::vector<int> deferredSeen;
        b.state.activationHook = [&](int step) { deferredSeen.push_back(step); };
        const ActionResult bl = applyAction(d2, Action::East);
        b.decide(Observation{2, bl.state, Action::East, bl.outcome});
        std::cout << "OB5 deferred";
        for (int x : deferredSeen) std::cout << " " << x;
        std::cout << "\n";
        SingleAgent c(Observation{1, d2, std::nullopt, std::nullopt}, st, demo());
        int reported = 0;
        ComputationObserver counter;
        counter.onEvaluation = [&](const WorldState&, Action, const ActionModel*, const Prediction&) { ++reported; };
        c.state.predictor.setObserver(&counter);
        c.state.deadCache.observer = &counter;
        const RunResult r = runSingleAgent(d2, c, st);
        std::cout << "OB6 reported " << reported << " evaluations " << r.evaluations << " lines " << r.lines.size() << "\n";
    }
    // OB7
    {
        AgentSettings st{200, 5, 100, 3};
        NaiveAgent a(Observation{1, demo(), std::nullopt, std::nullopt}, st);
        std::vector<std::string> seen;
        a.state.activationHook = [&](int step) {
            seen.push_back(std::to_string(step) + ":" + std::to_string(a.state.plan.size()) + ":" +
                           std::to_string(a.state.predictor.usedThisStep()));
        };
        runSingleAgent(demo(), a, st);
        std::cout << "OB7";
        for (auto& x : seen) std::cout << " " << x;
        std::cout << "\n";
    }
    // C0-3, C1-12a decide never called
    return 0;
}
