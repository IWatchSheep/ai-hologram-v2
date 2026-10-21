# Stage 4 (rebuild) — Fault injection and measurement, and the rest of the equivalence gate

| | |
|---|---|
| **Project** | AI Hologram v2 (`ai-hologram-v2`) |
| **Stage** | 4 of the rebuild (Brief Section 10, Stage 4: fault injection and measurement) |
| **Plan version** | 1.1 (after ChatGPT's review 02: two documentation clarifications) |
| **Date** | October 21, 2026 |
| **Status** | **APPROVED by ChatGPT** (review 02: "APPROVE THE STAGE 4 TECHNICAL DESIGN, SUBJECT TO TWO DOCUMENTATION CLARIFICATIONS AND THE OWNER'S EXPLICIT APPROVAL"; D-073 to D-080 accepted, D-077, D-079 and D-080 with conditions recorded here). v1.1 makes the two clarifications. **Awaits the owner's agreement** (D-064, D-065). |
| **Governing documents** | The brief v1.1; the frozen protocol v1.0 (Phase A 4.2–4.4 and 5, Phase B 12.2–12.6, Sections 13.8, 13.12 and 14; amendments A3 and A4); **Migration Addendum M1** (M1.3, M1.4, M1.5); `docs/RULES.md` (R1–R24); `AGENTS.md`; decisions D-030 to D-045 of the original (carried over, unchanged), D-064 to D-072. |
| **Builds on** | Stage 3, complete and tagged `stage-3-v2` (`ec4613f`). |
| **Original it rebuilds** | `IWatchSheep/ai-hologram`, Stage 4 (plan `docs/history/ai-hologram/docs/stages/STAGE_04.md` v1.2.2), **code-complete at `89ea9b6`** (618 checks; the original's Stage 4 gate and `stage-4` tag were never done, Section 12 of that plan). Its `src/experiment/` and `src/demo/fault_demo.*` are what this stage rebuilds. |
| **Reference** | `tools/reference/stage4_rebuild/` (R9, R10): the Python reference (`ref_material.py`, `ref_faults.py`, `ref_scorer.py`, `ref_fault_csv.py`, `ref_fault_demo.py`, `ref_fault_trace.py`, `ref_fault_reader.py`), the check mirror `checks.py` (195 checks), `df_check.py`, `gen_fault_cases.py`, two golden files produced by a tool built from the **original C++** (`orig_trace/`), and the expected program output. It builds on `tools/reference/stage3/` (C1, C0 and their trace text). The carried-over original Stage 4 reference stays unchanged in `tools/reference/stage4/`. |

---

## 1. Purpose
Stage 4 rebuilds the experiment's **measuring instrument**, for C0 and C1 (Brief, Stage 4: "the ability to introduce controlled, logged errors into an agent's beliefs or reasoning, and to measure detection and recovery"):
1. **Material.** Layouts and fault-event lists, generated from fixed seeds, so that anyone can regenerate exactly the same experiment.
2. **Faults.** Each scheduled fault is applied to a reasoner at its fault activation point, exactly as Phase B 12.2–12.3 and amendments A3 and A4 say, and what happened to every event is recorded.
3. **Measurement.** Each run is scored from the agent's own records (Phase B 12.5), by code the agent never sees.
4. **Output.** One CSV row per run and one per fault event.

It rebuilds the original Stage 4 **exactly**: the same draws, the same faults, the same scores, the same text. The original's plan and its readings D-030 to D-045 are carried over unchanged; this plan only changes how the code is organised (R1–R4) and how the run with faults is connected to the agent (3.6).

It also builds the **second part of the M1.4 equivalence gate**: 563 fault runs recorded from the original C++ at `89ea9b6`, replayed decision step by decision step, with every activation record and every Stage 4 score compared (3.15). **When this gate passes, the whole of M1.4 has passed**, and `c1-baseline-v2` is tagged at Stage 4's completion, before any two-agent code exists (M1.3 items 2 and 3; Section 10).

## 2. What Stage 4 must NOT contain
- **No message protocol, second reasoner or two-agent code** (Stages 5–6).
- **No experiment runner, no batch runs over the pilot or confirmatory sets, no comparison statistics** (Stages 7–8).
- **No agent is run on a pilot layout (900000–900039) or a confirmatory layout (100000–100399)** (R21). Generating their *material* (layouts and event lists) is allowed, so its balance can be checked (LY5, FE2, FE3, as in the original; D-080); nothing is run on it. Agent runs use hand-built worlds, the demonstration seeds 500000–500009, and the synthetic verification fixtures of D-070.
- **No change to C1's or C0's decisions.** The only changes to Stage 3 code are the two of 3.12 (the run loop's `onObservation` watcher and the shared `actionIndex`), each proved by the gate to change nothing.
- **No result files** are written into the repository; the demonstration prints its CSV lines to the screen.
- No exceptions, assertions that stop the program, inheritance, threads, external libraries, `<random>` or `rand()`.

If anything here seems necessary, **stop and ask the owner**.

## 3. Exact behaviour (the original's, unchanged)
The Python reference follows the original C++ function by function and draw by draw. **Where these words and the reference disagree, the reference and the golden traces win, and Claude must be told.** Every value below was produced by the reference and checked against the original C++ (`orig_trace/`). The original plan (`docs/history/.../STAGE_04.md`) explains each rule at greater length.

### 3.1 Seeded random numbers [Brief Stage 4; D-030]
`SeededRandom` is **SplitMix64**. Its state is one `std::uint64_t`; all arithmetic wraps modulo 2⁶⁴.
```
next():  state += 0x9E3779B97F4A7C15; z = state
         z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9
         z = (z ^ (z >> 27)) * 0x94D049BB133111EB
         return z ^ (z >> 31)
below(n), n >= 1:  threshold = (0 - n) % n   (unsigned 64-bit)
                   repeat r = next() until r >= threshold; return r % n
shuffle(list):     for i = size-1 down to 1: j = below(i+1); swap(list[i], list[j])
pickDistinct(list, k): for i = 0 to k-1: j = i + below(size-i); swap(list[i], list[j]);
                   return the first k, in that order
```
`below(1)` returns 0 and uses one `next()`. Exact values: seed 0 gives 16294208416658607535, 7960286522194355700, 487617019471545679; seed 1234567 gives 6457827717110365317, 3203168211198807973, 9817491932198370423; seed 42 gives ten `below(10)` 3, 1, 8, 4, 0, 2, 5, 8, 5, 4, and shuffles 1..10 to 1, 10, 6, 9, 7, 5, 8, 3, 2, 4; seed 7 `pickDistinct(1..10, 3)` gives 8, 1, 5.

### 3.2 The experimenter's search and the layout generator [Section 14; D-031]
- **`shortestSolutionLength(world)`**: breadth-first search over **true** (agent, object) states with the Stage 1 rules, actions in the order North, East, South, West, skipping Blocked and InvalidState. It returns the number of world steps of a shortest solution, 0 when the object is already on the destination, or none. It is experimenter code (`src/experiment/`), never seen by an agent, never charged to a budget.
- **`generateLayout(layoutSeed)`**, with `SeededRandom random(layoutSeed)`, up to 10,000 attempts: border cells are walls (no draw); every interior cell, row by row, is a wall when `random.below(100) < 12`; the free cells, row by row; fewer than 3 → next attempt; (agent, object, destination) = `random.pickDistinct(free, 3)`; accepted only if the world is valid, the shortest solution is 12–40, and at least one interior cell other than the destination is a true dead square (`computeDeadSquares` with no overrides). The result is `{found, world, attempts, shortest}`.
- Exact values: 500000: attempts 3, shortest 12, agent (7, 2), object (6, 5), destination (2, 6), 25 dead cells, 7 interior walls; 500001: 6, 16, (8, 1), (4, 5), (8, 4), 23, 9; 500002: 1, 29, (4, 1), (4, 7), (8, 5), 27, 8; 500003: 3, 14, (1, 1), (4, 5), (7, 4), 26, 7; 900000: 2, 17, (7, 1), (7, 6), (2, 8), 19, 7; 900001: 4, 19, (4, 3), (3, 7), (5, 6), 26, 11. Layouts 500004–500009: attempts 3, 1, 3, 8, 5, 2; shortest 20, 16, 12, 12, 12, 16. Layout 500000 renders as in Section 5.

### 3.3 The fault-event list [Phase A 4.2, Phase B 12.2, Sections 13.8 and 14; D-032, D-033]
Generated **once per experiment**, identical for every condition. Input: the runs `(layoutSeed, runSeed)` in order (layout seed, then run seed), their true layouts, and the **fault master seed 4242**.
- **Experiment-level draws** (`SeededRandom experimentRandom(4242)`): with R runs, n = 3R events (run r holds 3r, 3r+1, 3r+2); types F(i mod 5 + 1), shuffled once; then for each type F1…F5: `idx` its event numbers ascending, `perm` a shuffled copy, k = `commonModeCount(count)` = (2·count + 5) div 10; the first k of `perm` are common-mode (target `both`), the rest alternate R1, R2, …; for F1 only, `perm2` = another shuffled copy of `idx`, alternating free→blocked, blocked→free, ….
- **Per-run draws** (`SeededRandom runRandom(runStreamSeed)`, `runStreamSeed = (4242 << 40) + (layoutSeed << 8) + runSeed`): the three steps `pickDistinct(1..10, 3)` sorted ascending; then for each event in order: **F1** a cell from the interior cells (y 1..8 outer, x 1..8 inner) other than the destination whose true status matches the flip (`candidates[below(size)]`, or no cell and no draw); **F2** a shuffled order of (N, E, S, W); **F3** an action `(N, E, S, W)[below(4)]` then a rotation `(left, right)[below(2)]`; **F4** a true dead interior cell other than the destination, or no cell and no draw; **F5** nothing.
- **Identifier** `<layoutSeed>-<runSeed>-<j>`. **Text** (`faultEventText`): `<id> step <s> <type> target <R1|R2|both> | <details>`; details `(x, y) free->blocked` / `none blocked->free` (F1), the four directions (F2), `East right` (F3), `(x, y)` or `none` (F4), `-` (F5).
- Exact values: the 24 events of the demonstration experiment (500000–500003 × run seeds 1–2) are those of the original plan 3.3 (also in Section 5's demonstration); the pilot and confirmatory counts are those of the original plan 3.3 (FE2, FE3).

### 3.4 Applying a fault [Phase B 12.2, A3; D-033 to D-036]
The fault injector (experimenter code) applies one event to one reasoner's parts: belief, action model, "not dead" overrides and predictor (`ReasonerParts`, `partsOf(SingleAgentState&)`, `partsOf(NaiveAgentState&)`). It never touches the world or a budget.
| Type | Not applicable (reason, exact) | Effect |
|---|---|---|
| F1 | no cell → `no cell of the required kind`; free→blocked on the believed agent → `the wall would be under the believed agent`; on the believed object → `the wall would be under the believed object` | the belief's wall at the cell is flipped (only if inside the grid) |
| F2 | no direction of the order gives a cell that is not a wall (outside counts as a wall) and not the believed agent → `no direction passes the test` | the believed object moves one cell in the first direction that passes |
| F3 | never | the action's entry in the reasoner's `FaultedActions` table is set with its rotation; `setBase(makeFaultyModel(table))` (IE-1 H2); `unrestore(action)` |
| F4 | no cell → `the layout has no true dead square` | the cell is appended to the "not dead" overrides |
| F5 | the object has not moved in the true history → `the object has not moved yet`; its previous square is the believed agent → `the previous square is the believed agent square`; it is believed to be a wall → `the previous square is believed to be a wall` | the believed object goes back to the previous square |
- **The faulty model** (`makeFaultyModel(table)`, a copy of the table): for an action not in the table, or a standard result that is not Pushed, the standard result; otherwise the agent moves onto the object's cell and the object is predicted one cell further in the **rotated** direction (left: N→W→S→E→N; right: N→E→S→W→N), Blocked with the state unchanged if that cell is a wall.
- **F5's previous square**: the object position just before its most recent move in the **true** observation history (the earlier of the last pair of consecutive observations whose objects differ).
- **After any applied fault** the predictor's cache is emptied (`clearCache()`, D-036). A refused fault changes nothing.

### 3.5 Activation records and A4 [Phase B 12.3, A4, the 13.12 table]
One `ActivationRecord {eventId, type, scheduledStep, status, applicationStep, reason}` per event, status `Scheduled`, `Applied`, `NotApplicable` or `Pending`. At the activation point of decision step t, events are gone through in list order: a Scheduled event with step t is tried (applied → Applied at t; not applicable → NotApplicable with the reason for F1–F4, Pending with the reason for F5); a Pending event is tried at every later step (applied → Applied at t; otherwise it stays Pending with the latest reason).
`activationDelay` = applicationStep − scheduledStep when Applied, else 0. `reportedAs`: `Activated at its scheduled step`, `Activated after deferral`, `Not activated: run ended before the scheduled step`, `Not activated: not applicable at the scheduled step`, `Not activated: deferred, never applicable`.

### 3.6 The fault activation point and the run with faults [Phase B 11.3 step 4, 11.8]
**The activation point already exists** (Stage 3, IE-1 H1): `activationHook` in each agent's state, called once per decision step (C1: after step 3, and also just before the safe stop of a deferred-again investigation; C0: at the start of `decide`).

**The run with faults** (`runWithFaults`, one overload for C1 and one for C0) is **the Stage 3 run loop itself** (`runSingleAgent`), connected through watch-only watchers, instead of a second copy of the loop (the original wrote the loop again in `fault_run.cpp`):
1. it keeps the run's **true observation history**: the new run-loop watcher **`onObservation`** (3.12, D-075) is called with every observation just before the agent receives it, for `decide` and for the final check, and the fault run appends it;
2. it sets the agent's `activationHook` to: `scorer.endPhaseA(view, t)`; `injector.atActivationPoint(parts, t)`; `scorer.afterActivation(view, records, t)`;
3. `afterDecision` calls `scorer.afterDecide(view, decision)`; `afterFinalCheck` calls `scorer.endFinalCheck(view)`;
4. with `observe` true (the default), a watch-only `ComputationObserver` that only calls the scorer is attached to the predictor and the dead-square cache (IE-1 H3, H4);
5. after `runSingleAgent` returns: the hook is emptied, the observers are detached (predictor, cache and the cached map), `scorer.finish(view)` records each Corrected event's final record status, and the `FaultRunResult` is built: the `RunResult` exactly as `runSingleAgent` returned it, `decisionSteps`, the activation records, the event scores, the run score and the observed evaluations.

With an empty event list, `runWithFaults` gives exactly the `runSingleAgent` result and lines (FR4). Optional extra watchers (`FaultRunWatchers`, `NaiveFaultRunWatchers`: `afterDecision` and, for C1, `afterFinalCheck`, each receiving the agent, the observation, the injector and the scorer as const references) are called after the scorer's; they only watch, and the gate's trace writer uses them (FR7).

Exact values: FR1 (D2 true, belief true, one event `demo-1`: step 1, F1 at (4, 3), blocked→free) gives exactly Stage 3 demonstration 2's lines and 414 evaluations. The 16 demonstration-experiment runs give the outcomes, steps and evaluations of the original plan 3.6 (they are also in the gate, cases `demo-*`).

### 3.7 Scoring I: the life of each event [Phase B 12.3; D-037, D-045]
The scorer reads a `ScoredView` (read-only pointers to belief, action model, "not dead" and "dead" overrides, corrections, mismatches and committed prediction; the last three null for C0) and never writes to an agent.
- **Phases** (D-037): phase A(t) ends at the activation point (`endPhaseA`, before any fault is applied); phase B(t) ends when `decide` returns (`afterDecide`); the final check is a phase dated to the last decision step (`endFinalCheck`, D-045).
- **Active**: F1 while the believed wall differs from the true one; F2, F5 while the believed object differs from the object of the latest true observation; F3 until the action is marked restored; F4 until the cell is among the "dead" corrections.
- **Activation** (`afterActivation`): a record newly Applied makes its event activated at t and active; if it is not active at once, it ends Overwritten at t.
- **At the end of each phase** (`endPhase`), in this order: (1) every new correction record is a claim: claims + 1; a value that is not true is a wrong accepted correction; it strictly detects every event active at the start of the phase with the same element (first detection step kept); otherwise it is a false alarm. (2) every new mismatch record: mismatches + 1; its causes are the events recorded as committed-prediction causes (3.9) that were active at the start of the phase; none → a mismatch without fault cause; otherwise the label is scored, correct when its suspected source equals the type's source (S2 for F1, F2, F5; S1 for F3, F4), and each cause's first mismatch step is kept. (3) every activated event: active at the start and not active now → Corrected by the last new record of its element with the true value; or, for F3/F4 without such a record, Corrected by the last non-rejected record of its element (0 if none); otherwise Overwritten; with the current step. An event Corrected by a record that is now REJECTED and active again becomes active again (no end state). The committed causes are then cleared.
- **`finish`**: each Corrected event with a correcting id gets that record's final status.

### 3.8 Scoring II: claims, detection, corrections [Phase B 12.5; D-038, D-042, D-044]
`claimValueIsTrue(record, trueLayout, trueHistory, mismatches)`: a wall claim is true when its new value equals the true wall; a model claim is always true; a trap claim is true when the cell is truly dead (inside the grid); an object claim (D-042) is true when its new position equals the true object in `trueHistory[s − 2]`, where s is the latest mismatch record's step (false when there is no mismatch, s < 2, or the index is out of range). Strict detection, verified correction (D-038), mismatch detection, justified safe stops (a safe stop with an activated event still active, counted in `afterDecide`), source labels and the counts are as the original plan 3.8.

### 3.9 Scoring III: shadow evaluation [Phase B 12.4; D-039, D-040, D-043]
- **Committed prediction** (`afterDecide`, after the phase ends): for every activated, still-active event, `committedEffective` compares the committed prediction with its shadow: F4 never; F3 when the action is the faulted one and not restored, against the standard rule (outcome or state differ); F1 with the cell at its true value in the belief before; F2/F5 with the object at the true current position, only when the belief before has the object at the current believed position and that differs from the true one. An invalid shadow counts as effective; otherwise the shadow is predicted with the reasoner's current model and compared on the outcome and the two-prediction footprint. Effective → the event is a committed cause, `revealing` is set, and its opportunity step is set if empty.
- **Observed computations** (4e; `observe` true), only for activated, still-active events with no opportunity step yet:
  - every charged evaluation (`onEvaluation`, with the variant model or null for the reasoner's own current model): F3 as above but against the result passed in; F1/F2/F5 shadows repeat the evaluation **with the same model** (D-043), invalid → effective;
  - every dead-status read (`onDeadRead`): F1 and F4 compare with the shadow map (F1: the belief with the true wall; F4: the overrides without the cell), computed with fresh local counts;
  - every heuristic value (`onHeuristic`): F1/F4 with the shadow map; F2/F5 with the shadow state and a fresh map of the current belief; invalid shadow → effective.
  Shadow work never uses a budget and is never seen by an agent. `observedEvaluations` counts `onEvaluation` calls.
- **Primary scoring never depends on the observer** (IE-T4, SH4).

### 3.10 What one run produces
`FaultRunResult {RunResult run; int decisionSteps; std::vector<ActivationRecord> activations; std::vector<EventScore> eventScores; RunScore score; int observedEvaluations;}` with `EventScore {activated, activationStep, active, endState (None, Corrected, Overwritten), endStep, correctingId, correctionStatus (optional), opportunityStep (optional), revealing, mismatchStep (optional), strictStep (optional)}` and `RunScore {claims, falseAlarms, wrongAccepted, justifiedSafeStops, mismatches, mismatchesWithoutCause, labelsCorrect, labelsScored}`.

### 3.11 CSV output [Brief Stage 4]
Exactly the original plan 3.11: the two header lines (Section 5), `runCsvRow(condition, layoutSeed, runSeed, result)` and `eventCsvRow(condition, event, activation, score)`, fields joined by commas with no quoting, `1`/`0` for yes/no, empty fields for values that do not exist, cells `x:y`, the event's direction order with single spaces, and every measurement field empty for an event not activated (D-044). `writeTextFile(path, text)` writes the text exactly and returns false when it cannot. **Quirk Q7** (3.16): `Not activated: deferred, never applicable` contains a comma, so that row has one more comma-separated piece than the header (31 instead of 30); this is the original's output and is kept (D-078). **How later readers handle it** (Stages 7–8): `reported_as` (column 15) is the only field that can contain a comma, and this exact text is its only value that does; a reader splits a row on commas and, when it gets 31 pieces, joins pieces 15 and 16 with a comma and checks that the result is exactly that text; any other count is an error. No Stage 4 code reads CSV.

### 3.12 Changes to Stage 3 code
**IE-1 needs no new change**: H1–H4 were built into the Stage 3 agents (R5). Two small changes are made to Stage 3 code, in 4c, and the gate (G4–G10) proves they change nothing:
1. **`onObservation`** (D-075, an amendment of D-071): `RunWatchers` and `NaiveRunWatchers` get a third watch-only member, `std::function<void(const Agent&, const Observation&)> onObservation`, called with every observation just before the agent receives it (`decide`, and C1's final check). An empty one is not called. Nothing else in the loop changes.
2. **`actionIndex`** (D-076, R4): the private `actionIndex` of `agent_model.cpp` moves to `world/action.h/.cpp` (`int actionIndex(Action)`: North 0, East 1, South 2, West 3), so the fault model uses the same one (the original had two more private copies).

### 3.13 Settings
Every number is a parameter. Tests and demonstrations use the frozen Section 14 values: B = 200, S_max = 5, T_max = 100, T_v = 3; three events per run at steps 1–10; 20% shares; master seed 4242.

### 3.14 The demonstration and the program
`runFaultDemo(std::ostream&)` writes exactly the original's 68 lines (Section 5). `main.cpp` writes the three new header lines (Section 5), then `runScriptedDemo`, `runPredictionDemos`, `runAgentDemos` and `runFaultDemo`. `currentStage()` returns 4.

### 3.15 The equivalence gate, part 2: fault traces from the original code (M1.4; D-074)
**The golden files** (`tools/reference/stage4_rebuild/golden/`):
- `fault_cases.txt`: **563 cases**, each a Stage 3 case block (3.14 of the Stage 3 plan) followed, just before `end`, by `run <layoutSeed> <runSeed>` and the case's events, one line each: `event <id> <layoutSeed> <runSeed> <index> <step> <type> <target> <common 0|1> <details>`, details `cell X Y <direction>` or `cell none <direction>` (F1), the four directions (F2), `<action> <left|right>` (F3), `cell X Y` or `cell none` (F4), `-` (F5).
  1. FR1 (1 case); 2. FR4, the empty list (4: C1 and C0, on D and on D2 with belief D); 3. the demonstration experiment (16: 500000–500003 × run seeds 1–2 × C1, C0, master seed 4242); 4. **the 60 further demonstration-seed runs** of M1.4 (500000–500009 × run seeds 1–3 × C1, C0, with the event list of that 30-run set; the same layouts without faults are Stage 3's 20 demonstration-seed cases, since without faults the run seed changes nothing); 5. **482 synthetic verification fixtures** (D-070; `gen_fault_cases.py`, seed 20261023): 300 C1 and 100 C0 runs on random valid worlds with fault events from the generator's own families (standard, dense with several events per step and repeated F3 on one action, late, object faults, process faults, F1-then-F5), some with hand-written wrong beliefs and wrong models; 70 runs with the object starting in a true dead square and an F4 event on it (trap claims); 4 with the object boxed in (F2 not applicable); 8 chosen F1-then-F5 runs that reach A3's wall check. Together they reach every reason text, every end state and every correction status.
- `fault_traces.txt`: for each case, produced by `orig_trace/orig_fault_trace.cpp` built from the original `src/agent` and `src/experiment` at `89ea9b6`: `case <name>`; after each decision step the Stage 3 trace lines (C1 or C0, exactly 3.14 of the Stage 3 plan); then every activation record whose text changed (`activation <id> scheduled|applied at <t>|not-applicable|pending` with ` | <reason>` when there is one); then every event score whose text changed (`score <id> activated <yes|no> at <t> active <yes|no> end <none|corrected|overwritten> <step> by <id> opportunity <t|-> revealing <yes|no> mismatch <t|-> strict <t|->`); the same after the final check; then the Stage 3 `result` line, `csv run <row>`, one `csv event <row>` per event, `observed <n>` and `end`. The tool also runs each case with the original `runWithFaults` and stops unless every result, line, record and score is identical.
- SHA-256: cases `05a07ab8492243678016690c1454ce5d0fa260d5beadc6b3aa690aa330441631`, traces `7c27bbb11c2977355fd910732a96fd58e30cfbfe6310f1bfda6f9584b22451aa`. The reference reproduces the traces byte for byte.

**The reader** (`readFaultCases(lines)` → `{ok, cases, problem}`), bounds-safe, never throws (`ref_fault_reader.py`):
1. the Stage 3 reader (`readTraceCases`, unchanged) reads the file without its `run` and `event` lines; its problem, if any, is the result (its line numbers count that shorter file);
2. then, case by case: the block must start with its `case` line (otherwise `line <n>: expected a case header`, n in the real file); from its first `run` or `event` line up to `end` there must be one `run` line with two numbers (`case <k>: missing run line`, `case <k>: malformed run line`) and then only valid event lines (`case <k>: malformed event line`); nothing may follow the last case (`line <n>: expected a case header`).

**The gate** (`runFaultGate(caseLines, goldenLines, requiredCases)`, the order of Stage 3's `runTraceGate`): `a golden file is missing or empty`; the reader's problem; the split problem (`splitTraceBlocks`); `case file has <n> cases, required <r>`; `trace file has <n> cases, the case file <m>`; `trace <name>: expected case <name>`; the first difference (`firstTraceDifference`). **G8** requires exactly 563 cases (`kGoldenFaultCaseCount`) and all 563 blocks identical; **G9** shows the first three matching cases fail with exactly `case file has 3 cases, required 563`; **G10** is its control (3 required → passes).

### 3.16 Quirks kept on purpose
- **Q7.** The CSV text `Not activated: deferred, never applicable` contains a comma, and fields are never quoted (the original plan said no field contains one). Kept as a compatibility quirk, never silently repaired; later readers use the rule of 3.11 (D-078).
- **Q8.** Event identifiers are unique only within one event list (the demonstration experiment and the 30-run set both have `500000-1-1`); the gate's case names are unique.
- **Q9.** An F1 event on a cell outside the grid would be "applied" without changing the belief; the generator never makes one.
- **Q10.** For C0 the view has no corrections, mismatches or committed prediction, so C0 never claims, never mismatches and has no revealing opportunities; its events can only end Overwritten or stay active.

## 4. Files
All new code is in namespace `aih`. Source files go into `ai_hologram_core`, test files into `ai_hologram_tests`. The sizes are estimates, including comments. **If a file cannot hold its code cleanly within 200 lines, or a function within 40, stop and ask: never squeeze, never split only to pass the limit (R1).** `src/agent/` never includes `src/experiment/` (R7).

| File | Approx. lines | Job | Sub-stage |
|---|---|---|---|
| `src/experiment/seeded_random.h` / `.cpp` | 95 / 50 | `SeededRandom` (templates `shuffle`, `pickDistinct` in the header) | 4a |
| `src/experiment/true_search.h` / `.cpp` | 35 / 95 | `shortestSolutionLength` | 4a |
| `src/experiment/layout_generator.h` / `.cpp` | 45 / 150 | `GeneratedLayout`, `generateLayout` | 4a |
| `tests/test_seeded_random.cpp` | 140 | RN1–RN8 | 4a |
| `tests/test_layout_generator.cpp` | 190 | LY1–LY4 | 4a |
| `tests/test_layout_sets.cpp` | 80 | LY5, LY6 | 4a |
| `src/experiment/fault_event.h` / `.cpp` | 85 / 70 | `FaultType`, `FaultTarget`, `F1Direction`, `Rotation`, `FaultEvent`; their names (`faultTypeName`, `faultTargetName`, `f1DirectionName`, `rotationName`, the one copy of each, R4) | 4b |
| `src/experiment/fault_events.h` | 55 | `RunKey`, `commonModeCount`, `runStreamSeed`, `generateFaultEvents`, `faultEventText` | 4b |
| `src/experiment/fault_events.cpp` | 190 | the generation (3.3) | 4b |
| `src/experiment/fault_event_text.cpp` | 60 | `faultEventText` | 4b |
| `tests/fault_test_material.h` / `.cpp` | 45 / 90 | the shared demonstration material (layouts, events), hand-built events, `findActivation` (R4) | 4b |
| `tests/test_fault_events.cpp` | 170 | FE1, FE4 (demonstration list), FE5, FE6 | 4b |
| `tests/test_fault_event_sets.cpp` | 195 | FE2, FE3, FE4 (pilot, confirmatory) | 4b |
| `src/world/action.h` / `.cpp` | — | `actionIndex` (moved, R4) | 4c (changed) |
| `src/agent/agent_model.cpp` | — | uses the shared `actionIndex` | 4c (changed) |
| `src/agent/run_loop.h`, `run_loop.cpp`, `run_loop_naive.cpp` | — | `onObservation` (3.12) | 4c (changed) |
| `src/experiment/fault_model.h` / `.cpp` | 50 / 80 | `FaultedActions`, `rotatedAction`, `makeFaultyModel` | 4c |
| `src/experiment/fault_apply.h` / `.cpp` | 65 / 180 | `ReasonerParts`, `partsOf`, `ApplyOutcome`, `tryApplyFault` (3.4) | 4c |
| `src/experiment/fault_injector.h` / `.cpp` | 100 / 130 | `ActivationStatus`, `ActivationRecord`, `activationDelay`, `reportedAs`, `FaultInjector` (3.5) | 4c |
| `src/experiment/fault_run.h` | 95 | `FaultRunResult`, `FaultRunWatchers`, `NaiveFaultRunWatchers`, the two `runWithFaults` | 4c |
| `src/experiment/fault_run.cpp` / `fault_run_naive.cpp` | 150 / 120 | the C1 and C0 runs with faults (3.6) | 4c |
| `tests/test_fault_apply.cpp` | 180 | FI1, FI2 | 4c |
| `tests/test_fault_apply_more.cpp` | 195 | FI3, FI4 | 4c |
| `tests/test_fault_f5.cpp` | 160 | FI5, FI6 | 4c |
| `tests/test_fault_activation.cpp` | 170 | FI7, FI8 | 4c |
| `tests/test_fault_run.cpp` | 190 | FR1, FR4, IE-T1 | 4c |
| `tests/test_fault_run_more.cpp` | 190 | FR5, FR6, FR7, RL2, RL3 | 4c |
| `src/experiment/scorer_types.h` | 85 | `ScoredView`, `EndState`, `EventScore`, `RunScore` | 4d |
| `src/experiment/scorer.h` | 120 | `viewOf`, `claimValueIsTrue`, `Scorer` | 4d |
| `src/experiment/scorer.cpp` | 180 | views, `claimValueIsTrue`, `activeNow`, the phase entry points, `finish` | 4d |
| `src/experiment/scorer_claims.cpp` | 150 | `endPhase` steps (1) and (2): claims and mismatch records | 4d |
| `src/experiment/scorer_life.cpp` | 160 | `endPhase` step (3): the event life | 4d |
| `src/experiment/scorer_commit.cpp` | 160 | `committedEffective`, `afterDecide` | 4d |
| `src/experiment/fault_run.h`, `.cpp`, `fault_run_naive.cpp` | — | call the scorer; `FaultRunResult` gains scores | 4d (changed) |
| `tests/test_scorer.cpp` | 190 | SC1, SC2, SC3 | 4d |
| `tests/test_scorer_life.cpp` | 190 | SC4, SC6 | 4d |
| `src/experiment/shadow.h` / `.cpp` | 65 / 170 | `shadowInput`, `evaluationEffective`, `shadowMap` (3.9) | 4e |
| `src/experiment/scorer_observe.cpp` | 175 | `attachView`, `onEvaluation`, `onDeadRead`, `onHeuristic`, `observedEvaluations` | 4e |
| `src/experiment/fault_run.h`, `.cpp`, `fault_run_naive.cpp` | — | the `observe` flag and the observer | 4e (changed) |
| `tests/recording_observer.h` / `.cpp` | 40 / 70 | the one recording observer of the tests (moved from `test_dead_square_observers.cpp`, which then uses it; it also records evaluations), R4 | 4e |
| `tests/test_dead_square_observers.cpp` | — | uses `recording_observer` (no other change) | 4e (changed) |
| `tests/test_shadow.cpp` / `test_shadow_more.cpp` | 160 / 160 | SH1 | 4e |
| `tests/test_observed_runs.cpp` | 190 | SH2, SH3, SH4, SH5 | 4e |
| `tests/test_read_sites.cpp` / `test_read_sites_more.cpp` | 150 / 160 | SH6 | 4e |
| `src/experiment/csv_fields.h` / `.cpp` | 40 / 60 | `joinFields`, `bitText`, `optionalStepText` (R4) | 4f |
| `src/experiment/csv_output.h` | 55 | headers, rows, `writeTextFile` | 4f |
| `src/experiment/csv_output.cpp` / `csv_event_row.cpp` | 110 / 180 | the run row and headers; the event row | 4f |
| `src/demo/fault_demo.h` | 30 | `runFaultDemo` | 4f |
| `src/demo/fault_demo_runs.h` / `.cpp` | 55 / 120 | the demonstration material and its four runs | 4f |
| `src/demo/fault_demo.cpp` | 170 | the demonstration text | 4f |
| `src/main.cpp`, `src/core/project_info.cpp`, `tests/test_project_info.cpp`, `README.md` | — | header, stage 4, P2 checks 4, status (Section 9) | 4f (changed) |
| `tests/expected_fault_demo.h` / `.cpp` | 30 / 110 | the 68 expected demonstration lines | 4f |
| `tests/test_csv_output.cpp` | 160 | CSV1, CSV3, CSV4 | 4f |
| `tests/test_fault_demo.cpp` | 60 | FD1 | 4f |
| `tests/trace_agents.h` / `.cpp` | 35 / 60 | `applyTraceSeeds` (moved from `trace_writer.cpp`, which then uses it, R4) | 4g |
| `tests/fault_case_reader.h` / `.cpp` | 65 / 180 | `FaultCase`, `readFaultCases`, `parseFaultEventLine` | 4g |
| `tests/fault_trace_text.h` / `.cpp` | 40 / 80 | the activation and score lines | 4g |
| `tests/fault_trace_writer.h` / `.cpp` | 35 / 175 | runs a fault case with `runWithFaults` and its watchers and writes its trace | 4g |
| `tests/fault_gate.h` / `.cpp` | 40 / 75 | `kGoldenFaultCaseCount` (563), `runFaultGate` | 4g |
| `tests/test_fault_reader.cpp` | 190 | FN1–FN10, FT1, FT2 | 4g |
| `tests/test_fault_gate.cpp` | 120 | G8, G9, G10 | 4g |
| `tests/test_suites.h`, `test_main.cpp`, `CMakeLists.txt` | — | new groups and files | 4a–4g (changed) |

**75 new code files.** The code checker sees 178 → 187 (4a) → 196 (4b) → 211 (4c) → 219 (4d) → 229 (4e) → 242 (4f) → **254** (4g).

**R4, one helper per job:** `SeededRandom`; `actionIndex` (moved); `kActionOrder` (Stage 3); `positionText` (cells `(x, y)`); the fault-type, target, direction and rotation names (`fault_event`); `reportedAs` and `activationDelay`; the CSV field helpers; `computeDeadSquares` and `readDeadStatus` (Stage 3) for every shadow map and read; `computeFootprintForTwoPredictions` and `compareFootprint` (Stage 3) for every prediction comparison; the demonstration material (`fault_test_material`); the recording observer; `applyTraceSeeds`; the Stage 3 trace readers, printer, text and blocks.

## 5. Expected output
- **The program** (`tools/reference/stage4_rebuild/expected_program_output.txt`): **170 lines, 152 non-blank**. The first three lines are new:
  ```
  AI Hologram v2 -- Stage 4
  Grid world with a competent single agent (C1), a naive agent (C0), fault injection and measurement.
  Faults are injected only on demonstration material; no two-agent reasoning exists, and no experiment is run.
  ```
  Lines 4–102 are exactly the Stage 3 lines 4–102. Lines 103–170 are `runFaultDemo`, exactly the original's (lines 103–170 of the carried-over `tools/reference/stage4/expected_output_stage4.txt`): the layout 500000 drawing, the six events of runs 500000-1 and 500003-1, runs 500000-2 and 500003-1 for C1 and C0 with their activation records, four `runs.csv` rows and twelve `events.csv` rows.
- **The tests:**

  | After | Expected line | New checks |
  |---|---|---|
  | 4a | `379 checks, 0 failures` | RN1–RN8 (8), LY1 (7), LY2 (6), LY3 (3), LY4 (10), LY5 (2), LY6 (1) … 37 |
  | 4b | `397 checks, 0 failures` | FE1 (3), FE2 (3), FE3 (4), FE4 (3), FE5 (3), FE6 (2) … 18 |
  | 4c | `445 checks, 0 failures` | FI1 (6), FI2 (4), FI3 (7), FI4 (3), FI5 (6), FI6 (2), FI7 (2), FI8 (3), FR1 (2), FR4 (5), IE-T1 (2), FR5, FR6 (2), FR7, RL2, RL3 … 48 |
  | 4d | `465 checks, 0 failures` | SC1 (3), SC2 (2), SC3 (6), SC4 (7), SC6 (2) … 20 |
  | 4e | `513 checks, 0 failures` | SH1 (14), SH2 (5), SH3 (12), SH4 (8), SH5 (2), SH6 (7) … 48 |
  | 4f | `522 checks, 0 failures` | CSV1 (2), CSV3 (2), CSV4 (4), FD1 … 9 |
  | 4g | `537 checks, 0 failures` | FN1–FN10, FT1, FT2, G8, G9, G10 … 15 |

  The 537 are Stage 3's 342 and the 195 of Section 6. `checks.py` mirrors all 195: `failing: none (195 checks)` (`checks_output_2026-10-21.txt`).
- **CTest:** 2/2. **Code checker:** 0 violations (the file counts of Section 4). The test program takes about 15 seconds in the Debug build of `run_checks.ps1` (G8 replays 563 fault runs with scoring).
- **`run_checks.ps1`** ends with `ALL CHECKS PASSED (191 preserved files, 537 checks, 254 code files checked, formatting checked)`. The 191 are Stage 3's 163, the 26 original files of `orig_trace/original_src/` (24 of `src/experiment` and `fault_demo.h/.cpp`, at `89ea9b6`) and the 2 golden files.

## 6. Tests
Each check's description starts with its id. Exact values are in `checks.py`, which follows the original tests (`89ea9b6`, `tests/test_*`) check by check, with the original ids, except: the layout tests are **LY** (Stage 3 uses LG for the log); the demonstration-experiment tables FR2, FR3, SC5, SH5 (per run) and CSV2 are **not separate tests**, because the gate compares every one of their fields for the same 16 runs (cases `demo-*`) and FD1 compares CSV2's sixteen rows (D-077); the new checks are FR7, RL2, RL3, FN, FT and G8–G10. Fixtures are valid worlds (D is Stage 3's `demoWorld()`, D2 is D with a wall at (4, 3)).

### 6.1 Material (4a, 4b)
| Id | Check |
|---|---|
| RN1–RN8 | The six values of 3.1; RN7 1,000 equal values for one seed, seeds 1 and 2 differ at once; RN8 `below(n)` in range for n = 1, 2, 3, 7, 100, 1000 and every value of `below(7)` seen (seed 2026) |
| LY1 (7) | D valid; D's shortest solution 4; D with the object at (1, 1): valid, none; with the object on the destination: valid, 0; an agent outside the grid: invalid and none |
| LY2 (6) | layouts 500000–500003, 900000, 900001: attempts, shortest, positions, dead cells, interior walls (3.2) |
| LY3 (3) | the drawings of 500000 and 900000; 500004–500009 attempts and shortest |
| LY4 (10) | 500000–500009: found, valid, bordered, shortest 12–40 and equal to the search's, at least one dead cell |
| LY5 (2) | material only: all 40 pilot layouts found, largest attempts 10, shortest 12–35; all 400 confirmatory, 20, 12–38 |
| LY6 | generating 500002 twice is identical |
| FE1 (3) | the 24 demonstration lines; common-mode exactly for the 5 `both` events; ids match layout, run and index |
| FE2 (3) | material only: pilot counts per type (48, 10 common, 19 R1, 19 R2); F1 directions 24/24 and every F1/F4 has a cell; run 900000-1's lines |
| FE3 (4) | material only: confirmatory counts (1,200, 240, 480, 480); directions 600/600 and cells; events per step 580, 620, 603, 578, 609, 577, 614, 597, 605, 617; the first and last runs' lines |
| FE4 (3) | demonstration, pilot, confirmatory: 3 sorted distinct steps 1–10; F1 cells interior, not the destination, true status as the flip requires; F4 cells truly dead |
| FE5 (3) | twice identical (24 lines); master seed 4243's first line `500000-1-1 step 1 F3 target R1 | East left`; different run and layout counts give an empty list |
| FE6 (2) | `commonModeCount` 0, 1, 1, 2, 10, 240 for 2, 3, 5, 8, 48, 1200; `runStreamSeed(4242, 500000, 1)` |

### 6.2 Faults and the run with faults (4c)
| Id | Check |
|---|---|
| FI1 (6) | D valid; F1 free→blocked at (3, 3) and (5, 3) refused with the two reasons; at (6, 5) applied; blocked→free at (4, 6) applied; no cell refused |
| FI2 (4) | F2 N,E,S,W → object (5, 2); a hand-built boxed belief is valid and F2 is refused (`no direction passes the test`); with the agent believed at (5, 2) → object (6, 3) |
| FI3 (7) | the hand input is valid; East faulted right predicts (5, 4) (standard (6, 3)); Blocked against a wall at (5, 4); West standard; a second F3 North left keeps East; restoring East leaves North; `setBase` keeps the mark (H2) |
| FI4 (3) | F4 at (1, 1): not dead, 21 dead cells (22 before); a later dead correction makes it dead; no cell refused |
| FI5 (6) | D valid; the history states valid; the believed agent square; applied after the agent moved north (object (5, 3)); not moved yet; believed wall |
| FI6 (2) | an applied fault empties the cache; a refused one leaves it |
| FI7 (2) | demonstration 500003-1 (C1): `500003-1-1` applied at 9, delay 8; 500001-1: `500001-1-3` pending, `the object has not moved yet` |
| FI8 (3) | a step-10 event in D's 4-step run ends before its step; F1 at (3, 3) not applicable, run unchanged (19 evaluations); `reportedAs` and `activationDelay` for every status |
| FR1 (2) | D, D2 valid; FR1 gives demonstration 2's lines exactly, 414 evaluations, `demo-1` applied at 1 |
| FR4 (5) | D, D2 valid; the empty list gives exactly `runSingleAgent`'s result for C1 and C0 on D and on D2 with belief D, and the hook is empty afterwards |
| IE-T1 (2) | without instrumentation, demonstrations 1 and 2 give Stage 3's `C1_1` (19) and `C1_2` (414) |
| FR5 | demonstration run 500002-2 (C1) twice: identical lines, steps and records |
| FR6 (2) | D with the object on the destination is valid; its run makes no decision and the event stays scheduled |
| FR7 | FR1 with extra watchers: called at decision steps 1–18 and after the final check (step 19), and the lines and evaluations are unchanged |
| RL2 | demonstration 2 with `onObservation`: called 19 times, steps 1–19 in order, the last with the object on the destination; the lines are exactly `C1_2` |
| RL3 | C0 on D with `onObservation`: called at steps 1–4; Success after 4 world steps, 16 evaluations |

### 6.3 Scoring (4d)
| Id | Check |
|---|---|
| SC1 (3) | D2 valid; FR1's event: activated 1, revealing, opportunity 1, mismatch 2, strict 2, Corrected at 2 by record 1, VERIFIED; run: claims 1, false alarms 0, wrong 0, mismatches 1, without cause 0, labels 1/1, justified 0 |
| SC2 (2) | D, D2 valid; demonstration 2 with no events (a hand-written error): claims 1, false alarms 1, wrong 0, mismatches 1, without cause 1, labels 0/0 |
| SC3 (6) | D, D2 valid; the wall claim; the model claim; the trap claims (1, 1) and (5, 3); the object history valid; the object claim judged at its moment (D-042) |
| SC4 (7) | the hand-driven event life of the original plan 6.6 (SC4), step by step |
| SC6 (2) | D, D2 valid; FR1's lines unchanged with the scorer, hook empty afterwards |

### 6.4 Shadow evaluation (4e)
| Id | Check |
|---|---|
| SH1 (14) | F1 (2), F2 (5), F3 (3), F4 (4): the effective and non-effective cases of the original plan 6.6 (SH1) |
| SH2 (5) | D, D2 valid; with the recording observer, C1 and C0 on D and on D2 with belief D give the same results as without (IE-T2) |
| SH3 (12) | the observed evaluations equal `totalEvaluations()` for those four runs and for the eight demonstration runs, C1 and C0 (IE-T3) |
| SH4 (8) | each demonstration run, C1 and C0: primary scoring identical with the observer off, and nothing observed (IE-T4) |
| SH5 (2) | D2 valid; FR1's opportunity step is 1 |
| SH6 (7) | D valid; the six read sites: the heuristic, planner pruning, the cautious choice, the consequence check, the local check, the trap V2 test (exact reports, values equal the map) |

### 6.5 CSV and the demonstration (4f)
| Id | Check |
|---|---|
| CSV1 (2) | the two headers |
| CSV3 (2) | event 500001-1-3 (C1, not activated) row exactly; after the run the cache and cached-map observers are null |
| CSV4 (4) | `writeTextFile` writes, reads back identical, the file is removed, a missing folder returns false |
| FD1 | `runFaultDemo` writes exactly the 68 lines |

### 6.6 The gate, part 2 (4g)
| Id | Check |
|---|---|
| FN1–FN9 | `readFaultCases` problems, exactly: missing end; missing run line; a Stage 3 problem passed on (malformed agent line); malformed run line; an unknown type; an F1 cell with one number; F2 with three directions; F3 rotation `up`; a stray event line after the last case (`line 20: expected a case header`) |
| FN10 | a positive control: two cases, the first with seven events covering every type and `cell none`, the second with none; every field exact |
| FT1, FT2 | the activation lines for the four statuses, and one score line, exactly |
| G8 | all 563 golden fault blocks identical (exactly 563 required) |
| G9 | the first three cases and blocks fail with exactly `case file has 3 cases, required 563` |
| G10 | the same three with 3 required pass |

## 7. Implementation order
| Step | Sub-stage | Content | Tests (total) | Deliberate failure checks (tried by Claude first, R11; `df_check.py`) |
|---|---|---|---|---|
| 1 | — | The owner commits this plan, `tools/reference/stage4_rebuild/`, the provenance update (191), the decisions and the 4a prompt, on a new branch `stage-4-work` | | **Owner, DF4-P:** add one character to `fault_traces.txt`. It must stop at `[provenance]`; restore it with `git checkout`. |
| 2 | **4a** | seeded random, the experimenter's search, layouts | RN, LY (379) | **Cursor, DF4a-1:** `below`'s rejection test uses `>` instead of `>=`. **Exactly RN4 must fail.** |
| 3 | **4b** | fault events | FE (397) | **Cursor, DF4b-1:** `commonModeCount` rounds down (count / 5). **Exactly these must fail:** FE1 (the 24 lines and the common-mode check: 2 of its 3 checks), FE2 (the pilot counts), FE6 (`commonModeCount`): 4 checks. |
| 4 | **4c** | applying faults, records, the run with faults, `onObservation`, `actionIndex` | FI, FR, IE-T1, RL2, RL3 (445) | **Cursor, DF4c-1:** F5 forgets A3's believed-agent-square check. **Exactly these must fail:** FI5 (the believed agent square) and FI7 (`500003-1-1` is applied at a different step): 2 checks. |
| 5 | **4d** | the scorer: views, claims, the event life, the committed shadow | SC (465) | **Cursor, DF4d-1:** a claim is judged against the events active at the end of its phase instead of the start. **Exactly these must fail:** SC1 (the event and the run: 2 checks) and SC4 (corrected at step 3, and the totals at the end: 2 checks): 4 checks. |
| 6 | **4e** | shadow evaluation through the observers | SH (513) | **Cursor, DF4e-1:** the local check reads `map.dead` directly instead of through `readDeadStatus`. **Exactly these must fail:** Stage 3's OB2 and G5 (the local check's reads are no longer reported), SH6.4 and SH6.5: 4 checks. Nothing else changes behaviour (G8 still passes). |
| 7 | **4f** | CSV, the demonstration, the program, stage 4, README | CSV, FD1 (522) | **Cursor, DF4f-1:** the detection delay counts from the scheduled step instead of the activation step. **Exactly FD1 must fail.** |
| 8 | **4g** | the fault-case reader, the trace text and writer, the gate | FN, FT, G8–G10 (537) | **Cursor, DF4g-1:** in `onDeadRead`, compare dead-status reads for F4 events only (not F1). **Exactly G8 must fail:** only the gate catches it. **Owner, DF4g-2:** in `fault_apply.cpp`, change `"the object has not moved yet"` to `"the object did not move yet"`. `run_checks.ps1` must stop at `[tests]` with exactly FI5 (no object move), FI7 (`500001-1-3`'s reason), CSV3 (the not-activated row) and G8 failing. |
| 9 | — | Fresh clone of `stage-4-work`: `run_checks.ps1` passes | | |
| 10 | — | ChatGPT reviews the whole buildable tree; the completion gate; tags **`stage-4-v2`** and **`c1-baseline-v2`** on the completion commit; merge into `main` | | |

Each sub-stage is committed by the owner on `stage-4-work`, after Claude's review, and starts in a new Cursor chat with its prompt saved first in `docs/prompts/`.

## 8. Decisions to record (with the plan's commit, R12)
| ID | Decision | Reason | Alternatives |
|---|---|---|---|
| D-073 | The rebuilt `src/experiment/` and `src/demo/fault_demo.*` reproduce the original's at `89ea9b6` (Stage 4, sub-stages 4a–4f), with the original readings D-030 to D-045 unchanged. Namespace `aih`. Only the organisation changes (R1–R4). | M1.4 compares every Stage 4 score; the readings were approved in the original project and apply as they are. | Re-deciding the readings (not allowed: they define measures before any result). |
| D-074 | **The equivalence gate, part 2.** 563 fault runs are traced once from the original code (`orig_trace/orig_fault_trace.cpp`, `89ea9b6`): FR1, FR4, the 16 demonstration-experiment runs, the 60 further demonstration-seed runs, and 482 synthetic fixtures (D-070, seed 20261023). The traces compare every decision step (the Stage 3 fields), every activation record and every event score, and at the end the CSV rows and the observed evaluations. They are committed with their hashes and the 26 original files. G8 replays them in every test run and requires exactly 563 cases; G9, G10 prove the count is enforced. **With G4–G10 passing, M1.4 has passed in full.** | M1.4 and R20; the method of D-069 and ChatGPT's review 04 of Stage 3. | Only the named runs (too narrow: several fault rules would never be reached). |
| D-075 | Amendment of D-071: the run loop's watchers get `onObservation`, called with every observation just before the agent receives it, so the run with faults keeps the true history through the Stage 3 loop itself. Watch-only (R6); G4–G10 prove nothing changes. | The original wrote the run loop a second time in `fault_run.cpp`; one loop (R4) cannot drift. A watcher after the decision cannot give the observation of the final check before that check. | A second copy of the loop (the original); stepping a mirror of the world inside the fault run (duplicates the loop's job). |
| D-076 | `actionIndex` moves from `agent_model.cpp` to `world/action.h/.cpp`; the fault model uses it. | R4: the original had three copies. | A fourth private copy. |
| D-077 | The demonstration-experiment tables (original FR2, FR3, SC5, SH5 per run, CSV2) are covered by the gate (cases `demo-*`, every field) and FD1, not repeated as separate tests. | The same 16 runs are compared field by field at every step by G8; separate tables would be a second copy of the same expected values. | Keeping them (about 90 more checks, no more coverage). |
| D-078 | Quirks Q7–Q10 (3.16) are kept, in particular the unquoted comma in `Not activated: deferred, never applicable`. | The baseline's outputs are frozen (M1.3 item 5: never change an expected value); Stage 7 and 8 readers must count columns. | Quoting the field (would change the original's output). |
| D-079 | `c1-baseline-v2` is tagged on Stage 4's completion commit (the same commit as `stage-4-v2`), after ChatGPT's whole-tree review and the owner's agreement, with an annotated message naming M1, the gate record (G4–G10, the golden hashes) and that it replaces `stage-3` (`7587f38`) under M1 (M1.5). | M1.3 items 2 and 3: tagged only when the whole gate passes, and before any two-agent code. | Tagging in Stage 5 (no reason to wait; Stage 5 adds messages, not two-agent code). |
| D-080 | Generating the pilot and confirmatory **material** in tests (LY5, FE2, FE3, as the original did) is not an agent run and is allowed under R21 as clarified by D-070; no agent is ever run on it. | The protocol's balance (Section 14) must be checked on the real material. | Not checking the real material (its balance would go untested until Stage 7). |

## 9. Documentation
- **Headers and comments.** Every new file has the three-part header (Introduced in: Stage 4x (rebuild)). Beginner comments explain: why the project has its own random generator, and what a seed is; why the experimenter's search is never visible to an agent; why the event list is balanced over the whole experiment; value faults and process faults; A3 and A4 in plain words; the two phases of a step; what a shadow computation is, and why it never uses a budget; why credit comes only through records; why the run with faults reuses the Stage 3 run loop.
- **README** (4f). The status becomes: "Stage 4 of the rebuild: fault injection and measurement for C0 and C1, and the full M1.4 equivalence gate (789 fault-free and 563 fault runs recorded from the original code, compared step by step). Faults are injected only on demonstration material; no two-agent reasoning exists, and no experiment has been run."
- **Handoff notes.** One per sub-stage. **Review notes.** Every exchange with ChatGPT goes in `docs/reviews/stage4/`.

## 10. Completion checklist (owner verification gate)
- [ ] The plan was agreed and committed before any Stage 4 code.
- [ ] `run_checks.ps1` passes on the owner's computer **and on a fresh clone** made by the owner, with the exact final line of Section 5.
- [ ] The program output is exactly Section 5, at 170 lines; the tests show `537 checks, 0 failures`; CTest 2/2; 0 checker violations (254 files).
- [ ] **After the run-loop change (`onObservation`), G4 and G5 still pass all 789 fault-free cases** (Stage 3's gate, exactly 789 required, G6 and G7 still passing).
- [ ] **G8 passes all 563 fault cases** (exactly 563 required), and **G9 and G10 prove the exact-count guard**.
- [ ] **The golden records are unchanged**: the five golden files of Stages 3 and 4 match `docs/PROVENANCE.sha256` (the `[provenance]` step).
- [ ] The completion record (Section 12) states the coverage of D-077 explicitly: the 16 demonstration-experiment runs (cases `demo-*`) are compared by G8 at every decision step, field by field (the Stage 3 fields, every activation record, every event score) and at the end (both CSV rows and the observed evaluations), and FD1 compares the original CSV2 rows.
- [ ] All deliberate failure checks gave exactly the predicted failures.
- [ ] No agent was run on a pilot or confirmatory layout; LY5, FE2 and FE3 only generated material (D-080).
- [ ] D-073 to D-080 recorded; handoff notes appended; prompts and review notes saved.
- [ ] ChatGPT reviewed the whole buildable source tree of the final commit.
- [ ] The owner agrees (D-065) and approves the baseline; only then are `stage-4-v2` (Stage 4 complete) and `c1-baseline-v2` (the behaviour-equivalent C1 baseline, frozen from then on) tagged on the completion commit (D-079). Neither tag is created earlier.

## 11. Plain explanation (for any reader)
1. **The material is made from seeds.** The same seed always gives the same layouts and the same list of faults, on any computer.
2. **Faults go into a reasoner's head, never into the world.** A fault changes what the reasoner believes (a wall, the object) or how it reasons (a push rule, a trap judgement).
3. **Some faults must wait.** A wall cannot be imagined under the agent (A3), and "the object goes back" needs the object to have moved (A4).
4. **A referee scores the run from the agent's own records.** It never touches the agent; credit comes only from correction records.
5. **The "camera" sees every computation.** It tells the referee when a fault actually mattered, without the agent noticing.
6. **The new instrument must behave exactly like the old one.** 563 recorded fault runs of the original must be replayed frame by frame, with every score. Then the rebuilt C1 may stand in for the frozen one, under its own name, `c1-baseline-v2`.

## 12. What actually happened (filled in at the end)
```
Date completed:
Commits tagged stage-4-v2 and c1-baseline-v2:
Checks:
Did everything go as planned?
```

## 13. Instructions to the coding assistant
Every prompt says:
- follow `AGENTS.md` and `.cursor/rules`; implement only the named sub-stage;
- read the reference (`tools/reference/stage4_rebuild/`, and `tools/reference/stage3/` for C1 and C0) for exact behaviour and test data; it follows the original function by function and draw by draw: one reordered draw changes every later number;
- never commit; never change an expected or golden value; report any mismatch with its id or case;
- never squeeze code to fit a limit; stop and report instead; never run Python in `tools/reference/`;
- format with CLion's clang-format (never `CMakeLists.txt`);
- run `run_checks.ps1` and report its final line exactly.

## Revision history
| Version | Date | Change |
|---|---|---|
| 1.1 | 2026-10-21 | After ChatGPT's review 02 (design approved; D-073 to D-080 accepted, D-077, D-079 and D-080 with conditions). The two documentation clarifications: the 4a prompt names this approved version; the completion checklist states that G4/G5 must still pass all 789 fault-free cases after the run-loop change, that G8 must pass all 563 fault cases and G9/G10 prove the count guard, that the golden records stay unchanged, and that the completion evidence states D-077's coverage and the owner's fresh clone. Also recorded: how later readers handle Q7 (3.11), and the conditions of D-079 and D-080. No change to behaviour, checks, counts or golden values. |
| 1.0 | 2026-10-21 | First draft. Built first (R9, R11): the golden fault traces from the original C++ at `89ea9b6` (563 cases, each cross-checked against the original `runWithFaults`); the reference, which reproduces them byte for byte, the original demonstration text and the program output; the 195-check mirror; the deliberate failure predictions. |
