"""The code checker's snippet tests (plan STAGE_00 Section 6, CK1-CK..). Each: id, path, text, expected."""
import sys; sys.path.insert(0, '.')
from ref_code_checker import check_text

def body(n):   # a function whose braces span exactly n lines (inclusive)
    return 'void f() {\n' + ''.join('    g();\n' for _ in range(n - 2)) + '}\n'

S = [
 ('CK1',  'src/a.cpp', 'int f() {\n    return 1;\n}\n'),
 ('CK2',  'src/a.cpp', body(40)),
 ('CK3',  'src/a.cpp', body(41)),
 ('CK4',  'src/a.cpp', 'int x;\n' * 200),
 ('CK5',  'src/a.cpp', 'int x;\n' * 201),
 ('CK6',  'src/a.cpp', 'void f() {\n    if (x) return;\n}\n'),
 ('CK7',  'src/a.cpp', 'void f() {\n    if (x) {\n        g();\n    } else g();\n}\n'),
 ('CK8',  'src/a.cpp', 'void f() {\n    if (x) {\n        g();\n    } else if (y) {\n        h();\n    }\n}\n'),
 ('CK9',  'src/a.cpp', 'void f() {\n    for (int i = 0; i < 3; ++i) g();\n    while (x) g();\n}\n'),
 ('CK10', 'src/a.cpp', 'void f() {\n    do {\n        g();\n    } while (x);\n}\n'),
 ('CK11', 'src/a.cpp', 'void f() {\n    if (a &&\n        b) {\n        g();\n    }\n}\n'),
 ('CK12', 'src/a.cpp', 'void f() {\n    a = 1; b = 2;\n}\n'),
 ('CK13', 'src/a.cpp', 'void f() {\n    std::string s = "a;b;c";\n    g(); // h(); k();\n}\n'),
 ('CK14', 'src/a.cpp', 'void f() {\n    int a, b;\n    int c = 1, d = 2;\n}\n'),
 ('CK15', 'src/a.cpp', 'void f() {\n    std::map<int, int> m;\n    g(a, b);\n    std::vector<int> v{1, 2};\n    std::array<std::array<bool, 10>, 10> w{};\n    for (int i = 0, j = 0; i < 3; ++i) {\n        h();\n    }\n}\n'),
 ('CK16', 'src/a.cpp', 'enum class E { A, B };\nenum class F {\n    C,\n    D\n};\n'),
 ('CK17', 'src/a.cpp', 'void f() {\n    auto s = R"(x)";\n}\n'),
 ('CK18', 'src/agent/a.cpp', '#include "experiment/fault_event.h"\n#include "world/rules.h"\n'),
 ('CK19', 'src/experiment/a.cpp', '#include "experiment/fault_event.h"\n'),
 ('CK20', 'src/a.cpp', 'struct S {\n    bool operator==(const S& o) const {\n' + ''.join('        g();\n' for _ in range(40)) + '    }\n};\n'),
 ('CK21', 'src/a.cpp', 'class C {\npublic:\n    C() : a_(1), b_(2) {\n    }\n\nprivate:\n    int a_;\n    int b_;\n};\n'),
 ('CK22', 'src/a.cpp', 'namespace n {\nnamespace {\nconst int k[] = {1, 2, 3};\n}  // namespace\n' + body(41) + '}  // namespace n\n'),
 ('CK23', 'src/a.cpp', 'void f() {\n    auto g = [](int x) {\n        return x;\n    };\n    if (x) g(1);\n}\n'),
 ('CK24', 'src/a.cpp', '/* if (x) return; a; b;\n   int a, b; */\nint f() {\n    return 0;\n}\n'),
 ('CK27', 'src/a.h', 'template <class A, class B>\nstruct P;\n'),
 ('CK29', 'src/a.cpp', 'auto g = [](int x) {\n' + '    h();\n' * 39 + '};\n'),
 ('CK30', 'src/a.cpp', 'auto g = [](int x) {\n' + '    h();\n' * 38 + '};\n'),
 ('CK31', 'src/a.cpp', 'void f() {\n    auto g = [&](int x) {\n' + '        h();\n' * 39 + '    };\n}\n'),
 ('CK32', 'src/a.cpp', 'auto outer = []() {\n    auto inner = [](int y) {\n' + '        h();\n' * 39 + '    };\n};\n'),
 ('CK33', 'src/a.cpp', 'const bool k = std::any_of(a, b, [](int x) {\n' + '    h();\n' * 39 + '});\n'),
 ('CK34', 'src/a.cpp', 'void f() {\n    auto g = [a, b] {\n        int p, q;\n    };\n}\n'),
 ('CK35', 'src/a.cpp', 'auto f() -> std::pair<int, int> {\n' + '    h();\n' * 39 + '}\n'),
 ('CK36', 'src/a.cpp', 'int S::get(const std::map<int, int>& m) const noexcept {\n' + '    h();\n' * 39 + '}\n'),
 ('CK37', 'src/a.cpp', '[[nodiscard]] int f(int a,\n                  int b) {\n' + '    h();\n' * 39 + '}\n'),
 ('CK38', 'src/a.cpp', 'const int k[] = {\n' + '    1,\n' * 45 + '};\nconst std::vector<int> v{\n' + '    2,\n' * 45 + '};\n'),
 ('CK39', 'src/a.cpp', 'void f() {\n    bool c = a < b;\n    bool d = a > b;\n    bool e = a <= b, g = a >= b;\n}\n'),
 ('CK40', 'src/a.cpp', 'void f() {\n    int x = a << 2;\n    int y = a >> 1, z = 2;\n    std::cout << a << b;\n}\n'),
 ('CK41', 'src/a.cpp', 'void f() {\n    std::vector<std::vector<int>> v;\n    std::map<int, std::pair<int, int>> m, n;\n}\n'),
 ('CK42', 'src/a.cpp', 'void f() {\n    std::map<int, int> m{{1, 2}, {3, 4}};\n    int a[2][2] = {{1, 2}, {3, 4}};\n    S s = {1, 2}, t = {3, 4};\n}\n'),
 ('CK43', 'src/a.cpp', 'void f() {\n    int x = g(a, b);\n    h(a, g(b, c));\n    int y = g(a, b), z = 1;\n}\n'),
 ('CK44', 'src/a.cpp', 'void f() {\n    bool ok = x < y && y > z;\n    bool p = (x < y), q = (y > z);\n}\n'),
 ('CK45', 'src/a.cpp', 'void f() {\n    a = 1, b = 2;\n}\n'),
 ('CK46', 'src/a.cpp', 'void f() {\n    bool p = a<b, q = c>d;\n}\n'),
 ('CK47', 'src/agent/a.cpp', '#include "../experiment/fault_event.h"\n#include "../../src/experiment/x.h"\n#include <experiment/y.h>\n#include "agent/experiment_notes.h"\n#include "world/experiment.h"\n'),
 ('CK48', 'src/agent/sub/b.h', '// #include "experiment/x.h"\n/*\n#include "experiment/x.h"\n*/\n#include "..\\..\\experiment\\x.h"\n'),
 ('CK49', 'src/agent_tools/a.cpp', '#include "experiment/x.h"\n'),
 ('CK51', 'src/a.cpp', 'auto f = []() noexcept(false) {\n' + '    h();\n' * 39 + '};\n'),
 ('CK52', 'src/a.cpp', 'auto g = [](int x) mutable noexcept(true) -> decltype(x) {\n' + '    h();\n' * 39 + '};\nauto k = [](int x) noexcept(false) {\n    int p, q;\n};\n'),
 ('CK53', 'src/a.cpp', '<\nint p, q;\ntemplate\n'),
 ('CK50', 'src/a.cpp', 'auto make() {\n    return [](int x) {\n' + '        h();\n' * 39 + '    };\n}\nauto g() {\n    return [a, b] {\n        int p, q;\n    };\n}\n'),
]
PS = [
 ('CK25', 'run_checks.ps1', 'function A {\n' + '    x\n' * 39 + '}\n'),
 ('CK26', 'run_checks.ps1', 'function B {\n' + '    x\n' * 38 + '}\n'),
]

# The expected (line, rule) lists written in plan Section 6.2.
EXPECTED = {
 'CK1': [], 'CK2': [], 'CK3': [(1, 'SIZE-FUNCTION')], 'CK4': [],
 'CK5': [(1, 'SIZE-FILE')], 'CK6': [(2, 'STYLE-BRACES')],
 'CK7': [(4, 'STYLE-BRACES')], 'CK8': [],
 'CK9': [(2, 'STYLE-BRACES'), (3, 'STYLE-BRACES')], 'CK10': [], 'CK11': [],
 'CK12': [(2, 'STYLE-STATEMENTS')], 'CK13': [],
 'CK14': [(2, 'STYLE-DECLARATIONS'), (3, 'STYLE-DECLARATIONS')],
 'CK15': [], 'CK16': [], 'CK17': [(2, 'STYLE-RAW-STRING')],
 'CK18': [(1, 'ARCH-INCLUDE')], 'CK19': [], 'CK20': [(2, 'SIZE-FUNCTION')],
 'CK21': [], 'CK22': [(5, 'SIZE-FUNCTION')], 'CK23': [(5, 'STYLE-BRACES')],
 'CK24': [], 'CK27': [], 'CK25': [(1, 'SIZE-FUNCTION')], 'CK26': [],
 'CK29': [(1, 'SIZE-FUNCTION')], 'CK30': [],
 'CK31': [(1, 'SIZE-FUNCTION'), (2, 'SIZE-FUNCTION')],
 'CK32': [(1, 'SIZE-FUNCTION'), (2, 'SIZE-FUNCTION')],
 'CK33': [(1, 'SIZE-FUNCTION')], 'CK34': [(3, 'STYLE-DECLARATIONS')],
 'CK35': [(1, 'SIZE-FUNCTION')], 'CK36': [(1, 'SIZE-FUNCTION')],
 'CK37': [(2, 'SIZE-FUNCTION')], 'CK38': [],
 'CK39': [(4, 'STYLE-DECLARATIONS')], 'CK40': [(3, 'STYLE-DECLARATIONS')],
 'CK41': [(3, 'STYLE-DECLARATIONS')], 'CK42': [(4, 'STYLE-DECLARATIONS')],
 'CK43': [(4, 'STYLE-DECLARATIONS')], 'CK44': [(3, 'STYLE-DECLARATIONS')],
 'CK45': [(2, 'STYLE-DECLARATIONS')], 'CK46': [],
 'CK47': [(1, 'ARCH-INCLUDE'), (2, 'ARCH-INCLUDE'), (3, 'ARCH-INCLUDE')],
 'CK48': [(5, 'ARCH-INCLUDE')], 'CK49': [],
 'CK51': [(1, 'SIZE-FUNCTION')],
 'CK52': [(1, 'SIZE-FUNCTION'), (43, 'STYLE-DECLARATIONS')],
 'CK53': [(2, 'STYLE-DECLARATIONS')],
 'CK50': [(1, 'SIZE-FUNCTION'), (2, 'SIZE-FUNCTION'), (46, 'STYLE-DECLARATIONS')],
}

# Which files are checked (plan 3.4): FS1-FS10, path -> kind.
from ref_code_checker import checked_kind
FILES = [
 ('FS1', 'src/core/a.cpp', 'cpp'),
 ('FS2', 'tests/test_main.cpp', 'cpp'),
 ('FS3', 'tools/code_checker/checker.h', 'cpp'),
 ('FS4', 'tools/reference/stage0/x.cpp', ''),
 ('FS5', 'cmake-build-checks/CMakeFiles/a.cpp', ''),
 ('FS6', 'src/cmake-build-debug/a.cpp', ''),
 ('FS7', 'docs/a.cpp', ''),
 ('FS8', 'run_checks.ps1', 'ps1'),
 ('FS9', 'tools/checks/build_steps.ps1', 'ps1'),
 ('FS10', 'tools/reference/stage4/gen.ps1', ''),
 ('FS11', 'src/notes.txt', ''),
 ('FS12', 'main.cpp', ''),
]


def files_failing():
    return [fid for fid, path, kind in FILES if checked_kind(path) != kind]


def results():
    out = {}
    for sid, path, text in S:
        out[sid] = [(l, r) for l, r, _ in check_text(path, text, 'cpp')]
    for sid, path, text in PS:
        out[sid] = [(l, r) for l, r, _ in check_text(path, text, 'ps1')]
    return out

def failing():
    got = results()
    return [sid for sid in EXPECTED if got[sid] != EXPECTED[sid]]

if __name__ == '__main__':
    for sid, vs in results().items():
        print(sid, vs)
    bad = failing()
    print('mismatches:', bad if bad else 'none', f'({len(EXPECTED)} cases)')
    for fid, path, kind in FILES:
        print(fid, path, repr(checked_kind(path)))
    fbad = files_failing()
    print('file-kind mismatches:', fbad if fbad else 'none', f'({len(FILES)} cases)')
