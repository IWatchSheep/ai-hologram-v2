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
            src/core/project_info.cpp
            src/demo/scripted_demo.cpp
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
               tests/golden_files.cpp
               tests/golden_parse_reader.cpp
               tests/golden_rules_reader.cpp
               tests/test_code_checker_architecture.cpp
               tests/test_code_checker_declarations.cpp
               tests/test_code_checker_size.cpp
               tests/test_code_checker_style.cpp
               tests/test_demo.cpp
               tests/test_environment.cpp
               tests/test_layouts.cpp
               tests/test_budget_counter.cpp
               tests/test_main.cpp
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
