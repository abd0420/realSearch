#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <vector>
#include "ds/LinkedList.h"

using namespace std;

// Hash functions. A hasher is a small class with operator()(key) -> number.
template <typename K> struct DefaultHash;

template <> struct DefaultHash<string> {
    unsigned long long operator()(const string& s) const {
        unsigned long long h = 0;                       // polynomial hash
        for (size_t i = 0; i < s.size(); i++)
            h = h * 31 + static_cast<unsigned char>(s[i]);
        return h;
    }
};

template <> struct DefaultHash<int> {
    unsigned long long operator()(int k) const {
        return static_cast<unsigned long long>(static_cast<unsigned int>(k)) * 2654435761ULL;
    }
};

// Separate-chaining hash table (blueprint 5.3).
// Each slot holds a LinkedList of entries. Doubles its slots when
// entries / slots passes 0.7.
// WARNING: pointers returned by get() are invalidated by put() (a rehash
// rebuilds the entries). Use them immediately.
template <typename K, typename V, typename Hash = DefaultHash<K> >
class HashTable {
private:
    struct Entry {
        K key;
        V value;
        Entry(const K& k, const V& v) : key(k), value(v) {}
    };

    vector< LinkedList<Entry> > buckets_;
    int size_;
    Hash hash_;

    int indexFor(const K& key, int bucketCount) const {
        return static_cast<int>(hash_(key) % static_cast<unsigned long long>(bucketCount));
    }

    void rehash(int newBucketCount) {
        vector< LinkedList<Entry> > bigger(newBucketCount);
        for (size_t b = 0; b < buckets_.size(); b++) {
            for (typename LinkedList<Entry>::node* n = buckets_[b].getHead(); n; n = n->next)
                bigger[indexFor(n->data.key, newBucketCount)].pushFront(n->data);
        }
        buckets_.swap(bigger);
    }

public:
    explicit HashTable(int initialBuckets = 16)
        : buckets_(initialBuckets > 0 ? initialBuckets : 16), size_(0) {}

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&&) = default;
    HashTable& operator=(HashTable&&) = default;

    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    int bucketCount() const { return static_cast<int>(buckets_.size()); }

    // Insert a new key, or overwrite the value of an existing key.
    void put(const K& key, const V& value) {
        int b = indexFor(key, bucketCount());
        for (typename LinkedList<Entry>::node* n = buckets_[b].getHead(); n; n = n->next) {
            if (n->data.key == key) { n->data.value = value; return; }
        }
        buckets_[b].pushFront(Entry(key, value));
        size_++;
        if (size_ > 0.7 * bucketCount()) rehash(bucketCount() * 2);
    }

    // Pointer to the value, or nullptr if the key is absent.
    V* get(const K& key) {
        int b = indexFor(key, bucketCount());
        for (typename LinkedList<Entry>::node* n = buckets_[b].getHead(); n; n = n->next)
            if (n->data.key == key) return &n->data.value;
        return nullptr;
    }

    const V* get(const K& key) const {
        int b = indexFor(key, bucketCount());
        for (typename LinkedList<Entry>::node* n = buckets_[b].getHead(); n; n = n->next)
            if (n->data.key == key) return &n->data.value;
        return nullptr;
    }

    bool contains(const K& key) const { return get(key) != nullptr; }

    // Returns true if the key existed and was removed.
    bool erase(const K& key) {
        int b = indexFor(key, bucketCount());
        bool removed = buckets_[b].removeIf([&key](const Entry& e) { return e.key == key; });
        if (removed) size_--;
        return removed;
    }

    // Remove everything (bucket count stays the same).
    void clear() {
        for (size_t b = 0; b < buckets_.size(); b++) buckets_[b].deleteList();
        size_ = 0;
    }

    // Visit every entry: f(key, value). Order is unspecified.
    // Used to flush the per-page term counts. Do not put/erase inside f.
    template <typename F>
    void forEach(F f) {
        for (size_t b = 0; b < buckets_.size(); b++)
            for (typename LinkedList<Entry>::node* n = buckets_[b].getHead(); n; n = n->next)
                f(n->data.key, n->data.value);
    }

    template <typename F>
    void forEach(F f) const {
        for (size_t b = 0; b < buckets_.size(); b++)
            for (typename LinkedList<Entry>::node* n = buckets_[b].getHead(); n; n = n->next)
                f(n->data.key, n->data.value);
    }
};

#endif
