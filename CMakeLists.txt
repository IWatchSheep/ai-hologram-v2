# Purpose: Builds the library, program and test program for Stage 0a.
# Introduced in: Stage 0a (rebuild)
# Architecture: One static library holds the project code, the program
# and the tests both use it, and CTest runs the test program.

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
add_library(ai_hologram_core STATIC src/core/project_info.cpp)
target_include_directories(ai_hologram_core PUBLIC src)
ai_hologram_apply_warnings(ai_hologram_core)

# The program only prints the two Stage 0 lines for now.
add_executable(ai_hologram src/main.cpp)
target_link_libraries(ai_hologram PRIVATE ai_hologram_core)
ai_hologram_apply_warnings(ai_hologram)

# The test program lists its files by name, so nothing is picked
# up by accident.
add_executable(ai_hologram_tests
               tests/test_main.cpp
               tests/test_project_info.cpp
               tests/test_support.cpp)
target_link_libraries(ai_hologram_tests PRIVATE ai_hologram_core)
ai_hologram_apply_warnings(ai_hologram_tests)

# CTest runs the test program as its one test in Stage 0a.
enable_testing()
add_test(NAME correctness_tests COMMAND ai_hologram_tests)
