#pragma once

#include <algorithm>
#include <random>
#include <chrono>

#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <ctime>

using namespace std;

#include "speck.h"


static inline word_t eq(word_t a, word_t b, word_t c){
    return (~a ^ b) & (~a ^ c) ;
}

static inline word_t g(word_t a, word_t b, word_t c){
    return eq(a<<1, b<<1, c<<1) & (a^b^c^(b<<1));
}

static inline int wt(word_t val){
    return __builtin_popcountl(val);
}

struct Trail {
	vector<pair<word_t, word_t>> diffs;
	pair<word_t, word_t> ct1, ct2;
	size_t pair_id;
};