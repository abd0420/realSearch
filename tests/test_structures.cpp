#include <iostream>
#include <string>
#include <vector>
#include "ds/LinkedList.h"
#include "ds/DoublyLinkedList.h"
#include "ds/Trie.h"
#include "ds/HashTable.h"
#include "ds/InvertedIndex.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { failures++; \
    std::cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)

// ---------------- LinkedList ----------------
void testLinkedList() {
    LinkedList<int> l;
    CHECK(l.isEmpty() && l.size() == 0 && l.getHead() == nullptr);
    CHECK(!l.popFront());
    l.pushBack(1); l.pushBack(2); l.pushFront(0);             // 0 1 2
    CHECK(l.size() == 3 && l.getHead()->data == 0 && l.getTail()->data == 2);
    CHECK(l.find(1) != nullptr && l.find(9) == nullptr);
    CHECK(l.removeIf([](const int& x) { return x == 1; }));   // middle -> 0 2
    CHECK(l.size() == 2 && !l.removeIf([](const int& x) { return x == 1; }));
    CHECK(l.removeIf([](const int& x) { return x == 2; }));   // tail -> 0
    CHECK(l.getTail()->data == 0);
    l.pushBack(5);                                            // tail pointer must be valid
    CHECK(l.size() == 2 && l.getTail()->data == 5 && l.getHead()->next->data == 5);
    CHECK(l.popFront() && l.getHead()->data == 5);            // 5
    CHECK(l.removeIf([](const int& x) { return x == 5; }));   // only node
    CHECK(l.isEmpty() && l.getHead() == nullptr && l.getTail() == nullptr);
    l.pushBack(7);
    LinkedList<int> moved(static_cast<LinkedList<int>&&>(l)); // move
    CHECK(moved.size() == 1 && moved.getHead()->data == 7 && l.isEmpty());
    moved.deleteList();
    CHECK(moved.isEmpty());
}

// ---------------- DoublyLinkedList ----------------
std::string order(const DoublyLinkedList<std::string>& l) {   // front -> back
    std::string s;
    for (DoublyLinkedList<std::string>::Node* n = l.getHead(); n; n = n->next) {
        if (!s.empty()) s += ",";
        s += n->data;
    }
    return s;
}

void testDoublyLinkedList() {
    DoublyLinkedList<std::string> l;
    CHECK(l.isEmpty() && !l.popBack());
    typedef DoublyLinkedList<std::string>::Node N;
    N* a = l.pushFront("a");
    CHECK(l.getHead() == a && l.getTail() == a);
    l.moveToFront(a);                                         // single node: no change
    CHECK(order(l) == "a" && l.size() == 1);
    N* b = l.pushFront("b");
    N* c = l.pushFront("c");                                  // c b a
    CHECK(order(l) == "c,b,a" && l.getTail() == a);
    l.moveToFront(a);                                         // tail -> front
    CHECK(order(l) == "a,c,b" && l.getTail() == b);
    l.moveToFront(c);                                         // middle -> front
    CHECK(order(l) == "c,a,b");
    l.moveToFront(c);                                         // already front
    CHECK(order(l) == "c,a,b");
    CHECK(l.popBack() && order(l) == "c,a" && l.getTail() == a);
    l.remove(c);                                              // remove head
    CHECK(order(l) == "a" && l.getHead() == a && l.getTail() == a);
    l.remove(a);                                              // remove last node
    CHECK(l.isEmpty() && l.getHead() == nullptr && l.getTail() == nullptr);
    (void)b;
}

// LRU trace from blueprint 5.9 (capacity 3) using HashTable + DoublyLinkedList.
struct Item { std::string key; int value; };
void testLruTrace() {
    typedef DoublyLinkedList<Item> List;
    List list;
    HashTable<std::string, List::Node*> map;
    const int capacity = 3;
    // access(key): hit -> move to front; miss -> evict if full, insert at front
    auto access = [&](const std::string& key) -> bool {
        List::Node** found = map.get(key);
        if (found) { list.moveToFront(*found); return true; }
        if (list.size() == capacity) {
            map.erase(list.getTail()->data.key);              // node stores its own key
            list.popBack();
        }
        Item it; it.key = key; it.value = 0;
        map.put(key, list.pushFront(it));
        return false;
    };
    auto dump = [&]() {
        std::string s;
        for (List::Node* n = list.getHead(); n; n = n->next) { if (!s.empty()) s += ","; s += n->data.key; }
        return s;
    };
    CHECK(!access("binary tree") && dump() == "binary tree");
    CHECK(!access("graph")       && dump() == "graph,binary tree");
    CHECK(!access("sorting")     && dump() == "sorting,graph,binary tree");
    CHECK(access("binary tree")  && dump() == "binary tree,sorting,graph");   // hit
    CHECK(!access("heap")        && dump() == "heap,binary tree,sorting");    // evicts graph
    CHECK(!map.contains("graph") && map.contains("heap") && map.size() == 3);
}

// ---------------- Trie ----------------
void testTrie() {
    Trie t;
    CHECK(t.size() == 0 && t.find("binary") == INVALID_TERM_ID);
    CHECK(t.getOrCreate("binary") == 0);
    CHECK(t.getOrCreate("tree") == 1);
    CHECK(t.getOrCreate("graph") == 2);
    CHECK(t.getOrCreate("tree") == 1);                        // existing word: same id
    CHECK(t.getOrCreate("trees") == 3);                       // shares t-r-e-e
    CHECK(t.size() == 4);
    CHECK(t.find("binary") == 0 && t.find("tree") == 1 && t.find("trees") == 3);
    CHECK(t.find("bin") == INVALID_TERM_ID);                  // prefix is not a word
    CHECK(t.find("tre") == INVALID_TERM_ID);
    CHECK(t.find("treess") == INVALID_TERM_ID && t.find("zzz") == INVALID_TERM_ID);
    CHECK(t.getWord(0) == "binary" && t.getWord(3) == "trees");
    CHECK(t.getWord(-1) == "" && t.getWord(99) == "");

    // digits (regression: '0'..'9' must map to slots 26..35)
    TermId d0 = t.getOrCreate("0"), d9 = t.getOrCreate("9"), mix = t.getOrCreate("a5b9");
    CHECK(d0 >= 0 && d9 >= 0 && mix >= 0 && d0 != d9);
    CHECK(t.find("0") == d0 && t.find("9") == d9 && t.find("a5b9") == mix);
    CHECK(t.getWord(mix) == "a5b9");

    // invalid input is rejected and creates nothing
    int before = t.size();
    CHECK(t.getOrCreate("") == INVALID_TERM_ID);
    CHECK(t.getOrCreate("C++") == INVALID_TERM_ID);
    CHECK(t.getOrCreate("Tree") == INVALID_TERM_ID);          // not normalized
    CHECK(t.getOrCreate("zebra!") == INVALID_TERM_ID);
    CHECK(t.getOrCreate("don't") == INVALID_TERM_ID);
    CHECK(t.find("zebra") == INVALID_TERM_ID && t.find("") == INVALID_TERM_ID);
    CHECK(t.size() == before);

    t.clear();
    CHECK(t.size() == 0 && t.find("binary") == INVALID_TERM_ID);
    CHECK(t.getOrCreate("again") == 0 && t.getWord(0) == "again");

    // many words round-trip
    Trie big;
    for (int i = 0; i < 20000; i++) big.getOrCreate("w" + std::to_string(i));
    CHECK(big.size() == 20000);
    bool ok = true;
    for (int i = 0; i < 20000; i++) {
        TermId id = big.find("w" + std::to_string(i));
        if (id != i || big.getWord(id) != "w" + std::to_string(i)) ok = false;
    }
    CHECK(ok);
}

// ---------------- HashTable ----------------
void testHashTable() {
    HashTable<std::string, DocId> paths;
    CHECK(paths.empty() && paths.get("trees.html") == nullptr);
    paths.put("trees.html", 0);
    paths.put("graphs.html", 1);
    paths.put("sorting.html", 2);
    CHECK(paths.size() == 3);
    CHECK(*paths.get("sorting.html") == 2 && paths.contains("graphs.html"));
    paths.put("graphs.html", 10);                             // overwrite, size unchanged
    CHECK(*paths.get("graphs.html") == 10 && paths.size() == 3);
    CHECK(paths.erase("graphs.html") && !paths.contains("graphs.html") && paths.size() == 2);
    CHECK(!paths.erase("graphs.html") && !paths.erase("missing"));
    paths.put("", 5);                                         // empty key is legal
    CHECK(*paths.get("") == 5);

    // everything collides in ONE slot
    HashTable<int, int> one(1);
    for (int i = 0; i < 5; i++) one.put(i, i * i);            // triggers resizes too
    for (int i = 0; i < 5; i++) CHECK(*one.get(i) == i * i);

    // growth: 10000 keys, then erase half
    HashTable<int, int> h(2);
    for (int i = 0; i < 10000; i++) h.put(i, i + 1);
    CHECK(h.size() == 10000 && h.bucketCount() >= 10000 / 0.7 - 1);
    bool ok = true;
    for (int i = 0; i < 10000; i++) { const int* v = h.get(i); if (!v || *v != i + 1) ok = false; }
    CHECK(ok);
    for (int i = 0; i < 10000; i += 2) h.erase(i);
    CHECK(h.size() == 5000 && !h.contains(0) && h.contains(1) && !h.contains(9998) && h.contains(9999));
    long long sum = 0; int visited = 0;
    h.forEach([&](const int& k, int& v) { sum += k; visited++; (void)v; });
    CHECK(visited == 5000 && sum == 25000000LL);              // sum of odd numbers < 10000
    const HashTable<int, int>& ch = h;
    int cvisited = 0;
    ch.forEach([&](const int&, const int&) { cvisited++; });
    CHECK(cvisited == 5000);
    h.clear();
    CHECK(h.empty() && !h.contains(1));
    h.put(3, 4);
    CHECK(*h.get(3) == 4);

    // per-page counting use (blueprint 5.3): TermId -> count
    HashTable<int, int> counts;
    int words[] = {1, 0, 1, 2, 1};
    for (int w : words) { int* c = counts.get(w); if (c) (*c)++; else counts.put(w, 1); }
    CHECK(*counts.get(1) == 3 && *counts.get(0) == 1 && *counts.get(2) == 1);
}

// ---------------- InvertedIndex ----------------
void testInvertedIndex() {
    InvertedIndex idx;
    CHECK(idx.getPostings(0).empty() && idx.documentFrequency(0) == 0);
    CHECK(idx.getPostings(-1).empty());
    // blueprint 5.4 running example. TermId 0 = binary, TermId 1 = tree
    idx.addOccurrence(0, 0, 0, TAG_TITLE);
    idx.addOccurrence(1, 0, 1, TAG_TITLE);
    idx.addOccurrence(1, 0, 3, TAG_HEADING);
    idx.addOccurrence(0, 0, 6, TAG_BODY);
    idx.addOccurrence(1, 0, 7, TAG_BODY);
    idx.addOccurrence(1, 1, 9, TAG_BODY);
    idx.addOccurrence(0, 2, 6, TAG_BODY);
    idx.addOccurrence(1, 2, 7, TAG_BODY);

    const std::vector<Posting>& bin = idx.getPostings(0);
    CHECK(bin.size() == 2 && bin[0].doc == 0 && bin[0].tf == 2 && bin[0].tags == 5);
    CHECK(bin[0].positions.size() == 2 && bin[0].positions[0] == 0 && bin[0].positions[1] == 6);
    CHECK(bin[1].doc == 2 && bin[1].tf == 1 && bin[1].tags == 4 && bin[1].positions[0] == 6);
    const std::vector<Posting>& tree = idx.getPostings(1);
    CHECK(tree.size() == 3 && tree[0].tf == 3 && tree[0].tags == 7);
    CHECK(tree[0].positions.size() == 3 && tree[0].positions[2] == 7);
    CHECK((tree[0].tags & TAG_TITLE) && (tree[0].tags & TAG_HEADING) && !(tree[1].tags & TAG_TITLE));
    CHECK(tree[1].doc == 1 && tree[2].doc == 2);
    CHECK(idx.documentFrequency(0) == 2 && idx.documentFrequency(1) == 3);

    // AND / OR
    std::vector<DocId> a = docIdsOf(bin), b = docIdsOf(tree);
    std::vector<DocId> andIds = intersectDocIds(a, b), orIds = unionDocIds(a, b);
    CHECK(andIds.size() == 2 && andIds[0] == 0 && andIds[1] == 2);
    CHECK(orIds.size() == 3 && orIds[0] == 0 && orIds[1] == 1 && orIds[2] == 2);
    std::vector<DocId> none;
    CHECK(intersectDocIds(a, none).empty() && unionDocIds(none, a) == a && unionDocIds(none, none).empty());

    // proximity / phrase (doc 0: binary [0,6], tree [1,3,7])
    CHECK(minPositionGap(bin[0].positions.empty() ? std::vector<int>() : std::vector<int>{0, 6},
                         std::vector<int>{1, 3, 7}) == 1);
    CHECK(minPositionGap(std::vector<int>{10}, std::vector<int>{2}) == 8);
    CHECK(minPositionGap(std::vector<int>(), std::vector<int>{1}) == -1);
    CHECK(hasPhraseMatch(std::vector<int>{0, 6}, std::vector<int>{1, 3, 7}));
    CHECK(!hasPhraseMatch(std::vector<int>{1, 3, 7}, std::vector<int>{0, 6}));  // wrong order
    CHECK(!hasPhraseMatch(std::vector<int>{2}, std::vector<int>{5}));

    // sparse TermId (gap) and invalid input. Done LAST: addOccurrence may
    // reallocate, which invalidates the references 'bin' and 'tree' above.
    idx.addOccurrence(5, 3, 0, TAG_BODY);
    CHECK(idx.termCount() == 6 && idx.documentFrequency(5) == 1 && idx.documentFrequency(3) == 0);
    int dfBefore = idx.documentFrequency(1);
    idx.addOccurrence(-1, 0, 0, TAG_BODY);
    idx.addOccurrence(1, -1, 0, TAG_BODY);
    CHECK(idx.documentFrequency(1) == dfBefore);
}

int main() {
    testLinkedList(); testDoublyLinkedList(); testLruTrace();
    testTrie(); testHashTable(); testInvertedIndex();
    if (failures == 0) std::cout << "All Milestone 2 tests passed.\n";
    else std::cout << failures << " check(s) FAILED.\n";
    return failures == 0 ? 0 : 1;
}
