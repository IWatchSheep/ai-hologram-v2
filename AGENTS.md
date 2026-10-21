# AGENTS.md — how any AI assistant works on this project

This file is for **any** AI assistant (Cursor, Claude, another tool) and for any person continuing the work. Everything needed is in this repository (rule R24 in `docs/RULES.md`). If anything here disagrees with the governing documents, those documents win: stop and tell the owner.

## 1. The project
AI Hologram tests whether two cooperating reasoning agents, a **spatial agent** and a **temporal-spatial witness**, with overlapping abilities, detect and correct reasoning errors better than one competent single agent (C1) at equal computational budget, in a 10×10 grid world. It is a research project. Nothing is claimed until the pre-registered experiment has been run.

## 2. Governing documents, in order
1. `docs/AI_Hologram_Project_Brief_v1.1.txt`: the approved specification.
2. `docs/EXPERIMENT_1_PROTOCOL.md`: the Experiment 1 protocol, **v1.0, frozen**. It must never be edited; its hash is checked by `run_checks.ps1`.
3. `docs/EXPERIMENT_1_MIGRATION_ADDENDUM_M1.md`: which code may stand in for the frozen C1 baseline, and the equivalence gate it must pass. Also never edited.
4. `docs/RULES.md`: the development rules R1–R24 (approved October 20, 2026).
5. The current approved stage plan in `docs/stages/`.

## 3. Roles
- **The owner** (IWatchSheep) decides, approves, and does **every** Git operation (commit, push, branch, merge, tag).
- **Claude** writes stage plans, Python reference code with exact expected values, prompts and reviews.
- **ChatGPT** reviews independently.
- **The coding assistant** (Cursor, or any other) implements **one sub-stage at a time**, exactly as its prompt and the plan say, and **never commits**.

## 4. What the coding assistant must never do
- Run any Git command that changes anything. Only `git status`, `git diff` and `git log` are allowed.
- Edit `docs/`, `tools/reference/`, `LICENSE`, `.gitignore`, `.gitattributes`, `.clang-format`, `.cursor/` or this file.
- Change an expected value in a test to make it pass. Stop and report the actual value; Claude and the owner decide whether the code, the reference or the plan is wrong (R22).
- Weaken C1 (the baseline) or adjust anything because of how a two-agent condition performs.
- Run any agent on pilot layouts (seeds 900000–900039) or confirmatory layouts (100000–100399). Development uses demonstration seeds 500000–500009, hand-built worlds, and deterministic synthetic verification fixtures from an independent generator only (R21, clarified by D-070).
- Create scratch, debug or temporary files; rename, move or delete files.

## 5. Coding rules (details in `docs/RULES.md`)
- C++17 and the standard library only: no external libraries, threads, networking or GUI.
- No exceptions, no assertions that stop the program, no inheritance.
- **Files at most 200 lines, functions at most 40 lines** (R1), checked automatically. **One statement per line, braces on every `if`/`else`/`for`/`while`/`do`** (R2), checked automatically. Never squeeze code to fit a limit; never split a file only to pass a limit. The plan names every file.
- Every file starts with a three-part comment: Purpose / the stage that introduced it / Architecture. Beginner-friendly comments explain what and why.
- One shared helper per job (R4); never write a second copy of an existing helper.
- Randomness only through the project's `SeededRandom` (SplitMix64, D-030), never `<random>` or `rand()`.
- Agent code (`src/agent/`) never includes experimenter code (`src/experiment/`) (R7). Observers are watch-only: const inputs, `void` outputs, never read by agents (R6).
- Every model prediction goes through the budgeted predictor. "Take no action" (a safe stop) must always be possible. A correction is never accepted just because it fits the past.
- Agents communicate only through explicit messages; neither has automatic authority; agreement is never proof.

## 6. How to check your work
Run, from the repository root in PowerShell:
```
powershell -ExecutionPolicy Bypass -File .\run_checks.ps1
```
It sets the compiler PATH itself, then runs every check in order and **stops at the first failure** with a clear message (R14). Report its final line exactly. (In the first commit it checks only the provenance hashes and shows `git status`; each stage adds its steps.)

## 7. Where things are
| Path | What |
|---|---|
| `docs/` | Governing documents, `RULES.md`, `PROVENANCE.md`, `DECISIONS.md`, stage plans (`docs/stages/`), prompts (`docs/prompts/`). |
| `docs/protocol_validation/` | The protocol's validation record (Python), referred to by protocol Section 15. |
| `docs/history/ai-hologram/` | Every plan, decision, prompt and note of the original repository, unchanged. Its code is at tag `archive-2026-10-20` of `IWatchSheep/ai-hologram`. |
| `tools/reference/` | Claude's Python reference implementations and the scripts that produce expected values (R10). |
| `notes/handoff_notes.md` | The coding assistant's handoff notes (append only). |
| `src/`, `tests/` | The C++ code and tests (rebuilt from Stage 0 under these rules). |

## 8. Before you start any work
1. Read the prompt and the stage plan sections it names.
2. Read every existing file you will change, and tell the owner what is there.
3. If anything is unclear, impossible, or disagrees with the plan or protocol: stop and ask. Do not guess or redesign.

## 9. After you finish
Append a handoff note to `notes/handoff_notes.md`: what you built, what you ran, the exact `run_checks.ps1` result, every Stage 3-baseline file you touched (if any), and anything you were unsure about. Never claim something works unless you built and ran it. Do not commit.
