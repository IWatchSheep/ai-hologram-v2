// Purpose: Cleans C++ text and turns it into tokens, exactly as
// the reference checker in tools/reference/stage0 does.
// Introduced in: Stage 0b (rebuild)
// Architecture: One pass blanks comments and literal contents.
// Later rules work on the cleaned text and its tokens only.

#include "text_cleaner.h"

#include <string>
#include <vector>

namespace aih {

namespace {

bool isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isWordChar(char c) {
    return isLetter(c) || (c >= '0' && c <= '9') || c == '_';
}

// True when text starts with prefix at position at.
bool startsWith(const std::string& text, int at, const std::string& prefix) {
    int length = static_cast<int>(prefix.size());
    if (at + length > static_cast<int>(text.size())) {
        return false;
    }
    return text.compare(at, length, prefix) == 0;
}

// Blanks a // comment; i stops at the newline or the end.
void cleanLineComment(const std::string& text, int& i, CleanResult& result) {
    int size = static_cast<int>(text.size());
    while (i < size && text[i] != '\n') {
        result.cleaned.push_back(' ');
        i = i + 1;
    }
}

// Blanks a /* */ comment, keeping any newlines inside it.
void cleanBlockComment(const std::string& text, int& i, int& line,
                       CleanResult& result) {
    int size = static_cast<int>(text.size());
    result.cleaned += "  ";
    i = i + 2;
    while (i < size && !startsWith(text, i, "*/")) {
        char shown = text[i] == '\n' ? '\n' : ' ';
        result.cleaned.push_back(shown);
        if (text[i] == '\n') {
            line = line + 1;
        }
        i = i + 1;
    }
    if (i < size) {
        result.cleaned += "  ";
        i = i + 2;
    }
}

// Blanks one quoted literal; the quotes themselves stay.
void cleanLiteral(const std::string& text, int& i, CleanResult& result) {
    int size = static_cast<int>(text.size());
    char quote = text[i];
    result.cleaned.push_back(quote);
    i = i + 1;
    while (i < size && text[i] != quote && text[i] != '\n') {
        bool escaped = text[i] == '\\' && i + 1 < size && text[i + 1] != '\n';
        result.cleaned += escaped ? "  " : " ";
        i = i + (escaped ? 2 : 1);
    }
    if (i < size && text[i] == quote) {
        result.cleaned.push_back(quote);
        i = i + 1;
    }
}

// An R opens a raw string as R" with no word character before it.
bool isRawStringStart(const std::string& text, int i) {
    bool atStart = i == 0 || !isWordChar(text[i - 1]);
    return startsWith(text, i, "R\"") && atStart;
}

// Reads the word or number starting at i; i ends just past it.
Token readWord(const std::string& cleaned, int& i, int line, bool isNumber) {
    int size = static_cast<int>(cleaned.size());
    int j = i;
    bool extra = isNumber;
    while (j < size &&
           (isWordChar(cleaned[j]) || (extra && cleaned[j] == '.') ||
            (extra && cleaned[j] == '\''))) {
        j = j + 1;
    }
    TokenKind kind = isNumber ? TokenKind::Number : TokenKind::Identifier;
    Token token = {kind, cleaned.substr(i, j - i), line, i};
    i = j;
    return token;
}

}  // namespace

CleanResult cleanText(const std::string& text) {
    CleanResult result;
    int size = static_cast<int>(text.size());
    int line = 1;
    int i = 0;
    while (i < size) {
        char c = text[i];
        if (c == '\n') {
            result.cleaned.push_back('\n');
            line = line + 1;
            i = i + 1;
        } else if (startsWith(text, i, "//")) {
            cleanLineComment(text, i, result);
        } else if (startsWith(text, i, "/*")) {
            cleanBlockComment(text, i, line, result);
        } else if (c == 'R' && isRawStringStart(text, i)) {
            result.rawStringLines.push_back(line);
            result.cleaned.push_back('R');
            i = i + 1;
        } else if (c == '"' || c == '\'') {
            cleanLiteral(text, i, result);
        } else {
            result.cleaned.push_back(c);
            i = i + 1;
        }
    }
    return result;
}

std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;
    for (char c : text) {
        if (c == '\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    lines.push_back(current);
    if (!lines.empty() && lines.back().empty()) {
        lines.pop_back();
    }
    return lines;
}

std::vector<Token> tokenize(const std::string& cleaned) {
    std::vector<Token> tokens;
    int size = static_cast<int>(cleaned.size());
    int line = 1;
    int i = 0;
    while (i < size) {
        char c = cleaned[i];
        if (c == '\n') {
            line = line + 1;
            i = i + 1;
        } else if (c == ' ' || c == '\t' || c == '\r' || c == '\f' ||
                   c == '\v') {
            i = i + 1;
        } else if (isLetter(c) || c == '_') {
            tokens.push_back(readWord(cleaned, i, line, false));
        } else if (c >= '0' && c <= '9') {
            tokens.push_back(readWord(cleaned, i, line, true));
        } else {
            tokens.push_back(
                {TokenKind::Punctuation, std::string(1, c), line, i});
            i = i + 1;
        }
    }
    return tokens;
}

int findMatching(const std::vector<Token>& tokens, int start, char open,
                 char close) {
    int depth = 0;
    int size = static_cast<int>(tokens.size());
    for (int j = start; j < size; j = j + 1) {
        if (tokens[j].kind != TokenKind::Punctuation) {
            continue;
        }
        if (tokens[j].value == std::string(1, open)) {
            depth = depth + 1;
        } else if (tokens[j].value == std::string(1, close)) {
            depth = depth - 1;
            if (depth == 0) {
                return j;
            }
        }
    }
    return -1;
}

}  // namespace aih
