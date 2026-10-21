// Purpose: Declares the text cleaning and token helpers that
// every code checker rule is built on.
// Introduced in: Stage 0b (rebuild)
// Architecture: Comments and literal contents are blanked out
// here (and only here), so no other rule has to understand them.
// Tokens and the matching-bracket search also live only here.

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_TEXT_CLEANER_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_TEXT_CLEANER_H

#include <string>
#include <vector>

namespace aih {

// What a token is: a word, a number, or one punctuation mark.
enum class TokenKind { Identifier, Number, Punctuation };

// One token from cleaned text, with where it was found.
struct Token {
    TokenKind kind = TokenKind::Punctuation;
    std::string value;
    int line = 1;
    int position = 0;
};

// One rule violation: the line, the rule name and the message.
struct Violation {
    int line = 1;
    std::string rule;
    std::string message;
};

// The result of cleaning: the blanked text and the lines that
// contained a raw string literal (reported by the style rule).
struct CleanResult {
    std::string cleaned;
    std::vector<int> rawStringLines;
};

// The one set of character classes every rule uses (R4):
// a letter is a-z or A-Z, a word character is a letter, a
// digit or '_', and a space character is space, tab, '\r',
// '\f' or '\v' (a newline is not a space character).
bool isLetter(char c);
bool isWordChar(char c);
bool isSpaceChar(char c);

// Replaces comments and literal contents with spaces, keeping
// every line break, and records raw string lines.
CleanResult cleanText(const std::string& text);

// Splits text into lines; a final newline adds no empty line.
std::vector<std::string> linesOf(const std::string& text);

// Turns cleaned text into identifiers, numbers and punctuation.
std::vector<Token> tokenize(const std::string& cleaned);

// Finds the token matching the opening bracket at start, or
// returns -1. Brackets are counted, so nesting works.
int findMatching(const std::vector<Token>& tokens, int start, char open,
                 char close);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_TEXT_CLEANER_H
