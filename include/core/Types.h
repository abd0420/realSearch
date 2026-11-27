#pragma once
#include <string>
#include <vector>
#include <cstdint>

// our core identifiers 
using DocId = int;
using TermId = int;

constexpr DocId INVALID_DOC_ID = -1;
constexpr TermId INVALID_TERM_ID = -1;

//  HTML Section, bitmask FLAGS 
//this stores each flag for type of text in a single 8bit int, rather than storing seperate boolean flag variables for each
enum TagFlag : uint8_t { //this is a 8bit unsigned integer, like in COAL

    TAG_NONE    = 0,
    TAG_TITLE   = 1 << 0, // 1 ((1st bit from right)): <title>
    TAG_HEADING = 1 << 1, // 2(2nd bit from right): <h1> - <h6>
    TAG_BODY    = 1 << 2, // 4(3rd bit from right): <p>, <div>, <li>, etc.
    TAG_ANCHOR  = 1 << 3  // 8 (4th bit from right): <a href="..."> anchor text (your addition!)

    // we shift the number "1" by n times to the left, where n is the target bit position 

};

// --- Document Metadata (Stored in Document Table indexed by DocId) ---
struct DocInfo {
    DocId id = INVALID_DOC_ID;
    std::string path;
    std::string title;
    int wordCount = 0;      // for BM25 length normalization
    int inDegree = 0;       // Incoming link count 
    int outDegree = 0;      // Outgoing link count
};

// Inverted Index Entry, this is for a specific WORD inside the file we have
struct Posting {
    DocId doc = INVALID_DOC_ID;
    int tf = 0;                         // Term frequency in this doc
    uint8_t tags = TAG_NONE;            // Bitwise OR of TagFlags
    std::vector<int> positions;         // Word offsets for phrase & proximity checks
};

// Query Parser Tokens (For Shunting-Yard Stack)
enum class QueryTokenType {
    TERM,
    PHRASE,     // "exact phrase"
    AND,
    OR,
    NOT,
    LPAREN,     // (
    RPAREN      // )
};

struct QueryToken {
    QueryTokenType type;
    std::string value;                  // Normalized word or raw phrase
};

// --- Ranking Weights & :explain Breakdown ---
struct Weights {
    double lexical   = 0.40;            // TF-IDF or BM25 weight
    double tag       = 0.25;            // Title / Heading boost weight
    double anchor    = 0.15;            // Inbound Anchor Text boost weight
    double proximity = 0.10;            // Multi-word closeness bonus weight
    double authority = 0.10;            // Link Graph In-Degree / PageRank weight
};

// for the :explain <doc_id> breakdown thats shown to the user
struct ScoreBreakdown {
    DocId doc = INVALID_DOC_ID;
    double lexical   = 0.0;
    double tag       = 0.0;
    double anchor    = 0.0;
    double proximity = 0.0;
    double authority = 0.0;
    double total     = 0.0;
};

//for the top K result
struct Result {
    DocId doc = INVALID_DOC_ID;
    double score = 0.0;
};