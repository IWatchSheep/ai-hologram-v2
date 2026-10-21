# Stage 2 (rebuild) — Prediction and comparison

| | |
|---|---|
| **Project** | AI Hologram v2 (`ai-hologram-v2`) |
| **Stage** | 2 of the rebuild (Brief Section 10, Stage 2: prediction and comparison) |
| **Plan version** | 1.0 |
| **Date** | October 20, 2026 |
| **Status** | **APPROVED.** ChatGPT approved v1.0 with no changes, after building the repository, running the reference, reproducing all six deliberate failure predictions and verifying the golden file byte for byte. The owner agreed on 2026-10-20 ("I approve it."). Stage 2a may begin after this plan is committed. |
| **Governing documents** | The brief v1.1; `docs/RULES.md` (R1–R24); Migration Addendum M1; `AGENTS.md`; decisions D-064 and D-065. |
| **Builds on** | Stage 1, complete and tagged `stage-1-v2` (`5c2b3f9`). |
| **Original it rebuilds** | `IWatchSheep/ai-hologram`, Stage 2 (plan `docs/history/ai-hologram/docs/stages/STAGE_02.md` v1.3, tag `stage-2`). Its code (`src/model`, `src/compare`, `src/demo/prediction_demo.*`) never changed from `stage-2` to `89ea9b6`, so it is also the prediction and comparison code inside the frozen C1 baseline (`stage-3`, `7587f38`). |
| **Reference** | `tools/reference/stage2/` (R9, R10): `ref_model.py`, the reader `ref_prediction_reader.py`, the check mirror `checks.py`, `df_check.py`, `gen_golden.py`, and a golden file produced by a tool built from the **original C++** (`orig_trace/`). It reuses `tools/reference/stage1/`. |

---

## 1. Purpose
Stage 2 rebuilds the core of the project's idea: a disagreement between a **prediction** and an **observation** is information. It adds:
- a **belief**: what a reasoner *thinks* the world is, kept separate from the true world;
- a **world model**, which predicts the result of an action from a belief, using a replaceable action model;
- a **budget counter**, through which every prediction must pass;
- a **comparison**, which reports exactly how a prediction differs from an observation, in plain text;
- the **prediction demonstration**: one correct belief, and one wrong belief.

As in Stage 1, it must behave **exactly like the original**, because C1 will later be compared with the original step by step (M1.4). So a golden file of **1,000 prediction cases**, produced by a tool compiled from the original code, binds the rebuilt model, comparison and difference text to the original.

**Nothing in Stage 2 decides anything.** There is no planning, belief correction, choosing of actions, mismatch-source labels, safe stops, agents or faults. Those belong to later stages.

## 2. What Stage 2 must NOT contain
- No planning, search or choosing of actions; the demo's actions are hand-written.
- No belief correction. The demo replaces the belief with the observation after each step, and says so (3.7).
- No randomness, faults, experiment code, exceptions, assertions that stop the program, inheritance or external libraries.
- No change to the Stage 1 world (rules, representation, text format).

## 3. Exact behaviour (the original's, unchanged)

### 3.1 Belief and prediction
- `struct Belief { WorldState state; };` is a separate type from the true world, so the compiler stops "what I think" from being used as "what is".
- `struct Prediction { Action action; MoveOutcome outcome; WorldState state; };`

### 3.2 Budget counter
`class BudgetCounter`:
- `explicit BudgetCounter(int limit)`: a negative limit counts as 0;
- `bool tryUse()`: if used < limit, it adds 1 and returns true; otherwise it returns false and changes nothing;
- `int used() const`;
- `int remaining() const`, which is limit − used;
- `void reset()`, which sets used back to 0.

This is the **only** way a prediction may be paid for.

### 3.3 World model
- `using ActionModel = std::function<ActionResult(const WorldState&, Action)>;`
- `class WorldModel` has two constructors:
  - `WorldModel()` uses the true rules (`applyAction`);
  - `explicit WorldModel(ActionModel)` uses the given model. **If the given function is empty, it uses `applyAction` instead**, so an empty function is never called (calling one would throw).
- `std::optional<Prediction> predict(const Belief&, Action, BudgetCounter&) const`:
  - it calls `budget.tryUse()` first; if that fails, it returns an empty optional;
  - otherwise it applies the action model to `belief.state` and returns `{action, outcome, state}`;
  - it never changes the belief.

### 3.4 Comparison
- `struct PositionDifference { bool differs; Position predicted; Position observed; };`
- `struct WallDifference { Position cell; bool predictedBlocked; bool observedBlocked; };`
- `struct StateDifference { PositionDifference agent, object, destination; std::vector<WallDifference> walls; };`

  The C++ declares these one member per line (R2). The line above is shorthand.
- `compareStates(predicted, observed)` reports **every** difference, in this order:
  1. the agent;
  2. the object;
  3. the destination;
  4. the walls, **row by row** from the top (y = 0…9), and left to right within a row (x = 0…9).

  It reads walls only with its own loop counters, so it is safe for any input.
- `hasDifferences`: any position differs, or the wall list is not empty.
- `struct PredictionComparison { MoveOutcome predictedOutcome; MoveOutcome observedOutcome; StateDifference state; };`
- `comparePrediction(prediction, observedResult)` returns `{prediction.outcome, observed.outcome, compareStates(prediction.state, observed.state)}`.
- `outcomesDiffer(c)` is true when the two outcomes differ.
- `matches(c)` is true only when the outcomes are equal **and** there are no state differences.

### 3.5 Difference text
`describeDifferences(StateDifference)` gives one line per difference, in the order of 3.4, exactly:
```
  - agent: predicted (4, 4), observed (4, 3)
  - object: predicted (7, 7), observed (7, 6)
  - destination: predicted (1, 8), observed (2, 8)
  - wall at (5, 4): predicted free, observed blocked
```
Each line is two spaces, a hyphen and a space; positions are written `(x, y)`; walls are `blocked` or `free`.

### 3.6 Golden values from the original code (new in the rebuild)
**How it was made** (all in `tools/reference/stage2/`):
1. `orig_trace/original_src/` holds the original `src/model/*`, `src/compare/*` and `src/demo/prediction_demo.*` from `7587f38`, byte for byte: 12 files, identical at `stage-2`, with their hashes verified.
2. `orig_trace/orig_prediction_trace.cpp` is compiled only against those files plus the Stage 1 copies of the original world (`tools/reference/stage1/orig_trace/original_src/`).
3. `gen_golden.py cases` writes 1,000 cases (seed 20261021). Each is a valid random true world and a valid belief: the same world, sometimes with 1–3 wrong walls, or a wrong agent, object or destination position. Half the time the object is put next to the agent, and half of those times the action points at it, so that pushes are common.
4. The original tool computed the results. `ref_model.py` reproduces the file **byte for byte**.

**Each case in `golden/prediction_golden.txt`:**
- the line `case <n> action <Action> differences <k>`;
- 10 belief rows, then 10 true-world rows;
- the line `result <predicted outcome> <observed outcome> <match|MISMATCH>`;
- the k lines of `describeDifferences`.

The result comes from predicting with a fresh `WorldModel()` and `BudgetCounter(1)`, observing with `Environment(true world).step(action)`, and comparing with `comparePrediction`.

**Coverage.**
- **Results:**

  | Predicted / observed | Match | Mismatch |
  |---|---|---|
  | Moved / Moved | 196 | 282 |
  | Blocked / Blocked | 105 | 159 |
  | Pushed / Pushed | 58 | 43 |
  | Moved / Pushed | — | 51 |
  | Moved / Blocked | — | 59 |
  | Blocked / Moved | — | 33 |
  | Blocked / Pushed | — | 12 |
  | Pushed / Moved | — | 2 |
- **Difference lines:** 211 agent, 272 object, 167 destination and 437 wall lines.
- **Cases by number of differences:** 0 (361), 1 (306), 2 (224), 3 (103) and 4 (6).

**SHA-256** (added to `docs/PROVENANCE.sha256` with the 12 original files: 98 → 111 preserved files):
```
5e44df6977bcb5a261766f7445c000db86075ef8180a3fcc581aac9a8bc9f2cf  tools/reference/stage2/golden/prediction_golden.txt
```

### 3.7 The prediction demonstration
`runPredictionDemos(std::ostream&)` runs two demos. Both use the Stage 1 demo layout and five East actions, and each has its own `Environment` and `BudgetCounter(10)`. For each step, the demo:
1. predicts from the belief;
2. steps the environment;
3. compares the two;
4. prints the step line and any difference lines;
5. then replaces the belief with the observed state. This is a **demonstration shortcut, not a correction policy**, and the output says so.

The two demos:
- **Demo 1:** the belief equals the truth.
- **Demo 2:** the belief wrongly has a wall at (6, 3).

If a prediction were refused, the demo would print `No prediction available; stopping this demo.` and stop that demo. It never happens, because 10 > 5. The demo layout is not copied: `scripted_demo` exposes `demoLayout()`, and both demos use it (R4). In the original, `prediction_demo.cpp` kept its own copy; the text is identical.

### 3.8 The program (exactly; 55 lines, 50 non-blank)
```
AI Hologram v2 -- Stage 2
Grid world with prediction and comparison (scripted demonstrations). No reasoning components yet.
<the Stage 1 demo, exactly as in STAGE_01.md 3.8, from its first empty line to "Steps taken: 5">

Prediction demo 1: correct belief
Step 1: East | predicted Moved, observed Moved | match
Step 2: East | predicted Pushed, observed Pushed | match
Step 3: East | predicted Pushed, observed Pushed | match
Step 4: East | predicted Pushed, observed Pushed | match
Step 5: East | predicted Blocked, observed Blocked | match
Mismatches: 0
Model evaluations used: 5

Prediction demo 2: belief wrongly includes a wall at (6, 3)
Step 1: East | predicted Moved, observed Moved | MISMATCH
  - wall at (6, 3): predicted blocked, observed free
Step 2: East | predicted Pushed, observed Pushed | match
Step 3: East | predicted Pushed, observed Pushed | match
Step 4: East | predicted Pushed, observed Pushed | match
Step 5: East | predicted Blocked, observed Blocked | match
Mismatches: 1
Model evaluations used: 5
Note: in these demos the belief is replaced by the observation after every step.
How beliefs are corrected is decided in Stage 3.
```
Everything from line 3 on is byte-identical to the original's `runScriptedDemo` followed by `runPredictionDemos`. Both were checked against the original tools. The two header lines differ on purpose (D-062).

**A historical note.** The original plan's Section 5.6 observed that comparing full states reveals a wrong wall immediately, at step 1 of Demo 2. That question was settled in the frozen protocol and is not reopened here: Stage 2 provides the neutral comparison tools, exactly as the original did.

## 4. Files
All new code is in namespace `aih`, compiled into `ai_hologram_core`.

| File | Approx. lines | Job | Sub-stage |
|---|---|---|---|
| `src/model/belief.h` | 30 | `Belief` | 2a |
| `src/model/prediction.h` | 35 | `Prediction` | 2a |
| `src/model/budget_counter.h` / `.cpp` | 45 / 50 | `BudgetCounter` (3.2) | 2a |
| `src/model/world_model.h` / `.cpp` | 55 / 50 | `ActionModel`, `WorldModel::predict` (3.3) | 2a |
| `src/compare/state_difference.h` / `.cpp` | 75 / 85 | 3.4 | 2b |
| `src/compare/difference_text.h` / `.cpp` | 30 / 70 | `describeDifferences` (3.5) | 2b |
| `src/demo/prediction_demo.h` / `.cpp` | 25 / 130 | `runPredictionDemos` (3.7) | 2c |
| `src/demo/scripted_demo.h` / `.cpp` | — | Exposes `demoLayout()`; the demo text is unchanged | 2c (changed) |
| `src/main.cpp`, `src/core/project_info.cpp` | — | Header (3.8), then both demos; `currentStage()` returns 2 | 2c (changed) |
| `tests/test_budget_counter.cpp` | 90 | BC1–BC4b | 2a |
| `tests/test_world_model.cpp` | 180 | WM1a–WM8 | 2a |
| `tests/test_compare_states.cpp` | 160 | CM1–CM6 | 2b |
| `tests/test_compare_predictions.cpp` | 120 | CM7–CM10 | 2b |
| `tests/golden_prediction_reader.cpp` | 180 | The prediction-golden reader (6.5); declared in `golden_readers.h` | 2b |
| `tests/test_prediction_golden.cpp` | 130 | G3, PN1–PN7 | 2b |
| `tests/test_demo.cpp` | — | D2 added | 2c (changed) |
| `tests/test_suites.h`, `test_main.cpp`, `test_project_info.cpp`, `golden_readers.h`, `CMakeLists.txt` | — | New groups and files; P2 checks 2 from 2c | 2a–2c (changed) |
| `tools/reference/stage2/` | — | The reference, the golden file and the original-code tool (committed with this plan) | plan |

**Splits planned in advance.** The original's 200-line `test_compare.cpp` becomes `test_compare_states.cpp` (state comparison) and `test_compare_predictions.cpp` (prediction comparison and text): two jobs. If `test_world_model.cpp` or the reader cannot hold its code cleanly within 200 lines, the coding assistant stops and reports; nothing is squeezed.

**One shared helper per job (R4):**
- predictions only through `WorldModel::predict` and the counter;
- wall comparison only in `compareStates`;
- position text only in `difference_text`;
- the demo layout only in `scripted_demo` (`demoLayout()`);
- text helpers (`splitWords`, `startsWith`, `parseInteger`, `readLines`) only in `golden_files`;
- test layouts only in `test_layouts`.

## 5. Expected output
- **The program:** exactly 3.8, with 55 lines, 50 of them non-blank.
- **The tests:**

  | After | Expected line |
  |---|---|
  | 2a | `157 checks, 0 failures` |
  | 2b | `176 checks, 0 failures` |
  | 2c | `177 checks, 0 failures` |

  The 177 are:
  - the 141 Stage 1 checks (P2 now checks 2);
  - BC1–BC4b: 5;
  - WM1a–d, WM2–WM8: 11;
  - CM1–CM4, CM5a–b, CM6–CM10: 11;
  - G3: 1;
  - PN1–PN7: 7;
  - D2: 1.
- **CTest:** 2/2.
- **Code checker:** 0 violations.

  | After | Files checked |
  |---|---|
  | 2a | 69 |
  | 2b | 77 |
  | 2c | 79 |
- **`run_checks.ps1`** ends with `ALL CHECKS PASSED (111 preserved files, 177 checks, 79 code files checked, formatting checked)`.

## 6. Tests
Each description starts with its id. `tools/reference/stage2/checks.py` mirrors every check, with the exact data: `failing: none (36 checks)`. Every fixture is a valid world unless the check is about invalid states. Wall changes are only on interior cells that hold no agent, object or destination. OPEN, DEMO and `at` are the Stage 1 test layouts (`tests/test_layouts`).

### 6.1 Budget counter (`test_budget_counter.cpp`), 2a
| Id | Check |
|---|---|
| BC1 | `BudgetCounter(3)`: used 0, remaining 3 |
| BC2 | `tryUse` four times gives true, true, true, false; used 3 |
| BC3 | after `reset()`: used 0, and `tryUse` is true |
| BC4a | `BudgetCounter(0)`: `tryUse` false; used 0; remaining 0 |
| BC4b | `BudgetCounter(-2)` behaves like 0: `tryUse` false; used 0; remaining 0 |

### 6.2 World model (`test_world_model.cpp`), 2a
| Id | Check |
|---|---|
| WM1a–d | From OPEN, for North, East, South and West: the default model's prediction (budget 1) has the outcome and state of `applyAction` on OPEN. Four checks. |
| WM2 | two predictions with `BudgetCounter(5)` leave used = 2 |
| WM3 | a used-up `BudgetCounter(1)`: `predict` returns nothing; used stays 1; the belief is unchanged |
| WM4 | `predict` does not change its belief |
| WM5 | belief `at(OPEN,(0,4),(7,7))` (agent on a wall): outcome InvalidState, state unchanged, used 1 |
| WM6 | a custom model that always returns (state unchanged, Blocked): from OPEN, East is Blocked, although the true rules say Moved |
| WM7 | the prediction records the action asked for (West) |
| WM8 | a `WorldModel` built from an **empty** `ActionModel` gives the same predictions as the default for all four actions (one check) |

### 6.3 State comparison (`test_compare_states.cpp`), 2b
Base = parsed OPEN (agent (4,4), object (7,7), destination (1,8)).

| Id | Check |
|---|---|
| CM1 | base vs a copy: no differences |
| CM2 | base vs base with the agent at (4,3): exactly the agent differs, (4,4) → (4,3); no other difference |
| CM3 | the object at (7,6): exactly the object differs, (7,7) → (7,6) |
| CM4 | the destination at (2,8): exactly the destination differs, (1,8) → (2,8) |
| CM5a | base plus a wall at (5,5) as predicted, base as observed: exactly one wall difference ((5,5), predicted blocked, observed free) |
| CM5b | the reverse: ((5,5), predicted free, observed blocked) |
| CM6 | predicted: base plus walls at (5,4) and (4,5); observed: base with the agent at (4,3). The agent differs, nothing else does, and the walls are exactly [(5,4) blocked→free, (4,5) blocked→free], **in that order**: row 4 comes before row 5. This is not a diagonal, so a column-first order fails (the original's lesson). |

### 6.4 Prediction comparison and text (`test_compare_predictions.cpp`), 2b
| Id | Check |
|---|---|
| CM7 | prediction (East, Blocked, base) vs the observed `applyAction(base, East)` (Moved): `outcomesDiffer`, and not `matches` |
| CM8 | prediction (East, Moved, s) vs the observed (s, Moved), with s = `applyAction(base, East).state`: `matches` |
| CM9 | prediction (East, Moved, base plus a wall at (5,5)) vs the observed (base, Moved): equal outcomes, but not `matches` |
| CM10 | `describeDifferences(compareStates(base, observed))`, with observed = base with agent (4,3), object (7,6), destination (2,8) and walls at (5,4) and (4,5), gives exactly these five lines in order: `  - agent: predicted (4, 4), observed (4, 3)`, `  - object: predicted (7, 7), observed (7, 6)`, `  - destination: predicted (1, 8), observed (2, 8)`, `  - wall at (5, 4): predicted free, observed blocked`, `  - wall at (4, 5): predicted free, observed blocked` |

### 6.5 The prediction golden file (`test_prediction_golden.cpp`, `golden_prediction_reader.cpp`), 2b
**The reader.** `checkPredictionGoldenLines(lines)` returns `GoldenCheck {ok, cases, problem}`. It follows `ref_prediction_reader.py` exactly: the same order of checks and the same messages. It checks that each line exists before reading it, and it never throws. Per case:
1. The header must be exactly six words `case <n> action <Action> differences <k>`, with n and k integers (`parseInteger`), k ≥ 0, and a valid action name. Otherwise: `line <i>: invalid case header`.
2. Fewer than 20 lines left: `case <n>: missing world lines`.
3. The belief rows do not parse: `case <n>: belief does not parse: <error>`. The true rows do not parse: `case <n>: true world does not parse: <error>`.
4. No line left: `case <n>: missing result line`. The line does not start with `result `: `case <n>: malformed result line`.
5. Fewer than k lines left: `case <n>: missing difference lines`.
6. The reader then computes the case (3.6) and compares, in this order:
   - the result line: `case <n>: expected [<file>] actual [<computed>]`;
   - the number of difference lines: `case <n>: expected <k> difference lines, actual <m>`;
   - each line: `case <n>: difference line <j>: expected [<file>] actual [<computed>]`.

**G3:** the reader on the real file passes only with ok and exactly **1,000** cases. A missing file is a FAIL with a clear message.

**Malformed-file tests**, each comparing the whole (ok, cases, problem). R = `result Moved Moved match`, H = `case 1 action East differences 0`.

| Id | Lines | Expected |
|---|---|---|
| PN1 | `case 1 action Up differences 0`, DEMO, DEMO, R | (false, 0, `line 1: invalid case header`) |
| PN2 | H, DEMO's first 5 lines | (false, 0, `case 1: missing world lines`) |
| PN3 | H, DEMO with line 0 = `####.#####`, DEMO, R | (false, 0, `case 1: belief does not parse: Every border cell must be a wall (#).`) |
| PN4 | H, DEMO, DEMO | (false, 0, `case 1: missing result line`) |
| PN5 | H, DEMO, DEMO, `outcome Moved Moved match` | (false, 0, `case 1: malformed result line`) |
| PN6 | `case 1 action East differences 1`, DEMO, DEMO, R | (false, 0, `case 1: missing difference lines`) |
| PN7 | H, DEMO, DEMO, R | (true, 1, empty), a positive control |

### 6.6 The demo (`test_demo.cpp`), 2c
- **D2:** `runPredictionDemos` writing to a `std::ostringstream` gives exactly the prediction part of 3.8, from the empty line before `Prediction demo 1` to `How beliefs are corrected is decided in Stage 3.\n` (`checks.py` DEMO2_TEXT).

## 7. Implementation order
| Step | Sub-stage | Content | Tests (total) | Deliberate failure checks (tried by Claude first, R11; exact results in `df_check.py`) |
|---|---|---|---|---|
| 1 | — | The owner commits this plan, `tools/reference/stage2/`, the provenance update (111), the decisions and the 2a prompt, on a new branch `stage-2-work` | | **Owner, DF2-P:** add one character to `prediction_golden.txt`. It must stop at `[provenance]`; restore it with `git checkout`. |
| 2 | **2a** | belief, prediction, budget counter, world model | BC, WM (157) | **Cursor, DF2a-1:** `tryUse` uses `>` instead of `>=`. Exactly BC2, BC4a, BC4b and WM3 must fail. **Cursor, DF2a-2:** the custom-model constructor ignores its model and always uses `applyAction`. Exactly WM6 must fail. |
| 3 | **2b** | comparison, difference text, prediction-golden reader | CM, G3, PN (176) | **Cursor, DF2b-1:** the wall test becomes "predicted blocked and observed free" (the original Stage 2 check). Exactly CM5b, CM10 and G3 must fail. **Cursor, DF2b-2:** walls are reported column by column. Exactly CM6, CM10 and G3 must fail. |
| 4 | **2c** | prediction demo, `demoLayout()`, main, `currentStage()` = 2, README | D2 (177) | **Cursor, DF2c-1:** the demos' budget limit is 4 instead of 10. Exactly D2 must fail. **Owner, DF2c-2:** in `difference_text.cpp`, change `", observed "` to `", seen "`. `run_checks.ps1` must stop at `[tests]` with exactly CM10, G3 and D2 failing. |
| 5 | — | Fresh clone of `stage-2-work`: `run_checks.ps1` passes | | |
| 6 | — | ChatGPT reviews the **whole buildable tree** of the final commit (`git archive`); completion gate; tag `stage-2-v2`; merge into `main` | | |

Each sub-stage is committed by the owner on `stage-2-work`, after Claude's review.

## 8. Decisions to record (with the plan's commit, R12)
| ID | Decision | Reason |
|---|---|---|
| D-066 | The rebuilt model and comparison code reproduce the original (`src/model`, `src/compare`, `src/demo/prediction_demo` at `7587f38`) exactly, including the text formats. Namespace `aih`. The original's `test_compare.cpp` is split into two files by job. The demo layout is shared through `demoLayout()` instead of being copied. | C1 uses this code; M1 requires step-for-step equivalence. R4. |
| D-067 | A golden file of 1,000 prediction cases is produced once from the original C++ (`tools/reference/stage2/orig_trace/`) and committed with its commit and hash. The 12 original source files are hash-protected too (111 preserved files). Test G3 must reproduce the file; PN1–PN7 test its reader with malformed input. | The method of D-061, applied to Stage 2. |

## 9. Documentation
- **Headers and comments.** Every new file has the three-part header, and beginner comments explain:
  - `std::function`, and why it is used instead of inheritance;
  - `using` aliases;
  - `std::optional` as "no prediction";
  - a `const` member function with a non-`const` reference parameter;
  - why the comparison never indexes walls with stored positions.
- **README** (2c). The status becomes "Stage 2 of the rebuild: prediction and comparison (belief, world model, budget counter, comparison and difference text), checked against golden values from the original code. Nothing plans, chooses actions or corrects beliefs yet. No experiment has been run."
- **Handoff notes.** One per sub-stage.
- **Review notes.** Every exchange with ChatGPT goes in `docs/reviews/stage2/`.

## 10. Completion checklist (owner verification gate)
- [ ] The plan was agreed and committed before any Stage 2 code.
- [ ] `run_checks.ps1` passes on the owner's computer **and on a fresh clone**, with the exact final line of Section 5.
- [ ] The program output is exactly 3.8, at 55 lines; the tests show `177 checks, 0 failures`; CTest 2/2; 0 checker violations (79 files).
- [ ] G3 passes all 1,000 cases, and PN1–PN7 pass.
- [ ] All deliberate failure checks gave exactly the predicted failures.
- [ ] D-066 and D-067 recorded; handoff notes appended; prompts and review notes saved.
- [ ] ChatGPT reviewed the whole buildable tree of the final commit.
- [ ] The owner agrees (D-065).

## 11. Plain explanation (for any reader)
1. **A belief is a different type from the true world,** so code cannot mix "what I think" with "what is".
2. **Predicting never changes the belief,** and every prediction is paid for, so all conditions get the same computing budget.
3. **`std::function` lets a different action model be plugged in** (a faulty one, later) without class hierarchies.
4. **A prediction can have the right outcome and still not match**, as at step 1 of Demo 2: the wrong wall shows in the full comparison.
5. **The golden file films the original prediction code** on 1,000 cases, and the new code must match every one.

## 12. What actually happened (filled in at the end)
```
Date completed:
Commit tagged stage-2-v2:
Checks:
Did everything go as planned?
```

## 13. Instructions to the coding assistant
Every prompt says:
- follow `AGENTS.md` and `.cursor/rules`;
- implement only the named sub-stage;
- read the reference (`tools/reference/stage2/`) for exact behaviour and test data;
- never commit, and never change an expected or golden value; report any mismatch with its id or case;
- format with CLion's clang-format;
- run `run_checks.ps1` and report its final line exactly.

## Revision history
| Version | Date | Change |
|---|---|---|
| 1.0 | 2026-10-20 | First draft. The reference, the reader, the 36-check mirror, the deliberate failure predictions and the golden file (produced from the original C++, reproduced byte for byte by the reference) were all built first (R9, R11). The demo text was checked against the original. |
