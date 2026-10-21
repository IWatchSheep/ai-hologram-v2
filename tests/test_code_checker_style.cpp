// Purpose: Tests for the code checker's style rules: braces
// around bodies, one statement per line, no raw strings, and
// the exact format of one printed violation (CK28).
// Introduced in: Stage 0b (rebuild)
// Architecture: Each CK check compares the full (line, rule)
// list from checkText with the reference values in plan 6.2,
// except CK28, which compares the formatted line exactly.

#include <string>
#include <vector>

#include "checker.h"
#include "test_suites.h"
#include "test_support.h"
#include "text_cleaner.h"

namespace aih {

namespace {

// CK6 to CK11: bodies of if, else, for, while and do.
void checkBraceSnippets(TestCounter& counter) {
    checkSnippet(counter, "CK6: if without braces is reported", "src/a.cpp",
                 "void f() {\n    if (x) return;\n}\n", FileKind::Cpp,
                 {{2, "STYLE-BRACES"}});
    checkSnippet(counter, "CK7: else without braces is reported", "src/a.cpp",
                 "void f() {\n    if (x) {\n        g();\n    } else g();\n}\n",
                 FileKind::Cpp, {{4, "STYLE-BRACES"}});
    checkSnippet(counter, "CK8: else if with braces passes", "src/a.cpp",
                 "void f() {\n    if (x) {\n        g();\n    } else if (y) {\n"
                 "        h();\n    }\n}\n",
                 FileKind::Cpp, {});
    checkSnippet(counter, "CK9: for and while without braces", "src/a.cpp",
                 "void f() {\n    for (int i = 0; i < 3; ++i) g();\n"
                 "    while (x) g();\n}\n",
                 FileKind::Cpp, {{2, "STYLE-BRACES"}, {3, "STYLE-BRACES"}});
    checkSnippet(counter, "CK10: do while with braces passes", "src/a.cpp",
                 "void f() {\n    do {\n        g();\n    } while (x);\n}\n",
                 FileKind::Cpp, {});
    checkSnippet(
        counter, "CK11: condition split over two lines passes", "src/a.cpp",
        "void f() {\n    if (a &&\n        b) {\n        g();\n    }\n}\n",
        FileKind::Cpp, {});
}

// CK12, CK13, CK17, CK23 and CK24: statements, text, lambdas.
void checkOtherStyleSnippets(TestCounter& counter) {
    checkSnippet(counter, "CK12: two statements on one line", "src/a.cpp",
                 "void f() {\n    a = 1; b = 2;\n}\n", FileKind::Cpp,
                 {{2, "STYLE-STATEMENTS"}});
    checkSnippet(counter, "CK13: semicolons in strings and comments pass",
                 "src/a.cpp",
                 "void f() {\n    std::string s = \"a;b;c\";\n"
                 "    g(); // h(); k();\n}\n",
                 FileKind::Cpp, {});
    checkSnippet(counter, "CK17: raw string is reported", "src/a.cpp",
                 "void f() {\n    auto s = R\"(x)\";\n}\n", FileKind::Cpp,
                 {{2, "STYLE-RAW-STRING"}});
    checkSnippet(counter, "CK23: if after a lambda still needs braces",
                 "src/a.cpp",
                 "void f() {\n    auto g = [](int x) {\n        return x;\n"
                 "    };\n    if (x) g(1);\n}\n",
                 FileKind::Cpp, {{5, "STYLE-BRACES"}});
    checkSnippet(counter, "CK24: bad code inside a block comment passes",
                 "src/a.cpp",
                 "/* if (x) return; a; b;\n   int a, b; */\nint f() {\n"
                 "    return 0;\n}\n",
                 FileKind::Cpp, {});
}

// CK28: the formatted form of the CK6 violation, exactly.
void checkFormattedLine(TestCounter& counter) {
    std::string text = "void f() {\n    if (x) return;\n}\n";
    std::vector<Violation> violations =
        checkText("src/a.cpp", text, FileKind::Cpp);
    std::string line;
    if (!violations.empty()) {
        line = formatViolation("src/a.cpp", violations[0]);
    }
    std::string expected =
        "src/a.cpp:2: STYLE-BRACES 'if' body must be in braces";
    check(counter, line == expected, "CK28: formatted violation line is exact");
}

}  // namespace

void runCodeCheckerStyleTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Code checker: style ---\n";
    }
    checkBraceSnippets(counter);
    checkOtherStyleSnippets(counter);
    checkFormattedLine(counter);
}

}  // namespace aih
