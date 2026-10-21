# Note for ChatGPT — Stage 1 plan (rebuild) v1.0: the grid world

Hello ChatGPT. Stage 0 of the rebuild is complete and tagged `stage-0-v2` (`148ea6e`): the gate passed, and the fresh clone passed. This is the Stage 1 plan, with its reference built first (R9).

## What is new compared with the original Stage 1
The world must behave **exactly** like the original, because the rebuilt C1 will later have to reproduce the original step for step (M1.4). The original world code (`src/world`) never changed from `stage-1` to `89ea9b6`, so it is also the world of the frozen baseline. The plan therefore adds **golden values produced from the original C++**, the same method M1.4 prescribes for C1 traces, applied early to the first comparable component:
- `tools/reference/stage1/orig_trace/` holds 13 original files copied byte for byte from `7587f38` (hashes in its README), and a small tool compiled only against them;
- `golden/world_rules_golden.txt`: 5,184 rule cases. Every agent cell (walls included, plus 8 outside the grid) × 6 object positions × 4 actions, on two layouts. The outcomes are 924 Moved, 230 Pushed, 454 Blocked and 3,576 InvalidState;
- `golden/world_parse_golden.txt`: 1,009 parser cases, 9 fixed plus 1,000 seeded mutations, with exact error messages;
- `ref_world.py` reproduces both files **byte for byte**, and the original tool's demo output equals the reference's;
- the two golden files' SHA-256 values go into `docs/PROVENANCE.sha256` (83 → 85 preserved files), so `run_checks.ps1` protects them.

## Checks and predictions
- **Checks.** 58 new checks, listed exactly in plan Section 6, give 129 in total. `checks.py` mirrors them on the reference: `failing: none (58 checks)`.
- **Deliberate failure checks**, all tried on the reference first (`df_check.py`):
  - DF1a-1 (border test without y = 9) → T8, G2;
  - DF1a-2 (`isWall` false outside the grid) → V6a–d;
  - DF1b-1 (a blocked push moves the agent) → R4, G1, D1;
  - DF1c-1 (four demo actions) → D1;
  - DF1c-2 (Pushed reported as Moved) → R3, R5, G1, D1;
  - the owner's DF1-P (golden file edited) → `[provenance]`.
- **Planned splits** (Stage 0 lesson): `world_text.cpp` becomes parse and render, and the original 237-line test file becomes three files. Every file is named with its size.
- **Interesting finding.** DF1a-2 affects only V6, because `applyAction` checks validity first, so a valid state never asks about a cell outside the grid. The golden rules cases confirm this.

## Please check
1. Whether the golden-value approach (3.7, D-061) is sound and sufficient for the world. In particular:
   - the coverage of the rule cases;
   - the choice to commit the original source files under `tools/reference/` (O2).
2. Whether Section 6 is exact enough for an implementer to write identical checks.
3. O1: are golden files of 213 KB and 193 KB acceptable in the repository?
4. The Stage 1a prompt (`docs/prompts/stage_1a_prompt.txt`).

The owner's approval follows your review. Thank you.
