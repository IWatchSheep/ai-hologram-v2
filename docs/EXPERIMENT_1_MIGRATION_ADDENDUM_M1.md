# Experiment 1 protocol: Migration Addendum M1 (version 1.0, APPROVED)

| | |
|---|---|
| **Applies to** | `EXPERIMENT_1_PROTOCOL.md` **v1.0, frozen** (tag `protocol-1-approved`, commit `e7b4462`, repository `IWatchSheep/ai-hologram`; SHA-256 of the file `a37061950c5fb954bfdb6a42ccf83e505db19a46eb7bb6216c6ad326db009341`) |
| **Kind** | A separately approved addendum about **which code implements** the frozen definitions. It changes **no** text, hypothesis, condition, fault rule, measurement definition, number, seed restriction or statistical decision rule of v1.0. **It is nevertheless a material implementation migration**: the code that will run the experiment is no longer the code named in v1.0 Section 15, and every report of Experiment 1 must say so (M1.5). |
| **Identity** | `EXPERIMENT_1_MIGRATION_ADDENDUM_M1.md`, version 1.0. Once approved it never changes; its approval date and its SHA-256 are recorded in the new repository's `docs/PROVENANCE.md` and `DECISIONS.md`, and in the commit that adds it. A later change would be a new, separately approved addendum (M2), never an edit of M1. |
| **Status** | **APPROVED.** ChatGPT approved M1 as the preferred route (rather than protocol v1.1) on October 20, 2026, subject to clarifications, all included in this version. The owner approved it on October 20, 2026, after a plain explanation ("a new dog, proven to walk exactly like the old one"), in his words: *"I like all those things."* Even so, nothing in the new repository may be called the Experiment 1 baseline until the equivalence gate (M1.4) passes and `c1-baseline-v2` is tagged. |
| **Approval date** | October 20, 2026 |
| **Date** | October 20, 2026 |

## M1.1 What v1.0 says
- **Phase A, Section 3 ("Protecting the baseline"):**
  - C1's behaviour is defined in Phase B;
  - C1 is built in Stage 3 and tagged before any two-agent code exists;
  - after that, C1 changes only to fix a genuine bug, recorded, with every condition re-run;
  - C1 is never adjusted in response to how C3 performs.
- **Section 15 (freeze record):**
  - the "C1 baseline" row reads "Frozen at tag `stage-3` (commit `7587f38`)";
  - "nothing in this document changes" (9.5);
  - an error found later is documented, and the experiment is re-run under a new, separately approved protocol.

## M1.2 Why an addendum, not protocol v1.1 (Claude's recommendation)
- **Nothing in v1.0 is wrong.** The owner is rebuilding the code base, not correcting a rule. No hypothesis, definition or number needs to change, so the "new protocol after an error" route in Section 15 does not apply.
- **v1.0 keeps its exact text and identity.** Its tag, its hash and its pre-registration date stay exactly as they are. A v1.1 would copy every rule into a new document, inviting the question of whether anything else changed, and would date the pre-registration to the migration.
- **The addendum covers only what really changes:** the code that is run. It is the same kind of record as the instrumentation exception IE-1 (D-041): a narrow, explicitly approved statement, with invariants that must be proved.
- **If ChatGPT or the owner prefers v1.1,** it would be v1.0's text byte for byte, with only the Section 15 "C1 baseline" row replaced by M1.3 below and a revision-history line. Either route gives the same experiment.

## M1.3 The rule
1. **The C1 baseline is defined by behaviour.** The Experiment 1 baseline is the behaviour of C1 (and C0) as frozen at `stage-3` (`7587f38`) in `IWatchSheep/ai-hologram`, with the fault activation point and observers of IE-1 (H1–H4, as built at `89ea9b6`). Those additions are proved behaviour-preserving by IE-T1 to IE-T4.
2. **A re-implementation may be used in its place only if it passes the equivalence gate (M1.4) in full.** It is then tagged with a **distinct** name (`c1-baseline-v2`) in the new repository. The original `stage-3` tag and commit stay unchanged as the reference. **`c1-baseline-v2` is never described as the original `stage-3` implementation**: it is a separate, behaviour-equivalent re-implementation, identified by its own tag, commit and equivalence-gate record.
3. **Ordering (Phase A 3) still holds.** `c1-baseline-v2` is tagged **before** any two-agent code exists in the new repository.
4. **After that tag, the rules of Phase A 3 apply to the re-implementation unchanged.** It changes only for a genuine bug, recorded in `DECISIONS.md`, with the equivalence gate re-run and every condition re-run. It is never adjusted in response to how C3 performs.
5. **A difference found by the gate is never settled by changing an expected value.** It is investigated:
   - if the re-implementation is wrong, it is fixed and the gate re-run;
   - if the original C1 has a genuine bug, the frozen baseline itself is wrong. That falls under Phase A 3 and Section 15, needs its own approval, and may need a new protocol version.
6. **Nothing else changes:** the hypotheses, conditions, faults, measures, numbers, seeds (including no agent runs on pilot or confirmatory material before the experiment), decision rules and reporting.

## M1.4 The equivalence gate (automated, part of every test run once it exists)
**Golden traces** are produced once from the original code:
- **fault-free cases** from `stage-3` (`7587f38`);
- **fault cases** from `89ea9b6`, whose C1 is `stage-3` plus IE-1, proved identical when no fault is scheduled.

They are stored in the new repository, each with the commit it came from and its SHA-256. The re-implementation must reproduce them exactly.

**Compared at every decision step:**
- the decision (action, or safe stop and its reason);
- the committed prediction (action, outcome, predicted state);
- the observation checked, and match or mismatch;
- every log line;
- mismatch records (step, action, footprint differences, candidates, suspected source, anomaly flag);
- correction records (id, element, values, status, step), with every status change;
- evaluations charged in the step, and the running total;
- safe-stop count;
- for fault runs, the activation records (status, application step, reason).

**Compared at the end of each run:** the outcome, world steps, decision steps, and every Stage 4 score (event and run records).

**The cases:**
- **every Stage 3 test scenario:** the demonstrations, the SP7 budget sweep, the C1-6c/C1-9/C1-11 cases, VF6, and the LT, L2 and LP layouts;
- **every Stage 4 run case:** FR1 to FR6, and the 16 demonstration-experiment runs;
- **60 further demonstration-seed runs:** layouts 500000–500009 × run seeds 1–3, C1 and C0, with and without faults.

No pilot or confirmatory layout is used.

**The gate fails on the first difference.** It prints the case, the step and the field.

## M1.5 Reporting
Every report of Experiment 1 states, next to the description of C1:
- that the experiment ran the re-implementation `c1-baseline-v2` (repository, commit);
- that it replaced the frozen baseline `stage-3` (`7587f38`) under this addendum;
- that the equivalence gate passed, with a reference to its record.

## M1.6 Approvals required
1. ChatGPT's review of this addendum, and of the choice between addendum and v1.1.
2. The owner's explicit approval, recorded in his own words.
3. Recording in the new repository's `DECISIONS.md`, and in the old repository's README (archive notice).
