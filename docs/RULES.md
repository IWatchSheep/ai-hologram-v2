# Starting a new repository: rules, version 3 (APPROVED)

**From:** Claude · **For:** ChatGPT · **Via:** the owner · **Date:** October 20, 2026

Version 2 took in ChatGPT's review of version 1. **Version 3 takes in ChatGPT's approval in principle of version 2 and its four final clarifications:** M1 as a material implementation migration with its own identity; the README not implying that M1 is approved; separate checks for braces and one statement per line; hashing that works on Windows; the 200-line rule's scope; and preserving `gen_c.py`. It awaits the owner's approval. Nothing has been created yet, and nothing in the old repository has been changed.

## The owner's words
- **First request:** "I want to start a new github repository from the very beginning … I think the project would be better if we started over with the new rules we made … Please make a list of rules you would do better like the 200 rule cursor kept going over, and then I will take it to chatgpt we will agree then I will make the new github repository … I will give it the same license."
- **Added today:** "I prefer adding the files to github in case I need to use a different ai agent to help with coding later on."

## 1. Closing the original repository (`IWatchSheep/ai-hologram`)
Prepared, **not yet committed**. The owner commits it only after this proposal is agreed.
- **`docs/stages/STAGE_04.md`, Section 12 filled in honestly.** It lists:
  - the commit and check count of each sub-stage (4a `45c6b44` 390 … 4f `89ea9b6` 618);
  - the fresh-build check after 4f;
  - "**NOT DONE:** the Stage 4 completion gate and the `stage-4` tag. Stage 4 is code-complete but NOT formally completed in this repository";
  - the reason (the rebuild).
- **`README.md`: an archive notice** with the same facts. The status paragraph is not altered.
- **`docs/stage4_reference/`.** The Stage 4 Python reference (`ref_stage4.py`, `ref_score.py`, `ref_csv.py`, the generating scripts, the logs and the expected 170-line output) was previously only in Claude's workspace and the review zips. It is committed so that it has a commit of origin (the owner's R24 request).
- **An annotated tag `archive-2026-10-20`** on that final commit. The message states: "Final state; Stage 4 code-complete (618 checks), Stage 4 gate and stage-4 tag not done."

  It is deliberately **not** named `stage-4`, so that nothing implies the gate passed.
- **The repository is then archived on GitHub (read-only).** It is never deleted, rewritten or force-pushed.

## 2. The frozen protocol and the C1 baseline
I inspected the exact wording. **Phase A 3** protects the baseline: defined in Phase B, built and tagged before any two-agent code, changed only for a genuine bug. **Section 15** names it: "Frozen at tag `stage-3` (commit `7587f38`)", and says nothing in the document changes.

**My recommendation: a separately approved Migration Addendum M1, not protocol v1.1.** The draft is attached as `EXPERIMENT_1_MIGRATION_ADDENDUM_M1_draft.md`. In short:
- **v1.0 is not reopened.** Its text, tag (`protocol-1-approved`), hash and date stay unchanged, and nothing in it is wrong.
- **M1 changes no rule.** It states only that the baseline is the **behaviour** frozen at `stage-3` (plus IE-1's proved behaviour-preserving hooks). A re-implementation may stand in for it only after passing an **automated equivalence gate** against golden traces from `7587f38` (fault-free cases) and `89ea9b6` (fault cases).
- **The new C1 gets a distinct tag,** `c1-baseline-v2`, before any two-agent code exists. Phase A 3 then applies to it unchanged.
- **What the gate compares,** at every decision step:
  - the actions and decisions;
  - the predictions and observed outcomes;
  - every log line;
  - the investigation, mismatch and correction records, with every status change, so verification and rejection are compared;
  - the safe stops;
  - the evaluations charged per step and in total;
  - fault activation and deferral;
  - the final outcomes and all Stage 4 scores.

  This covers every item on your list.
- **If you or the owner prefer v1.1,** it would be v1.0 byte for byte, with only the Section 15 baseline row replaced by M1's rule. I have no objection to that route. It gives the same experiment.

**Nothing in the new repository may be called the baseline until M1 (or v1.1) is approved by you and the owner.**

## 3. The rules
### A. Code size and style, checked automatically
- **R1. Size limits** *(refined as you asked)*. No file over **200 lines** and no function over **40 lines**, enforced by a checking tool run with every test run.
  - **Scope** *(v3)*: the limits apply to the project's own C++ source and tests (`src/`, `tests/`, `tools/` C++), and to `run_checks.ps1`. They do **not** apply to preserved historical material (the protocol, the brief, the old plans, the Python references, `docs/protocol_validation/`), to documents, or to generated files (golden traces, expected outputs, CSV). Preserved files are never altered to fit a rule.
  - A **formatting check**, using a committed `.clang-format`, fails if code is not formatted, so statements cannot be squeezed onto one line to pass the limit.
  - Splits are planned in the stage plan, each file with one clear job. A file is never split only to pass the limit.
- **R2. One statement per line, braces always** *(v3)*. `.clang-format` alone does not enforce either rule, so the project's own style checker (a small C++ tool, run by CTest) also fails when:
  - an `if`, `else`, `for`, `while` or `do` body is not in braces;
  - a line holds more than one statement (more than one `;` outside a `for (...)` header, string or comment);
  - a line holds more than one declaration.

  The checker has its own tests, including deliberately bad snippets that must fail.
- **R3. The plan names every file and its approximate size** before code is written.
- **R4. One shared helper per job** (one `actionIndex`, one footprint comparison, one `readDeadStatus`), listed in the plan.

### B. Architecture
- **R5. Instrumentation is designed in from the start:** the fault activation point, the watch-only observers and `readDeadStatus` exist in the first version of the agents. It must not change C1's externally observable behaviour; the equivalence gate (R20) proves this.
- **R6. Watch-only is enforced by types.** Observers receive const references or copies, return `void`, and are never read by agent code. In the owner's words: "the chain can be replaced by the camera", and ours must not pull.
- **R7. Experimenter code is separate, and that is tested:** a check fails if `src/agent/` includes `src/experiment/`.
- **R8. Records carry the time they refer to** (the owner's "stale address" lesson, D-042).

### C. Plans, reference code and values
- **R9. The full Python reference for a stage is built before the plan is reviewed,** so readings surface before review, not between sub-stages.
- **R10. The reference lives in the repository** (`tools/reference/`), with a **reference contract**: it follows the C++ data flow.
- **R11. Every deliberate failure check is tried by Claude first,** with its exact expected failures.
- **R12. Decisions are recorded when made,** in the same commit as the plan.
- **R13. Owner approvals are recorded in his own words,** with the scenario he was shown.

### D. The owner's Windows computer
- **R14. `run_checks.ps1`** *(refined as you asked)*. It:
  - sets the MinGW PATH itself, so it works after a restart;
  - configures (CMake) and compiles;
  - runs the tests (failures and total only), CTest, the size, formatting and architecture checks (R1, R2, R7) and the equivalence gate (R20);
  - prints the program's total and non-blank line counts, and `git status`.

  It **stops at the first failing step** with a clear message and a non-zero exit code.
- **R15. Quiet tests by default,** with `--verbose` for every PASS.
- **R16. `.gitattributes`** (`* text=auto eol=lf`) from the first commit.
- **R17. Work branches** *(as you asked)*. Cursor's work goes on `stage-N-work`. The owner does **every** commit, push, merge and approval; Cursor never commits. Claude reviews from GitHub.
- **R18. Tool-neutral names:** `docs/prompts/`, `docs/handoff_notes.md`. The Cursor rules and permissions exist from the first commit.

### E. New rules
- **R19. Provenance** *(yours)*. Every carried-forward file is listed in `docs/PROVENANCE.md` with its source repository, path, commit and SHA-256.
  - Hashes are of the exact committed bytes (LF line endings), as listed in Section 5.
  - *(v3)* **How it works on Windows, with no extra utility:** `.gitattributes` uses `* text=auto eol=lf`, so the working copy also has LF line endings, byte-identical to the committed content. `run_checks.ps1` uses PowerShell's built-in `Get-FileHash -Algorithm SHA256` on each preserved file, and **first checks that the file contains no carriage return**. A file with CRLF fails clearly with a message, never with a wrong hash.
  - The hashes in the manifest were computed from the committed blobs (`git show 89ea9b6:<path>`), so they match the LF files exactly.
  - The manifest for the current files is in Section 5.
- **R20. The baseline equivalence gate** *(yours)* is an automated regression gate (M1.4). It runs on every test run once C1 exists, and fails on the first difference in any traced field, fault and fault-free.
  - The golden traces are produced once, by a small trace tool built against the original commits' sources.
  - They are stored with their commit and hash.
- **R21. No pilot or confirmatory agent runs while rebuilding** *(yours)*. Only demonstration seeds (500000–500009) and hand-built cases.
- **R22. Never change an expected value to make a test pass** *(yours)*. Investigate whether the implementation, the reference or the specification is wrong, and record any authorized correction.
- **R23. Every stage plan is complete before implementation** *(yours)*: the exact expected checks, the files and their approximate sizes, the deliberate failure checks and the completion gate.
- **R24. Everything lives in the repository** *(the owner's request)*. Any other AI agent, or person, must be able to continue the work from GitHub alone, so nothing that matters stays only in a chat or a zip. That means:
  - the plans, prompts, notes to and from ChatGPT, review findings, the reference code and its logs, decisions, the owner's words, and the handoff notes;
  - a short `AGENTS.md` at the root, telling any AI assistant: the roles, the rules (this list), where things are, how to run `run_checks.ps1`, and what it must never do (commit, edit `docs/`, change expected values).

### F. Kept as before
- **Roles:** Claude plans, reviews and builds reference code; ChatGPT reviews independently; Cursor (or another assistant, under `AGENTS.md`) implements one sub-stage at a time and never commits; the owner does all Git operations, approvals and gates.
- **Coding rules:** no exceptions, no assertions that stop the program, no inheritance, no external libraries.
- **Seeds:** the reserved seed ranges.
- **The owner's ideas:** kept verbatim in `PHASE_C_design_inputs.md`.

## 4. What is carried over, and what is rebuilt
- **Carried over, unchanged, with provenance (R19):**
  - the brief v1.1;
  - the frozen protocol v1.0;
  - `docs/protocol_validation/`;
  - `PHASE_C_design_inputs.md`;
  - the Stage 3 and Stage 4 Python references, including `gen_c.py`, the one-off pilot crash check disclosed at the v1.0 review. Its exact console output from October 20 is preserved, and its README states its limited purpose and that it was run with the earlier reference.
- **Carried over as historical reference (your item 4):**
  - `STAGE_03.md`, `STAGE_04.md` and `DECISIONS.md`, including IE-1 and D-030 to D-045;
  - the Stage 3 and Stage 4 expected outputs;
  - the demonstration fault-event lists.
- **Rebuilt under the new rules:** all C++ source and tests. The new stage plans may cite the old expected values. They are references, and R20 makes them binding for C1.

## 5. Provenance manifest (SHA-256 of committed content)
**Source:** `IWatchSheep/ai-hologram` at commit `89ea9b6`, except `docs/stage4_reference/`, which gets its commit when the closing commit is made.

```
c55ddf2756c83fc1abef0814b69063a39ebdd93783ca984757046519acf49d5c  docs/AI_Hologram_Project_Brief_v1.1.txt
a37061950c5fb954bfdb6a42ccf83e505db19a46eb7bb6216c6ad326db009341  docs/EXPERIMENT_1_PROTOCOL.md
b1c8d91b4e178edcc8bc03dcbb7e1f146a95305b08e8a0a7703b4d6332b9d9d8  docs/PHASE_C_design_inputs.md
67504e1bc24092ac9b003da47da08504ae68e36923cb6bf2952246c95eee3e20  docs/DECISIONS.md
f54b318ec4a7f3a7f1363798cac57b6337799df96a5ae755aef20054f175b842  docs/stages/STAGE_03.md
2a68c8ee24ec0ef34b492d9debb90e33a525799af75f8cb0a028c519844c37dd  docs/stages/STAGE_04.md   (before Section 12 is filled in)
751d51381c093d498e36d3bb5290f24742c10c6954a853a1db06d7779ee5ea62  docs/protocol_validation/ref_c1.py   (identical to docs/stage4_reference/ref_c1.py)
8dbd75a7f633bfff7f86ff89bdee78c2c718df470db6d853d633840d9ecce9a4  docs/stage4_reference/ref_stage4.py
cc941cf3172bd583dd955ea7bb0ebfa7f2d4ec545c91a759bf00a66b1bc9daca  docs/stage4_reference/ref_score.py
2e169cdb9afda6627523ea9e0fe977fd3dfeaa2bc57410ae1f953b4e1276f21a  docs/stage4_reference/ref_csv.py
1a334703f53a6ecfefde12d7112d1385ee09a1cc8fd5185ac05ba2038032c8fe  docs/stage4_reference/expected_output_stage4.txt
```
(The full list, including every `docs/protocol_validation/` file, goes in `docs/PROVENANCE.md`. The final hashes of the files added or changed by the closing commit, STAGE_04.md, README.md and `docs/stage4_reference/` including `gen_c.py`, are recorded from that commit.)

## 6. Order (ChatGPT's final sequence)
1. **The owner approves M1 and these final rules,** in his own words, with the repository name.
2. **The owner commits the closing package** to the old repository, tags `archive-2026-10-20`, and archives the repository on GitHub.
3. **The owner creates the new repository,** with the approved name and the same LICENSE.
4. **First commit:** the governing documents (the protocol, unchanged; M1 with its approval date), `docs/RULES.md` (this document), `docs/PROVENANCE.md`, the references, `AGENTS.md`, `.gitattributes`, `.clang-format`, the Cursor rules and permissions, and `run_checks.ps1`.
5. **The C++ and tests are rebuilt** under complete plans (R23).
6. **The full behavioural-equivalence gate is run** (R20, M1.4).
7. **`c1-baseline-v2` is tagged** only when the gate passes, and **before** any two-agent code.

## Status of agreement
- **ChatGPT:** approved the plan and M1 in principle (October 20, 2026), with the clarifications now made.
- **The owner:** approved M1, these final rules, the closing package and the repository name `ai-hologram-v2` on October 20, 2026, after scenarios for each, in his words: *"I like all those things."*
