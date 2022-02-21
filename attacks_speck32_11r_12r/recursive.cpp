/*
Memory management:

We allocate a vector of TrailState at the beginning of each round recursion
(pos = 0, 16, 32).
Then, we only keep vectors of pointers to that states.
These vectors are being "sieved"/filtered when descending in the recursion.
TrailStates themselves are not modified, except at the beginning of new round,
where their new copies are decrypted by 1 round.
*/
#include "common.h"
#include "speck.h"
#include "recursive.h"

using namespace std;

// Declarations

struct TrailState;
static void attack_bitwise_recursive(
    vector<TrailState*> &states,  // is ok to modify as it'll be cleared
    int round_no,
    int pos // 0=LSB .. 15=MSB, but could be incremented by 2,3,4,...
);


// Globals

static size_t g_min_trails = 1;
static const vector<Trail> *g_trails;
static const vector<
    pair<  pair<word_t, word_t>, pair<word_t, word_t>  >
> *g_testvecs;
static vector<word_t> g_cur_keys;
static int g_n_bits;
static bool g_completed;

static const int INITIAL_JUMP = 4;
static word_t g_saved_B;

static vector<TrailState> g_states_values[4];

// TrailState

struct TrailState {
    const Trail &trail;

    word_t x, xx; // x = addition output l XOR key
    word_t r, rr; // r = addition right input (derived from y)
    word_t dz;
    
    word_t dx_, dz_, dr_;

    TrailState(const Trail &t) : trail(t)
    {}

    TrailState clone() const {
        return TrailState(trail);
    }

    void init_ct() {
        const size_t dsz = trail.diffs.size();
        x = trail.ct1.first;
        xx = trail.ct2.first;

        dz = ROTR(trail.diffs[dsz-2].first, ALPHA);
        r = ROTR(trail.ct1.second ^ x, BETA);
        rr = ROTR(trail.ct2.second ^ xx, BETA);

        int round_no = 0;
        assert(dsz - 3 - round_no >= 0);
        dx_ = trail.diffs[dsz - 2 - round_no].first;
        dz_ = ROTR(trail.diffs[dsz - 3 - round_no].first, ALPHA);
        dr_ = trail.diffs[dsz - 3 - round_no].second;
    }

    void advance_to(TrailState &t, word_t key, int round_no) const {
        const size_t dsz = trail.diffs.size();

        word_t tmp;

        tmp = x ^ key;
        tmp -= r;
        t.x = ROTL(tmp, ALPHA);

        tmp = xx ^ key;
        tmp -= rr;
        t.xx = ROTL(tmp, ALPHA);

        t.r = ROTR(word_t(r ^ t.x), BETA);
        t.rr = ROTR(word_t(rr ^ t.xx), BETA);
        
        word_t prev_dx = trail.diffs[dsz-2-round_no].first;
        t.dz = ROTR(prev_dx, ALPHA);

        t.dx_ = trail.diffs[dsz - 2 - round_no].first;
        t.dz_ = ROTR(trail.diffs[dsz - 3 - round_no].first, ALPHA);
        t.dr_ = trail.diffs[dsz - 3 - round_no].second;
    }

    bool check(word_t k, word_t mask) const {
        word_t z = (x ^ k) - r;  // 1 Speck round (-1xor)
        word_t zz = (xx ^ k) - rr; // 1 Speck round (-1xor)
        return !((dz ^ z ^ zz) & mask); // xors = saved xors
    }
};


void full_attack_bitwise_recursive(
    const vector<Trail> &trails,
    const vector<
        pair<  pair<word_t, word_t>, pair<word_t, word_t>  >
    > & testvecs,
    size_t min_trails,
    int nbits
) {
    g_states_values[0].reserve(trails.size());
    g_states_values[1].reserve(trails.size());
    g_states_values[2].reserve(trails.size());
    g_states_values[3].reserve(trails.size());

    g_states_values[0].clear();
    for (auto &trail : trails) {
        g_states_values[0].push_back(TrailState(trail));
        g_states_values[0].back().init_ct();
    }
    assert(g_states_values[0].size());
    
    g_min_trails = min_trails;
    g_trails = &trails;
    g_cur_keys.clear();
    g_n_bits = nbits;
    g_testvecs = &testvecs;
    g_completed = false;
    
    // Similar to Dinur, but smaller jump to save RAM (proof-of-concept)
    assert(1 <= INITIAL_JUMP && INITIAL_JUMP <= 14);
    word_t mask = (1ull << INITIAL_JUMP) - 1;
    
    vector<TrailState*> states;
    states.reserve(g_states_values[0].size());
    for (uint32_t k0 = 0; k0 <= mask; k0++) {
        states.clear();
        for (auto &state_value : g_states_values[0]) {
            if (state_value.check(k0, mask)) {
                states.push_back(&state_value);
            }
        }
        if (!(k0 & (mask >> 4))) {
            printf("progress: k0 %u/16 trails %lu\n", k0, states.size());
        }
        if (states.size()) {
            g_cur_keys = {(word_t)k0};
            attack_bitwise_recursive(states, 0, INITIAL_JUMP);
        }
        if (g_completed) {
            break;
        }
    }
    
    g_cur_keys.clear();
}

void states_init_round(
    const vector<TrailState*> & states,
    word_t key,
    int round_no
) {
    assert(states.size());
    vector<TrailState> & ret = g_states_values[round_no];
    ret.clear();
    for (TrailState * state : states) {
        TrailState state2(state->trail);
        state->advance_to(state2, key, round_no);
        ret.push_back(state2);
    }
    return;
}

static void test_candidate(const vector<TrailState*> &states) {
    // if have 5th round trail, can check it faster 
    // than full decryption
    // check all passed trails! need at least C
    if (states[0]->trail.diffs.size() >= 6) {
        // recover 5-th round key (from the end), using KS
        // can be optimized a bit (TBD)
        word_t tmpK = g_saved_B;
        word_t k = g_cur_keys[3];
        DR(tmpK, k, NROUNDS-5);

        uint32_t cnt_fail = 0;
        for (auto p_state: states) {
            auto &trail = p_state->trail;
            TrailState st(p_state->trail);
            
            // decrypt 4th round
            // p_state->advance_to(st, g_cur_keys.back(), 4);
            // word_t z = (st.x ^ k) - st.r;
            // word_t zz = (st.xx ^ k) - st.rr;
            // if ((z ^ zz) != st.dz) {
            // return
            // }

            word_t tmp;

            // these partial decryptions (4th round)
            // could be retrieved from previous checks
            // (up to MSBits)
            tmp = (p_state->x ^ g_cur_keys.back()) - p_state->r;
            word_t x = ROTL(tmp, ALPHA);

            tmp = (p_state->xx ^ g_cur_keys.back()) - p_state->rr;
            word_t xx = ROTL(tmp, ALPHA);

            word_t r = ROTR(p_state->r ^ x, BETA);
            word_t rr = ROTR(p_state->rr ^ xx, BETA);
            
            word_t prev_dx = trail.diffs[trail.diffs.size() - 6].first;
            word_t dz = ROTR(prev_dx, ALPHA);
            
            // 5th round decryption to test differential
            word_t z = (x ^ k) - r;
            word_t zz = (xx ^ k) - rr;
            if ((z ^ zz) != dz) {
                // false positive with the right key?
                // unlikely, can be dropped
                return;
            }
        }   
    }

    auto rk = speckKS_Revert(g_cur_keys, NROUNDS);
    int good = 1;
    for (const auto &testvec: *g_testvecs) {
        auto [px, py] = testvec.first;
        auto [x, y] = testvec.first;
        for (int i = 0; i < NROUNDS; i++) {
            ER(x, y, rk[i]);
        }
        auto [cx, cy] = testvec.second;
        if (x != cx || y != cy) {
            good = 0;
            break;
        }
    }
    if (good ) {
        printf("!!! Key recovered and confirmed %d: ", good);
        word_t A = rk[0];
        
        word_t B = rk[1] ^ ROTL(rk[0], BETA);
        word_t tmpB = rk[1];
        DR(B, tmpB, 0);
        
        word_t C = rk[2] ^ ROTL(rk[1], BETA);
        word_t tmpC = rk[2];
        DR(C, tmpC, 1);

        word_t D = rk[3] ^ ROTL(rk[2], BETA);
        word_t tmpD = rk[3];
        DR(D, tmpD, 2);
        
        printf("%04x %04x %04x %04x\n", A, B, C, D);
        fflush(stdout);
        g_completed = true;
    }
    return;
}

static void attack_bitwise_recursive(
    vector<TrailState*> &states,  // is ok to reuse
    int round_no,
    int pos // 0=LSB .. 15=MSB, but could be incremented by 2,3,4,...
) {
    if (g_completed) {
        return;
    }
    assert(pos != 15); // see optimization below

    if (round_no == 4) {
        // key candidate test
        test_candidate(states);
        return;
    }
    
    if (round_no*16 + pos >= g_n_bits) {
        return;
    }

    // check # unique pairs if has chances to drop below the counter
    if (states.size() <= 30) {
        size_t n_unique_pairs = 0;
        size_t last_pair_id = -1ull;
        for(TrailState *state: states) {
            if (state->trail.pair_id != last_pair_id) {
                last_pair_id = state->trail.pair_id;
                n_unique_pairs++;
            }
        }
        if (n_unique_pairs < g_min_trails) {
            return;
        }
    }

    if (pos == 0) {
        assert(round_no != 0); // should be covered by INITIAL_JUMP

        // precompute info for 5-th round key recovery
        // see test_candidate()
        if (round_no == 2) {
            word_t B = g_cur_keys[0] ^ ROTL(g_cur_keys[1], BETA);
            word_t tmpB = g_cur_keys[0];
            DR(B, tmpB, NROUNDS-2);
            g_saved_B = B;
        }

        // decrypt 1 round of (surviving) states
        states_init_round(
            states, g_cur_keys.back(), round_no
        );
        
        g_cur_keys.push_back(0);
        
        // refresh states ptrs
        states.clear();
        for (TrailState &state_value : g_states_values[round_no]) {
            states.push_back(&state_value);
        }
    }
    
    // check 1 more bit than guessed (ARX property)
    word_t mask = (1ull << (pos + 2)) - 1;

    word_t subkey0 = g_cur_keys.back();
    word_t subkey1 = subkey0 | (1 << pos);

    // if we filter given states list for one bit value at a time
    // we'll have original "large" list and a smaller one
    // but we can split it *in place* into two smaller ones
    // note: not partition, they can intersect or even be full duplicates
    vector<TrailState*> &states0 = states;
    vector<TrailState*> states1;
    states1.reserve(states.size());

    size_t orig = states.size();
    size_t top = 0;
    for (size_t i = 0; i < states0.size(); i++) {
        if (states0[i]->check(subkey1, mask)) {
            states1.push_back(states0[i]);
        }
        if (states0[i]->check(subkey0, mask)) {
            states0[top++] = states0[i];
        }
    }
    states0.erase(states0.begin() + top, states0.end());
        
    if (states0.size() >= g_min_trails) {
        if (pos <= 13) {
            g_cur_keys.back() = subkey0;
            attack_bitwise_recursive(states0, round_no, pos + 1);
        }
        else {
            // skip MSB guess and jump to next round
            assert(pos == 14);
            
            auto copy = states0;
            g_cur_keys.back() = subkey0;
            attack_bitwise_recursive(copy, round_no + 1, 0);
            copy.clear();

            g_cur_keys.back() = subkey0 ^ 0x8000;
            attack_bitwise_recursive(states0, round_no + 1, 0);
        }
    }
    states0.clear();

    if (states1.size() >= g_min_trails) {
        if (pos <= 13) {
            g_cur_keys.back() = subkey1;
            attack_bitwise_recursive(states1, round_no, pos + 1);
        }
        else {
            // skip MSB guess and jump to next round
            assert(pos == 14);
            
            auto copy = states1;
            g_cur_keys.back() = subkey1;
            attack_bitwise_recursive(copy, round_no + 1, 0);
            copy.clear();

            g_cur_keys.back() = subkey1 ^ 0x8000;
            attack_bitwise_recursive(states1, round_no + 1, 0);
        }
    }
    states1.clear();
    
    g_cur_keys.back() = subkey0;

    if (pos == 0) {
        g_cur_keys.pop_back();
    }
}