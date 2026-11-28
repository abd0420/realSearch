#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstddef>
#include <string>
#include <vector>

using namespace std;

// Shared word splitter. The HTML parser (indexing) and the QueryProcessor
// (searching) MUST both use this, so a word is normalized identically on both
// sides.
//
// Rules:
//   * a word is a run of ASCII letters and digits (a-z, A-Z, 0-9)
//   * letters are lowercased
//   * every other character (spaces, punctuation, and every non-ASCII byte,
//     so "cafe" with an accent becomes "caf") separates words
//   * words longer than MAX_TOKEN_LENGTH are dropped (garbage such as long
//     encoded strings should not pollute the vocabulary)
// Examples: "C++" -> c   "O(n log n)" -> o n log n   "don't" -> don t

const size_t MAX_TOKEN_LENGTH = 50;

inline bool isWordChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

// Own lowercase: tolower on a negative char (byte >= 128) is undefined.
inline char toLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
    return c;
}

// Splits text into normalized words and APPENDS them to outWords
// (it does not clear it, so callers may call it repeatedly).
inline void tokenizeText(const string& text, vector<string>& outWords) {
    string current;
    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (isWordChar(c)) {
            if (current.size() <= MAX_TOKEN_LENGTH)      // stop growing garbage words
                current.push_back(toLowerAscii(c));
        } else {
            if (!current.empty() && current.size() <= MAX_TOKEN_LENGTH)
                outWords.push_back(current);
            current.clear();
        }
    }
    if (!current.empty() && current.size() <= MAX_TOKEN_LENGTH)
        outWords.push_back(current);
}

// Convenience wrapper that returns a fresh vector.
inline vector<string> tokenize(const string& text) {
    vector<string> words;
    tokenizeText(text, words);
    return words;
}

#endif
