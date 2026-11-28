#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "parser/HtmlParser.h"

using namespace std;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { failures++; \
    cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)

static string words(const ParsedDocument& d) {
    string s;
    for (size_t i = 0; i < d.tokens.size(); i++) { if (i) s += " "; s += d.tokens[i].word; }
    return s;
}
static bool wordsAre(const string& html, const string& expected) {
    string got = words(parseHtml(html));
    if (got != expected) cout << "  got [" << got << "] expected [" << expected << "]\n";
    return got == expected;
}

int main() {
    // ---- blueprint page trees.html (section 2.3): 14 tokens, 2 links ----
    const string trees =
        "<html><head><title>Binary Tree Notes</title></head>\n"
        "<body><h1>Tree Basics</h1>\n"
        "<p>A binary tree stores nodes. See <a href=\"graphs.html\">graphs</a> for more.</p>\n"
        "<a href=\"sorting.html\"></a>\n"
        "</body></html>";
    ParsedDocument d = parseHtml(trees);
    CHECK(d.title == "Binary Tree Notes");
    CHECK(words(d) == "binary tree notes tree basics a binary tree stores nodes see graphs for more");
    CHECK(d.tokens.size() == 14);
    const unsigned char expectTag[14] = {1,1,1, 2,2, 4,4,4,4,4,4,4,4,4};
    bool tagsOk = d.tokens.size() == 14;
    for (size_t i = 0; tagsOk && i < 14; i++) if (d.tokens[i].tag != expectTag[i]) tagsOk = false;
    CHECK(tagsOk);
    CHECK(d.links.size() == 2);
    CHECK(d.links[0].href == "graphs.html" && d.links[0].anchorText == "graphs");
    CHECK(d.links[1].href == "sorting.html" && d.links[1].anchorText == "");

    // ---- basics ----
    CHECK(wordsAre("", ""));
    CHECK(wordsAre("   \n\t ", ""));
    CHECK(wordsAre("<html><body></body></html>", ""));
    CHECK(wordsAre("just plain text", "just plain text"));
    CHECK(wordsAre("<TITLE>Up</TITLE><H2>Head</H2><P>Body</P>", "up head body"));
    ParsedDocument up = parseHtml("<TITLE>Up</TITLE><H2>Head</H2><P>Body</P>");
    CHECK(up.tokens.size() == 3 && up.tokens[0].tag == TAG_TITLE && up.tokens[1].tag == TAG_HEADING && up.tokens[2].tag == TAG_BODY);
    CHECK(wordsAre("<b>bi</b>nary", "bi nary"));                       // tag = word boundary
    CHECK(wordsAre("<!DOCTYPE html><?xml version=\"1.0\"?><p>hi</p>", "hi"));
    CHECK(wordsAre("<br/>one<br />two<img src=x.png alt=\"a pic\">three", "one two three"));
    CHECK(wordsAre("<p>line1\r\nline2\ttab</p>", "line1 line2 tab"));

    // ---- headings: levels, nesting, return to body ----
    ParsedDocument h = parseHtml("<h1>A</h1>b<h3>c</h3><h6>d</h6>e");
    CHECK(words(h) == "a b c d e");
    CHECK(h.tokens[0].tag == TAG_HEADING && h.tokens[1].tag == TAG_BODY && h.tokens[2].tag == TAG_HEADING
          && h.tokens[3].tag == TAG_HEADING && h.tokens[4].tag == TAG_BODY);
    CHECK(parseHtml("<h7>x</h7>").tokens[0].tag == TAG_BODY);           // not a heading
    ParsedDocument th = parseHtml("<title>t <h1>x</h1></title>");
    CHECK(th.tokens.size() == 2 && th.tokens[1].tag == TAG_TITLE);     // title beats heading

    // ---- title ----
    CHECK(parseHtml("<title>\n  My   Page &amp; More \n</title>").title == "My Page & More");
    CHECK(parseHtml("<p>no title here</p>").title == "");
    CHECK(parseHtml("<title>First</title><title>Second</title>").title == "First");
    CHECK(parseHtml("<title>unclosed title").title == "unclosed title");

    // ---- entities ----
    CHECK(wordsAre("<p>fish &amp; chips&nbsp;and &lt;b&gt; &copy; &#65;bc &#x41;bd</p>", "fish chips and b abc abd"));
    CHECK(wordsAre("a &bogus b &amp c &; d &#; e", "a bogus b amp c d e"));   // no ";" right after the name: plain text
    CHECK(wordsAre("&#233;t&#xE9;", "t"));                                // non-ASCII entities -> separator

    // ---- comments, script, style ----
    CHECK(wordsAre("<p>one<!-- hidden words --> two</p>", "one two"));
    CHECK(wordsAre("one<!-- never closed two three", "one"));
    CHECK(wordsAre("<p>a</p><script>var x = \"secret\"; if (a < b) {}</script><p>b</p>", "a b"));
    CHECK(wordsAre("<p>a</p><SCRIPT type=text/javascript>x</SCRIPT ><style>.c{color:red}</style><p>b</p>", "a b"));
    CHECK(wordsAre("<p>a</p><script>never closed words", "a"));
    CHECK(wordsAre("<script src=\"x.js\"></script>after", "after"));

    // ---- stray '<' and malformed tags ----
    CHECK(wordsAre("x < y", "x y"));
    CHECK(wordsAre("5<6 and 7>3", "5 6 and 7 3"));
    CHECK(wordsAre("<p>hello <b", "hello"));                              // unclosed tag at the end
    CHECK(wordsAre("hello <", "hello"));
    CHECK(wordsAre("<p>one</p></unknown></ >two", "one two"));

    // ---- links ----
    ParsedDocument l1 = parseHtml("<a href=\"d.html\">dq</a><a href='s.html'>sq</a><a href=u.html>uq</a><A HREF=\"U.html\">up</A>");
    CHECK(l1.links.size() == 4 && l1.links[0].href == "d.html" && l1.links[1].href == "s.html"
          && l1.links[2].href == "u.html" && l1.links[3].href == "U.html");
    CHECK(l1.links[3].anchorText == "up");
    ParsedDocument l2 = parseHtml("<a href='x.html' title=\"a > b\">text here</a> tail");
    CHECK(l2.links.size() == 1 && l2.links[0].href == "x.html" && l2.links[0].anchorText == "text here");
    CHECK(words(l2) == "text here tail");                                  // '>' in a quoted attribute
    CHECK(parseHtml("<a name=\"top\">anchor</a>").links.empty());        // no href -> no link
    CHECK(parseHtml("<a href=\"\">empty</a>").links.empty());
    CHECK(parseHtml("<a href>bare</a>").links.empty());
    ParsedDocument l3 = parseHtml("<a href=\"p.html?a=1&amp;b=2#sec\">q</a>");
    CHECK(l3.links.size() == 1 && l3.links[0].href == "p.html?a=1&b=2#sec");
    ParsedDocument l4 = parseHtml("<a href=\"u.html\">dangling text");     // unclosed <a> at EOF
    CHECK(l4.links.size() == 1 && l4.links[0].anchorText == "dangling text");
    ParsedDocument l5 = parseHtml("<a href=\"1.html\">one<a href=\"2.html\">two</a>");
    CHECK(l5.links.size() == 2 && l5.links[0].anchorText == "one" && l5.links[1].anchorText == "two");
    ParsedDocument l6 = parseHtml("<h2>Go <a href=\"a.html\">here</a></h2>");
    CHECK(words(l6) == "go here" && l6.tokens[1].tag == TAG_HEADING && l6.links[0].anchorText == "here");
    ParsedDocument l7 = parseHtml("<a href=\"x.html>text</a><p>after</p>");   // unbalanced quote
    CHECK(l7.links.size() == 1 && l7.links[0].href == "x.html" && words(l7) == "text after");
    CHECK(parseHtml("</a>stray close").links.empty());
    ParsedDocument l8 = parseHtml("<a href=\"i.html\"><img src=\"p.png\" alt=\"pic\"></a>");
    CHECK(l8.links.size() == 1 && l8.links[0].anchorText == "" && l8.tokens.empty());

    // ---- robustness ----
    string garbage;
    for (int b = 0; b < 256; b++) garbage += static_cast<char>(b);
    garbage += "<p>ok</p>";
    CHECK(words(parseHtml(garbage)).find("ok") != string::npos);     // no crash on any byte value
    string big;
    for (int k = 0; k < 100000; k++) big += "<p>word</p>";
    CHECK(parseHtml(big).tokens.size() == 100000);
    string manyLt(200000, '<');
    CHECK(parseHtml(manyLt).tokens.empty());
    string deep;
    for (int k = 0; k < 50000; k++) deep += "<div>";
    deep += "core";
    CHECK(words(parseHtml(deep)) == "core");

    // ---- readFile ----
    const char* tmp = "test_parser_tmp.html";
    { ofstream f(tmp, ios::binary); f << trees; }
    string content;
    CHECK(readFile(tmp, content) && content == trees);
    { ofstream f(tmp, ios::binary); }                           // empty file
    CHECK(readFile(tmp, content) && content.empty());
    remove(tmp);
    CHECK(!readFile("definitely_missing_file.html", content) && content.empty());

    cout << (failures == 0 ? "All parser tests passed.\n" : "Some parser checks FAILED.\n");
    return failures == 0 ? 0 : 1;
}
