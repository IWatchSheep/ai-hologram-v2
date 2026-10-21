// Purpose: Tests for the code checker's size rules: files,
// C++ functions and lambdas, and PowerShell functions.
// Introduced in: Stage 0b (rebuild)
// Architecture: Each CK check compares the full (line, rule)
// list from checkText with the reference values in plan 6.2.
// Long snippet texts are built with repeatLine.

#include <string>

#include "checker.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {

namespace {

// CK1 to CK5, CK20 and CK22: plain functions and files.
void checkPlainSizes(TestCounter& counter) {
    std::string body40 = "void f() {\n" + repeatLine("    g();\n", 38) + "}\n";
    std::string body41 = "void f() {\n" + repeatLine("    g();\n", 39) + "}\n";
    checkSnippet(counter, "CK1: short function has no violations", "src/a.cpp",
                 "int f() {\n    return 1;\n}\n", FileKind::Cpp, {});
    checkSnippet(counter, "CK2: function of exactly 40 lines passes",
                 "src/a.cpp", body40, FileKind::Cpp, {});
    checkSnippet(counter, "CK3: function of 41 lines is too long", "src/a.cpp",
                 body41, FileKind::Cpp, {{1, "SIZE-FUNCTION"}});
    checkSnippet(counter, "CK4: file of exactly 200 lines passes", "src/a.cpp",
                 repeatLine("int x;\n", 200), FileKind::Cpp, {});
    checkSnippet(counter, "CK5: file of 201 lines is too long", "src/a.cpp",
                 repeatLine("int x;\n", 201), FileKind::Cpp,
                 {{1, "SIZE-FILE"}});
    std::string member =
        "struct S {\n    bool operator==(const S& o) const {\n" +
        repeatLine("        g();\n", 40) + "    }\n};\n";
    checkSnippet(counter, "CK20: long operator function is too long",
                 "src/a.cpp", member, FileKind::Cpp, {{2, "SIZE-FUNCTION"}});
    std::string nested =
        "namespace n {\nnamespace {\nconst int k[] = {1, 2, 3};\n"
        "}  // namespace\n" +
        body41 + "}  // namespace n\n";
    checkSnippet(counter, "CK22: long function inside namespaces", "src/a.cpp",
                 nested, FileKind::Cpp, {{5, "SIZE-FUNCTION"}});
}

// CK29 to CK33: lambdas at namespace level and inside code.
void checkLambdaSizes(TestCounter& counter) {
    std::string named =
        "auto g = [](int x) {\n" + repeatLine("    h();\n", 39) + "};\n";
    checkSnippet(counter, "CK29: namespace-level lambda over 40 lines",
                 "src/a.cpp", named, FileKind::Cpp, {{1, "SIZE-FUNCTION"}});
    std::string shorter =
        "auto g = [](int x) {\n" + repeatLine("    h();\n", 38) + "};\n";
    checkSnippet(counter, "CK30: namespace-level lambda of 40 lines passes",
                 "src/a.cpp", shorter, FileKind::Cpp, {});
    std::string inside = "void f() {\n    auto g = [&](int x) {\n" +
                         repeatLine("        h();\n", 39) + "    };\n}\n";
    checkSnippet(counter, "CK31: long lambda inside a function", "src/a.cpp",
                 inside, FileKind::Cpp,
                 {{1, "SIZE-FUNCTION"}, {2, "SIZE-FUNCTION"}});
    std::string nested = "auto outer = []() {\n    auto inner = [](int y) {\n" +
                         repeatLine("        h();\n", 39) + "    };\n};\n";
    checkSnippet(counter, "CK32: nested long lambdas", "src/a.cpp", nested,
                 FileKind::Cpp, {{1, "SIZE-FUNCTION"}, {2, "SIZE-FUNCTION"}});
    std::string argument = "const bool k = std::any_of(a, b, [](int x) {\n" +
                           repeatLine("    h();\n", 39) + "});\n";
    checkSnippet(counter, "CK33: long lambda passed as an argument",
                 "src/a.cpp", argument, FileKind::Cpp, {{1, "SIZE-FUNCTION"}});
}

// CK35 to CK38: trailing returns, members, attributes, data.
void checkSignatureSizes(TestCounter& counter) {
    std::string trailing = "auto f() -> std::pair<int, int> {\n" +
                           repeatLine("    h();\n", 39) + "}\n";
    checkSnippet(counter, "CK35: long function with trailing return type",
                 "src/a.cpp", trailing, FileKind::Cpp, {{1, "SIZE-FUNCTION"}});
    std::string member =
        "int S::get(const std::map<int, int>& m) const noexcept {\n" +
        repeatLine("    h();\n", 39) + "}\n";
    checkSnippet(counter, "CK36: long member defined outside its class",
                 "src/a.cpp", member, FileKind::Cpp, {{1, "SIZE-FUNCTION"}});
    std::string split =
        "[[nodiscard]] int f(int a,\n                  int b) {\n" +
        repeatLine("    h();\n", 39) + "}\n";
    checkSnippet(counter, "CK37: long function with a split signature",
                 "src/a.cpp", split, FileKind::Cpp, {{2, "SIZE-FUNCTION"}});
    std::string data = "const int k[] = {\n" + repeatLine("    1,\n", 45) +
                       "};\nconst std::vector<int> v{\n" +
                       repeatLine("    2,\n", 45) + "};\n";
    checkSnippet(counter, "CK38: long initializers are not functions",
                 "src/a.cpp", data, FileKind::Cpp, {});
}

// CK25, CK26, CK50, CK51 and CK52: PowerShell and lambda heads.
void checkPowerShellAndHeads(TestCounter& counter) {
    std::string psLong = "function A {\n" + repeatLine("    x\n", 39) + "}\n";
    checkSnippet(counter, "CK25: PowerShell function of 41 lines is too long",
                 "run_checks.ps1", psLong, FileKind::PowerShell,
                 {{1, "SIZE-FUNCTION"}});
    std::string psExact = "function B {\n" + repeatLine("    x\n", 38) + "}\n";
    checkSnippet(counter, "CK26: PowerShell function of 40 lines passes",
                 "run_checks.ps1", psExact, FileKind::PowerShell, {});
    std::string returned = "auto make() {\n    return [](int x) {\n" +
                           repeatLine("        h();\n", 39) +
                           "    };\n}\nauto g() {\n    return [a, b] {\n"
                           "        int p, q;\n    };\n}\n";
    checkSnippet(counter, "CK50: returned lambdas and their declarations",
                 "src/a.cpp", returned, FileKind::Cpp,
                 {{1, "SIZE-FUNCTION"},
                  {2, "SIZE-FUNCTION"},
                  {46, "STYLE-DECLARATIONS"}});
    std::string conditional = "auto f = []() noexcept(false) {\n" +
                              repeatLine("    h();\n", 39) + "};\n";
    checkSnippet(counter, "CK51: long lambda with noexcept(false)", "src/a.cpp",
                 conditional, FileKind::Cpp, {{1, "SIZE-FUNCTION"}});
    std::string specifiers =
        "auto g = [](int x) mutable noexcept(true) -> decltype(x) {\n" +
        repeatLine("    h();\n", 39) +
        "};\nauto k = [](int x) noexcept(false) {\n    int p, q;\n};\n";
    checkSnippet(counter, "CK52: lambda heads with specifier groups",
                 "src/a.cpp", specifiers, FileKind::Cpp,
                 {{1, "SIZE-FUNCTION"}, {43, "STYLE-DECLARATIONS"}});
}

}  // namespace

void runCodeCheckerSizeTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Code checker: size ---\n";
    }
    checkPlainSizes(counter);
    checkLambdaSizes(counter);
    checkSignatureSizes(counter);
    checkPowerShellAndHeads(counter);
}

}  // namespace aih
