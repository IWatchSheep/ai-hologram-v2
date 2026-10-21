// Purpose: Tests for ARCH-INCLUDE (agent code must not
// include experiment code) and for checkedKind, which
// decides which files are checked at all (FS1 to FS12).
// Introduced in: Stage 0b (rebuild)
// Architecture: Each CK check compares the full (line, rule)
// list from checkText, and each FS check compares the kind
// checkedKind returns, with the values in plan 6.2.

#include "checker.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {

namespace {

// CK18, CK19, CK47, CK48 and CK49: include paths of all forms.
void checkIncludeSnippets(TestCounter& counter) {
    checkSnippet(counter, "CK18: agent including experiment is reported",
                 "src/agent/a.cpp",
                 "#include \"experiment/fault_event.h\"\n"
                 "#include \"world/rules.h\"\n",
                 FileKind::Cpp, {{1, "ARCH-INCLUDE"}});
    checkSnippet(counter, "CK19: experiment code may include experiment",
                 "src/experiment/a.cpp",
                 "#include \"experiment/fault_event.h\"\n", FileKind::Cpp, {});
    checkSnippet(
        counter, "CK47: relative and angle includes are caught",
        "src/agent/a.cpp",
        "#include \"../experiment/fault_event.h\"\n"
        "#include \"../../src/experiment/x.h\"\n"
        "#include <experiment/y.h>\n"
        "#include \"agent/experiment_notes.h\"\n"
        "#include \"world/experiment.h\"\n",
        FileKind::Cpp,
        {{1, "ARCH-INCLUDE"}, {2, "ARCH-INCLUDE"}, {3, "ARCH-INCLUDE"}});
    checkSnippet(counter, "CK48: commented includes do not count",
                 "src/agent/sub/b.h",
                 "// #include \"experiment/x.h\"\n/*\n"
                 "#include \"experiment/x.h\"\n*/\n"
                 "#include \"..\\..\\experiment\\x.h\"\n",
                 FileKind::Cpp, {{5, "ARCH-INCLUDE"}});
    checkSnippet(counter, "CK49: agent_tools is not the agent folder",
                 "src/agent_tools/a.cpp", "#include \"experiment/x.h\"\n",
                 FileKind::Cpp, {});
}

// FS1 to FS6: C++ files in the tree and the exempt folders.
void checkKindsFirstHalf(TestCounter& counter) {
    checkKind(counter, "FS1: src file is checked as cpp", "src/core/a.cpp",
              FileKind::Cpp);
    checkKind(counter, "FS2: tests file is checked as cpp",
              "tests/test_main.cpp", FileKind::Cpp);
    checkKind(counter, "FS3: tools header is checked as cpp",
              "tools/code_checker/checker.h", FileKind::Cpp);
    checkKind(counter, "FS4: reference code is not checked",
              "tools/reference/stage0/x.cpp", FileKind::None);
    checkKind(counter, "FS5: build folder files are not checked",
              "cmake-build-checks/CMakeFiles/a.cpp", FileKind::None);
    checkKind(counter, "FS6: nested build folder files are not checked",
              "src/cmake-build-debug/a.cpp", FileKind::None);
}

// FS7 to FS12: other folders, PowerShell files and the root.
void checkKindsSecondHalf(TestCounter& counter) {
    checkKind(counter, "FS7: docs files are not checked", "docs/a.cpp",
              FileKind::None);
    checkKind(counter, "FS8: root script is checked as ps1", "run_checks.ps1",
              FileKind::PowerShell);
    checkKind(counter, "FS9: tools script is checked as ps1",
              "tools/checks/build_steps.ps1", FileKind::PowerShell);
    checkKind(counter, "FS10: reference script is not checked",
              "tools/reference/stage4/gen.ps1", FileKind::None);
    checkKind(counter, "FS11: text files are not checked", "src/notes.txt",
              FileKind::None);
    checkKind(counter, "FS12: root cpp file is not checked", "main.cpp",
              FileKind::None);
}

}  // namespace

void runCodeCheckerArchitectureTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Code checker: architecture ---\n";
    }
    checkIncludeSnippets(counter);
    checkKindsFirstHalf(counter);
    checkKindsSecondHalf(counter);
}

}  // namespace aih
