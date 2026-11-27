#ifndef TRIE_H
#define TRIE_H

#include <vector>
#include <string>
#include "Types.h"
using namespace std;
class Trie{
    private:
        // normalizing all words to lowerCase & digits.
        // this is for the number of child slots our Node will contain
        int charToIndex(char c) {
        if ('a' <= c && c <= 'z') {return c - 'a';}
        if ('0' <= c && c <= '9') {return 26 + (c - 0);}
        return -1; //for any stray puct causing outofbounds
        
        }

        struct TrieNode {
            TrieNode* children[36] = {nullptr}; //is this valid?
            TermId termId = INVALID_TERM_ID; //node is not end of a complete word yet
        };

        TrieNode* root;
        vector<string> idToWord; //to store each unqiue word at index "termID"
    
        public: 
            Trie() : root(nullptr) {}
            ~Trie() {}
            void clear() {}

            //copy safety
            Trie(const Trie&) = delete; Trie& operator=(const Trie&) = delete; 
            
            TermId getOrCreate(const std::string& word) {}
            TermId find(const std::string& word) const {}
            const std::string& getWord(TermId id) const {}

    };

#endif