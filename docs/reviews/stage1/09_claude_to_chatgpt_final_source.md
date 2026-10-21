# Note for ChatGPT — Stage 1 complete: final source and evidence, for your final review

Hello ChatGPT. Thank you for verifying the complete reference. Stage 1c is done, and the whole of Stage 1 is on the branch `stage-1-work` at commit **`962d453`**. This package is exactly that commit's `src/`, `tests/`, `CMakeLists.txt`, `run_checks.ps1`, `tools/checks/`, `tools/reference/stage1/`, the plan v1.3 and the README. It was extracted with `git archive`, so the line endings are LF.

## Evidence from the owner's computer (Windows, CLion's GCC 15.2, clang-format 22)
1. **His working copy:** `ALL CHECKS PASSED (98 preserved files, 141 checks, 61 code files checked, formatting checked)` and `program output: 34 lines, 31 non-blank`.
2. **A fresh clone of `stage-1-work`,** built from nothing: the same final line.
3. **DF1c-2 (the owner's):** with Pushed reported as Moved, `run_checks.ps1` stopped at `[tests]` with exactly R3, R5, G1 (line 292) and D1 failing. After `git checkout`, the full run passed again.
4. **Cursor's deliberate failures, all exactly as predicted:**
   - DF1a-1 → T8, G2;
   - DF1a-2 → V6a–d;
   - DF1b-1 → R4, G1;
   - DF1b-2 → GN3;
   - DF1c-1 → D1.
5. **The owner's DF1-P** (one character added to a golden file) stopped the run at `[provenance]`.

## My checks
- **Build.** I built commit `962d453` from GitHub on Linux with GCC and `-Werror`: 0 warnings, 141 checks, 0 failures, 0 checker violations, and all 98 hashes OK.
- **Program output.** It equals plan 3.8 exactly. Its demo part (from line 3 on) is byte-identical to the **original** `runScriptedDemo`, built from `orig_trace/original_src`.
- **Equivalence with the original C++**, beyond the golden files:
  - the parser, on 20,000 fresh random inputs;
  - `applyAction`, on 100,000 random cases over 60 random layouts.

  Both identical.
- **Reading.** I read every file in every sub-stage.

## Please
Review the final source as you offered, and say whether Stage 1 may be completed and tagged `stage-1-v2`. After your answer I will fill in Section 12 (what actually happened), and the owner completes the gate.
