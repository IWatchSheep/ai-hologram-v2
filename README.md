# ai-hologram-v2

## About

**AI Hologram** is a research project that investigates an alternative approach to artificial reasoning that does not use a large language model as its central reasoning mechanism. It **proposes** a design in which two cooperating reasoning agents, a **spatial agent** (incremental, step-by-step reasoning) and a **temporal-spatial witness**, have overlapping abilities and check each other's predictions against what is actually observed. Whether this design detects and corrects errors better than a competent single agent is an **open research question**. It has not yet been tested.

## Current status

**Stage 3 of the rebuild: the competent single agent C1 and the naive agent C0.** The project contains a 10×10 grid world with tested movement rules; a world model, budget counter and comparison (Stage 2); and C1, the competent single agent defined in the approved Experiment 1 protocol (Phase B). C1 plans with a budgeted search over its own belief, checks each prediction against the observation, investigates mismatches, verifies corrections, and can take safe stops. C0, a naive agent used only as a reference, plans from its belief and never checks a prediction. Demonstrations use a hand-written wrong belief. **No faults are injected, no two-agent reasoning exists, and no experiments have been run.** The step-by-step check against 789 runs recorded from the original code (the first part of the M1.4 equivalence gate) is added in sub-stage 3g.

This repository continues the project from [`IWatchSheep/ai-hologram`](https://github.com/IWatchSheep/ai-hologram), which is archived as the permanent record (final tag `archive-2026-10-20`). The C++ code and tests are being rebuilt from the beginning under the development rules in [`docs/RULES.md`](docs/RULES.md), agreed between the owner, ChatGPT and Claude.

Carried over unchanged, with recorded provenance ([`docs/PROVENANCE.md`](docs/PROVENANCE.md)): the project brief, the **frozen** Experiment 1 protocol v1.0 and its validation record, the owner's design inputs, the Python reference implementations, and the full history of the original repository's plans, decisions and prompts.

Migration Addendum M1 ([`docs/EXPERIMENT_1_MIGRATION_ADDENDUM_M1.md`](docs/EXPERIMENT_1_MIGRATION_ADDENDUM_M1.md)), approved on October 20, 2026, sets the condition under which the rebuilt C1 may serve as the experiment's baseline: it must pass an automated behavioural-equivalence gate against the original, and is then tagged `c1-baseline-v2`.

## Where to start reading
- [AGENTS.md](AGENTS.md): how any AI assistant or person works on this project.
- [Development rules](docs/RULES.md)
- [Project brief](docs/AI_Hologram_Project_Brief_v1.1.txt)
- [Experiment 1 protocol (frozen)](docs/EXPERIMENT_1_PROTOCOL.md) and [Migration Addendum M1](docs/EXPERIMENT_1_MIGRATION_ADDENDUM_M1.md)
- [Decisions](docs/DECISIONS.md)
- [The owner's design inputs](docs/PHASE_C_design_inputs.md)

## How to check everything
In PowerShell, from the repository folder:
```
powershell -ExecutionPolicy Bypass -File .\run_checks.ps1
```

## License
See [LICENSE](LICENSE) (the same license as the original repository).
