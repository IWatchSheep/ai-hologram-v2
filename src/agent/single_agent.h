// Purpose: Declares C1, the competent single agent: its
// decision step, its final check and its counts (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: SingleAgent owns one SingleAgentState (the
// public member state) and works only on it. Its private
// helpers are split over the eight single_agent_*.cpp files
// by job (plan Section 4), exactly as the original's larger
// files were. Like the state, the agent cannot be copied:
// its predictor model lambda refers to its own state.

#ifndef AI_HOLOGRAM_SRC_AGENT_SINGLE_AGENT_H
#define AI_HOLOGRAM_SRC_AGENT_SINGLE_AGENT_H

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "agent/agent_types.h"
#include "agent/dead_squares.h"
#include "agent/investigation.h"
#include "agent/observation.h"
#include "agent/planner.h"
#include "agent/records.h"
#include "agent/single_agent_state.h"
#include "agent/verification.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The competent single agent (plan 3.9). It predicts, acts,
// observes, compares on the footprint only, investigates,
// corrects and replans, always through the budgeted step
// predictor, and it can always take a safe stop instead.
class SingleAgent {
public:
    // The belief is the starting belief when one is given,
    // and the first observation's state otherwise. The base
    // model and the "not dead" overrides are optional.
    SingleAgent(const Observation& first, AgentSettings settings,
                std::optional<WorldState> startingBelief = std::nullopt,
                std::optional<ActionModel> baseModel = std::nullopt,
                std::vector<Position> notDeadOverrides = {});

    // The state cannot be copied (see single_agent_state.h),
    // so neither can the agent that owns it.
    SingleAgent(const SingleAgent&) = delete;
    SingleAgent& operator=(const SingleAgent&) = delete;

    // One decision step, in the exact order of plan 3.9.
    Decision decide(const Observation& observation);
    // The last check of a run, after the goal is reached or
    // the step limit is hit: it checks the committed action.
    void finalCheck(const Observation& observation);

    // The log lines written so far, in order.
    const std::vector<std::string>& logLines() const;
    // The cautious action of plan 3.9, public so C1-6a can
    // test it alone (in a full run it can never commit, Q1).
    std::optional<PlanStep> chooseCautiousAction();

    int mismatchCount() const;
    int correctionCount() const;
    int verifiedCount() const;
    int safeStopCount() const;
    int totalEvaluations() const;

    // The whole state, public so tests and the trace writer
    // can read and seed it (plan 3.9, D-019).
    SingleAgentState state;

private:
    // single_agent_records.cpp: records and corrections.
    CorrectionRecord* findCorrection(int correctionId);
    CorrectionRecord& addRecord(CorrectionRecord record);
    Decision safeStop(const std::string& reason);
    void applyCorrection(const CorrectionRecord& record);
    void revertCorrection(const CorrectionRecord& record);
    bool objectDead(const WorldState& predicted,
                    const DeadSquareMap& map) const;

    // single_agent_check.cpp: the check and the retry.
    void checkCommitted(const Observation& observation);
    void settleTested(const CommittedPrediction& committed, bool matched);
    std::optional<Decision> retryDeferred();

    // single_agent_investigate.cpp: handling a result.
    void handleInvestigation(const InvestigationResult& result,
                             const WorldState& beliefBefore, Action action,
                             const Observation& after, bool logLines,
                             const std::vector<Observation>& historyUsed);
    void logInvestigation(const InvestigationResult& result,
                          const WorldState& beliefBefore, Action action);
    void createBeliefCorrections(const InvestigationResult& result,
                                 const WorldState& beliefBefore);

    // single_agent_pending.cpp: pending work, trap V2.
    void continuePendingWork();
    void attemptTrapV2(int correctionId, Position cell);
    void settleTrapV2(CorrectionRecord* record, TrapV2Result result,
                      int correctionId, Position cell);

    // single_agent_choose.cpp: choose and commit.
    Decision chooseAndCommit();
    std::optional<Decision> tryKeepPlan(const DeadSquareMap& map);
    Decision commitAction();
    std::vector<int> collectTestedCorrections(Action action,
                                              const Prediction& predicted);

    // single_agent_active_test.cpp: the active test.
    bool tryActiveTest(const DeadSquareMap& map, int reserve);
    bool tryTestFor(const CorrectionRecord& record, const DeadSquareMap& map,
                    int reserve);
    bool finishActiveTest(
        const CorrectionRecord& record,
        const std::vector<std::pair<Position, Action>>& targets,
        const SearchResult& result, int reserve);

    // single_agent_search.cpp: main search and fallback.
    Decision runMainSearch(const DeadSquareMap& map, int reserve);
    void handleExhausted(const DeadSquareMap& map);
    std::optional<Decision> chooseFallback(const SearchResult& result);
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_SINGLE_AGENT_H
