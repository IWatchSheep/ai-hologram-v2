// Purpose: Tests for STYLE-DECLARATIONS: one declaration per
// statement, with templates, brace initializers, enums and
// lambda bodies handled as plan 3.4 says.
// Introduced in: Stage 0b (rebuild)
// Architecture: Each CK check compares the full (line, rule)
// list from checkText with the reference values in plan 6.2,
// including the documented limitations CK45 and CK46.

#include "checker.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {

namespace {

// CK14 to CK16: plain multiples, containers and enums.
void checkBasicDeclarations(TestCounter& counter) {
    checkSnippet(
        counter, "CK14: two declarations in one statement", "src/a.cpp",
        "void f() {\n    int a, b;\n    int c = 1, d = 2;\n}\n", FileKind::Cpp,
        {{2, "STYLE-DECLARATIONS"}, {3, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK15: commas in templates and calls pass",
                 "src/a.cpp",
                 "void f() {\n    std::map<int, int> m;\n    g(a, b);\n"
                 "    std::vector<int> v{1, 2};\n"
                 "    std::array<std::array<bool, 10>, 10> w{};\n"
                 "    for (int i = 0, j = 0; i < 3; ++i) {\n        h();\n"
                 "    }\n}\n",
                 FileKind::Cpp, {});
    checkSnippet(counter, "CK16: commas inside enums pass", "src/a.cpp",
                 "enum class E { A, B };\nenum class F {\n    C,\n    D\n};\n",
                 FileKind::Cpp, {});
}

// CK21, CK27 and CK34: initializers, templates, lambda bodies.
void checkStructuredDeclarations(TestCounter& counter) {
    checkSnippet(counter, "CK21: constructor initializer list passes",
                 "src/a.cpp",
                 "class C {\npublic:\n    C() : a_(1), b_(2) {\n    }\n\n"
                 "private:\n    int a_;\n    int b_;\n};\n",
                 FileKind::Cpp, {});
    checkSnippet(counter, "CK27: template parameter list passes", "src/a.h",
                 "template <class A, class B>\nstruct P;\n", FileKind::Cpp, {});
    checkSnippet(counter, "CK34: declarations inside a lambda body count",
                 "src/a.cpp",
                 "void f() {\n    auto g = [a, b] {\n        int p, q;\n"
                 "    };\n}\n",
                 FileKind::Cpp, {{3, "STYLE-DECLARATIONS"}});
}

// CK39 to CK42: comparisons, shifts, nested templates, braces.
void checkOperatorDeclarations(TestCounter& counter) {
    checkSnippet(counter, "CK39: comparisons with spaces are not templates",
                 "src/a.cpp",
                 "void f() {\n    bool c = a < b;\n    bool d = a > b;\n"
                 "    bool e = a <= b, g = a >= b;\n}\n",
                 FileKind::Cpp, {{4, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK40: shift operators are not templates",
                 "src/a.cpp",
                 "void f() {\n    int x = a << 2;\n    int y = a >> 1, z = 2;\n"
                 "    std::cout << a << b;\n}\n",
                 FileKind::Cpp, {{3, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK41: nested templates close correctly", "src/a.cpp",
                 "void f() {\n    std::vector<std::vector<int>> v;\n"
                 "    std::map<int, std::pair<int, int>> m, n;\n}\n",
                 FileKind::Cpp, {{3, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK42: brace initializers nest correctly",
                 "src/a.cpp",
                 "void f() {\n    std::map<int, int> m{{1, 2}, {3, 4}};\n"
                 "    int a[2][2] = {{1, 2}, {3, 4}};\n"
                 "    S s = {1, 2}, t = {3, 4};\n}\n",
                 FileKind::Cpp, {{4, "STYLE-DECLARATIONS"}});
}

// CK53 and CK54: a '<' or '{' at the very first character.
void checkFirstCharacter(TestCounter& counter) {
    checkSnippet(counter, "CK53: less-than at the first character", "src/a.cpp",
                 "<\nint p, q;\ntemplate\n", FileKind::Cpp,
                 {{2, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK54: brace at the first character starts a block",
                 "src/a.cpp", "{\n    int a, b;\n}\nint x\n", FileKind::Cpp,
                 {{2, "STYLE-DECLARATIONS"}});
}

// CK43 to CK46: calls, parentheses and the known limitations.
void checkCallDeclarations(TestCounter& counter) {
    checkSnippet(counter, "CK43: commas inside calls pass", "src/a.cpp",
                 "void f() {\n    int x = g(a, b);\n    h(a, g(b, c));\n"
                 "    int y = g(a, b), z = 1;\n}\n",
                 FileKind::Cpp, {{4, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK44: parenthesized comparisons pass", "src/a.cpp",
                 "void f() {\n    bool ok = x < y && y > z;\n"
                 "    bool p = (x < y), q = (y > z);\n}\n",
                 FileKind::Cpp, {{3, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK45: comma expression uses declaration message",
                 "src/a.cpp", "void f() {\n    a = 1, b = 2;\n}\n",
                 FileKind::Cpp, {{2, "STYLE-DECLARATIONS"}});
    checkSnippet(counter, "CK46: limitation L1, a<b is read as a template",
                 "src/a.cpp", "void f() {\n    bool p = a<b, q = c>d;\n}\n",
                 FileKind::Cpp, {});
}

}  // namespace

void runCodeCheckerDeclarationTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Code checker: declarations ---\n";
    }
    checkBasicDeclarations(counter);
    checkStructuredDeclarations(counter);
    checkOperatorDeclarations(counter);
    checkCallDeclarations(counter);
    checkFirstCharacter(counter);
}

}  // namespace aih
