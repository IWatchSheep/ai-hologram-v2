# The original-code golden tool (Stage 1 rebuild, plan STAGE_01 3.7)

`original_src/` holds files copied **byte for byte** from the original repository
`IWatchSheep/ai-hologram`, commit `7587f38` (tag `stage-3`), paths `src/world/*` and
`src/demo/scripted_demo.*`. `src/world` is identical at `stage-1` and `89ea9b6`.
They are reference material only: never built into the project, exempt from the size and
style rules (R1), and never edited.

SHA-256 of each file (verify with `git show 7587f38:src/<path> | sha256sum` in a clone of
the original repository):

```
4fd66f9f76946f833b0a92ea98777cb32052e4108e1956890693cdd90b5752ff  demo/scripted_demo.cpp
62b3180257e20b3e65e9a4adfb66318389e08819aa0a020e5c7570c04a30f2ac  demo/scripted_demo.h
354660e75eb4150fde4089033c0aff762ff9422fba11f0543a25b54c1d9fd713  world/action.cpp
def1bf29f141b78f8c4e9f4daa17e6cf25509394949446ef95d625919d1261b3  world/action.h
353947ef24e484c74c391dcd984e57fdba8cf7c0dd14b62425d54e8e603cfd34  world/environment.cpp
804497ef32d5dfd1c429602fa7218c20bd49579a85875e41fdb9607e92a6bb61  world/environment.h
21876ea74d3091b0a87797d81fe1726bd735d9acf032e1b1e284d1afe782a413  world/position.h
9578a574e1d268c466470ec006b92aaa00c056944b8f7b0968a318fcb71d7ad4  world/rules.cpp
45a2b9ea2f03f7bf2b18ea6cf6837e7fcd6a4ced8c529d0d0f75bc44e63ec343  world/rules.h
38c09d368977e3b09bc94c293253a275890680d40c9681e87ecfc89a6ddab9c6  world/world_state.cpp
50af28b5eb3ab3be77f055630a8e5bfb8a98cc8e418486b108f8bce6198e1130  world/world_state.h
b3a227b06ed12ff06dbda0bb5b99ce0de84a11099952502e9238afb27cd0c30b  world/world_text.cpp
ce8398ac6a57867aa1b0a21f53c4fac045a59a6f6adce6cd5ced8757e5680ff4  world/world_text.h
```

`orig_world_trace.cpp` is a small tool compiled **only** against those files. It has three modes:
- `rules <cases>`;
- `parse <cases>`;
- `demo`.

## How the golden files were produced (on 2026-10-20, Linux, GCC)
```
g++ -std=c++17 -O1 -Ioriginal_src orig_world_trace.cpp original_src/world/*.cpp original_src/demo/scripted_demo.cpp -o orig_world_trace
python3 ../gen_golden.py cases <scratch>
./orig_world_trace rules <scratch>/rules_cases.txt > ../golden/world_rules_golden.txt
./orig_world_trace parse <scratch>/parse_cases.txt > <scratch>/orig_parse_out.txt
python3 ../gen_golden.py merge <scratch>      # writes <scratch>/orig_world_parse_golden.txt
cp <scratch>/orig_world_parse_golden.txt ../golden/world_parse_golden.txt
```
(The include path is `original_src`, so the original `#include "world/..."` lines resolve.)

The reference then produced byte-identical files with `python3 ../gen_golden.py golden <scratch>`.
`./orig_world_trace demo` printed exactly the text `ref_world.scripted_demo()` produces.
