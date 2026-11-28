#include <iostream>
#include <string>
#include <vector>
#include "core/Tokenizer.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { failures++; \
    std::cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)

// compare against a space-separated expected string ("" = no words)
static bool is(const std::string& input, const std::string& expected) {
    std::vector<std::string> got = tokenize(input);
    std::string joined;
    for (size_t i = 0; i < got.size(); i++) { if (i) joined += " "; joined += got[i]; }
    if (joined != expected) std::cout << "  input [" << input << "] gave [" << joined << "], expected [" << expected << "]\n";
    return joined == expected;
}

int main() {
    CHECK(is("Binary Tree", "binary tree"));
    CHECK(is("C++", "c"));
    CHECK(is("O(n log n)", "o n log n"));
    CHECK(is("std::vector", "std vector"));
    CHECK(is("node.js", "node js"));
    CHECK(is("don't", "don t"));
    CHECK(is("binary-search 3.14", "binary search 3 14"));
    CHECK(is("  ", ""));
    CHECK(is("", ""));
    CHECK(is("!!! ... ???", ""));
    CHECK(is("caf\xC3\xA9 au lait", "caf au lait"));      // UTF-8 e-acute is two non-ASCII bytes
    CHECK(is("\xFF\xFE" "abc", "abc"));                    // bytes >= 128 must not crash
    CHECK(is("ABC123xyz", "abc123xyz"));
    CHECK(is("one\ttwo\nthree\r\nfour", "one two three four"));
    CHECK(is("  leading and trailing  ", "leading and trailing"));

    // appends instead of replacing
    std::vector<std::string> w;
    tokenizeText("alpha beta", w);
    tokenizeText("Gamma", w);
    CHECK(w.size() == 3 && w[0] == "alpha" && w[2] == "gamma");

    // length limit: 50 kept, 51 dropped, neighbours unaffected
    std::string ok(MAX_TOKEN_LENGTH, 'a'), tooLong(MAX_TOKEN_LENGTH + 1, 'b');
    CHECK(tokenize(ok).size() == 1 && tokenize(ok)[0] == ok);
    CHECK(is(tooLong, ""));
    CHECK(is("x " + tooLong + " y", "x y"));
    CHECK(tokenize(std::string(1000000, 'z')).empty());    // huge single word

    CHECK(isWordChar('a') && isWordChar('Z') && isWordChar('7') && !isWordChar('_') && !isWordChar('-'));
    CHECK(toLowerAscii('Q') == 'q' && toLowerAscii('q') == 'q' && toLowerAscii('5') == '5');

    std::cout << (failures == 0 ? "All tokenizer tests passed.\n" : "Some tokenizer checks FAILED.\n");
    return failures == 0 ? 0 : 1;
}