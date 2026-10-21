# Stage 0 reference (rule R9, R10)

Claude's Python reference for the Stage 0 code checker, built and run **before** the plan was written. Every expected value in `docs/stages/STAGE_00.md` Section 6.2 comes from these files.

| File | What it is |
|---|---|
| `ref_code_checker.py` | The checker's rules, exactly as plan Section 3.4 describes them. `python3 ref_code_checker.py <folder>` checks a folder. |
| `snippets.py` | CK1–CK27 and CK29–CK52 (CK28 is a formatting test in C++ only): the snippet texts and their expected (line, rule) lists. Running it prints each result and `mismatches: none (51 cases)`. |
| `df_check.py` | The deliberate failure checks DF0b-1, DF0b-2, DF0b-3 and DF0b-6 tried on the reference first (R11). |
| `snippets_output_2026-10-20.txt`, `df_check_output_2026-10-20.txt` | The recorded outputs. |

Predicted and confirmed on the reference:
- DF0b-1 (function limit 40 → 41): exactly CK3, CK22, CK25, CK29, CK31, CK32, CK33, CK35, CK36, CK37, CK50, CK51, CK52 fail.
- DF0b-2 (the braces rule ignores `else`): exactly CK7 fails.
- DF0b-3 (lambda introducers never recognised): exactly CK29, CK31, CK32, CK33, CK50, CK51, CK52 fail.
- DF0b-6 (no `noexcept(...)`/`decltype(...)` groups in lambda heads): exactly CK51, CK52 fail.

Plan version: STAGE_00 v1.2 (v1.1: lambda bodies, lambda-aware declarations, include paths; v1.2: `noexcept(...)` and `decltype(...)` in lambda heads).

Calibration: on the original repository's C++ at `89ea9b6` the reference finds 125 violations (10 SIZE-FILE, 28 SIZE-FUNCTION, 58 STYLE-BRACES, 29 STYLE-STATEMENTS), all genuine on review.

The coding assistant never edits these files. A disagreement between the C++ and this reference is reported, never "fixed" by changing an expected value (R22).
