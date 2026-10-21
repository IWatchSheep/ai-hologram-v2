# Stage 3 (rebuild) — The competent single agent (C1) and the naive agent (C0)

| | |
|---|---|
| **Project** | AI Hologram v2 (`ai-hologram-v2`) |
| **Stage** | 3 of the rebuild (Brief Section 10, Stage 3: the competent single agent) |
| **Plan version** | 1.3 (after 3d: one shared file for the agent tests' fixtures, removing five helpers written twice) |
| **Date** | October 21, 2026 |
| **Status** | **APPROVED.** ChatGPT approved the v1.0 technical design (review 02), subject to one R21 clarification and the owner's approval; v1.1 records that clarification (D-070). The owner on 2026-10-21: "remember the rule, I already detailed my plan and now I grant you two creative freedom and all you do is say do you agree then I say yes I agree." (D-064, D-065). v1.2 (after 3c: one more test file) agreed by the owner: "sure I agree." **v1.3 (after 3d: the shared test fixtures, R4) awaits the owner's agreement.** |
| **Governing documents** | The brief v1.1; the frozen protocol v1.0 (Phase B Sections 10–12 define C1 and C0); **Migration Addendum M1** (M1.3, M1.4); `docs/RULES.md` (R1–R24); `AGENTS.md`; decisions D-064 and D-065. |
| **Builds on** | Stage 2, complete and tagged `stage-2-v2` (`a1a32d9`). |
| **Original it rebuilds** | `IWatchSheep/ai-hologram`, Stage 3 (plan `docs/history/ai-hologram/docs/stages/STAGE_03.md` v1.2, tag `stage-3`, commit `7587f38`) **plus the IE-1 instrumentation hooks H1–H4 as built at `89ea9b6`** (M1.3 item 1; R5: instrumentation is designed in from the start). The original's `src/agent/` is the frozen C1 baseline. |
| **Reference** | `tools/reference/stage3/` (R9, R10): the Python reference (`ref_parts.py`, `ref_verify.py`, `ref_agents.py`, `ref_trace.py`, `ref_trace_reader.py`), the check mirror `checks.py` (163 checks), `df_check.py`, `gen_golden.py`, three golden files produced by tools built from the **original C++** (`orig_trace/`), and the expected program output. It reuses `tools/reference/stage1/`, `stage2/` and (for the demonstration-seed layouts only) `stage4/`. |

---

## 1. Purpose
Stage 3 rebuilds **C1**, the competent single agent, and **C0**, the naive agent. C1 is the strong baseline of Experiment 1. It runs the full loop: predict, act, observe, compare, investigate, correct, replan.

It must behave **exactly** like the original, because M1 allows the rebuilt C1 to stand in for the frozen baseline only after an automated equivalence gate passes (M1.4). So this stage also builds the **first part of that gate**:
- **789 recorded runs** of the original C1 and C0, recorded decision step by decision step by a tool compiled from the original code (3.14);
- a test that replays every one of them through the rebuilt agents and fails on the first difference in any traced field (G4, G5).

The second part of the gate (the fault cases and the Stage 4 scores) comes with Stage 4, and **`c1-baseline-v2` is tagged only when the whole gate passes**, before any two-agent code exists (M1.3 items 2 and 3).

The rebuild changes how the code is organised (smaller files, one helper per job), never what it does. Where the original has quirks, the rebuild keeps them (Section 3.15).

## 2. What Stage 3 must NOT contain
- **No fault injection, fault schedules, randomness or experiment runner.** The fault activation points (H1) exist as hooks that are empty in every Stage 3 run.
- **No scoring, shadow evaluation, CSV or experiment code**, and no `src/experiment/` folder.
- **No messages, second reasoner or two-agent code.**
- **No change to C1's or C0's behaviour** from the original, in anything the gate traces or the tests check.
- **No change to the Stage 1 and 2 code**, except:
  - two private text helpers move from `difference_text.cpp` to a shared file (3.12, R4);
  - `currentStage()` returns 3;
  - `main.cpp` adds the agent demonstrations.
- No exceptions, assertions that stop the program, inheritance, threads or external libraries.

If anything here seems necessary, **stop and ask the owner**.

## 3. Exact behaviour (the original's, unchanged)
The Python reference follows the original function by function. **Where these words and the reference disagree, the reference and the golden traces win, and Claude must be told.** Section numbers in brackets refer to the protocol (Phase B).

### 3.1 Observation and history [10.2]
```
struct Observation {
    int decisionStep = 0;                     // the decision step it is delivered at
    WorldState state{};                       // the exact true state
    std::optional<Action> lastAction;         // empty at the start and after a safe stop
    std::optional<MoveOutcome> lastOutcome;
};
```
- The run loop builds every observation from the `Environment` only.
- C1 keeps every observation in `history`, in order; nothing ever changes an entry.
- `observedOutcome(o)` is `o.lastOutcome`, or `Blocked` when it is empty. This is the original's `value_or(Blocked)`, written once (R4).

### 3.2 Footprint [10.5]
- **`computeFootprint(beliefBefore, action, observedAfter, observedOutcome)`** returns c0, c1 and possibly c2:
  - c0 is the agent's cell in `beliefBefore`;
  - c1 = c0 + a;
  - c2 = c1 + a.

  c2 is included if (i) `beliefBefore.object == c1`, or (ii) `observedOutcome == Pushed`, or (iii) `observedAfter.object == c1`.
- **`compareFootprint(predictedState, predictedOutcome, observedState, observedOutcome, cells)`** returns `FootprintComparison {outcomesDiffer, cells}`.
  - For each cell in order, it computes the wall, agent-here and object-here values in both states. Cells outside the grid are walls (`isWall`).
  - It keeps a `FootprintCellDifference` only when at least one of the three differs. The record holds the cell, the three `…Differs` flags and the six values.
- **`isFootprintMatch(comparison)`** is `!outcomesDiffer && cells.empty()`. It is the one match test (R4), used by the check (3.9) and by the investigation (3.7).
- **`computeFootprintForTwoPredictions(firstBelief, firstOutcome, secondBelief, secondOutcome, action)`**:
  - c0 is `firstBelief.agent`;
  - c2 is included if either belief has the object at c1, or either outcome is Pushed.
- **`applyFootprintUpdate(belief, cells, observedState)`**. For each cell inside the grid, in order:
  1. the wall becomes the observed one;
  2. if the observed agent is in this cell, the belief's agent becomes it;
  3. if the observed object **or the believed object** is in this cell, the belief's object becomes the observed object.

  Nothing outside the footprint changes.
- **`footprintAfter(beliefBefore, action, after)`** is `computeFootprint(beliefBefore, action, after.state, observedOutcome(after))`. It is the one copy (R4); the original had two.

### 3.3 Dead squares, local check, heuristic [11.2, 10.7] and the H4 reads
- **`DeadSquareMap`** holds:
  - `dead[y][x]`;
  - `pushDistance[y][x]`, where −1 means not reached;
  - `const ComputationObserver* observer = nullptr`.
- **`FreeComputationCounts`** holds `analysisRuns`, `cellOperations`, `localChecks` and `heuristicCalls`.
- **`computeDeadSquares(belief, notDeadOverrides, deadCorrections, counts)`**:
  1. Fill `pushDistance` with −1, and add 1 to `analysisRuns`.
  2. If the destination is inside the grid, set its distance to 0 and put it in a queue (a `std::vector` with a front index).
  3. For each queued cell r, and for each direction d in the order N, E, S, W:
     - add 1 to `cellOperations`;
     - let q = r − d and s = r − 2d;
     - skip unless q is inside the grid, q is believed free, and s is believed free (outside counts as a wall);
     - if q has no distance yet, set its distance to r's + 1 and queue it.
  4. Every cell that is believed free and is not the destination gets `dead = (pushDistance < 0)`.
  5. Apply the "not dead" overrides (`dead = false`), then the dead corrections (`dead = true`). Both skip cells that are outside the grid, walls, or the destination.
- **`readDeadStatus(map, cell)`** (IE-1 H4) returns the stored `dead` value. If `map.observer` and its `onDeadRead` are set, it first reports `(cell, value)`. **Every** agent read of a dead status goes through it, except inside `computeDeadSquares`:
  - the planner's pruning;
  - `objectDead` (the cautious, keep-plan and active-test checks);
  - `localCheckSaysNotDead`;
  - the heuristic;
  - `checkConsequence`;
  - the trap V2 test.
- **`localCheckSaysNotDead(belief, map, p, counts)`**:
  1. Add 1 to `localChecks`.
  2. If p is the destination, return true.
  3. For each d in N, E, S, W: if p − d and p + d are believed free and `readDeadStatus(map, p + d)` is false, return true.
  4. Otherwise return false.
- **`DeadSquareCache::get(belief, notDead, deadCorrections, counts)`**:
  - It recomputes the map **into the same `map` member** only when something has changed since the last run (or there was none): the belief walls, the destination, or either override list (compared as whole vectors).
  - Every call sets `map.observer = observer`.
  - It returns a reference to `map`. References stay valid across recomputations, and C1 relies on that (3.15 Q3).
- **`heuristic(state, map, counts)`**:
  1. Add 1 to `heuristicCalls`.
  2. The value is `kInfiniteHeuristic` (1000000000) when `readDeadStatus(map, state.object)` is true.
  3. Otherwise it is P + max(0, manhattan(agent, object) − 1). P is `pushDistance[object]` when it is at least 0, and manhattan(object, destination) when it is −1, which happens only on an overridden cell.
  4. If `map.observer` and its `onHeuristic` are set, it reports `(state, value)`.
- **`manhattan(a, b)`** is |dx| + |dy|. It is the one copy, in `heuristic.h` (R4).
- **`kActionOrder`** is `{North, East, South, West}`. It is the one direction order for every loop over directions (R4).

### 3.4 Step predictor [10.7] and the H3 report
`class StepPredictor(WorldModel model, int budgetPerStep)`:
- **`startStep()`** makes a new `BudgetCounter(budgetPerStep)` (unused budget is lost) and calls `clearCache()`.
- **The cache.**
  - It is a list of entries (agent, object, action, prediction).
  - It has a **context**: the walls and destination the entries were computed under, or no context.
  - `clearCache()` empties the list and removes the context.
- **`isCached(state, a)`** is true only if there is a context, the context equals `state`'s walls and destination, and an entry with `state`'s agent, object and `a` exists.
- **`predict(state, a)`**, the reasoner's own model:
  1. If there is a context and it differs from `state`'s walls or destination, `clearCache()`.
  2. If an entry matches, return its prediction. This costs nothing.
  3. Otherwise call `model.predict(Belief{state}, a, budget)`. If it is refused, return nothing; nothing changes.
  4. Otherwise add 1 to `usedTotal`; set the context if there is none; append the entry.
  5. If an observer with `onEvaluation` is set, report `(state, a, nullptr, prediction)`.
- **`predictVariant(state, a, model)`**:
  - It never reads or fills the cache, and pays one evaluation through `WorldModel(model).predict`.
  - On success it adds 1 to `usedTotal` and reports `(state, a, &model, prediction)`.
- **`setObserver(const ComputationObserver*)`** (IE-1 H3). **Accessors:** `remaining()`, `usedThisStep()` (the counter's used) and `usedTotal()`.
- **`ComputationObserver`** (`computation_observer.h`) holds three `std::function`s, every one returning `void` and taking only const references, copies or a const pointer (R6):
  - `onEvaluation(const WorldState&, Action, const ActionModel*, const Prediction&)`;
  - `onDeadRead(Position, bool)`;
  - `onHeuristic(const WorldState&, int)`.

  An empty function is not called. No agent code ever reads anything from an observer.

### 3.5 The agent's action model [11.5] and H2
`class AgentActionModel`:
- **Construction.** `AgentActionModel()` uses `applyAction` as the base model. `explicit AgentActionModel(ActionModel base)` uses the given model, or `applyAction` if it is empty.
- **Restoring an action.** `restore(a)`, `unrestore(a)` and `isRestored(a)` work on four marks, one per action, in `kActionOrder`.
- **`current()`** returns an `ActionModel` that uses `applyAction` for restored actions and the base model for the others. It is a **snapshot**: it copies the base and the marks when called (3.15 Q2).
- **`base()`** returns the base model, which is the old model of a `model(a)` correction.
- **`setBase(model)`** (IE-1 H2) replaces the base and keeps the marks; an empty model means `applyAction`. Only the Stage 4 fault injector will call it. In Stage 3 only test AM5 does.

### 3.6 The planner [11.2]
- **Types:**
  - `PlanStep {action, expected}`;
  - `SearchKind {Found, OutOfBudget, Exhausted}`;
  - `DiscoveredState {state, bestG, h, parentIndex (−1 for the root), action, discoveryNumber}`;
  - `SearchResult {kind, plan, discovered, improvedPaths, reopenedStates}`.
- **`search(root, map, goalTest, heuristic, predictor, reserve)`** is A* over the agent's own belief:
  1. Discover the root (g 0, h = heuristic(root), discovery number 0), and push its queue entry.
  2. **The queue.** Each entry is (f = g + h, h, entry number, state index, g). The smallest (f, h, entry number) comes first; entry numbers count every push. The C++ uses `std::priority_queue` with a comparator.
  3. Pop the first entry. Skip it if its g is no longer the state's `bestG`, or if the state is closed.
  4. If `goalTest(state)`, return **Found** with the plan to it. The goal is accepted when popped, not when generated.
  5. Otherwise close the state and try each action in `kActionOrder`:
     - if the prediction is not cached and `remaining() <= reserve`, return **OutOfBudget**;
     - otherwise predict through the step predictor. If that returns nothing, also return OutOfBudget;
     - skip the result if its outcome is Blocked or InvalidState, or if `readDeadStatus(map, result.object)`;
     - find the discovered state with the same agent and object. If it exists and g + 1 ≥ its `bestG`, skip it;
     - if it exists and g + 1 is smaller, **improve** it: count `improvedPaths`; if it was closed, reopen it and count `reopenedStates`; set its `bestG`, parent and action; push a new entry. Its stored state and h are unchanged;
     - if it is new, discover it (h computed now, the next discovery number) and push an entry.
  6. If the queue is empty, return **Exhausted**.
- **The plan to a state** follows parents back to the root. Each step is (action, the discovered state's stored state). One function builds it (R4) and is used by `search` and `fallback`.
- **`fallback(result)`** chooses the discovered **non-root** state with h < `kInfiniteHeuristic` and the smallest (h, `bestG`, discovery number), and returns the plan to it. If there is none, it returns an empty plan.

### 3.7 Records, investigation and the consequence check [11.4, 10.6]
- **Records** (`records.h`):
  - `SourceLabel {S1…S5}`, `CorrectionStatus {Consistent, Verified, Rejected}`, `ElementKind {Wall, Object, ActionModel, Trap}`;
  - `Element {kind, cell, action}`;
  - `CorrectionRecord {id, element, newBlocked, oldBlocked, newPosition, oldPosition, status, stepConsistent, tested}`;
  - `MismatchRecord {decisionStep, action, differences, candidates, suspected, anomaly}`;
  - **`reserveFor(corrections)`**: 1 plus the number of corrections with status Consistent whose kind is not Trap. It is the one reserve rule (R4), used by C1 and by the trace writer.
- **History checks** (`history_checks.h`):
  - `wallHistoryCheck(cell, blocked, history)`: every entry has that wall value;
  - `objectHistoryCheck(value, before)`: `before.state.object == value`;
  - `trapHistoryPasses(p, destination, history)`: false if some entry has the object at p and a **later** entry has it on the destination. It is the one copy (the original had two).
- **`investigateMismatch(beliefBefore, action, history, predictor, currentModel, reserve)`** returns `InvestigationResult {outcome, candidates, suspected, anomaly}`. The defaults are ModelSuspected, no candidates, S5 and no anomaly.
  1. If `history.size() < 2`, return the defaults.
  2. Let *before* be the second-last entry and *after* the last. Set `anomaly = beliefBefore.agent != before.state.agent`.
  3. **Candidates**, using cells = `footprintAfter(beliefBefore, action, after)`:
     - for each cell whose wall in `beliefBefore` differs from *before*, a wall candidate with *before*'s value;
     - if the believed or the recorded object is in a footprint cell, and they differ, one object candidate with *before*'s object position. That position may lie outside the footprint.
  4. **No candidates:** return ModelSuspected, S1.
  5. **Otherwise suspected is S2**, and the candidates are verified together:
     - if any history check fails (walls against the whole history, the object against *before*), or the corrected belief (`beliefBefore` with every candidate applied) is not a valid world, the result is NoCandidatePassed;
     - else, if `remaining() <= reserve`, the result is Deferred;
     - else make **one** `predictVariant(corrected, action, currentModel)`. If that returns nothing, the result is Deferred;
     - else compare its footprint (computed from the corrected belief) with *after*. A match gives BeliefCandidatesConsistent; otherwise NoCandidatePassed.

     NoCandidatePassed sets suspected to S5.
- **`checkConsequence(belief, map, history, counts)`** gives NotApplicable, FullStateDeadEnd or Mismatch. With p the believed object:
  - NotApplicable if p is outside the grid, `readDeadStatus(map, p)` is true, or p is the destination;
  - FullStateDeadEnd if `localCheckSaysNotDead` is true;
  - NotApplicable if `trapHistoryPasses` is false;
  - otherwise Mismatch.

### 3.8 Verification [11.5]
- **Model V1** (`model_v1.h`):
  - `startModelV1(a, history)` gives `ModelV1Work {action, pendingIndices}`, the indices k (in order) whose `lastAction` is a.
  - `continueModelV1(work, history, predictor, reserve)` works through the indices while any remain:
    - Pending if `remaining() <= reserve`;
    - Failed if k < 1 or k ≥ size;
    - replay `predictVariant(history[k−1].state, a, applyAction)`. Pending if refused;
    - Failed if the replayed state differs from `history[k].state`, or `history[k].lastOutcome` is empty or differs;
    - otherwise remove the index.

    When none remain, the work is Done.
- **`trapV1Check(belief, map, p, history, counts)`** is `!localCheckSaysNotDead(…) && trapHistoryPasses(…)`.
- **`trapV2Test(belief, map, p, predictor, currentModel, reserve, anomalies)`**:
  1. For each d in N, E, S, W, let stand = p − d. Skip d if stand is a believed wall.
  2. The hypothetical state is the belief with the agent at stand and the object at p. If it is not a valid world, add 1 to `anomalies` and skip d.
  3. With k the number of pushes kept: if `remaining() − k < reserve`, return Waiting.
  4. For each push, predict it with `predictVariant(…, currentModel)`. If that returns nothing, return Waiting. A push is an escape if its object lands on a cell ≠ p that is inside the grid and not `readDeadStatus`.
  5. Any escape gives Rejected; otherwise Verified.
- **`alternativePrediction(correction, belief, action, predictor, currentModel, oldModel)`**, with c1 and c2 one and two cells ahead of the believed agent:
  - **model(b):** none unless b = action and the belief is valid; then `predictVariant(belief, action, oldModel)`;
  - **trap:** none;
  - **wall at c:** none unless c is c1 or c2 and inside the grid. The alternative is the belief with that wall set to `oldBlocked`;
  - **object:** none unless the old position or the believed object is c1 or c2. The alternative is the belief with the object at the old position.

  For a wall or an object: none if the alternative is not a valid world; otherwise `predictVariant(alternative, action, currentModel)`.
- **`predictionsDiffer(x, y)`**: the outcome, the agent or the object differs.
- **`activeTestTargets(belief, cell)`**: for d in N, E, S, W, the pair (cell − d, d) when cell − d is not a believed wall and is not the believed object.

### 3.9 C1's decision step [11.3–11.7] and H1
**The state** (`SingleAgentState`, all plain public members, so that tests and the trace writer can read and set them, D-019):
- `settings`, `belief` and `history`;
- `deadCache`, `freeCounts`, `notDeadOverrides` and `deadOverrides`;
- `agentModel` and `predictor`;
- `plan` and `planKind {None, Goal, Fallback, Test}`;
- `committed {has, prediction, beliefBefore, decisionStep, testedCorrectionIds}`;
- `corrections` and `mismatches`;
- `pendingV1` (optional `ModelV1Work`);
- `deferred {has, beliefBefore, action, history}` and `waitingTrap {has, correctionId, cell}`;
- `safeStops`, `decisionStep`, `nextCorrectionId` (1), `trapAnomalies` and `log`;
- `activationHook` (`std::function<void(int)>`; IE-1 H1).

The predictor's model is a lambda that calls `agentModel.current()` at every evaluation, so a restoration takes effect at once. The state therefore cannot be copied (its copy operations are deleted).

**Construction.** `SingleAgent(first, settings, startingBelief = nullopt, baseModel = nullopt, notDeadOverrides = {})`:
- the belief is `startingBelief`, or else `first.state`;
- `history = {first}`.

**`decide(observation)`:**
1. Add 1 to `decisionStep`; `predictor.startStep()`. If `decisionStep != 1`, append the observation to the history.
2. If `committed.has`, **check** it (below).
3. **Retry a deferred investigation.** If it is deferred again, take the safe stop `budget too small for verification`. **Before returning, call the activation hook** (H1: a step that ends here still has its activation point).
4. **Continue pending work:** first a waiting trap V2 test, then model V1 work:
   - Done: create a model correction (Consistent, `stepConsistent` = this step), apply it (restore the action, clear the cache) and log its line;
   - Failed: drop the work and log the unexplained-update line.
5. **The fault activation point (H1):** if `activationHook` is set, call it with `decisionStep`. It is empty in every Stage 3 run.
6. **Choose and commit** (below), and return the decision.

**`finalCheck(observation)`** runs `predictor.startStep()`, appends the observation, and checks the committed prediction if there is one.

**The check** (step 2):
1. Take the committed prediction and set `committed.has = false`.
2. Compute cells = `footprintAfter(beliefBefore, action, observation)`. Compare the committed prediction with the observation on those cells.
3. Log the step line for the committed step: `Step <committed step>: <Action> | predicted <O>, observed <O> | match` or `| MISMATCH`.
4. **Settle the tested corrections**, in the order of `testedCorrectionIds`. Each that is still Consistent becomes Verified on a match, with its line, or Rejected on a mismatch, with its line, and is then reverted.
5. **On a match:** `applyFootprintUpdate` with the observation.
6. **On a mismatch:**
   - `investigateMismatch(beliefBefore, action, history, predictor, agentModel.current(), reserveFor(corrections))`;
   - store a `MismatchRecord` with this decision step, the action, the comparison, the candidates' elements, the suspected source and the anomaly flag;
   - **handle** the result (below), with its lines logged.

**Handling an investigation result:**
1. **If logging,** write the investigation lines first:
   - the model-suspicion line, when there are no candidates and the outcome is ModelSuspected;
   - then one line per candidate (wall: `believed <belief's wall> recorded <candidate value>`; object: `believed <belief's object> recorded <candidate position>`);
   - then the anomaly line, if flagged.
2. **Then, by outcome:**
   - **BeliefCandidatesConsistent:** one correction per candidate, in order. Each takes `oldBlocked` from `beliefBefore`'s wall, `oldPosition` from `beliefBefore`'s object, status Consistent and `stepConsistent` = this step. Apply each and log its line. Then `applyFootprintUpdate`;
   - **NoCandidatePassed:** log the unexplained-update line; `applyFootprintUpdate`;
   - **ModelSuspected:** `applyFootprintUpdate` at once (D-025); `pendingV1 = startModelV1(action, history)`;
   - **Deferred:** log the deferred line (only when logging); store `deferred` (the belief before, the action and a copy of the history). Nothing is updated.

   A retried deferred investigation is handled **without** logging its investigation lines again, using the stored history and its last entry.

**Applying and reverting a correction:**

| Kind | Apply | Revert (when REJECTED) |
|---|---|---|
| Wall (inside the grid) | the belief wall = `newBlocked` (the walls change, so the predictor clears its own cache at the next prediction, 3.4) | the belief wall = `oldBlocked` |
| Object | the belief object = `newPosition` | nothing (only marked) |
| Model(a) | `agentModel.restore(a)`; `predictor.clearCache()` | `unrestore(a)`; `clearCache()` |
| Trap(p) | add p to `deadOverrides` if absent; `clearCache()` | remove p; `clearCache()` |

**Choose and commit** (step 6):
1. **Cautious mode, while `pendingV1` exists.** `chooseCautiousAction()` tries each action a in `kActionOrder`:
   - it needs 1 evaluation if the current-model prediction is cached, or 2 if not. It stops if `remaining() − need < reserve`;
   - it predicts a (cached allowed), then `predictVariant(belief, a, applyAction)`, stopping if either returns nothing;
   - the first a whose two predictions do not differ, and whose result's object is not dead (`objectDead`: inside the grid and `readDeadStatus`), is chosen.

   If one is chosen: log the cautious line; plan = {it}; planKind Goal; commit. Otherwise take the safe stop `investigation unfinished, no cautious action`.
2. **Otherwise,** reserve = `reserveFor(corrections)` and map = `deadCache.get(…)`.
3. **Active test**, if planKind ≠ Test. For each correction in order that is Consistent, a wall, and `decisionStep − stepConsistent >= activeTestDelay`:
   - the targets are `activeTestTargets(belief, cell)`. If there are none, try the next correction;
   - search with goal "the agent is on a target cell". The heuristic is `kInfiniteHeuristic` if `objectDead`, else the smallest manhattan from the agent to a target. It makes no `heuristicCalls` count and no observer report;
   - if Found: end = the plan's last expected state, or the belief if the plan is empty. For each target whose cell is end's agent, skip it if its move is not cached and `remaining() <= reserve`. Otherwise predict the move from end; if that returns nothing, skip it. Else plan = the walk plus (move, predicted state); planKind Test; log the active-test line with the plan's length; commit;
   - a correction without a usable test lets the loop try the next one.
4. **Keep the plan**, if one exists:
   - predict its first action. If no prediction can be made: clear the plan and take the safe stop `no budget for a committed prediction`;
   - if the predicted state equals the expected state and its object is not dead, commit;
   - otherwise clear the plan.
5. **Main search** with goal "object on destination" and the heuristic of 3.3:
   - **Found with an empty plan:** take the safe stop `belief shows goal reached`;
   - **Found:** plan; planKind Goal; log the search line;
   - **Exhausted:** handle a possible consequence surprise, then use the fallback. **OutOfBudget:** use the fallback;
   - **Fallback empty:** take the safe stop `budget too small to plan` (after OutOfBudget) or `no safe action` (after Exhausted). Otherwise: plan; planKind Fallback; log the fallback line;
   - then predict the plan's first action. If that returns nothing: clear the plan and take the safe stop `no budget for a committed prediction`. Otherwise commit.
6. **The consequence surprise** (after Exhausted). With p the believed object, `checkConsequence`:
   - **FullStateDeadEnd:** log the surprise line and the full-state line;
   - **Mismatch:** log the surprise line and the mismatch line; create a trap correction (Consistent, this step); apply it; log its line; then **attempt the trap V2 test at once**.

**Committing:**
1. Predict the plan's first action from the belief. If that returns nothing: clear the plan and take the safe stop `no budget for a committed prediction`.
2. Remove the step from the plan. If the plan is now empty, planKind is None.
3. **V2 bookkeeping.** For each correction in order that is Consistent, not yet tested, and while `remaining() > 0`: `alternativePrediction(record, belief, action, predictor, agentModel.current(), agentModel.base())`. If it exists and differs from the committed prediction, mark the correction tested and add its id to the tested list.
4. Store `committed` (the prediction, a copy of the belief, this step, the tested ids) and return the action.

**Attempting a trap V2 test:**
- If the record is missing or not Consistent, clear `waitingTrap`.
- Otherwise run `trapV2Test(belief, deadCache.get(…), cell, predictor, agentModel.current(), reserveFor(corrections), trapAnomalies)`, and log one invalid-hypothetical line per new anomaly. Then:
  - **Verified:** the record becomes Verified; log its line; clear `waitingTrap`;
  - **Rejected:** the record becomes Rejected; log its line; revert it; clear `waitingTrap`;
  - **Waiting:** set `waitingTrap` to (id, cell).

**A safe stop:**
- adds 1 to `safeStops`;
- sets `committed.has = false`;
- logs `Step <decisionStep>: safe stop (<reason>)`;
- returns `Decision {false, North, reason}`.

**Counts:** `mismatchCount`, `correctionCount`, `verifiedCount`, `safeStopCount`, `totalEvaluations` (= `usedTotal`) and `logLines()`.

### 3.10 C0, the naive agent [11.8]
**The state** (`NaiveAgentState`):
- the settings and the belief;
- `deadCache`, `freeCounts` and empty override lists;
- `agentModel` and the predictor;
- `plan`, `decisionStep` and `searches`;
- `activationHook`.

**`decide`:**
1. Add 1 to `decisionStep`; `startStep()`.
2. **Call the activation hook** (H1). C0 never checks, so its activation point is at the start of the step.
3. If the plan is empty, **plan from the belief**:
   - search with reserve 0, add 1 to `searches`;
   - Found with a non-empty plan gives the plan; otherwise the fallback;
   - if that is empty, use the first action (N, E, S, W) predicted Moved or Pushed with its predicted state, or else North with the unchanged belief.
4. Take the first plan step, set the belief to its **stored** expected state, and return its action.

C0 never reads an observation after the first, and makes no checks, records or safe stops. `searchCount()` and `totalEvaluations()`.

### 3.11 The run loop [Phase A 5.1], with watch-only watchers
- **`RunResult`:**
  - `outcome` (`Success`, `Step limit` or `Stopped -- unresolved`);
  - `worldSteps`, `safeStops`, `mismatches`, `corrections`, `verified` and `evaluations`;
  - `lines`: C1's log, then the four result lines (3.12). C0 has only the four lines, with zero records.
- **`RunWatchers`** (new; D-071): `afterDecision(const SingleAgent&, const Observation&, const Decision&)` and `afterFinalCheck(const SingleAgent&, const Observation&)`, both `std::function` returning `void`. `NaiveRunWatchers` has `afterDecision(const NaiveAgent&, const Observation&, const Decision&)`. They only watch (R6), and empty ones are not called.
- **`runSingleAgent(trueInitialState, SingleAgent&, settings, watchers = {})`:**
  - if the object is already on the destination: Success, 0 world steps, and `decide` is never called;
  - otherwise it calls `continueRun` with a new `Environment`, the observation {1, state, none, none} and 0 world steps.
- **`continueRun(environment, current, worldSteps, agent, settings, watchers)`** repeats:
  1. **Decide**, then call `afterDecision(agent, current, decision)`.
  2. **On a safe stop:** if `safeStopCount() >= safeStopLimit`, the outcome is Stopped -- unresolved. Otherwise the next observation is {decisionStep + 1, the same state, none, none}.
  3. **On an action:** step the environment and count the world step. The next observation is {decisionStep + 1, the new state, action, outcome}. Then:
     - if the object is on the destination: `finalCheck`, `afterFinalCheck`, Success;
     - otherwise, if worldSteps ≥ `maxWorldSteps`: `finalCheck`, `afterFinalCheck`, Step limit.

  Tests call `continueRun` directly to start after a seeded action (3.14).
- **`runSingleAgent(trueInitialState, NaiveAgent&, settings, watchers = {})`** is the same loop without safe stops or a final check.

### 3.12 Text: log lines, demonstrations and the program
**Shared text helpers** (R4). `positionText(Position)` writes `(x, y)`, and `blockedText(bool)` writes `blocked` or `free`. They move from the private helpers of `difference_text.cpp` into `src/core/text_words.h/.cpp`, unchanged; `difference_text.cpp` uses them, and CM10, G3 and D2 prove it.

**Log lines** (`agent_log.h`; exact):

| Function | Text |
|---|---|
| `planSearchLine(k)` | `Plan: search found a plan of <k> actions` |
| `planFallbackLine(k)` | `Plan: fallback toward the best state found (<k> actions)` |
| `planActiveTestLine(n, k)` | `Plan: active test of correction <n> (<k> actions)` |
| `planCautiousLine()` | `Plan: cautious mode (investigation unfinished)` |
| `stepLine(t, a, p, o, m)` | `Step <t>: <Action> \| predicted <O>, observed <O> \| match` (or `\| MISMATCH`) |
| `safeStopLine(t, r)` | `Step <t>: safe stop (<r>)` |
| `correctionVerifiedLine(n)` / `correctionRejectedLine(n)` | `  - correction <n>: VERIFIED` / `REJECTED` |
| `investigationWallLine(c, b, r)` | `  - investigation: wall at (x, y) believed <blocked\|free>, recorded <blocked\|free> (source S2)` |
| `investigationObjectLine(b, r)` | `  - investigation: object believed at (x, y), recorded at (x, y) (source S2)` |
| `investigationModelLine(a)` | `  - investigation: belief matched the record; action model for <Action> suspected (source S1)` |
| `anomalyAgentLine()` | `  - anomaly: believed agent position differs from the record` |
| `investigationDeferredLine()` | `  - investigation: budget too small for verification; retried next step` |
| `correctionWallLine(n, c, b)` | `  - correction <n>: wall at (x, y) set to <blocked\|free>: CONSISTENT` |
| `correctionObjectLine(n, p)` | `  - correction <n>: object position set to (x, y): CONSISTENT` |
| `correctionModelLine(n, a)` | `  - correction <n>: action model for <Action> restored to the standard rule: CONSISTENT` |
| `unexplainedLine()` | `  - investigation: no candidate passed V1 (source S5); unexplained update` |
| `consequenceSurpriseLine(p)` | `  - consequence surprise: search exhausted with the object at (x, y)` |
| `fullStateDeadEndLine()` | `  - full-state dead end (dead-square classification consistent; no claim)` |
| `consequenceMismatchLine(p)` | `  - consequence mismatch: (x, y) violates the dead-square rule (source S1)` |
| `correctionTrapLine(n, p)` | `  - correction <n>: (x, y) classified as a dead square: CONSISTENT` |
| `anomalyInvalidLine()` | `  - anomaly: invalid hypothetical state skipped` |
| `resultLine(o, w, s)` | `Result: <outcome> after <w> world steps, <s> safe stops` |
| `mismatchesLine(m)`, `correctionsLine(c, v)`, `evaluationsLine(e)` | `Mismatches: <m>`, `Corrections: <c> (<v> verified)`, `Model evaluations used: <e>` |

**The agent demonstrations** (`runAgentDemos(std::ostream&)`):
- **Settings:** B 200, S_max 5, T_max 100 and T_v 3.
- **Demo 1:** the demo layout (`demoLayout()`, shared, R4), with a correct belief.
- **Demo 2:** the true world is the demo layout with a wall added at (4, 3). The starting belief is the demo layout without it: a hand-written wrong belief, labelled as such, not fault injection.
- **Text:** byte-identical to the original's `runAgentDemos` (46 lines; `checks.py` AD1_TEXT). It starts with an empty line, then the two settings lines, an empty line, `C1 demo 1: correct belief` and its run lines, an empty line, `C1 demo 2: belief wrongly has no wall at (4, 3)` and its run lines, and the note line.
- If a layout ever failed to parse, it would print `Agent demo layout is invalid.` and return.

**The program** (exactly; **102 lines, 94 non-blank**; the full text is `tools/reference/stage3/expected_program_output.txt`):
```
AI Hologram v2 -- Stage 3
Grid world with a competent single agent (C1), a naive agent (C0) and scripted demonstrations.
No faults are injected, no two-agent reasoning exists, and no experiment is run.
<the Stage 1 and Stage 2 demonstrations, unchanged (STAGE_02.md 3.8, from the first empty line to
 "How beliefs are corrected is decided in Stage 3.")>
<runAgentDemos, 46 lines>
```
The demonstration parts are byte-identical to the original. Only the header lines differ, on purpose (D-062). The original's demo-2 evaluation counts, **19 and 414**, are part of the expected text.

### 3.13 IE-1 instrumentation, designed in (R5, R6; M1.3)
- **H1:** the activation hooks (3.9 steps 3 and 5; 3.10 step 2);
- **H2:** `AgentActionModel::setBase` (3.5);
- **H3:** the step predictor's observer (3.4);
- **H4:** `readDeadStatus` and the heuristic report (3.3).

They do nothing unless something is attached, and nothing in C1 or C0 reads anything back.
- Tests OB1–OB7 check what they report.
- G5 checks, for all 789 runs, that they report exactly what the original 89ea9b6 code reports, at every decision step.
- G4 is traced with nothing attached, and it equals the trace of 7587f38, which has no hooks at all.

### 3.14 The equivalence gate, part 1: golden traces from the original code (M1.4; D-069)
**How they were made** (all in `tools/reference/stage3/`, reproduced by `orig_trace/build.sh`):
1. `orig_trace/original_src/` holds the original `src/agent/*` (31 files) and `src/demo/agent_demo.*` (2) from `7587f38`, byte for byte. `orig_trace/original_src_ie1/agent/` holds the 16 files that differ at `89ea9b6` (15 changed and `computation_observer.h`). Each hash was verified against `git show <commit>:src/<file>`. The world, model and compare files are the Stage 1 and 2 copies, identical at both commits.
2. `orig_trace/orig_c1_trace.cpp` is built twice, against `7587f38` and against `89ea9b6`. For each case it runs the original agent in a copy of the original run loop, printing the agent's state after every decision. It then reruns the case with the original `runSingleAgent` and stops with an error unless the outcome, the counts and every result line are identical.
3. `gen_golden.py cases` writes the 789 cases. Both builds produce the **same** trace file byte for byte. The 89ea9b6 build also produces the observer file. The Python reference reproduces all three files byte for byte.

**The 789 cases** (`golden/c1_cases.txt`):
- **39 named cases, from M1.4's list:**
  - the two demonstrations;
  - C1-3 (LP, D2), C1-5 (LT), C1-6c (two), C1-9, C1-11 (deferred, no budget), C1-12 (two) and VF6;
  - SP7 at B = 1…11;
  - LT with the "not dead" override (two budgets), L2 (two budgets);
  - four trap cases (rejected twice, waiting twice);
  - seven C0 cases (C0-4, C0-5, C0-3a, C0-3b, LP, L2, the C1-9 model).
- **20 demonstration-seed runs:** layouts 500000–500009 (R21), fault-free, C1 and C0, at the protocol's settings (B 200, S_max 5, T_max 100, T_v 3).
  - The layouts are written into the case file as text.
  - They come from the carried-over Stage 4 reference (`make_layout`). Claude checked that the original C++ generator at `89ea9b6` gives the same ten layouts.
  - The run seeds 1–3 of M1.4 matter only with faults, so the fault-free runs are one per layout and agent.
- **730 synthetic cases** (630 C1, 100 C0), generated by `gen_golden.py` with seed **20261022**: deterministically generated synthetic verification fixtures: random valid worlds from an independent generator. **No pilot or confirmatory seed, layout, run or other reserved experimental material is used** (D-070, the R21 clarification). They come in 12 families, each aimed at a part of C1:
  - missing and extra walls;
  - wrong object positions;
  - the wrong action models of the catalogue below;
  - wrong agent or destination positions (anomalies);
  - stuck objects with "not dead" overrides (trap claims), with and without wrong models, at large and tight budgets;
  - seeded correct and wrong corrections;
  - tight budgets;
  - seeded committed actions (deferred investigations, unexplained updates).

  Budgets run from 0 to 100,000.

**Coverage** (log lines in the 789 traced runs):
- 11,235 matching and 430 mismatching step lines;
- 304 search plans, 7,382 fallback plans and 166 active tests;
- 244 model suspicions with 235 model corrections; 154 wall and 32 object investigations;
- 21 unexplained updates and 48 deferred investigations;
- 505 consequence surprises: 458 full-state dead ends and 47 trap claims;
- 177 VERIFIED and 28 REJECTED (all four correction kinds are rejected at least once);
- every safe-stop reason;
- 2 waiting trap tests, 27 steps with unfinished model V1 work and 158 V2 tests;
- outcomes: 143 Success, 349 Step limit and 297 Stopped -- unresolved;
- 15,674 decision steps and 374 final checks.

**The case file.** Each case is, line by line:
- `case <name>`;
- `agent C1` or `agent C0`;
- `settings <B> <S_max> <T_max> <T_v>`;
- `world` and 10 rows;
- `belief true`, or `belief given` and 10 rows;
- `model <spec>`;
- `notdead <n> <x1> <y1> … <xn> <yn>`;
- any `seed` lines;
- `end`.

The seeds, applied in this order after the agent is built:
- `seed wall <x> <y> <new> <old>` (blocked or free): a correction record with id 1, 2, … in order, kind wall, those values, Consistent, `stepConsistent` 0 and untested. `nextCorrectionId` becomes the count + 1. The belief already shows the new value.
- `seed v1 <A> <k>`: from s0 = the true world, s_i = `applyAction(s_{i−1}, A)`. Each {i + 1, s_i, A, outcome} is appended to the history, and `pendingV1 = {A, [1..k]}`.
- `seed plan <A>`: plan = {(A, belief)}; planKind Goal.
- `seed committed <A>`: `decisionStep = 1`, and `committed` = {A, `applyAction(belief, A)`'s outcome and state, a copy of the belief, step 1, no tests}. The run then starts with `Environment(world).step(A)` (1 world step) and the observation {2, the new state, A, outcome}, through `continueRun`.

**The model catalogue** (`model <spec>`; a is an action, b an action other than a):

| Spec | The model |
|---|---|
| `standard` | no base model given (the standard rule) |
| `stuck <a>` | a: `applyAction`'s result if InvalidState, otherwise (state unchanged, Blocked); other actions standard |
| `nopush <a>` | a push in a becomes (state unchanged, Blocked); everything else standard |
| `slip <a>` | a move (Moved) in a goes one cell further when that cell is not a wall and not the object; everything else standard |
| `swap <a> <b>` | a is predicted as `applyAction(state, b)`; other actions standard |
| `pushturn <a> <b>` | a push in a moves the agent onto the object's cell and the object one cell in direction b, or (unchanged, Blocked) if that cell is a wall; everything else standard |

**The trace of a C1 case** (`golden/c1_traces.txt`). Here *state* means `agent (x, y) object (x, y) destination (x, y) walls <h>`. The `<h>` is the 100 walls as 25 lowercase hex digits: cell index i = 10y + x, digit j covers cells 4j … 4j + 3, and the first of them is the digit's highest bit.
```
case <name>
step <decisionStep after decide>                       one block per decision
observation <its decisionStep> <state> last <Action> <Outcome>      or: ... last none
decision <Action>                                      or: decision stop <reason>
committed <Action> <Outcome> <state> tests <ids>       or: ... tests none / committed none
belief <state>
plan <none|goal|fallback|test> <plan size>
work v1 <Action> <indices>    or work v1 none   then: " | deferred <yes|no> | trap <id|none>"
evaluations <usedThisStep> total <usedTotal> safe stops <safeStops> reserve <reserveFor>
log <line>                                             each log line written since the last block
mismatch step <s> action <A> outcomes <differ|same> suspected <S1..S5> anomaly <yes|no>
  cell (x, y) wall <pw><ow> agent <pa><oa> object <po><oo>       each cell difference (1/0)
  candidate <element>                                  each new mismatch record, in full
correction <id> <element> [new <v> old <v>] <STATUS> since <stepConsistent> tested <yes|no>
                                                       each record that is new or changed
final                                                  the final check, if any, then its
observation ...                                        observation line, and the lines from
                                                       "belief" to the corrections, as above
result <outcome> | world steps <w> | decision steps <d> | safe stops <s> | mismatches <m> | corrections <c> | verified <v> | evaluations <e>
end
```
- *element* is `wall (x, y)`, `object`, `model <Action>` or `trap (x, y)`.
- The `[new … old …]` part is `new <blocked|free> old <blocked|free>` for a wall, `new (x, y) old (x, y)` for an object, and absent otherwise.
- A correction line is printed when its text differs from the last one printed for that record, or when the record is new.

**A C0 case** has, per decision:
- `step <d>`, `observation …`, `decision <Action>` and `belief <state>`;
- `plan <size> searches <n>`;
- `evaluations <used> total <total>`.

Then the result line (safe stops, mismatches, corrections and verified 0) and `end`. A run that ends at once has only `case`, `result` and `end`.

**The observer file** (`golden/c1_observers.txt`) has the same `case`, `step`, `final`, `result` and `end` lines. After each `step` or `final` line it has one line:
```
observer evaluations <own> variant <variant> dead reads <r> true <t> heuristics <h> infinite <i> sum <s>
```
These are the reports since the previous such line, from an observer attached to the predictor (`setObserver`) and to the dead-square cache (`deadCache.observer`). `sum` adds the finite heuristic values; `infinite` counts the others.

**The readers** (test code; `ref_trace_reader.py` is the authority). They are bounds-safe and exception-free, use the shared helpers (`readLines`, `splitWords`, `parseInteger`), and give exact problems:
- **`readTraceCases(lines)`** gives `{ok, cases, problem}`. Per case, in order:
  - `line <i>: expected a case header` (not exactly `case <name>`);
  - `case <k>: missing agent line` / `malformed agent line`;
  - `missing settings line` / `malformed settings line` (four integers ≥ 0);
  - `missing world line`; `missing world lines`; `world does not parse: <error>`;
  - `malformed belief line`; `missing belief lines`; `belief does not parse: <error>`;
  - `malformed model line` (the catalogue's exact forms);
  - `malformed notdead line`;
  - `malformed seed line` (unknown form, a bad value, or a second v1, committed or plan seed);
  - `missing end line`.

  Here k is the case's position, from 1.
- **`splitTraceBlocks(lines)`** gives `{ok, blocks (name, lines from case to end), problem}`, with the problems `line <i>: expected a case header` and `trace <name>: missing end line`.
- **`firstTraceDifference(name, expected, actual)`** gives the empty string, or `case <name>, <step or start>, line <k>: expected [<line>] actual [<line>]`. `<step or start>` is the last `step …` or `final` line before the difference, which names the decision step; the line itself names the field.

**G4 and G5.** For every case, in order, the test builds the agent from the case, runs it with the watchers, writes the trace (or the observer lines), and compares it with the golden block of the same name.
- The **first** difference fails the check and prints its message (M1.4: "the gate fails on the first difference; it prints the case, the step and the field").
- Different case counts or names are also failures.
- G4 passes only with all 789 traces identical, and G5 only with all 789 observer blocks identical.
- **The gate runs in every test run** (R20).

**SHA-256** (added to `docs/PROVENANCE.sha256` with the 49 original files: 111 → 163 preserved files):
```
27d69906449eec9336969c6850afa5a21819698134e1ba4574f1e27d0eda55ad  tools/reference/stage3/golden/c1_cases.txt
ce3679e3ec9c66d6ddaa3a52bac810b9c92f738bd8cbe919f0d4b0314ec35897  tools/reference/stage3/golden/c1_traces.txt
e38505b078b53342e6c3e2fa77482540c93836be47c63b67705aa305cc6ee0f1  tools/reference/stage3/golden/c1_observers.txt
```

### 3.15 Quirks kept on purpose (the baseline is frozen)
- **Q1. Cautious mode can never commit in a full run.** Model V1 work pauses only when `remaining() <= reserve`, and the cautious check needs `remaining() − need >= reserve`, with need ≥ 1. So in a run, unfinished V1 work always ends in the safe stop `investigation unfinished, no cautious action`, and none of the 789 traces commits a cautious action. This is the original's approved budget order (Phase B 11.7; original Stage 3 record). It is kept. `chooseCautiousAction()` stays public, and C1-6a tests it alone.
- **Q2. `current()` is a snapshot.** A model handed out before a restoration keeps the old marks. The predictor's lambda asks for `current()` at every evaluation, so the agent always predicts with the up-to-date model.
- **Q3. One dead-square map object.** `deadCache.get` recomputes into the same object, so a map reference taken at the start of the choice sees a trap override applied during that choice. The rebuilt code keeps a reference, exactly as the original does.
- **Q4. The trap V2 invalid-hypothetical branch cannot be reached.** p is never a wall or a border cell, and stand is checked to be free and differs from p. It is kept, and not claimed as tested (as in the original).
- **Q5. A mismatch record's step is the step of the check,** that is, the step after the committed action. This is the original's value, traced by the gate.
- **Q6. "No budget for a committed prediction"** can happen only with B = 0 and a plan already in place (the named case C1-11-nobudget).

## 4. Files
All new code is in namespace `aih`. Source files go into `ai_hologram_core`, test files into `ai_hologram_tests`. The sizes are estimates, including comments. **If a file cannot hold its code cleanly within 200 lines, or a function within 40, stop and report; never squeeze.**

| File | Approx. lines | Job | Sub-stage |
|---|---|---|---|
| `src/core/text_words.h` / `.cpp` | 30 / 35 | `positionText`, `blockedText` (moved, R4) | 3a |
| `src/compare/difference_text.cpp` | — | uses `text_words`; its two private copies are deleted | 3a (changed) |
| `src/agent/observation.h` | 35 | `Observation` (3.1) | 3a |
| `src/agent/footprint.h` / `.cpp` | 85 / 150 | 3.2, with `observedOutcome`, `footprintAfter`, `isFootprintMatch` | 3a |
| `src/agent/agent_log.h` | 85 | every log-line function (3.12) | 3a |
| `src/agent/agent_log.cpp` | 110 | the plan, step, safe-stop, V2 and result lines | 3a |
| `src/agent/agent_log_findings.cpp` | 140 | the investigation, correction, consequence and anomaly lines | 3a |
| `src/agent/directions.h` | 30 | `kActionOrder` | 3b |
| `src/agent/computation_observer.h` | 55 | `ComputationObserver` (H3, H4) | 3b |
| `src/agent/dead_squares.h` / `.cpp` | 85 / 150 | map, counts, `computeDeadSquares`, `readDeadStatus`, `localCheckSaysNotDead` | 3b |
| `src/agent/dead_square_cache.h` / `.cpp` | 60 / 60 | `DeadSquareCache` | 3b |
| `src/agent/heuristic.h` / `.cpp` | 45 / 60 | `kInfiniteHeuristic`, `manhattan`, `heuristic` | 3b |
| `src/agent/step_predictor.h` / `.cpp` | 115 / 150 | 3.4 | 3c |
| `src/agent/agent_model.h` / `.cpp` | 75 / 95 | 3.5 | 3c |
| `src/agent/planner.h` | 90 | the planner types, `search`, `fallback` | 3c |
| `src/agent/planner_internal.h` | 75 | the queue entry, its order, the search state and the helpers the planner files share (not used outside them) | 3c |
| `src/agent/planner.cpp` | 120 | `search`'s loop, `addEntry` | 3c |
| `src/agent/planner_expand.cpp` | 170 | finding, discovering, improving, `trySuccessor`, expanding | 3c |
| `src/agent/planner_fallback.cpp` | 90 | the one plan builder, `fallback` | 3c |
| `src/agent/records.h` | 120 | 3.7 records, and `reserveFor` (a short `inline` function, the one reserve rule) | 3d |
| `src/agent/history_checks.h` / `.cpp` | 50 / 80 | the three history checks | 3d |
| `src/agent/investigation.h` | 95 | investigation and consequence types and functions | 3d |
| `src/agent/investigation.cpp` | 185 | candidates, corrected belief, verifying, `investigateMismatch` | 3d |
| `src/agent/consequence.cpp` | 55 | `checkConsequence` | 3d |
| `src/agent/model_v1.h` / `.cpp` | 60 / 85 | model V1 | 3d |
| `src/agent/verification.h` | 90 | 3.8 declarations | 3d |
| `src/agent/verification.cpp` | 150 | `trapV1Check`, `trapV2Test`, `predictionsDiffer`, `activeTestTargets` | 3d |
| `src/agent/alternative_prediction.cpp` | 90 | `alternativePrediction` | 3d |
| `src/agent/agent_types.h` | 70 | `AgentSettings`, `Decision`, `PlanKind` | 3e |
| `src/agent/single_agent_state.h` | 130 | the state structs (3.9) | 3e |
| `src/agent/single_agent.h` | 130 | `class SingleAgent` | 3e |
| `src/agent/single_agent.cpp` | 150 | constructors, `decide`, `finalCheck`, counts | 3e |
| `src/agent/single_agent_records.cpp` | 150 | finding and adding records, the safe stop, applying and reverting | 3e |
| `src/agent/single_agent_check.cpp` | 150 | the check, settling, the mismatch record, retrying the deferred investigation | 3e |
| `src/agent/single_agent_investigate.cpp` | 160 | handling a result, its lines, the belief corrections | 3e |
| `src/agent/single_agent_pending.cpp` | 130 | pending work, the trap V2 attempt | 3e |
| `src/agent/single_agent_choose.cpp` | 170 | choose and commit, cautious, keep plan, commit | 3e |
| `src/agent/single_agent_active_test.cpp` | 150 | the active test | 3e |
| `src/agent/single_agent_search.cpp` | 140 | the main search, the consequence surprise, the fallback | 3e |
| `src/agent/run_loop.h` / `.cpp` | 95 / 160 | `RunResult`, the watchers, `runSingleAgent` and `continueRun` for C1 | 3e |
| `src/agent/naive_agent.h` / `.cpp` | 100 / 140 | C0 | 3f |
| `src/agent/run_loop_naive.cpp` | 80 | `runSingleAgent` for C0 (declared in `run_loop.h` from 3f) | 3f |
| `src/demo/agent_demo.h` / `.cpp` | 25 / 110 | `runAgentDemos` | 3f |
| `src/main.cpp`, `src/core/project_info.cpp` | — | the header (3.12), then the three demos; `currentStage()` returns 3 | 3f (changed) |
| `tests/agent_test_support.h` / `.cpp` | 86 / 196 | shared test helpers (R4): the demo world and its edited copies, the named layouts of Section 6, a line-list comparison that prints the first difference (3a); the search helpers shared by the two planner test files (3c). **Full since 3c: nothing more is added to it** | 3a (3c changed) |
| `tests/test_footprint.cpp` | 170 | FP1, FP10–FP13 | 3a |
| `tests/test_footprint_rows.cpp` | 190 | FP2–FP9 | 3a |
| `tests/test_agent_log.cpp` | 150 | LG1–LG7 | 3a |
| `tests/test_dead_squares.cpp` | 170 | DS1–DS5b | 3b |
| `tests/test_heuristic.cpp` | 150 | DS6–DS8d | 3b |
| `tests/test_dead_square_observers.cpp` | 150 | OB1–OB3 | 3b |
| `tests/test_step_predictor.cpp` | 190 | SP1–SP6, OB4 | 3c |
| `tests/test_agent_model.cpp` | 140 | AM1–AM5 | 3c |
| `tests/test_planner.cpp` | 180 | PL1, PL2a, PL2b, PL3, PL8–PL10 | 3c |
| `tests/test_planner_budget.cpp` | 150 | PL4a–PL7 | 3c |
| `tests/test_investigation.cpp` | 180 | IV1–IV7 | 3d |
| `tests/test_investigation_more.cpp` | 190 | IV9–IV11, VF1–VF3b, HC1, AT1, RC1 | 3d |
| `tests/test_model_v1.cpp` | 130 | VF4a–VF5 | 3d |
| `tests/test_verification.cpp` | 180 | VF7a–VF8c | 3d |
| `tests/test_trap_checks.cpp` | 180 | VF9a–VF13 | 3d |
| `tests/agent_test_fixtures.h` / `.cpp` | 60 / 170 | v1.3 (was `agent_test_seeds`, v1.2): the agent tests' shared inputs (R4): observations and histories (`makeObservation`, `twoObservations`, `eastHistory`); the hand-written test models (`pushNorth`, `alwaysBlocked`, `alwaysMovedEast`); and seeding a C1 state: wall corrections (`seed wall`), model V1 work with its history (`seed v1`), a plan in place, a committed action at step 1 (used by C1-6b/6c, C1-11, VF6, SP7, OB5, and by the trace writer in 3g). The copies of the first five in `test_investigation.cpp`, `test_investigation_more.cpp`, `test_model_v1.cpp`, `test_agent_model.cpp` and `test_step_predictor.cpp` are deleted in 3e, and those files use the shared ones | 3e |
| `tests/expected_demo_lines.h` / `.cpp` | 30 / 90 | the expected run lines of the two C1 demos (`C1_1`, `C1_2`), shared by C1-1, C1-2, OB6 and AD1 (R4) | 3e |
| `tests/test_single_agent.cpp` | 190 | C1-1, C1-10, C1-2, C1-5, C1-9, C1-12a, C1-12b | 3e |
| `tests/test_single_agent_cases.cpp` | 190 | C1-3, C1-4, C1-6a, C1-6b, VF5b, C1-6c-1, C1-6c-2, C1-7 | 3e |
| `tests/test_single_agent_budget.cpp` | 180 | C1-11a, C1-11b, VF6, SP7-B1…B11 | 3e |
| `tests/test_run_loop.cpp` | 150 | RL1, OB5, OB6 | 3e |
| `tests/test_naive_agent.cpp` | 190 | C0-1–C0-5, OB7 | 3f |
| `tests/test_agent_demo.cpp` | 110 | AD1 | 3f |
| `tests/trace_cases.h` / `.cpp` | 80 / 195 | `TraceCase`, `readTraceCases`, `splitTraceBlocks`, `firstTraceDifference` | 3g |
| `tests/trace_models.h` / `.cpp` | 40 / 150 | the model catalogue | 3g |
| `tests/trace_text.h` / `.cpp` | 60 / 190 | the state, observation, mismatch, correction and result text | 3g |
| `tests/trace_writer.h` / `.cpp` | 50 / 195 | building an agent from a case, running it with watchers, writing the trace or the observer lines | 3g |
| `tests/test_trace_reader.cpp` | 170 | TN1–TN15 | 3g |
| `tests/test_trace_gate.cpp` | 170 | MC1–MC5, TW1, G4, G5 | 3g |
| `tests/test_suites.h`, `test_main.cpp`, `test_project_info.cpp`, `test_layouts.h`/`.cpp` (if a layout helper is needed), `CMakeLists.txt` | — | new groups and files; P2 checks 3 from 3f | 3a–3g (changed) |
| `tools/reference/stage3/` | — | the reference, the golden files, the original-code tools (committed with this plan) | plan |

**91 new code files.** The code checker sees 79 → 92 (3a) → 103 (3b) → 116 (3c) → 132 (3d) → 153 (3e) → 160 (3f) → **170** (3g).

**One shared helper per job (R4):**
- `positionText` and `blockedText` live only in `text_words`;
- `observedOutcome`, `footprintAfter` and `isFootprintMatch` only in `footprint`;
- `kActionOrder` only in `directions.h`, and `manhattan` only in `heuristic.h`;
- `readDeadStatus` is the only dead-status read outside the analysis;
- the plan builder is only in `planner_fallback.cpp`;
- `trapHistoryPasses` only in `history_checks`, and `reserveFor` only in `records.h`;
- `objectDead` is one private helper of C1;
- every prediction goes through `StepPredictor`;
- the demo layout comes only from `demoLayout()`;
- the test text helpers come only from `golden_files`, the test worlds only from `agent_test_support` and `test_layouts`, the observations, histories, test models and the seeding of a C1 state only from `agent_test_fixtures`, and the demos' expected run lines only from `expected_demo_lines`.

**Splits planned in advance** (each file has one job):
- the original's 230-line `planner.cpp` becomes three files;
- its five `single_agent*.cpp` files become eight;
- `investigation.cpp` loses `checkConsequence` to `consequence.cpp`;
- `verification.cpp` loses `alternativePrediction`;
- the log text has two files.

## 5. Expected output
- **The program:** exactly 3.12, with 102 lines, 94 of them non-blank.
- **The tests:**

  | After | Expected line | New checks |
  |---|---|---|
  | 3a | `199 checks, 0 failures` | FP1–FP13 (15), LG1–LG7 (7) |
  | 3b | `215 checks, 0 failures` | DS1–DS8d (13), OB1–OB3 (3) … 16 |
  | 3c | `242 checks, 0 failures` | SP (8), OB4, AM1–AM5, PL (13) … 27 |
  | 3d | `277 checks, 0 failures` | IV (10), VF1–VF13 (22), HC1, AT1, RC1 … 35 |
  | 3e | `309 checks, 0 failures` | C1 (18), SP7-B1…B11 (11), RL1, OB5, OB6 … 32 |
  | 3f | `317 checks, 0 failures` | C0 (6), OB7, AD1 … 8 |
  | 3g | `340 checks, 0 failures` | TN1–TN15, MC1–MC5, TW1, G4, G5 … 23 |

  The 340 are the 177 Stage 2 checks (P2 now checks 3) and the 163 of Section 6. `checks.py` mirrors all 163: `failing: none (163 checks)`.
- **CTest:** 2/2. **Code checker:** 0 violations (the file counts above).
- **`run_checks.ps1`** ends with `ALL CHECKS PASSED (163 preserved files, 340 checks, 170 code files checked, formatting checked)`.

## 6. Tests
Each description starts with its id. Every fixture is a valid world unless the check is about invalid states. D is the demo layout; D2 is D with a wall at (4, 3). LP, L2, LT, LG, LTa, BOXED and SMALL are the layouts in `checks.py`, the same as the original's tests. `checks.py` holds the exact data of every check. Where this table and `checks.py` differ, `checks.py` wins, and Claude must be told.

### 6.1 Footprint and log text (3a)
| Id | Check |
|---|---|
| FP1 | From D, the 30 actions of `FP1_ACTIONS`, with a footprint update after each one, keep the belief equal to the true state after every step; 23 Moved, 3 Pushed, 4 Blocked |
| FP2–FP9 | Each row of the disagreement table (`FP_ROWS`): exactly the listed cells, both outcomes, and the listed differences in order, each with all six values (one check per row) |
| FP10 | Belief D + wall (2, 7), true D, East: cells (3,3), (4,3), no differences; after the update the agent and object are the observed ones, the footprint walls are observed, and (2, 7) is still believed blocked |
| FP11a | Two-prediction footprint: the beliefs D with the agent at (4,3), and the same with the object at (5,4); outcomes Pushed and Moved. Cells (4,3), (5,3), (6,3), the same with the predictions swapped |
| FP11b | Two Moved predictions: cells (4,3), (5,3) only |
| FP12a | Cells outside the grid are walls in both states: no differences, and the update changes nothing |
| FP12b | Agent (1,3), West: cells (1,3), (0,3); outcome Blocked |
| FP13 | `footprintAfter` uses the observation's outcome (a Pushed observation gives three cells); `observedOutcome` of an observation without one is Blocked; with Blocked, North from D gives (3,3), (3,2) |
| LG1–LG7 | Every function of 3.12 gives exactly the lines in `LG_EXPECTED` for the inputs in `lg_lines()`: plan lines; step and safe-stop lines; V2 lines; investigation lines; correction lines; consequence lines; result lines |

### 6.2 Dead squares, heuristic and the H4 observer (3b)
| Id | Check |
|---|---|
| DS1 | D: (1,1) is dead; the destination (8,3) is not dead and has distance 0; no wall is dead |
| DS2a | D: every cell matches `DS2_GRID` (walls, dead cells with distance −1, the distances); exactly 22 dead cells |
| DS2b | D: one analysis run and exactly **160** cell operations (40 reached cells × 4) |
| DS3a | L2: the object cell (2,2) is not dead, with distance 3 |
| DS4 | D with "not dead" at (1,1): the local check is false at (1,1), true at (5,3) and at (8,3); `localChecks` is 3 |
| DS5a | That override changes exactly one cell, (1,1), and no distance |
| DS5b | A dead correction on the same cell wins (it is applied last) |
| DS6 | `DeadSquareCache` analysis runs after six `get`s (D; D; D with the agent moved; D + wall (2,7); the same with "not dead" (1,1); the same again): 1, 1, 1, 2, 3, 3 |
| DS7 | The local check and the heuristic need no model and no budget; on D they give true and 4 (and a counting model nearby is never called) |
| DS8a | heuristic: D gives 4; the agent at (4,3) gives 3 |
| DS8b | the object on (1,1) gives `kInfiniteHeuristic` |
| DS8c | with "not dead" (1,1), the agent at (5,5) and the object at (1,1): 16 (9 + 7, the manhattan fallback) |
| DS8d | `heuristicCalls` counts each call (3, and 1 on the second counts) |
| OB1 | `readDeadStatus` returns the stored values (true at (1,1), false at (5,3)) with and without an observer; with one, it reports exactly (1,1) true, then (5,3) false |
| OB2 | With an observer on D's map: the heuristic of D, then of D with the object at (1,1), then the local check at (2,2) report exactly `OB2_EVENTS` (six reports, in order) |
| OB3 | `DeadSquareCache::get` sets `map.observer` to the cache's observer on every call (none, then the attached one), and the map's dead values are the same |

### 6.3 Step predictor, the H3 observer, the agent model and the planner (3c)
| Id | Check |
|---|---|
| SP1 | B 10: an uncached prediction costs 1 (remaining 9) and is then cached; repeating it is free and gives the same outcome and state |
| SP2a | `startStep()` clears the cache; the same prediction costs again |
| SP2b | A state with other walls is not served from the cache and costs 1 (used 2), and is then cached; `clearCache()` empties the cache |
| SP3 | After `startStep()`, remaining is B (5), used 0, `usedTotal` 1 |
| SP4a | B 1: the second prediction is refused, nothing is charged or cached |
| SP4b | A variant is refused too when the budget is used up |
| SP5 | A variant (always Blocked) ignores and never fills the cache and always costs 1; the ordinary entry stays (East Moved, then 4 used, South not cached) |
| SP6 | A counting model: two calls for a miss, a hit and a variant; used 2, `usedTotal` 2 |
| OB4 | With an observer, B 3: East (own), East again (a hit: no report), North as a variant, South (own), West (refused: no report) report exactly three evaluations: East own Moved, North variant Blocked, South own Moved; `usedTotal` 3 |
| AM1 | `AgentActionModel()` is the standard rule (an East push from (4,3) moves the object to (6,3)) |
| AM2 | With the push-north base: not restored, and the push moves the object to (5,2) |
| AM3 | After `restore(East)`: the standard rule; `base()` still pushes north; a model taken before the restoration still pushes north (a snapshot) |
| AM4 | `unrestore(East)`: back to the base model |
| AM5 | `setBase` (H2) to an always-Blocked model keeps the East mark (East still standard, West Blocked); `setBase` with an empty model is the standard rule (West Moved) |
| PL1 | D, B 200, reserve 1: Found, East ×4, 16 evaluations; the last expected state has the object on (8,3) |
| PL2a | LP, B 1000000: Found, the 9-action plan W, W, W, N, W, N, E, E, E, 60 evaluations, 1 improved path, 0 reopened |
| PL2b | SMALL: Found, East, East |
| PL3 | In PL1, PL2a, PL2b, PL6 and PL7, no discovered state has its object on a dead square |
| PL4a | D2, B 10, reserve 1: OutOfBudget, 9 used, 1 remaining |
| PL4b | D2, B 10, reserve 3: OutOfBudget, 7 used, 3 remaining |
| PL5a | The fallback of both is North, East |
| PL5b | D2, B 1, reserve 1: only the root is discovered; the fallback is empty |
| PL6 | L2, B 1000000: Exhausted, 156 evaluations, 39 states (the search part of DS3) |
| PL7 | D2 with the agent at (7,1), the walk goal to (4,4), (3,3) or (4,2): South, West, West, West to (4,2), 16 evaluations |
| PL8 | D and LP searched twice: identical kinds, plans and counts |
| PL9 | With a counting model, the model calls equal the charged evaluations (D and LP) |
| PL10 | LG (object already on the destination): heuristic 4; Found with an empty plan and 0 evaluations |

### 6.4 Records, investigation and verification (3d)
| Id | Check |
|---|---|
| IV1 | Belief D, true D2, East: BeliefCandidatesConsistent, [wall (4,3) blocked], S2, no anomaly, 1 evaluation |
| IV2 | The agent at (4,3), the belief with a wrong wall at (6,3): [wall (6,3) free], S2 |
| IV3 | Believed object at c1 but not there (true object (5,4)): [object (5,4)], read from *before*, outside the footprint (also IV8), S2 |
| IV4 | Object at c1 but believed at (5,4): [object (5,3)], S2 |
| IV5 | Belief = truth, the push-north model: ModelSuspected, no candidates, S1, 0 evaluations |
| IV6 | A wrong wall at (6,3) and a wrong object: [wall (6,3) free, object (5,3)], verified together with 1 evaluation, S2 |
| IV7 | A current model under which East always "moves": NoCandidatePassed, S5 |
| IV9 | A believed agent that differs from *before*: the anomaly flag |
| IV10 | B 1, reserve 1: Deferred, 0 evaluations |
| IV11 | A belief whose object (5,4) differs from the record's (5,3), with a Blocked East record: the object candidate's re-prediction does not match the observation, so NoCandidatePassed, S5, 1 evaluation |
| VF1 | `objectHistoryCheck` against o2 of the East history: (6,3) passes, (5,3) fails |
| VF2 | The 4-entry East history with the belief's object wrongly at (6,4): [object (6,3)] is consistent, although earlier entries had it elsewhere (time-aware V1) |
| VF3a | `wallHistoryCheck((4,3), blocked)` is true when every entry has the wall |
| VF3b | It is false when the first entry lacks it |
| VF4a | `startModelV1(East)` gives 1, 2, 3; West gives none |
| VF4b | The replays are Done with 3 variant evaluations, and the predictor's own model is never called |
| VF4c | A recorded state altered by hand makes it Failed |
| VF5 | B 3, reserve 1: 2 replays, Pending with [3] left; after `startStep()`, Done with 1 more |
| VF7a | The wall correction at (4,3) gives an alternative for East (1 evaluation), none for North (0) |
| VF7b | A model(East) correction: one for East, none for West |
| VF7c | An object correction (old (5,4)), the agent at (4,4): an alternative for East, Pushed, agent (5,4), object (6,4), 1 evaluation; none for West |
| VF8a | The investigation's re-prediction calls the **current** model exactly once |
| VF8b | A model correction's alternative calls the **old** model once and never the current one |
| VF8c | `predictionsDiffer`: identical, no; a different outcome, yes; a different agent, yes |
| VF9a | LT with "not dead" (1,1): `trapV1Check((1,1))` is true |
| VF9b | `trapV2Test` is Verified with exactly 2 evaluations through the current model (North, then West) |
| VF9c | B 2, reserve 1: Waiting, 0 evaluations |
| VF9d | D at (5,3): `trapV1Check` is false and `trapV2Test` is Rejected |
| VF10 | No anomaly in VF9b, and both hypothetical states are valid |
| VF11 | L2: `checkConsequence` gives FullStateDeadEnd |
| VF12 | LT with the override: Mismatch (labelled S1 by 3.9) |
| VF13 | NotApplicable when a later entry has the object on the destination, when p is dead, and when the object is on the destination (LG) |
| HC1 | `trapHistoryPasses`: true for LT alone; false when a later entry has the object on (8,7); true when that entry comes first |
| AT1 | `activeTestTargets(D2-belief, (4,3))` is exactly (4,4) North, (3,3) East, (4,2) South |
| RC1 | A new correction record's defaults (id 0, wall, Consistent, untested, step 0); an `InvestigationResult`'s defaults (ModelSuspected, S5) |

### 6.5 C1 and the run loop (3e)
| Id | Check |
|---|---|
| C1-1 | Demo 1 (D, B 200, S_max 5, T_max 100, T_v 3): the run lines are exactly `C1_1` (19 evaluations) |
| C1-10 | Its 5th line is `Step 4: … match`: the last step is logged by the final check; `decisionStep` 4 |
| C1-2 | Demo 2: the run lines are exactly `C1_2` (414 evaluations), with the active-test line at index 8 (also C1-8) |
| C1-3 | D, LP and D2 with correct beliefs: Success, the belief equals the observation after every check, and no mismatch |
| C1-4 | D: the history holds the 5 delivered observations (4 decisions and the final check) unchanged and in order |
| C1-5 | LT, S_max 2: exactly `C1_5`; `decisionStep` 2; the second observation has no last action |
| C1-6a | `chooseCautiousAction()` alone on D, after `startStep()`, picks North with 2 evaluations |
| C1-6b | B 2, V1 work for East with 2 replays: the first decision safe-stops `investigation unfinished, no cautious action`, with [2] left |
| VF5b | The next decision finishes the work: no V1 work, 1 correction, East restored, and its line logged |
| C1-6c-1 | B 2, S_max 2, V1 work with 3 replays: exactly `C1_6C_2`; [3] left; East not restored |
| C1-6c-2 | The same with S_max 5: exactly `C1_6C_5`; East restored |
| C1-7 | In demo 1 and demo 2, every world action has a committed prediction, and no step charges more than B |
| C1-9 | The push-turn model (`pushturn East North`), T_max 6: exactly `C1_9` (798 evaluations) |
| C1-11a | Deferred: D2, belief D, B 1, a committed East at step 1: two `budget too small for verification` stops, the deferred line, 1 mismatch, 0 corrections |
| C1-11b | B 0 with a plan in place: the safe stop `no budget for a committed prediction` |
| C1-12a | LG: Success after 0 world steps, 0 evaluations, `decide` never called |
| C1-12b | A belief with the object on the destination, S_max 2: exactly `C1_12B` |
| VF6 | Belief D + wall (4,3), true D, with a seeded wrong wall correction: exactly `VF6_LINES` (216 evaluations; REJECTED, then a new correction) |
| SP7-B1…B11 | D with three seeded wall corrections ((4,3), (4,2), (3,4): new free, old blocked), S_max 3, T_max 30, for each B: the outcome, world steps and safe stops of `SP7_EXPECTED`; at B ≤ 4, three `budget too small to plan` stops and no world step; no step over B; every action committed (11 checks) |
| RL1 | Watchers on demo 2: 18 decisions and 1 final check seen (the observation of step 19); outcome, steps and log identical to a run without watchers |
| OB5 | The activation hook on demo 2 is called once per decision step (1…18). At step 2 the log already has the check's 4 lines but not the new plan line, and planKind is still Goal. In the deferred case it is called once (step 2) |
| OB6 | Observers attached to demo 2: the run lines are still exactly `C1_2`, and 414 evaluations are reported |

### 6.6 C0 and the demonstrations (3f)
| Id | Check |
|---|---|
| C0-1 | D2, belief D: over 20 decisions, an agent given the true observations and one given the first observation every time choose the same actions and use the same evaluations |
| C0-2 | D: East ×4; `usedThisStep` 16, 0, 0, 0; one search |
| C0-3a | LTa: the first decision is East with 4 evaluations and one search; the belief's agent is at (6,1) |
| C0-3b | BOXED: North with 4 evaluations; the belief is unchanged |
| C0-4 | D: the run lines are exactly `C0_4` (16 evaluations) |
| C0-5 | D2, belief D: exactly `C0_5` (Step limit after 100, 207 evaluations), 97 searches, and the first ten actions of `C0_5_FIRST_TEN` |
| OB7 | C0's activation hook on D is called at the start of each step: (step, plan size, used) = (1,0,0), (2,3,0), (3,2,0), (4,1,0) |
| AD1 | `runAgentDemos` writes exactly `AD1_TEXT` (46 lines) |

### 6.7 The equivalence gate (3g)
| Id | Check |
|---|---|
| TN1–TN12 | `readTraceCases` on `READER_CASES` gives exactly the expected (ok, cases, problem); TN12 is a positive control with two cases |
| TN13, TN14 | `splitTraceBlocks` on `SPLIT_CASES` |
| TN15 | `firstTraceDifference` gives exactly `case x, step 1, line 3: expected [decision East] actual [decision West]` |
| MC1–MC5 | The catalogue models `stuck`, `nopush`, `slip`, `swap` and `pushturn` give exactly the results in `check_models` |
| TW1 | The walls of D are `ffe0180601806018c601807ff`, and D's state text is exactly `agent (3, 3) object (5, 3) destination (8, 3) walls ffe0180601806018c601807ff` |
| G4 | All 789 cases of `c1_cases.txt`, traced by the rebuilt agents, equal `c1_traces.txt` |
| G5 | All 789 observer blocks equal `c1_observers.txt` |

A missing golden file is a FAIL with a clear message. The golden files are found through `AIH_SOURCE_DIR`.

## 7. Implementation order
| Step | Sub-stage | Content | Tests (total) | Deliberate failure checks (tried by Claude first, R11; `df_check.py`) |
|---|---|---|---|---|
| 1 | — | The owner commits this plan, `tools/reference/stage3/`, the provenance update (163), the decisions and the 3a prompt, on a new branch `stage-3-work` | | **Owner, DF3-P:** add one character to `c1_traces.txt`. It must stop at `[provenance]`; restore it with `git checkout`. |
| 2 | **3a** | `text_words`, observation, footprint, log text | FP, LG (199) | **Cursor, DF3a-1:** the footprint forgets rule (iii). Exactly FP7 must fail. |
| 3 | **3b** | directions, observer type, dead squares, cache, heuristic | DS, OB1–3 (215) | **Cursor, DF3b-1:** the overrides are applied in the other order. Exactly DS5b must fail. |
| 4 | **3c** | step predictor, agent model, planner | SP, OB4, AM, PL (242) | **Cursor, DF3c-1:** the search's reserve test becomes `remaining() < reserve`. Exactly PL4a, PL4b, PL5a and PL5b must fail. |
| 5 | **3d** | records, history checks, investigation, consequence, model V1, verification | IV, VF, HC1, AT1, RC1 (277) | **Cursor, DF3d-1:** the investigation checks an object candidate against `history.front()` instead of *before*. Exactly VF2 must fail. |
| 6 | **3e** | C1's decision step, the run loop with watchers | C1, SP7, RL1, OB5, OB6 (309) | **Cursor, DF3e-1:** `reserveFor` always returns 1. Exactly C1-2, C1-6c-2, VF6, SP7-B2–B7, SP7-B10 and OB6 must fail (11). |
| 7 | **3f** | C0, the C0 run loop, the agent demonstrations, main, `currentStage()` = 3, README | C0, OB7, AD1 (317) | **Cursor, DF3f-1:** C0 clears its plan at the start of every decision, so it searches every step. Exactly C0-2, C0-4, C0-5 and OB7 must fail. |
| 8 | **3g** | the trace readers, the model catalogue, the trace text and writer, the gate | TN, MC, TW1, G4, G5 (340) | **Cursor, DF3g-1:** after a consequence mismatch, the trap V2 test is left waiting for the next step instead of being tried at once. **Exactly G4 and G5 must fail:** only the gate catches it. **Owner, DF3g-2:** in `agent_log.cpp`, change `"Plan: active test of correction "` to `"Plan: active check of correction "`. `run_checks.ps1` must stop at `[tests]` with exactly LG1, C1-2, VF6, OB6, AD1 and G4 failing. |
| 9 | — | Fresh clone of `stage-3-work`: `run_checks.ps1` passes | | |
| 10 | — | ChatGPT reviews the **whole buildable tree** of the final commit; the completion gate; tag `stage-3-v2`; merge into `main` | | |

Each sub-stage is committed by the owner on `stage-3-work`, after Claude's review. **`c1-baseline-v2` is not tagged in Stage 3** (M1.3; Section 1).

## 8. Decisions to record (with the plan's commit, R12)
| ID | Decision | Reason |
|---|---|---|
| D-068 | The rebuilt C1 and C0 reproduce the original `src/agent` at `89ea9b6`: the `stage-3` behaviour plus the IE-1 hooks H1–H4, designed in from the start (R5). Namespace `aih`. The code is split into the files of Section 4, each with one job. One helper per job (R4): the text helpers move to `src/core/text_words`; the two copies of `footprintOf` and of `trapHistoryPasses` become one each; one direction order; one `manhattan`; one plan builder; one reserve rule. The original's quirks are kept (3.15). | M1.3 defines the baseline as that behaviour; the gate (M1.4) checks it. The rules R1, R2 and R4 need smaller, single-job files. |
| D-069 | The equivalence gate, part 1: 789 fault-free runs are traced once from the original code. The traces come from `7587f38` and are identical from `89ea9b6`; the observer counts come from `89ea9b6`. They are committed with their hashes, together with the 49 original source files (163 preserved files). G4 and G5 replay them in every test run and fail on the first difference, naming the case, the step and the field. The fault cases and the Stage 4 scores are added in Stage 4. Only then can the gate pass in full and `c1-baseline-v2` be tagged. | M1.4, R20. The method of D-061 and D-067, applied to a whole agent. |
| D-070 | **Synthetic verification fixtures for the gate, and the R21 clarification.** Besides the named cases and the demonstration seeds, the gate uses 730 runs on **deterministically generated synthetic verification fixtures** (`gen_golden.py`, an independent generator, seed 20261022). No experiment generator, pilot or confirmatory seed, layout or run is used, and no comparative result is computed. R21 is clarified, in ChatGPT's words, approved by the owner: "During rebuilding, deterministic synthetic verification cases may be generated using an independent generator, provided no pilot or confirmatory seed, layout, run or reserved experimental material is used. These cases are for implementation verification only, are not experiment results and do not alter the frozen protocol." The restriction on pilot and confirmatory material is not weakened. | M1.4's named cases alone exercise only part of C1 (for example, no unexplained update, few traps). The fixtures cover every reachable path (3.14). |
| D-071 | The run loop gets watch-only watchers (`RunWatchers`, `NaiveRunWatchers`) and `continueRun`. They change nothing about how a run behaves (RL1). The original-code tool uses a copy of the original loop and checks every case against the original `runSingleAgent`. | The gate needs the agent's state after every decision, and one named case starts after a seeded action. |
| D-072 | Two branches of the original cannot be reached in a run, and are kept and documented, not changed: a cautious commit (3.15 Q1) and the invalid-hypothetical skip of the trap V2 test (Q4). Neither is claimed as covered by the gate. | The baseline is frozen (M1.3). Changing either would change C1. |

## 9. Documentation
- **Headers and comments.** Every new file has the three-part header (Introduced in: Stage 3x (rebuild)). Beginner comments explain:
  - the open list's order, and `std::priority_queue` with a comparator;
  - why every successor state comes from the step predictor;
  - why the cache's context includes the walls;
  - what a variant prediction is;
  - what the reserve protects;
  - the footprint rule, and why C1 does not re-check its whole belief (Phase B 12.1);
  - a dead square versus a full-state dead end;
  - why the activation hooks are empty in Stage 3, and why the observers can only watch (R6: "the chain can be replaced by the camera");
  - `std::function` members as hooks.
- **README** (3f). The status becomes: "Stage 3 of the rebuild: the competent single agent C1 and the naive agent C0, checked step by step against 789 runs recorded from the original code (the first part of the M1.4 equivalence gate). No faults are injected, no two-agent reasoning exists, and no experiment has been run."
- **Handoff notes.** One per sub-stage. **Review notes.** Every exchange with ChatGPT goes in `docs/reviews/stage3/`.

## 10. Completion checklist (owner verification gate)
- [ ] The plan was agreed and committed before any Stage 3 code.
- [ ] `run_checks.ps1` passes on the owner's computer **and on a fresh clone**, with the exact final line of Section 5.
- [ ] The program output is exactly 3.12, at 102 lines; the tests show `340 checks, 0 failures`; CTest 2/2; 0 checker violations (170 files).
- [ ] G4 and G5 pass all 789 cases, and TN1–TN15 pass.
- [ ] All deliberate failure checks gave exactly the predicted failures.
- [ ] D-068 to D-072 recorded (D-070 with the owner's approval of the R21 clarification); handoff notes appended; prompts and review notes saved.
- [ ] ChatGPT reviewed the whole buildable tree of the final commit.
- [ ] The owner agrees (D-065).

## 11. Plain explanation (for any reader)
1. **C1 is the strong single agent** the two-agent idea must beat. It plans, predicts, acts, checks its prediction against what it sees, and investigates and corrects when they disagree.
2. **It checks only where its action reaches** (the footprint): a few cells per step, not the whole board.
3. **Every prediction costs one unit of a fixed budget per step**, and C1 always keeps enough budget to check its next action.
4. **The 789 recordings are a film of the original agent.** The new agent must play every frame identically: every decision, prediction, log line, record and budget count.
5. **The "camera" hooks are built in now, switched off.** Stage 4 will attach the camera without changing the agent.

## 12. What actually happened (filled in at the end)
```
Date completed:
Commit tagged stage-3-v2:
Checks:
Did everything go as planned?
```

## 13. Instructions to the coding assistant
Every prompt says:
- follow `AGENTS.md` and `.cursor/rules`;
- implement only the named sub-stage;
- read the reference (`tools/reference/stage3/`) for exact behaviour and test data; it follows the original function by function;
- never commit, and never change an expected or golden value; report any mismatch with its id or case;
- never squeeze code to fit a limit; stop and report instead;
- format with CLion's clang-format;
- run `run_checks.ps1` and report its final line exactly.

## Revision history
| Version | Date | Change |
|---|---|---|
| 1.0 | 2026-10-21 | First draft. Everything was built first (R9, R11):<br>- the golden traces, from the original C++ at both commits;<br>- the reference, which reproduces all three golden files byte for byte;<br>- the 163-check mirror; the values of the new checks were confirmed against the original C++ (`orig_trace/unit_values_output_2026-10-21.txt`);<br>- the deliberate failure predictions.<br>The agent-demo text was checked against the original. |
| 1.1 | 2026-10-21 | ChatGPT approved the v1.0 technical design (review 02): D-068, D-069, D-071 and D-072 accepted; the seven sub-stages, the file organisation and the 3a prompt accepted; independently verified (177 C++ checks, the 163 reference checks, all eight deliberate failure predictions, the golden hashes). D-070 accepted in principle: the synthetic cases are now called deterministically generated synthetic verification fixtures, and ChatGPT's narrow R21 clarification is recorded with the owner's approval. No other change. |
| 1.2 | 2026-10-21 | After 3c (242 checks; Claude also compared the new planner with the reference on 3,156 searches over the 789 case worlds: identical). Cursor put the search helpers shared by the two planner test files into `tests/agent_test_support` (R4), which is now at 196 lines, so the seeding helpers planned for it in 3e get their own file, `tests/agent_test_seeds.h/.cpp`. 91 new code files; the final line says 170 code files. No change to behaviour, checks or golden values. |
| 1.3 | 2026-10-21 | After 3d (277 checks). An R4 finding in Claude's review: five small test helpers are written twice. `makeObservation`, `twoObservations` and `eastHistory` are in two 3d test files; `pushNorth` and `alwaysBlocked` are in a 3c and a 3d file (`agent_test_support` was full, and Cursor kept them local). In 3e, the file planned in v1.2 for the seeding helpers becomes `tests/agent_test_fixtures.h/.cpp` and also holds these five; the copies are deleted. No change to behaviour, checks, counts or golden values. |
