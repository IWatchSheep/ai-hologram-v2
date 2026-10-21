# Note for ChatGPT — review of the Stage 0 plan (rebuild), v1.0

Hello ChatGPT. This is the first stage plan for the new repository `IWatchSheep/ai-hologram-v2` (first commit `0484b6d`), written under the rules v3 (`docs/RULES.md`) you reviewed and the owner approved.

**What Stage 0 is:** only the frame: a C++17 CMake project, a quiet test program, a **code checker** that enforces R1 (200 lines per file, 40 per function), R2 (braces, one statement and one declaration per line) and R7 (agent code never includes experiment code), and the complete `run_checks.ps1`. No world, agents or experiment code.

**Rules followed while writing it:**
- R9/R10: the checker's reference (`tools/reference/stage0/ref_code_checker.py`) was built and run first; all 27 snippet expectations come from it, and `snippets.py` checks itself.
- R11: the two code-level deliberate failure checks were tried on the reference first, and gave exactly the predicted failures (CK3, CK22, CK25; and CK7).
- R23: every file is named, with its approximate size and sub-stage.
- Calibration: on the old repository's C++ the reference finds 125 genuine violations, which shows the rules catch what Cursor did before.

**Please look especially at:**
1. **Section 3.4.** Are the checker's rules exact enough for an implementer to get the same answers as the reference, with no guessing? In particular, the function-body detection and the template `<`/`>` rule for declarations.
2. **The snippet table (6.2).** Are important cases missing? We know of these limits:
   - a `,` inside a macro call;
   - `>>` closing two templates (handled: each `>` closes one);
   - a function-try-block (not allowed anyway: no exceptions).
3. **Section 3.5 step 4.** Is it acceptable to fail the build on any line containing `warning:`?
4. **Open points O2–O3** (Section 14). O1 is settled: the formatter is CLion's own plugin copy of `clang-format` (D-054).
5. Anything that could let a rule be bypassed quietly.

The owner's approval follows your review. Thank you.
