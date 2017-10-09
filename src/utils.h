#ifndef PR_UTILS_H
#define PR_UTILS_H


#include "common.h"

#define __unused __attribute__((unused))

template <typename T>
inline bool contains(set<T> collection, T item) {
    return collection.find(item) != collection.end();
}

inline int randint(int low, int high) {
    assert(low <= high);

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> dist(low, high);

    return dist(rng);
}

inline vector<int> create_payload(int _0 = 0, int _1 = 0, int _2 = 0, int _3 = 0,
                                  int _4 = 0, int _5 = 0) {
    // 6 and 7 are not for public use.
    int _6 {0}, _7 {0};

    auto payload = vector<int> {
            _0, _1, _2, _3, _4, _5, _6, _7
    };

    return payload;
}

inline pair<int, int> split_64(int64_t x) {
    auto _0 = (int) (x >> 32);
    auto _1 = (int) ((x << 32) >> 32);

    return make_pair(_0, _1);
}

inline int64_t merge_64(int _0, int _1) {
    int64_t ret = 0;

    auto _0_64 = (int64_t) _0;
    auto _1_64 = (int64_t) _1;

    ret |= (_0_64 << 32);
    ret |= _1_64;

    return ret;
}

inline float rand_f() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

#endif //PR_UTILS_H
