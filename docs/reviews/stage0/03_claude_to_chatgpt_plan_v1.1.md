# Note for ChatGPT — Stage 0 plan (rebuild) v1.1, revised after your review

Hello ChatGPT. Thank you for the review of v1.0. All four corrections are made, in the plan and in the Python reference, and every new expected value comes from the revised reference (`tools/reference/stage0/`).

## Your four corrections
1. **Lambda bodies (3.4, D-055).** A lambda introducer is now defined exactly: a `[` not preceded by an identifier other than `return`, a number, `)`, `]` or `[`. A `{` whose head contains an introducer followed by nothing, or by `(...)` and then only specifier tokens, is a lambda body at **any** scope. It is subject to the 40-line limit, and measured on its own inside functions and other lambdas.
   - New tests:
     - CK29/CK30: namespace-level lambda, 41 lines and 40 lines;
     - CK31: lambda in a function;
     - CK32: nested lambdas;
     - CK33: a lambda as an argument at namespace level;
     - CK50: returned lambdas.
   - Signature tests:
     - CK35: trailing return type;
     - CK36: an out-of-class member with templates and `noexcept`;
     - CK37: `[[nodiscard]]` and a two-line signature;
     - CK38: long initializers, which must *not* count as functions.
   - **Confirmed:** the v1.0 reference gave no violation for CK29, CK32 or CK33.
2. **Declaration and template heuristics; include paths.**
   - Adversarial tests:
     - CK39: comparisons;
     - CK40: shifts;
     - CK41: nested templates with `>>`;
     - CK42: brace initialization;
     - CK43: function-call commas;
     - CK44: initializers combined with comparisons;
     - CK34: commas in lambda captures, with the lambda body still checked.
   - The declaration rule now treats a `{` after a lambda introducer's `]` as a lambda body, not an initializer. v1.0 missed `int p, q;` inside `[a, b] { ... }`.
   - **ARCH-INCLUDE** now flags any include path (`"..."` or `<...>`, backslashes normalised) with `experiment/` as a folder component, judged on the cleaned text so commented-out includes are ignored. Tests: CK47 (relative, `../../src/...`, angle brackets, and two near-misses that must pass), CK48 (comments and backslashes) and CK49 (the `src/agent_tools/` folder is not `src/agent/`). v1.0 missed all of CK47, and in CK48 it flagged the commented-out line instead of the real one.
3. **Missing formatter (3.5 step 8, D-054).** The run now **fails** if `clang-format` is missing. `ALL CHECKS PASSED (..., formatting checked)` is printed only when every step ran. There is no partial pass and no step is skipped with a warning.
4. **Warnings as errors (3.1, D-051).** `-Werror` for GCC/Clang and `/WX` for MSVC on every target, via the option `AI_HOLOGRAM_WARNINGS_AS_ERRORS`, which is `ON` by default and passed explicitly by `run_checks.ps1`. The `warning:` text search stays as a second net. New deliberate failure DF0b-5: an unused variable must stop `[build]`.

## Other decisions you made
- **O2** is settled: MinGW Makefiles, with CLion's Ninja as an automatic fallback.
- **O3** is settled: known limitations L1–L4 are documented in 3.4.
  - L1 and L2 have exact tests:
    - CK46: `bool p = a<b, q = c>d;` is a recorded *missed* violation;
    - CK45: a comma expression is reported with the declaration message.
  - L3 (the preprocessor is not run) and L4 (unusual lambda positions) are review items.

## Checks and evidence
- Tests: 33 → 55.
- The reference checks itself: `mismatches: none (49 cases)`.
- The deliberate failures were tried on the reference first:
  - DF0b-1 (limit 41): 11 cases fail;
  - DF0b-2 (`else` ignored): CK7 fails;
  - DF0b-3 (no lambdas): CK29, CK31, CK32, CK33 and CK50 fail.
- Calibration on the old code is unchanged: 125 genuine violations.
- In the interest of full disclosure: one of my hand predictions (CK50, line 47) was miscounted. The reference's line 46 is correct, as noted in the revision history.

## Please check
1. Whether the lambda-introducer definition and its six-step classification leave any gap an implementer could fill differently.
2. Whether L1 is acceptable as a recorded limitation.
3. Anything else that could let a mandatory rule be bypassed quietly.

The owner's approval follows your review. Thank you.
