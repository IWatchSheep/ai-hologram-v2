// Purpose: Implements STYLE-DECLARATIONS: a statement ending
// in ';' with a comma outside (), [], {} and template <> is
// more than one declaration in one statement.
// Introduced in: Stage 0b (rebuild)
// Architecture: One character scan of the cleaned text keeps
// the nesting counts. Enum bodies are exempt, and a '{' that
// opens a lambda body resets the statement like a block does.

#include "declaration_rule.h"

#include <algorithm>
#include <utility>

#include "function_rule.h"
#include "text_cleaner.h"

namespace aih {

namespace {

// The state of one scan: nesting counts and the first comma.
struct DeclarationScan {
    const std::string* cleaned = nullptr;
    const std::vector<Token>* tokens = nullptr;
    std::vector<std::pair<int, int>> enumRanges;
    std::vector<Violation> violations;
    int par = 0;
    int brk = 0;
    int brc = 0;
    int ang = 0;
    int commaLine = 0;
    int line = 1;
    int at = 0;
};

bool isAlphaNumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

// Collects the character ranges of enum bodies from tokens.
std::vector<std::pair<int, int>> enumRangesOf(
    const std::vector<Token>& tokens) {
    std::vector<std::pair<int, int>> ranges;
    int size = static_cast<int>(tokens.size());
    for (int k = 0; k < size; k = k + 1) {
        if (tokens[k].kind != TokenKind::Identifier ||
            tokens[k].value != "enum") {
            continue;
        }
        for (int j = k + 1; j < size; j = j + 1) {
            if (tokens[j].value == ";") {
                break;
            }
            if (tokens[j].value == "{") {
                int end = findMatching(tokens, j, '{', '}');
                if (end >= 0) {
                    ranges.push_back(
                        {tokens[j].position, tokens[end].position});
                }
                break;
            }
        }
    }
    return ranges;
}

bool inEnum(const DeclarationScan& scan, int position) {
    for (const std::pair<int, int>& range : scan.enumRanges) {
        if (range.first < position && position < range.second) {
            return true;
        }
    }
    return false;
}

// True when the ']' at position closes a lambda introducer,
// using the one lambda test from function_rule.
bool closesLambdaIntro(const DeclarationScan& scan, int position) {
    const std::vector<Token>& tokens = *scan.tokens;
    int size = static_cast<int>(tokens.size());
    int close = -1;
    for (int k = 0; k < size && close < 0; k = k + 1) {
        if (tokens[k].position == position && tokens[k].value == "]") {
            close = k;
        }
    }
    int open = -1;
    for (int k = 0; k < close && open < 0; k = k + 1) {
        if (tokens[k].value == "[" &&
            findMatching(tokens, k, '[', ']') == close) {
            open = k;
        }
    }
    if (open < 0) {
        return false;
    }
    const Token* previous = open > 0 ? &tokens[open - 1] : nullptr;
    return isLambdaIntroducer(previous);
}

// True when the text before i, blanks trimmed, ends in template.
bool beforeEndsWithTemplate(const std::string& cleaned, int i) {
    std::size_t last = cleaned.find_last_not_of(" \t\n\r", i - 1);
    bool found = last != std::string::npos && last >= 7;
    return found && cleaned.compare(last - 7, 8, "template") == 0;
}

// Handles '<' and '>': '<' opens a template only in the cases
// plan 3.4 lists; '>' closes one unless part of -> or >=.
void scanAngle(DeclarationScan& scan, char c) {
    const std::string& cleaned = *scan.cleaned;
    char prev = scan.at > 0 ? cleaned[scan.at - 1] : ' ';
    int size = static_cast<int>(cleaned.size());
    char next = scan.at + 1 < size ? cleaned[scan.at + 1] : ' ';
    bool followsWord = isAlphaNumeric(prev) || prev == '_';
    bool opens = (followsWord || beforeEndsWithTemplate(cleaned, scan.at)) &&
                 next != '<' && next != '=';
    if (c == '<' && opens) {
        scan.ang = scan.ang + 1;
    }
    if (c == '>' && prev != '-' && next != '=') {
        scan.ang = scan.ang - 1;
    }
}

// Handles '{' and '}': at statement level an initializer
// nests, while a lambda body or block resets the statement.
void scanBrace(DeclarationScan& scan, char c) {
    const std::string& cleaned = *scan.cleaned;
    if (scan.par != 0 || scan.brk != 0 || scan.ang != 0) {
        scan.brc = c == '{' ? scan.brc + 1 : std::max(0, scan.brc - 1);
        return;
    }
    if (c == '}') {
        if (scan.brc > 0) {
            scan.brc = scan.brc - 1;
        } else {
            scan.commaLine = 0;
        }
        return;
    }
    std::size_t found = cleaned.find_last_not_of(" \t\n", scan.at - 1);
    int j = found == std::string::npos ? -1 : static_cast<int>(found);
    char prev = j >= 0 ? cleaned[j] : '\0';
    bool initial = isAlphaNumeric(prev) || prev == '_' || prev == '=';
    if (prev == ']' && closesLambdaIntro(scan, j)) {
        scan.commaLine = 0;
    } else if (initial || prev == '>' || prev == ']') {
        scan.brc = scan.brc + 1;
    } else {
        scan.commaLine = 0;
    }
}

// Processes the one character at scan.at, then moves past it.
void scanChar(DeclarationScan& scan) {
    char c = (*scan.cleaned)[scan.at];
    if (c == '\n') {
        scan.line = scan.line + 1;
    } else if (c == '(' || c == ')') {
        scan.par = std::max(0, scan.par + (c == '(' ? 1 : -1));
    } else if (c == '[' || c == ']') {
        scan.brk = std::max(0, scan.brk + (c == '[' ? 1 : -1));
    } else if (c == '<') {
        scanAngle(scan, c);
    } else if (c == '>' && scan.ang > 0) {
        scanAngle(scan, c);
    } else if (c == '{' || c == '}') {
        scanBrace(scan, c);
    } else if (c == ',' && scan.par == 0 && scan.brk == 0 && scan.brc == 0 &&
               scan.ang == 0 && !inEnum(scan, scan.at)) {
        scan.commaLine = scan.commaLine == 0 ? scan.line : scan.commaLine;
    } else if (c == ';' && scan.par == 0) {
        if (scan.commaLine != 0 && scan.brc == 0) {
            std::string message = "more than one declaration in one statement";
            scan.violations.push_back(
                {scan.commaLine, "STYLE-DECLARATIONS", message});
        }
        scan.commaLine = 0;
    }
    scan.at = scan.at + 1;
}

}  // namespace

std::vector<Violation> checkDeclarations(const std::string& cleaned,
                                         const std::vector<Token>& tokens) {
    DeclarationScan scan;
    scan.cleaned = &cleaned;
    scan.tokens = &tokens;
    scan.enumRanges = enumRangesOf(tokens);
    int size = static_cast<int>(cleaned.size());
    while (scan.at < size) {
        scanChar(scan);
    }
    return scan.violations;
}

}  // namespace aih
