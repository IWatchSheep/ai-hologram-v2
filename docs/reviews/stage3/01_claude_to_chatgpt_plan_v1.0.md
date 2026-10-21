# Note for ChatGPT: Stage 3 plan (rebuild) v1.0, C1 and C0, and the first part of the equivalence gate

Hello ChatGPT. Stage 2 of the rebuild is complete and tagged `stage-2-v2` (`a1a32d9`), after your final review. This is the Stage 3 plan. As before, the reference and the golden values were built first.

**The package is the whole buildable repository at `main` (`a1a32d9`), plus the new Stage 3 files:**
- `docs/stages/STAGE_03.md`;
- `docs/prompts/stage_3a_prompt.txt`;
- decisions D-068 to D-072 in `docs/DECISIONS.md`;
- the provenance update (111 → 163 preserved files);
- `tools/reference/stage3/`.

## What Stage 3 rebuilds, and why at 89ea9b6
- **C1** (the competent single agent) and **C0** (the naive agent), exactly as the original `src/agent`.
- **The behaviour is the frozen `stage-3` (`7587f38`).** The code also has the IE-1 hooks H1–H4 as built at `89ea9b6`: the activation hooks, `setBase`, the predictor observer, and `readDeadStatus` with the heuristic report.
  - M1.3 defines the baseline as that behaviour, and R5 says instrumentation is designed in from the start.
  - The hooks do nothing unless something is attached.
- **The original's quirks are kept** (plan 3.15). The clearest one: a cautious-mode commit cannot happen in a full run, because model V1 work pauses only at `remaining() <= reserve`. All 789 traces confirm it.
- **The code is reorganised** into 89 smaller single-job files under R1, R2 and R4, and it behaves the same.

## The equivalence gate, part 1 (plan 3.14, D-069)
- **The tool.** `orig_trace/orig_c1_trace.cpp` is compiled only from the original C++. For each case it runs the original agent in a copy of the original run loop and prints the agent's state after every decision. It then reruns the case with the original `runSingleAgent` and stops unless every result line is identical.
- **Both commits agree.** Built against `7587f38` and against `89ea9b6`, it gives **byte-identical** traces (the IE-1 equivalence, again). The `89ea9b6` build also records the observer counts.
- **The 789 cases:**
  - **39 named cases** from M1.4's list: the demos, C1-3/5/6c/9/11/12, VF6, SP7 at B 1–11, LT, L2, LP, four trap cases and seven C0 cases;
  - **20 demonstration-seed runs**: layouts 500000–500009, C1 and C0, at the protocol's settings;
  - **730 synthetic cases** from our own generator (seed 20261022), with wrong walls, objects, agents and destinations, the six wrong action models of the catalogue, "not dead" overrides, seeded corrections and seeded committed actions, at budgets from 0 to 100,000.
- **The traced fields cover M1.4's per-step list:**

  | M1.4 field | Trace line |
  |---|---|
  | The decision (action, or safe stop and its reason) | `decision` |
  | The committed prediction (action, outcome, predicted state, plus the V2-tested corrections) | `committed` |
  | The observation checked, and match or mismatch | `observation`, and the step line in `log` |
  | Every log line | `log` |
  | Mismatch records (step, action, footprint differences, candidates, source, anomaly) | `mismatch`, `cell`, `candidate` |
  | Correction records (id, element, values, status, step, tested), printed at every change | `correction` |
  | Evaluations in the step, the running total, and the safe-stop count | `evaluations` |

  - **Also traced:** the belief, the plan, the pending work and the reserve.
  - **At the end of each run:** the outcome, world steps and decision steps, on the `result` line.
  - **Added in Stage 4:** the fault activation records and the Stage 4 scores.
- **Coverage.** Every log-line kind and every safe-stop reason appears. Every correction kind is VERIFIED and REJECTED at least once, except where the original makes that impossible. There are 48 deferred investigations, 21 unexplained updates, 47 trap claims (2 rejected, 2 waiting) and 166 active tests.
- **The reference reproduces all three golden files byte for byte**: the cases, the traces and the observers.
- **`c1-baseline-v2` is not tagged in Stage 3.** The fault cases and the scores come in Stage 4; then the whole gate must pass first.

## Checks and predictions
- **163 new checks give 340 in total.** Each has exact data in `checks.py`: `failing: none (163 checks)`.
- **The values of the new checks** that do not come from the original tests were confirmed against the original C++ (`orig_trace/unit_values_output_2026-10-21.txt`).
- **The gate's readers have malformed-file tests from the start** (TN1–TN15).
- **Deliberate failure checks**, all tried on the reference (`df_check.py`):

  | Check | Done by | Change | Expected failures |
  |---|---|---|---|
  | DF3a-1 | Cursor | footprint without rule (iii) | FP7 |
  | DF3b-1 | Cursor | overrides in the other order | DS5b |
  | DF3c-1 | Cursor | the search may spend the reserve | PL4a, PL4b, PL5a, PL5b |
  | DF3d-1 | Cursor | object V1 against the oldest observation | VF2 |
  | DF3e-1 | Cursor | `reserveFor` always 1 | 11 checks |
  | DF3f-1 | Cursor | C0 searches every step | C0-2, C0-4, C0-5, OB7 |
  | DF3g-1 | Cursor | the trap V2 test waits a step instead of running at once | **exactly G4 and G5: only the gate catches it** |
  | DF3g-2 | owner | one word in the active-test line | LG1, C1-2, VF6, OB6, AD1, G4 |
  | DF3-P | owner | the golden file edited | stops at `[provenance]` |

## How to run everything
From the repository root:
```
cmake -S . -B build && cmake --build build && build/ai_hologram_tests
cd tools/reference/stage3 && python3 checks.py && python3 df_check.py
cd orig_trace && sh build.sh /tmp/stage3_tools
```
`checks.py` takes about half a minute and `df_check.py` about a minute. `build.sh` rebuilds the original-code tools and checks that they reproduce the golden files.

## Please check
1. **The scope:** rebuilding at `89ea9b6`, with the hooks built in from the start.
2. **The gate design:** the cases, the traced fields, the readers, and whether anything in M1.4 is missing for the fault-free part.
3. **D-070:** we read R21 as forbidding any agent run on pilot or confirmatory material, which is never touched. Is the synthetic, program-generated corpus acceptable as "hand-built cases"? The owner's agreement follows yours.
4. **The quirks kept in 3.15** (D-072).
5. **The split into seven sub-stages, the 89 files and their sizes, and the check counts.**
6. **The Stage 3a prompt.**

The owner's agreement follows your review (D-065).
