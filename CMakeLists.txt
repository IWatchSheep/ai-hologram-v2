# Purpose: Builds the library, program and test program for Stage 0a.
# Introduced in: Stage 0a (rebuild)
# Architecture: One static library holds the project code, the program
# and the tests both use it, and CTest runs the test program. Stage 0b
# adds the code checker library and program, whose rules the tests
# also use, and a second CTest test that runs the checker itself.

cmake_minimum_required(VERSION 3.20)
project(ai_hologram_v2 VERSION 0.0.0 LANGUAGES CXX)

# The project uses C++17, with no compiler-specific extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings are treated as errors unless the owner turns this off.
option(AI_HOLOGRAM_WARNINGS_AS_ERRORS
       "Treat compiler warnings as errors" ON)

# Applies the project's warning flags to one target.
function(ai_hologram_apply_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
        if(AI_HOLOGRAM_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        target_compile_options(
            ${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        if(AI_HOLOGRAM_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# The library holds the code shared by the program and the tests.
add_library(ai_hologram_core STATIC
            src/agent/agent_log.cpp
            src/agent/agent_log_findings.cpp
            src/agent/agent_model.cpp
            src/agent/alternative_prediction.cpp
            src/agent/consequence.cpp
            src/agent/dead_square_cache.cpp
            src/agent/dead_squares.cpp
            src/agent/footprint.cpp
            src/agent/heuristic.cpp
            src/agent/history_checks.cpp
            src/agent/investigation.cpp
            src/agent/model_v1.cpp
            src/agent/naive_agent.cpp
            src/agent/planner.cpp
            src/agent/planner_expand.cpp
            src/agent/planner_fallback.cpp
            src/agent/run_loop.cpp
            src/agent/run_loop_naive.cpp
            src/agent/single_agent.cpp
            src/agent/single_agent_active_test.cpp
            src/agent/single_agent_check.cpp
            src/agent/single_agent_choose.cpp
            src/agent/single_agent_investigate.cpp
            src/agent/single_agent_pending.cpp
            src/agent/single_agent_records.cpp
            src/agent/single_agent_search.cpp
            src/agent/step_predictor.cpp
            src/agent/verification.cpp
            src/core/project_info.cpp
            src/core/text_words.cpp
            src/demo/agent_demo.cpp
            src/demo/prediction_demo.cpp
            src/demo/scripted_demo.cpp
            src/experiment/fault_apply.cpp
            src/experiment/fault_event.cpp
            src/experiment/fault_event_text.cpp
            src/experiment/fault_events.cpp
            src/experiment/fault_injector.cpp
            src/experiment/fault_model.cpp
            src/experiment/fault_run.cpp
            src/experiment/fault_run_naive.cpp
            src/experiment/layout_generator.cpp
            src/experiment/scorer.cpp
            src/experiment/scorer_claims.cpp
            src/experiment/scorer_commit.cpp
            src/experiment/scorer_life.cpp
            src/experiment/seeded_random.cpp
            src/experiment/true_search.cpp
            src/compare/difference_text.cpp
            src/compare/state_difference.cpp
            src/model/budget_counter.cpp
            src/model/world_model.cpp
            src/world/action.cpp
            src/world/environment.cpp
            src/world/rules.cpp
            src/world/world_parse.cpp
            src/world/world_render.cpp
            src/world/world_state.cpp)
target_include_directories(ai_hologram_core PUBLIC src)
ai_hologram_apply_warnings(ai_hologram_core)

# The program only prints the two Stage 0 lines for now.
add_executable(ai_hologram src/main.cpp)
target_link_libraries(ai_hologram PRIVATE ai_hologram_core)
ai_hologram_apply_warnings(ai_hologram)

# The code checker rules live in their own static library, so
# the checker program and the test program share them (R4).
add_library(code_checker_core STATIC
            tools/code_checker/architecture_rule.cpp
            tools/code_checker/checker.cpp
            tools/code_checker/declaration_rule.cpp
            tools/code_checker/function_rule.cpp
            tools/code_checker/size_rules.cpp
            tools/code_checker/style_rules.cpp
            tools/code_checker/text_cleaner.cpp)
target_include_directories(code_checker_core PUBLIC tools/code_checker)
ai_hologram_apply_warnings(code_checker_core)

# The checker program adds only its entry point to the library.
add_executable(code_checker tools/code_checker/checker_main.cpp)
target_link_libraries(code_checker PRIVATE code_checker_core)
ai_hologram_apply_warnings(code_checker)

# The test program lists its files by name, so nothing is picked
# up by accident.
add_executable(ai_hologram_tests
               tests/agent_test_fixtures.cpp
               tests/agent_test_support.cpp
               tests/expected_demo_lines.cpp
               tests/fault_test_material.cpp
               tests/golden_files.cpp
               tests/golden_parse_reader.cpp
               tests/golden_prediction_reader.cpp
               tests/golden_rules_reader.cpp
               tests/test_agent_log.cpp
               tests/test_agent_model.cpp
               tests/test_code_checker_architecture.cpp
               tests/test_dead_square_observers.cpp
               tests/test_dead_squares.cpp
               tests/test_heuristic.cpp
               tests/test_investigation.cpp
               tests/test_investigation_more.cpp
               tests/test_model_v1.cpp
               tests/test_naive_agent.cpp
               tests/test_agent_demo.cpp
               tests/test_planner.cpp
               tests/test_planner_budget.cpp
               tests/test_run_loop.cpp
               tests/test_single_agent.cpp
               tests/test_single_agent_budget.cpp
               tests/test_single_agent_cases.cpp
               tests/test_step_predictor.cpp
               tests/test_trace_gate.cpp
               tests/test_trace_reader.cpp
               tests/test_trap_checks.cpp
               tests/trace_blocks.cpp
               tests/trace_gate.cpp
               tests/trace_case_parts.cpp
               tests/trace_cases.cpp
               tests/trace_models.cpp
               tests/trace_printer.cpp
               tests/trace_text.cpp
               tests/trace_writer.cpp
               tests/test_verification.cpp
               tests/test_code_checker_declarations.cpp
               tests/test_code_checker_size.cpp
               tests/test_code_checker_style.cpp
               tests/test_demo.cpp
               tests/test_environment.cpp
               tests/test_footprint.cpp
               tests/test_footprint_rows.cpp
               tests/test_layouts.cpp
               tests/test_fault_activation.cpp
               tests/test_fault_apply.cpp
               tests/test_fault_apply_more.cpp
               tests/test_fault_event_sets.cpp
               tests/test_fault_events.cpp
               tests/test_fault_f5.cpp
               tests/test_fault_run.cpp
               tests/test_fault_run_more.cpp
               tests/test_layout_generator.cpp
               tests/test_layout_sets.cpp
               tests/test_scorer.cpp
               tests/test_scorer_life.cpp
               tests/test_seeded_random.cpp
               tests/test_budget_counter.cpp
               tests/test_compare_predictions.cpp
               tests/test_compare_states.cpp
               tests/test_main.cpp
               tests/test_prediction_golden.cpp
               tests/test_project_info.cpp
               tests/test_rules.cpp
               tests/test_support.cpp
               tests/test_world_golden.cpp
               tests/test_world_golden_rules.cpp
               tests/test_world_model.cpp
               tests/test_world_parse.cpp
               tests/test_world_render.cpp
               tests/test_world_state.cpp)
# Tests find data files through this definition (D-063), so a
# fresh clone anywhere can locate the golden files.
target_compile_definitions(ai_hologram_tests PRIVATE
                           AIH_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(ai_hologram_tests
                      PRIVATE ai_hologram_core code_checker_core)
ai_hologram_apply_warnings(ai_hologram_tests)

# CTest runs the test program and, from Stage 0b, the code
# checker over the whole repository.
enable_testing()
add_test(NAME correctness_tests COMMAND ai_hologram_tests)
add_test(NAME code_checks COMMAND code_checker ${CMAKE_SOURCE_DIR})
