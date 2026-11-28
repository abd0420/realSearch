#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include "core/Types.h"
#include "core/Tokenizer.h"

using namespace std;

// HTML -> words with positions and tags, plus links with anchor text.
//
// parseHtml() is a single left-to-right scan (a small state machine):
//   * text between tags is collected, entity-decoded, then split into words
//     with tokenizeText() (the same rules the queries use)
//   * every tag boundary separates words ("<b>bi</b>nary" -> bi, nary)
//   * words are tagged TAG_TITLE inside <title>, TAG_HEADING inside <h1>-<h6>,
//     otherwise TAG_BODY (title beats heading beats body)
//   * <a href="..."> collects a link: the raw href and the words inside the
//     anchor (those words are ALSO normal tokens of this page)
//   * <script>/<style> contents and <!-- comments --> are skipped
//
// A token's position is its index in ParsedDocument::tokens (0-based, whole page).
// The parser never resolves links and never reads a title from the file name:
// that is the Indexer's job.

struct ParsedToken {
    string word;
    uint8_t tag;
};

struct ParsedLink {
    string href;         // raw value of href (entities decoded, spaces collapsed)
    string anchorText;   // normalized words joined by single spaces, may be empty
};

struct ParsedDocument {
    string title;                  // trimmed text of the first <title>, "" if none
    vector<ParsedToken> tokens;
    vector<ParsedLink> links;
};

const size_t MAX_HTML_BYTES = 10 * 1024 * 1024;   // refuse files larger than 10 MB

// Reads a whole file into out. False if it cannot be opened/read or is too big.
inline bool readFile(const string& path, string& out) {
    out.clear();
    ifstream f(path.c_str(), ios::binary);
    if (!f) return false;
    f.seekg(0, ios::end);
    streamoff size = f.tellg();
    if (size < 0 || static_cast<size_t>(size) > MAX_HTML_BYTES) return false;
    f.seekg(0, ios::beg);
    if (size > 0) {
        out.resize(static_cast<size_t>(size));
        f.read(&out[0], size);
        if (!f) { out.clear(); return false; }
    }
    return true;
}

namespace htmlDetail {

inline bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }
inline bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
inline bool isAlnum(char c) { return isAlpha(c) || (c >= '0' && c <= '9'); }

inline string lowerCopy(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.size(); i++) r[i] = toLowerAscii(r[i]);
    return r;
}

// Whitespace runs become one space; leading/trailing whitespace removed.
inline string collapseSpaces(const string& s) {
    string r;
    bool pendingSpace = false;
    for (size_t i = 0; i < s.size(); i++) {
        if (isSpace(s[i])) { pendingSpace = !r.empty(); }
        else {
            if (pendingSpace) { r.push_back(' '); pendingSpace = false; }
            r.push_back(s[i]);
        }
    }
    return r;
}

// Character for an entity name (without & and ;). Unknown entities become a space,
// so "&copy;" does not turn into the word "copy". Non-ASCII numeric entities too.
inline char entityChar(const string& name) {
    if (name == "amp")  return '&';
    if (name == "lt")   return '<';
    if (name == "gt")   return '>';
    if (name == "quot") return '"';
    if (name == "apos") return '\'';
    if (name == "nbsp") return ' ';
    if (name.size() >= 2 && name[0] == '#') {
        bool hex = (name[1] == 'x' || name[1] == 'X');
        size_t k = hex ? 2 : 1;
        if (k >= name.size()) return ' ';
        unsigned long value = 0;
        for (; k < name.size(); k++) {
            char c = name[k];
            int digit;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (hex && c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
            else if (hex && c >= 'A' && c <= 'F') digit = 10 + (c - 'A');
            else return ' ';
            value = value * (hex ? 16 : 10) + static_cast<unsigned long>(digit);
            if (value > 0x10FFFF) return ' ';
        }
        if (value > 0 && value < 128) return static_cast<char>(value);
    }
    return ' ';
}

// Replaces &name; and &#N; / &#xH; with a character. A '&' that does not start
// a well-formed entity is kept as is.
inline string decodeEntities(const string& s) {
    string out;
    out.reserve(s.size());
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] != '&') { out.push_back(s[i++]); continue; }
        size_t j = i + 1;
        while (j < s.size() && j - i <= 10 && (isAlnum(s[j]) || s[j] == '#')) j++;
        if (j < s.size() && s[j] == ';' && j > i + 1) {
            out.push_back(entityChar(s.substr(i + 1, j - i - 1)));
            i = j + 1;
        } else {
            out.push_back(s[i++]);
        }
    }
    return out;
}

// Case-insensitive search; needleLower must already be lowercase.
inline size_t findIgnoreCase(const string& hay, const string& needleLower,
                                  size_t from) {
    if (needleLower.empty() || hay.size() < needleLower.size()) return string::npos;
    for (size_t p = from; p + needleLower.size() <= hay.size(); p++) {
        size_t k = 0;
        while (k < needleLower.size() && toLowerAscii(hay[p + k]) == needleLower[k]) k++;
        if (k == needleLower.size()) return p;
    }
    return string::npos;
}

// Looks for attribute `wanted` (lowercase) in the part of a tag starting at k
// (just after the tag name). Accepts "...", '...' and unquoted values.
inline bool findAttribute(const string& tag, size_t k, const string& wanted,
                          string& value) {
    const size_t n = tag.size();
    while (k < n) {
        while (k < n && (isSpace(tag[k]) || tag[k] == '/')) k++;
        if (k >= n) break;
        size_t nameStart = k;
        while (k < n && !isSpace(tag[k]) && tag[k] != '=' && tag[k] != '/') k++;
        if (k == nameStart) { k++; continue; }          // stray '=' : skip it
        string name = lowerCopy(tag.substr(nameStart, k - nameStart));
        while (k < n && isSpace(tag[k])) k++;
        string val;
        bool hasValue = false;
        if (k < n && tag[k] == '=') {
            hasValue = true;
            k++;
            while (k < n && isSpace(tag[k])) k++;
            if (k < n && (tag[k] == '"' || tag[k] == '\'')) {
                char quote = tag[k++];
                size_t vs = k;
                while (k < n && tag[k] != quote) k++;
                val = tag.substr(vs, k - vs);
                if (k < n) k++;                          // skip closing quote
            } else {
                size_t vs = k;
                while (k < n && !isSpace(tag[k])) k++;
                val = tag.substr(vs, k - vs);
            }
        }
        if (hasValue && name == wanted) { value = val; return true; }
    }
    return false;
}

struct Scanner {
    const string& html;
    ParsedDocument doc;
    string textBuf;                 // raw text since the last tag
    size_t i;                       // scan position
    bool inTitle, titleDone;
    string titleRaw;
    int headingDepth;
    bool inAnchor;
    string href, anchorText;

    explicit Scanner(const string& h)
        : html(h), i(0), inTitle(false), titleDone(false), headingDepth(0), inAnchor(false) {}

    uint8_t currentTag() const {
        if (inTitle) return TAG_TITLE;
        if (headingDepth > 0) return TAG_HEADING;
        return TAG_BODY;
    }

    // Turn the collected text into tokens (called at every tag boundary).
    void flushText() {
        if (textBuf.empty()) return;
        string text = decodeEntities(textBuf);
        textBuf.clear();
        if (inTitle && !titleDone) titleRaw += text;
        vector<string> words;
        tokenizeText(text, words);
        uint8_t tag = currentTag();
        for (size_t w = 0; w < words.size(); w++) {
            ParsedToken t;
            t.word = words[w];
            t.tag = tag;
            doc.tokens.push_back(t);
            if (inAnchor) {
                if (!anchorText.empty()) anchorText.push_back(' ');
                anchorText += words[w];
            }
        }
    }

    void closeAnchor() {
        if (!inAnchor) return;
        ParsedLink link;
        link.href = href;
        link.anchorText = anchorText;
        doc.links.push_back(link);
        inAnchor = false;
        href.clear();
        anchorText.clear();
    }

    // Index of the '>' that ends the tag starting after '<' at `from`.
    // '>' inside quotes does not count; if quotes are unbalanced, ignore them.
    size_t findTagEnd(size_t from) const {
        char quote = 0;
        for (size_t j = from; j < html.size(); j++) {
            char ch = html[j];
            if (quote) { if (ch == quote) quote = 0; }
            else if (ch == '"' || ch == '\'') quote = ch;
            else if (ch == '>') return j;
        }
        return html.find('>', from);
    }

    // t = text between '<' and '>'. May move `i` (to skip <script>/<style> bodies).
    void handleTag(const string& t) {
        if (t.empty() || t[0] == '!' || t[0] == '?') return;        // doctype, <?xml ?>
        size_t k = 0;
        bool closing = false;
        if (t[0] == '/') { closing = true; k = 1; }
        while (k < t.size() && isSpace(t[k])) k++;
        size_t nameStart = k;
        while (k < t.size() && isAlnum(t[k])) k++;
        if (k == nameStart) return;
        string name = lowerCopy(t.substr(nameStart, k - nameStart));

        size_t last = t.size();
        while (last > 0 && isSpace(t[last - 1])) last--;
        bool selfClosing = (last > 0 && t[last - 1] == '/');

        if (name == "title") {
            if (closing) {
                if (inTitle) {
                    inTitle = false;
                    if (!titleDone) { doc.title = collapseSpaces(titleRaw); titleDone = true; }
                }
            } else {
                inTitle = true;
            }
        } else if (name.size() == 2 && name[0] == 'h' && name[1] >= '1' && name[1] <= '6') {
            if (closing) { if (headingDepth > 0) headingDepth--; }
            else headingDepth++;
        } else if (name == "a") {
            closeAnchor();                     // an unclosed earlier <a> ends here
            if (!closing) {
                string value;
                if (findAttribute(t, k, "href", value)) {
                    string cleaned = collapseSpaces(decodeEntities(value));
                    if (!cleaned.empty()) { inAnchor = true; href = cleaned; anchorText.clear(); }
                }
            }
        } else if ((name == "script" || name == "style") && !closing && !selfClosing) {
            size_t p = findIgnoreCase(html, "</" + name, i);
            if (p == string::npos) i = html.size();
            else {
                size_t gt = html.find('>', p);
                i = (gt == string::npos) ? html.size() : gt + 1;
            }
        }
    }

    void run() {
        const size_t n = html.size();
        while (i < n) {
            char c = html[i];
            if (c != '<') { textBuf.push_back(c); i++; continue; }

            if (html.compare(i, 4, "<!--") == 0) {                    // comment
                flushText();
                size_t e = html.find("-->", i + 4);
                i = (e == string::npos) ? n : e + 3;
                continue;
            }
            // A '<' that does not start a tag ("a < b", "5<6") is ordinary text.
            if (i + 1 >= n) { textBuf.push_back(c); i++; continue; }
            char nx = html[i + 1];
            if (!(isAlpha(nx) || nx == '/' || nx == '!' || nx == '?')) { textBuf.push_back(c); i++; continue; }

            flushText();
            size_t end = findTagEnd(i + 1);
            if (end == string::npos) { i = n; break; }           // unclosed tag: drop the rest
            string tagText = html.substr(i + 1, end - (i + 1));
            i = end + 1;
            handleTag(tagText);
        }
        flushText();
        if (inTitle && !titleDone) doc.title = collapseSpaces(titleRaw);
        closeAnchor();                                                 // unclosed <a> at EOF
    }
};

}  // namespace htmlDetail

// Parse an HTML string. Never fails: malformed input gives the best partial result.
inline ParsedDocument parseHtml(const string& html) {
    htmlDetail::Scanner scanner(html);
    scanner.run();
    return scanner.doc;
}

#endif