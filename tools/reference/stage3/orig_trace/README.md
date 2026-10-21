# The original-code golden tools (Stage 3 rebuild, plan STAGE_03 3.14)

## The original files
- **`original_src/`** holds files copied **byte for byte** from the original repository `IWatchSheep/ai-hologram` at commit `7587f38` (tag `stage-3`, the frozen C1 baseline): `src/agent/*` (31 files) and `src/demo/agent_demo.*` (2).
- **`original_src_ie1/agent/`** holds the 16 `src/agent` files that differ at `89ea9b6` (stage-3 plus the IE-1 hooks H1–H4): 15 changed files and the new `computation_observer.h`. The other 16 agent files are identical at both commits, so they are not copied twice.
- Every hash was verified against `git show <commit>:src/<path>`, and is listed in `docs/PROVENANCE.md` and `docs/PROVENANCE.sha256`.
- The world, model and compare files are the Stage 1 and Stage 2 copies (`../../stage1/orig_trace/original_src/`, `../../stage2/orig_trace/original_src/`). They are identical at both commits.
- These files are reference material only. They are never built into the project, are exempt from R1, and are never edited.

## The tools (Linux, g++; `sh build.sh`)
- **`orig_c1_trace.cpp`** is built twice:
  - against `7587f38` (`orig_c1_trace_3`);
  - against `89ea9b6`, with `-DWITH_OBSERVER` (`orig_c1_trace_ie1`; `original_src_ie1` first on the include path, and its `.cpp` files instead of the older ones).

  `traces <cases>` prints the decision-step traces, and `observers <cases>` the IE-1 observer counts (89ea9b6 only).

  For each case, it runs the original agent in a copy of the original run loop, printing the agent's state after every decision. It then reruns the case with the original `runSingleAgent` and stops with an error unless the outcome, the counts and every result line are identical. Cases that start after a seeded action cannot be rerun that way.
- **`orig_unit_values.cpp`** prints, from the 89ea9b6 code, the values of the plan's new unit checks that do not come from the original tests. Its output, `unit_values_output_2026-10-21.txt`, matches `checks.py`.
- **`orig_demo.cpp`** prints the original `runAgentDemos` text, which equals `checks.py`'s AD1_TEXT.

## How the golden files were produced (2026-10-21)
```
python3 ../gen_golden.py cases <scratch>          # the 789 cases (seed 20261022)
sh build.sh <tools folder>                        # builds the tools and checks:
#   orig_c1_trace_3   traces    c1_cases.txt  == ../golden/c1_traces.txt
#   orig_c1_trace_ie1 traces    c1_cases.txt  == ../golden/c1_traces.txt   (byte-identical)
#   orig_c1_trace_ie1 observers c1_cases.txt  == ../golden/c1_observers.txt
```
The reference produced byte-identical files with `python3 ../gen_golden.py traces <dir>` and `observers <dir>`.

**The demonstration-seed layouts** 500000–500009 in the case file come from `../../stage4/ref_stage4.py` (`make_layout`). Claude also built the original C++ layout generator from `89ea9b6` (`src/experiment/layout_generator`, `seeded_random` and `true_search`, with the original `dead_squares.cpp`) in a scratch folder, and it printed the same ten layouts, attempts and shortest lengths. Those experiment files are not copied here, because Stage 3 contains no experiment code.
