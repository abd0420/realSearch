#ifndef SORTING_H
#define SORTING_H

#include <vector>
#include "core/Types.h"

// Merge sort for ranked results, used by Ranker::getTopK().
// Order: higher score first; on equal scores the smaller DocId first, so the
// output is identical on every run. (Scores are assumed to be real numbers,
// never NaN.)

inline bool resultComesBefore(const Result& a, const Result& b) {
    if (a.score != b.score) return a.score > b.score;
    return a.doc < b.doc;
}

// Merge the two sorted halves [lo, mid) and [mid, hi) of v, using tmp as scratch.
inline void mergeRanges(std::vector<Result>& v, std::vector<Result>& tmp,
                        int lo, int mid, int hi) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (resultComesBefore(v[j], v[i])) tmp[k++] = v[j++];
        else                               tmp[k++] = v[i++];
    }
    while (i < mid) tmp[k++] = v[i++];
    while (j < hi)  tmp[k++] = v[j++];
    for (int t = lo; t < hi; t++) v[t] = tmp[t];
}

// Sort the half-open range [lo, hi) of v.
inline void mergeSortRange(std::vector<Result>& v, std::vector<Result>& tmp,
                           int lo, int hi) {
    if (hi - lo < 2) return;                 // 0 or 1 element: already sorted
    int mid = lo + (hi - lo) / 2;
    mergeSortRange(v, tmp, lo, mid);
    mergeSortRange(v, tmp, mid, hi);
    mergeRanges(v, tmp, lo, mid, hi);
}

// Sort the whole vector, best result first. O(M log M) time, O(M) extra space.
inline void mergeSortResults(std::vector<Result>& v) {
    if (v.size() < 2) return;
    std::vector<Result> scratch(v.size());   // one buffer, allocated once
    mergeSortRange(v, scratch, 0, static_cast<int>(v.size()));
}

// The K best results, best first. Takes the candidates by value (a copy it may
// reorder). k <= 0 gives an empty vector; k > size gives everything.
// To switch to a MaxHeap later, change only the body of this function.
inline std::vector<Result> getTopK(std::vector<Result> candidates, int k) {
    if (k <= 0 || candidates.empty()) return std::vector<Result>();
    mergeSortResults(candidates);
    if (static_cast<size_t>(k) < candidates.size()) candidates.resize(k);
    return candidates;
}

#endif