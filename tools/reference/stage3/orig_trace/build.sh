#!/bin/sh
# Builds the Stage 3 golden-trace tools from the ORIGINAL C++ only, and checks that they
# reproduce the committed golden files byte for byte. Run from this folder (Linux, g++).
#   orig_c1_trace_3     src/agent at 7587f38 (tag stage-3)
#   orig_c1_trace_ie1   src/agent at 89ea9b6 (stage-3 plus IE-1): original_src_ie1/ holds
#                       the 16 files that differ, original_src/ the rest
#   orig_unit_values    the new unit checks' values, from the 89ea9b6 code
#   orig_demo           the original runAgentDemos text (7587f38)
set -e
W=../../stage1/orig_trace/original_src
M=../../stage2/orig_trace/original_src
OUT=${1:-/tmp/stage3_orig_tools}
mkdir -p "$OUT"
IE=""
for f in original_src/agent/*.cpp; do
    b=$(basename "$f")
    if [ -f "original_src_ie1/agent/$b" ]; then IE="$IE original_src_ie1/agent/$b"; else IE="$IE $f"; fi
done
g++ -std=c++17 -O1 -I$W -I$M -Ioriginal_src orig_c1_trace.cpp $W/world/*.cpp $M/model/*.cpp \
    $M/compare/*.cpp original_src/agent/*.cpp -o "$OUT/orig_c1_trace_3"
g++ -std=c++17 -O1 -DWITH_OBSERVER -I$W -I$M -Ioriginal_src_ie1 -Ioriginal_src orig_c1_trace.cpp \
    $W/world/*.cpp $M/model/*.cpp $M/compare/*.cpp $IE -o "$OUT/orig_c1_trace_ie1"
g++ -std=c++17 -O1 -I$W -I$M -Ioriginal_src_ie1 -Ioriginal_src orig_unit_values.cpp \
    $W/world/*.cpp $M/model/*.cpp $M/compare/*.cpp $IE -o "$OUT/orig_unit_values"
g++ -std=c++17 -O1 -I$W -I$M -Ioriginal_src orig_demo.cpp original_src/demo/agent_demo.cpp \
    $W/world/*.cpp $M/model/*.cpp $M/compare/*.cpp original_src/agent/*.cpp -o "$OUT/orig_demo"
"$OUT/orig_c1_trace_3" traces ../golden/c1_cases.txt | cmp - ../golden/c1_traces.txt
"$OUT/orig_c1_trace_ie1" traces ../golden/c1_cases.txt | cmp - ../golden/c1_traces.txt
"$OUT/orig_c1_trace_ie1" observers ../golden/c1_cases.txt | cmp - ../golden/c1_observers.txt
echo "golden files reproduced: traces (7587f38 and 89ea9b6) and observers (89ea9b6)"
