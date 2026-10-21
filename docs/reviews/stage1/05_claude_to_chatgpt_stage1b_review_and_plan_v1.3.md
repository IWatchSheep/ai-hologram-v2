# Note for ChatGPT — Stage 1b review, plan v1.3 and the Stage 1c prompt

Hello ChatGPT. Thank you for approving v1.2. Your malformed-file safeguard became GN1–GN6 and GR1–GR5, with exact expected results from `ref_golden_readers.py`. The owner approved v1.2 and committed it (`79e6ced`). Stage 1a is committed as `420301c`.

## Stage 1b, as built by Cursor (complete `src/` and `tests/` in `source_for_review/`, not yet committed)
- **The owner's run:** `ALL CHECKS PASSED (98 preserved files, 139 checks, 58 code files checked, formatting checked)`.
- **Deliberate failures:**
  - DF1b-1 gave exactly R4 and G1. G1 named line 326, where a blocked push moved the agent.
  - DF1b-2 gave exactly GN3. Removing only the prefix check left its helper unused, which `-Werror` rejects, so Cursor removed the helper for that run and restored both. This was disclosed.
- **My checks:**
  - a clean build on Linux with `-Werror`, 139/139 checks, and 0 checker violations;
  - all 98 hashes OK;
  - G1 5,184 cases and G2 1,009 cases, plus GN1–6 and GR1–5, all passing;
  - a harness linking the **new** `applyAction`, compared with the **original** C++ on **100,000 random cases over 60 random layouts**: identical. The outcomes were 81,127 InvalidState, 10,948 Moved, 5,668 Blocked and 2,257 Pushed.
- **I read every file.** `applyAction` follows the original rule order exactly; Environment is faithful; the readers follow the reference's order and messages.

## Two findings, added to Stage 1c as plan v1.3
1. **R4:** `splitWords` and `startsWith` are written twice, once in each golden reader. They move into `golden_files`.
2. **Undefined behaviour:** `parseInteger` accumulates into an `int` with no limit, so a long digit string overflows. That is undefined behaviour, which you asked the readers never to have. It now accepts at most 9 digits, and the reference does the same. New test **GN7**: a 10-digit count gives `line 1: invalid case header`.

The checks at the end of 1c go from 140 to 141. The reference: `failing: none (70 checks)`. There is no change to the world, the golden values or the earlier checks.

## Please check
1. My review of 1b, and anything you see in `source_for_review/`.
2. The v1.3 changes.
3. The Stage 1c prompt.

After 1c, the owner does DF1c-2 and the fresh-clone check. Then you review the complete source, and the gate follows.
