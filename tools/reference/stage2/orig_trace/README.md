# The original-code golden tool (Stage 2 rebuild, plan STAGE_02 3.6)

`original_src/` holds files copied **byte for byte** from the original repository
`IWatchSheep/ai-hologram`, commit `7587f38` (tag `stage-3`), paths `src/model/*`,
`src/compare/*` and `src/demo/prediction_demo.*`. They are identical at `stage-2` and
`89ea9b6`. They are reference material only: never built into the project, exempt from R1,
and never edited. Their hashes (verified against `git show 7587f38:src/<path>`) are also
in `docs/PROVENANCE.sha256`:

```
1288f442cf753d66726dfc5c6e81a23c3e3885346dec94023e1aa0a833655ad3  compare/difference_text.cpp
9d57e5613681f507e7dea575b8b6531cb7fb54fc8365fce968922d97b6ffa0b6  compare/difference_text.h
eef8e91a02f0f698bdf3187045951f5c39f80b5a02f5021eb6836fc9e2fdd124  compare/state_difference.cpp
84679f96a44d2eee11a47bedc383772a480bddeb6081bc8d6ad466b57d180ecd  compare/state_difference.h
822c0822104515a261f34d73eedd4af9e4c2fe1efd94170571091d7de36d9c67  demo/prediction_demo.cpp
6b5e75ed262534f01e14339459e92144d8c4f69c24810fcd606f40d7603ce39f  demo/prediction_demo.h
feaccae79180dbabbd9de9cdc178a94fc711e85c418672a57f372e1375452146  model/belief.h
48e9fdb61c8cb9e4bbf2bed5d9d7cc61e681336e91f3e1bc516c84f57f0c3743  model/budget_counter.cpp
900aa805985d00cc4ae458deef052d733431563ad991b4e2c271a1705687b10a  model/budget_counter.h
1e1fc9bbf02f6b125de3a847b4e67e3b2508d73cc7890d7ddec0ca449e44e7db  model/prediction.h
8ae0fd940f3aa901c5fbd32b9eca3cdf14d8686fc9be697aa4a312c3ea992b5a  model/world_model.cpp
f7a0cd33834c18f67f14bbd207fa58e634ef572e4d6591e1a7fc092fc893bbee  model/world_model.h
```

`orig_prediction_trace.cpp` is a small tool compiled **only** against those files and the original
world files kept for Stage 1 (`../../stage1/orig_trace/original_src/`). It has two modes:
- `predict <cases>`;
- `demo`.

## How the golden file was produced (2026-10-20, Linux, GCC)
```
W=../../stage1/orig_trace/original_src
g++ -std=c++17 -O1 -I$W -Ioriginal_src orig_prediction_trace.cpp $W/world/*.cpp original_src/model/*.cpp original_src/compare/*.cpp original_src/demo/prediction_demo.cpp -o orig_prediction_trace
python3 ../gen_golden.py cases <scratch>
./orig_prediction_trace predict <scratch>/prediction_cases.txt > ../golden/prediction_golden.txt
```
The reference then produced a byte-identical file with `python3 ../gen_golden.py golden <scratch>`.
`./orig_prediction_trace demo` printed exactly the text `ref_model.prediction_demo_text()` produces.
