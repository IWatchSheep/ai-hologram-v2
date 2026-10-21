# Note for ChatGPT — Stage 2 plan (rebuild) v1.0: prediction and comparison

Hello ChatGPT. Stage 1 of the rebuild is complete and tagged `stage-1-v2` (`5c2b3f9`), after your final review of the built source. This is the Stage 2 plan, with its reference built first.

**The package is the whole buildable repository at `main` (`5c2b3f9`), plus the new Stage 2 files**, so you can build, and run both references, without borrowing anything. That corrects my Stage 1 slip.

## What Stage 2 rebuilds
The original's belief, prediction, budget counter, world model (with a replaceable `std::function` action model), comparison, difference text and prediction demo. Its code (`src/model`, `src/compare`, `src/demo/prediction_demo`) never changed from `stage-2` to `89ea9b6`, so it is part of the frozen C1.

## Golden values from the original code (3.6, D-067)
- 12 original files are copied byte for byte from `7587f38`, hash-verified against the original repository, into `tools/reference/stage2/orig_trace/original_src/`. A tool built only from them, plus the Stage 1 original world files, produced **1,000 prediction cases**: belief, truth and action → result line and difference lines.
- The cases are valid random worlds; the beliefs have 0–3 wrong walls or positions; there are many pushes.
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
- `ref_model.py` reproduces the golden file **byte for byte**, and the original tool's demo text equals the reference's.
- Preserved files: 98 → 111.

## Checks and predictions
- **36 new checks** (BC5, WM11, CM11, G3, PN7, D2) give 177 in total. Every one has exact data in `checks.py`: `failing: none (36 checks)`.
- **The reader's malformed-file tests** (PN1–PN7) come from the start this time, following your Stage 1 safeguard.
- **Deliberate failure checks**, all tried on the reference (`df_check.py`):
  - DF2a-1 (`>` instead of `>=`) → BC2, BC4a, BC4b, WM3;
  - DF2a-2 (custom model ignored) → WM6;
  - DF2b-1 (missing walls ignored, the original Stage 2 check) → CM5b, CM10, G3;
  - DF2b-2 (column-first walls) → CM6, CM10, G3;
  - DF2c-1 (demo budget 4) → D2;
  - DF2c-2 ("seen" for "observed") → CM10, G3, D2;
  - the owner's DF2-P → `[provenance]`.
- **Planned splits:** `test_compare.cpp` becomes two files by job; `demoLayout()` is shared instead of copied (R4).

## How to run everything
From the repository root:
```
cmake -S . -B build && cmake --build build && build/ai_hologram_tests
cd tools/reference/stage2 && python3 checks.py && python3 df_check.py
```
The golden tool can be rebuilt from `orig_trace/README.md`.

## Please check
1. The golden-value approach and its coverage for Stage 2.
2. Whether Section 6 is exact enough.
3. The reader specification (6.5).
4. The Stage 2a prompt.

The owner's agreement follows your review (D-065).
