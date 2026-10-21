"""Reference for the Stage 0 code checker (Python; not project code).
Rules (plan STAGE_00 v1.1, Section 3.4): SIZE-FILE, SIZE-FUNCTION, STYLE-BRACES, STYLE-STATEMENTS,
STYLE-DECLARATIONS, STYLE-RAW-STRING, ARCH-INCLUDE. Input text uses LF line endings."""
import re, sys, os

MAX_FILE = 200
MAX_FUNCTION = 40


def clean(text):
    """Replaces comments and string/char literal contents with spaces, keeping every newline.
    Returns (cleaned, raw_string_lines)."""
    out = []; i = 0; n = len(text); raw = []
    line = 1
    while i < n:
        c = text[i]
        if c == '\n':
            out.append('\n'); line += 1; i += 1; continue
        if text.startswith('//', i):
            while i < n and text[i] != '\n':
                out.append(' '); i += 1
            continue
        if text.startswith('/*', i):
            out.append('  '); i += 2
            while i < n and not text.startswith('*/', i):
                if text[i] == '\n': out.append('\n'); line += 1
                else: out.append(' ')
                i += 1
            if i < n: out.append('  '); i += 2
            continue
        if c == 'R' and text.startswith('R"', i) and (i == 0 or not (text[i-1].isalnum() or text[i-1] == '_')):
            raw.append(line)
            out.append('R'); i += 1
            continue
        if c == '"' or c == "'":
            q = c; out.append(q); i += 1
            while i < n and text[i] != q and text[i] != '\n':
                if text[i] == '\\' and i + 1 < n and text[i+1] != '\n':
                    out.append('  '); i += 2; continue
                out.append(' '); i += 1
            if i < n and text[i] == q: out.append(q); i += 1
            continue
        out.append(c); i += 1
    return ''.join(out), raw


def lines_of(text):
    ls = text.split('\n')
    if ls and ls[-1] == '': ls = ls[:-1]
    return ls


def tokens(cleaned):
    """(kind, value, line, pos) for identifiers and single punctuation chars."""
    toks = []; line = 1; i = 0; n = len(cleaned)
    while i < n:
        c = cleaned[i]
        if c == '\n': line += 1; i += 1; continue
        if c.isspace(): i += 1; continue
        if c.isalpha() or c == '_':
            j = i
            while j < n and (cleaned[j].isalnum() or cleaned[j] == '_'): j += 1
            toks.append(('id', cleaned[i:j], line, i)); i = j; continue
        if c.isdigit():
            j = i
            while j < n and (cleaned[j].isalnum() or cleaned[j] in "._'"): j += 1
            toks.append(('num', cleaned[i:j], line, i)); i = j; continue
        toks.append(('p', c, line, i)); i += 1
    return toks


def matching(toks, k, open_c, close_c):
    depth = 0
    for j in range(k, len(toks)):
        if toks[j][0] == 'p' and toks[j][1] == open_c: depth += 1
        elif toks[j][0] == 'p' and toks[j][1] == close_c:
            depth -= 1
            if depth == 0: return j
    return None


def check_braces(toks):
    v = []
    for k, t in enumerate(toks):
        if t[0] != 'id': continue
        w = t[1]
        if w in ('if', 'for', 'while', 'switch'):
            if k + 1 < len(toks) and toks[k+1][1] == 'constexpr' and w == 'if': k2 = k + 1
            else: k2 = k
            if k2 + 1 >= len(toks) or toks[k2+1][1] != '(': continue
            close = matching(toks, k2 + 1, '(', ')')
            if close is None or close + 1 >= len(toks): continue
            nxt = toks[close + 1]
            if w == 'while' and nxt[1] == ';' and k > 0 and toks[k-1][1] == '}':
                continue   # the end of a do-while
            if nxt[1] != '{':
                v.append((t[2], 'STYLE-BRACES', f"'{w}' body must be in braces"))
        elif w == 'else':
            if k + 1 < len(toks) and toks[k+1][1] not in ('{', 'if'):
                v.append((t[2], 'STYLE-BRACES', "'else' body must be in braces"))
        elif w == 'do':
            if k + 1 < len(toks) and toks[k+1][1] != '{':
                v.append((t[2], 'STYLE-BRACES', "'do' body must be in braces"))
    return v


def check_statements(cleaned):
    """More than one ';' on a line outside parentheses (a for-header's ';' are inside parentheses)."""
    v = []; depth = 0
    for no, ln in enumerate(lines_of(cleaned), 1):
        count = 0
        for c in ln:
            if c == '(': depth += 1
            elif c == ')': depth = max(0, depth - 1)
            elif c == ';' and depth == 0: count += 1
        if count > 1: v.append((no, 'STYLE-STATEMENTS', 'more than one statement on one line'))
    return v


def closes_lambda_intro(cleaned, j):
    """cleaned[j] is ']'. True if its matching '[' starts a lambda introducer."""
    depth = 0
    k = j
    while k >= 0:
        if cleaned[k] == ']': depth += 1
        elif cleaned[k] == '[':
            depth -= 1
            if depth == 0: break
        k -= 1
    if k < 0: return False
    m = k - 1
    while m >= 0 and cleaned[m] in ' \t\n': m -= 1
    if m < 0: return True
    if cleaned[:m + 1].endswith('return') and (m < 6 or not (cleaned[m - 6].isalnum() or cleaned[m - 6] == '_')):
        return True
    pc = cleaned[m]
    return not (pc.isalnum() or pc in '_)][')


def check_declarations(cleaned, toks):
    """A statement ending in ';' with a comma outside (), [], {} and template <> is a multiple
    declaration, unless it is inside an enum body. '<' opens a template only when it directly
    follows an identifier character with no space (and is not '<<' or '<=')."""
    v = []
    # enum body ranges (character positions)
    enum_ranges = []
    for k, t in enumerate(toks):
        if t[0] == 'id' and t[1] == 'enum':
            for j in range(k + 1, len(toks)):
                if toks[j][1] == ';': break
                if toks[j][1] == '{':
                    e = matching(toks, j, '{', '}')
                    if e is not None: enum_ranges.append((toks[j][3], toks[e][3]))
                    break
    def in_enum(p): return any(a < p < b for a, b in enum_ranges)
    n = len(cleaned); i = 0; line = 1
    start_line = 1; par = 0; brk = 0; brc = 0; ang = 0; comma_line = None
    while i < n:
        c = cleaned[i]
        if c == '\n': line += 1; i += 1; continue
        if c == '(': par += 1
        elif c == ')': par = max(0, par - 1)
        elif c == '[': brk += 1
        elif c == ']': brk = max(0, brk - 1)
        elif c == '<':
            prev = cleaned[i-1] if i > 0 else ' '
            nxt = cleaned[i+1] if i + 1 < n else ' '
            before = cleaned[:i].rstrip()
            if ((prev.isalnum() or prev == '_') or before.endswith('template')) and nxt not in '<=':
                ang += 1
        elif c == '>' and ang > 0:
            prev = cleaned[i-1] if i > 0 else ' '
            nxt = cleaned[i+1] if i + 1 < n else ' '
            if prev != '-' and nxt != '=':
                ang -= 1
        elif c == '{' or c == '}':
            if par == 0 and brk == 0 and ang == 0:
                if c == '{':
                    # '{' at statement level: either a block or a brace-initializer
                    j = i - 1
                    while j >= 0 and cleaned[j] in ' \t\n': j -= 1
                    prevc = cleaned[j] if j >= 0 else ''
                    if prevc == ']' and closes_lambda_intro(cleaned, j):
                        comma_line = None; i += 1; continue
                    if prevc.isalnum() or prevc in '_=>]':
                        # could be an initializer like x{1, 2} or = {...}: track as nesting
                        brc += 1; i += 1; continue
                    comma_line = None
                else:
                    if brc > 0: brc -= 1; i += 1; continue
                    comma_line = None
            else:
                if c == '{': brc += 1
                else: brc = max(0, brc - 1)
        elif c == ',' and par == 0 and brk == 0 and brc == 0 and ang == 0 and not in_enum(i):
            if comma_line is None: comma_line = line
        elif c == ';' and par == 0:
            if comma_line is not None and brc == 0:
                v.append((comma_line, 'STYLE-DECLARATIONS', 'more than one declaration in one statement'))
            comma_line = None
        i += 1
    return v


HEAD_STOP = set(';{}')


def is_lambda_intro(prev):
    """A '[' starts a lambda introducer unless it follows an identifier (other than the
    keyword 'return'), a number, ')' or ']' (array subscripts and declarators) or another '['
    (attributes like [[nodiscard]])."""
    if prev is None or prev[1] == 'return':
        return True
    return not (prev[0] in ('id', 'num') or prev[1] in (')', ']', '['))


def only_specifiers(rest):
    """True if rest holds no '(', ')', '[', ']' or '=', except that the words 'noexcept' and
    'decltype' may each be followed by one balanced (...) group, e.g. noexcept(false)."""
    k = 0
    while k < len(rest):
        word = rest[k][1]
        if word in ('noexcept', 'decltype') and k + 1 < len(rest) and rest[k + 1][1] == '(':
            close = matching(rest, k + 1, '(', ')')
            if close is None:
                return False
            k = close + 1
            continue
        if word in ('(', ')', '=', '[', ']'):
            return False
        k += 1
    return True


def lambda_in_head(head):
    """True if the '{' that ends this head opens a lambda body: the head contains a lambda
    introducer [..] followed by nothing, or by (..) and then only specifier tokens
    (no '(', ')', '=' after the parameter list)."""
    for a in range(len(head) - 1, -1, -1):
        if head[a][1] != '[':
            continue
        if a + 1 < len(head) and head[a + 1][1] == '[':
            continue
        prev = head[a - 1] if a > 0 else None
        if not is_lambda_intro(prev):
            continue
        b = matching(head, a, '[', ']')
        if b is None:
            continue
        rest = head[b + 1:]
        if rest and rest[0][1] == '(':
            c = matching(rest, 0, '(', ')')
            if c is None:
                continue
            rest = rest[c + 1:]
        if only_specifiers(rest):
            return True
    return False


def check_functions(cleaned, toks):
    """Function bodies: a '{' at namespace or class scope whose head (since the previous ';', '{'
    or '}') contains ')' and no '=' outside parentheses. Length = closing line - opening line + 1."""
    v = []; stack = []   # scope kinds
    head = []
    for k, t in enumerate(toks):
        if t[0] == 'p' and t[1] == '{':
            words = [x[1] for x in head]
            kind = 'block'
            outer = stack[-1][0] if stack else 'namespace'
            if lambda_in_head(head): kind = 'lambda'
            elif 'namespace' in words: kind = 'namespace'
            elif outer in ('function', 'block', 'lambda'): kind = 'block'
            elif any(w in ('class', 'struct', 'union', 'enum') for w in words) and ')' not in words: kind = 'class'
            elif ')' in words:
                depth = 0; eq = False
                for w in words:
                    if w == '(': depth += 1
                    elif w == ')': depth -= 1
                    elif w == '=' and depth == 0: eq = True
                kind = 'block' if (eq and 'operator' not in words) else 'function'
            stack.append((kind, t[2]))
            head = []
        elif t[0] == 'p' and t[1] == '}':
            if stack:
                kind, open_line = stack.pop()
                if kind in ('function', 'lambda'):
                    length = t[2] - open_line + 1
                    if length > MAX_FUNCTION:
                        v.append((open_line, 'SIZE-FUNCTION', f'function is {length} lines (limit {MAX_FUNCTION})'))
            head = []
        elif t[0] == 'p' and t[1] == ';':
            head = []
        else:
            head.append(t)
    return v


def check_text(path, text, kind):
    """kind: 'cpp' or 'ps1'. Returns sorted list of (line, rule, message)."""
    v = []
    ls = lines_of(text)
    limit_file = MAX_FILE
    if len(ls) > limit_file:
        v.append((1, 'SIZE-FILE', f'file is {len(ls)} lines (limit {limit_file})'))
    if kind == 'ps1':
        v += check_ps1_functions(ls)
        return sorted(v)
    cleaned, raw = clean(text)
    for r in raw: v.append((r, 'STYLE-RAW-STRING', 'raw string literals are not allowed'))
    toks = tokens(cleaned)
    v += check_functions(cleaned, toks)
    v += check_braces(toks)
    v += check_statements(cleaned)
    v += check_declarations(cleaned, toks)
    p = path.replace('\\', '/')
    if p.startswith('src/agent/'):
        cl = lines_of(cleaned)
        for no, ln in enumerate(ls, 1):
            if not re.match(r'\s*#\s*include\b', cl[no - 1]):
                continue
            m = re.search(r'["<]([^">]*)[">]', ln)
            if m and re.search(r'(^|/)experiment/', m.group(1).replace('\\', '/')):
                v.append((no, 'ARCH-INCLUDE', 'agent code must not include experiment code'))
    return sorted(set(v))


def check_ps1_functions(ls):
    v = []
    for no, ln in enumerate(ls, 1):
        if re.match(r'\s*function\s+[\w-]+', ln, re.I):
            depth = 0; started = False
            for j in range(no - 1, len(ls)):
                for c in ls[j]:
                    if c == '{': depth += 1; started = True
                    elif c == '}': depth -= 1
                if started and depth == 0:
                    length = j - (no - 1) + 1
                    if length > MAX_FUNCTION:
                        v.append((no, 'SIZE-FUNCTION', f'function is {length} lines (limit {MAX_FUNCTION})'))
                    break
    return v


def format_v(path, vs):
    return [f'{path}:{l}: {r} {m}' for l, r, m in vs]


if __name__ == '__main__':
    root = sys.argv[1]
    total = 0
    for base in ('src', 'tests', 'tools'):
        for d, _, fs in sorted(os.walk(os.path.join(root, base))):
            for f in sorted(fs):
                if not f.endswith(('.h', '.cpp')): continue
                full = os.path.join(d, f); rel = os.path.relpath(full, root).replace('\\', '/')
                vs = check_text(rel, open(full, encoding='utf-8').read().replace('\r\n', '\n'), 'cpp')
                for s in format_v(rel, vs): print(s)
                total += len(vs)
    print(f'{total} violations')
