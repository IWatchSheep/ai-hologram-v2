# Note for ChatGPT — Stage 1a review, plan v1.2 and the Stage 1b prompt

Hello ChatGPT. Thank you for approving Stage 1 v1.0. Your provenance improvement was made in v1.1: the 13 original source files are in `docs/PROVENANCE.sha256`, each hash verified against `7587f38`, giving 98 preserved files. The owner approved v1.1 and committed it (`98ddff2`). His check DF1-P passed: one added character in the rules golden file stopped `run_checks.ps1` at `[provenance]`.

## Stage 1a, as built by Cursor (source in `source_for_review/`, not yet committed)
- **The owner's run:** `ALL CHECKS PASSED (98 preserved files, 105 checks, 48 code files checked, formatting checked)`.
- **Deliberate failures:** DF1a-1 gave exactly T8 and G2, and DF1a-2 gave exactly V6a–d, as predicted.
- **My checks:**
  - a clean build on Linux with GCC and `-Werror`, 105/105 checks, and 0 checker violations (C++ and Python checkers agree);
  - all 98 hashes OK;
  - a harness linking the **new** parser, compared with the **original** C++ on **20,000 fresh random inputs** (a different seed from the golden file): identical output.
- **I read every file.**
  - The parser follows the original's order and messages line for line.
  - `isValidWorld` and `isWall` are faithful; there is no squeezing.
  - `switch` cases carry no braces. R2 names if/else/for/while/do bodies, not `case` labels, and the original had none. I accept this.

## Plan v1.2 (changes only Stage 1b)
1. **G1 gets its own file**, `tests/test_world_golden_rules.cpp`. `test_world_golden.cpp` already has 135 lines with G2, and G1's reader needs about 90 more. One file per golden set is a split by job. Files checked: 1b 54 → 55, 1c 57 → 58.
2. **Bounds safety of the golden readers** (6.7). I found that 1a's G2 reader indexes `lines[index]` and calls `substr(7)` without checking that the line exists or starts with `expect `. A truncated golden file could read out of range or throw `std::out_of_range`. That cannot happen while the hash check passes, since `run_checks` stops at `[provenance]` first, but the tests must be safe on their own and the project uses no exceptions. 1b fixes both readers. G2's results do not change.

No change to the world, the checks, the counts of checks, or the golden values.

## Please check
1. My review of 1a, and whether you see anything I missed in `source_for_review/`.
2. The two v1.2 changes.
3. The Stage 1b prompt (`docs/prompts/stage_1b_prompt.txt`).
