#include <unistd.h>
#include "common.h"
#include "recursive.h"

struct ClusterTrail {
    Trail trail;

    word_t alpha1;
    word_t beta1;
    word_t dy2rotl;
    
    ClusterTrail(const vector<pair<word_t, word_t>> &diffs) {
        trail = Trail{diffs, {0, 0}, {0, 0}, 0};

        auto [dx, dy] = trail.diffs.back();
        alpha1 = ROTR(dx, ALPHA);
        beta1 = dy;
        dy2rotl = ROTL(dy, BETA);
    }
};

vector<Trail> trails;
int attackC = 1;

// precomputed cluster here:
// bottom part of the trails
// needs to be extened by 2 rounds to reach CT
#if SPECK32_ATTACK == 11
    const word_t DELTA_Px = 0x0a20;
    const word_t DELTA_Py = 0x4205;
    
    const vector<ClusterTrail> cluster = {
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x802a,0xd4a8}})
    };
#elif SPECK32_ATTACK == 12
    const word_t DELTA_Px = 0x7458;
    const word_t DELTA_Py = 0xb0f8;
    
    const vector<ClusterTrail> cluster = {
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x801a,0xd498}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x8026,0xd4a4}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x802a,0xd4a8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x802e,0xd4ac}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x803a,0xd4b8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x806a,0xd4e8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x822a,0xd6a8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x8e2a,0xdaa8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0x882a,0xdca8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0xbe2a,0xeaa8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0xb82a,0xeca8}}),
        ClusterTrail({{0x8100,0x8102},{0x8000,0x840a},{0x850a,0x9520},{0xa02a,0xf4a8}}),
    };
#endif

static inline void MiF_simple(size_t pair_id, word_t x1, word_t y1, word_t x2, word_t y2) {
    // Structure:
    // dx4 dy4 (cluster trail)
    // dx3 dy3 (cluster trail)
    // dx2 dy2 cluster trail end = filter trail start
    // dx1 dy1 filter trail (pre-last round)
    // dx0 dy0 filter trail (last round: ct diff))

    word_t dx0 = x1 ^ x2;
    word_t dy0 = y1 ^ y2;
    word_t dy1 = ROTR(dx0 ^ dy0, BETA);

    for(auto &prec: cluster) {
        // gamma for penultimate round's ADD
        word_t gamma = dy1 ^ prec.dy2rotl;
        if (g(prec.alpha1, prec.beta1, gamma)) {
            continue;
        }

        if (g(ROTR(gamma, ALPHA), dy1, dx0)) {
            continue;
        }

        Trail trail = prec.trail;
        trail.diffs.push_back({gamma, dy1});
        trail.diffs.push_back({dx0, dy0});
        
        trail.ct1 = {x1, y1};
        trail.ct2 = {x2, y2};
        
        trail.pair_id = pair_id;
        
        trails.push_back(trail);
    }
}

int main(int argc, char * argv[])
{
    if (argc != 3 && argc != 4) {
        printf("Usage: %s <n_pairs_log> <c> [seed]\n", argv[0]);
        return -1;
    }
    double n_pairs_log;
    sscanf(argv[1], "%lf", &n_pairs_log);
    assert(0 <= n_pairs_log && n_pairs_log <= 31.0);
    size_t n_pairs = pow(2.0, n_pairs_log);

    attackC = atoi(argv[2]);
    unsigned int seed = time(NULL);
    srand(seed);
    srand(rand() ^ getpid());
    seed = rand();
    if (argc == 4) {
        seed = atoi(argv[3]);
    }

    assert(attackC >= 1);
    assert(1 <= attackC && attackC <= n_pairs);

    printf("n_rounds: %lu\n", NROUNDS);
    printf("seed: %u = 0x%08x\n", seed, seed);
    printf("c: %d\n", attackC);
    printf("n_pairs: %lu = 2^%.2f\n", n_pairs, n_pairs_log);

    srand(seed);

    word_t master_key[4];
    printf("[DEBUG] Secret master key: ");
    for (int i = 0; i < 4; i++) {
        master_key[i] = rand();
        printf("%04x ", master_key[i]);
    }

    // key schedule
    word_t subkeys[NROUNDS];
    word_t A = master_key[0];
    word_t B = master_key[1];
    word_t C = master_key[2];
    word_t D = master_key[3];
    for (int i = 0; i < NROUNDS;) {
        subkeys[i] = A;
        ER(B, A, i++);
        if (i >= NROUNDS) break;
        subkeys[i] = A;
        ER(C, A, i++);
        if (i >= NROUNDS) break;
        subkeys[i] = A;
        ER(D, A, i++);
    }
    printf(" | round keys: ");
    for (int i = 0; i < NROUNDS; i++) {
        printf("%04x ", subkeys[i]);
    }
    printf("\n");
    printf("\n");

    
    // Stage 0 + Stage 1:
    // encryptions + MiF tool (collect trails
    vector<
        pair<  pair<word_t, word_t>, pair<word_t, word_t>  >
    > testvecs;

    {
    printf("Stage 0+1 (data encryption + simplified MiF)\n");
    printf("============================================\n");
    auto t0 = chrono::high_resolution_clock::now();
    uint32_t xy = 0;

    for (size_t i = 0; i < n_pairs; i++) {
        // permute plaintext space to make it random-like
        xy += 0x36a9f1d9;
        
        word_t x1 = xy >> 16;
        word_t y1 = xy;

        word_t x2 = x1 ^ DELTA_Px;
        word_t y2 = y1 ^ DELTA_Py;

        // pre-invert 1 round
        DR(x1, y1, 0);
        DR(x2, y2, 0);

        // ignore repeated pairs
        if (x1 > x2 || (x1 == x2 && y1 > y2)) {
            i--;
            continue;
        }

        // query 2 encryptions
        for(int j = 0; j < NROUNDS; j++) {
            ER(x1, y1, subkeys[j]);
            ER(x2, y2, subkeys[j]);
        }

        // run simplified mif
        MiF_simple(i, x1, y1, x2, y2);

        // save some test vectors for decryption testing
        if (i < 8) {
            word_t px1 = xy >> 16;
            word_t py1 = xy;

            // pre-invert 1 round
            DR(px1, py1, 0);

            testvecs.push_back({
                {px1, py1}, {x1, y1}
            });
        }
    }
    auto t1 = chrono::high_resolution_clock::now();
    double time_enc_mif = \
        chrono::duration_cast<chrono::microseconds>(t1 - t0).count() / (double)1e6;
    printf("Encryptions + MiF time (enc + T_mif): %.6lfs\n", time_enc_mif);
    printf("Collected %lu trails\n", trails.size());
    printf("\n");
    }

    if (1) {
        size_t n_right_trails = 0;
        for (auto & trail: trails) {
            auto [x1, y1] = trail.ct1;
            auto [x2, y2] = trail.ct2;
            assert(trail.diffs.back().first == (x1 ^ x2));
            assert(trail.diffs.back().second == (y1 ^ y2));
            
            int good = 1;
            for (int i = 0; i < trail.diffs.size() - 1; i++) {
                auto &diff = trail.diffs[trail.diffs.size()-1-i];
                if (diff.first != (x1 ^ x2)) {
                    good = 0;
                    break;
                }
                if (diff.second != (y1 ^ y2)) {
                    good = 0;
                    break;
                }
                DR(x1, y1, subkeys[NROUNDS-1-i]);
                DR(x2, y2, subkeys[NROUNDS-1-i]);
            }
            if (good) {
                printf("[DEBUG] caught good trail (pair #%lu)\n", trail.pair_id);
                n_right_trails++;
            }
        }
        printf("\n");
        if (n_right_trails > attackC) {
            printf("[DEBUG] More right pairs than expected, direct attack may take a bit longer (current version does not detect it)\n");
            printf("\n");
        }
        if (n_right_trails < attackC) {
            printf("[DEBUG] Less right pairs than expected, the attack will fail.\n");
            printf("\n");
        }
    }

    // Stage 2:
    // recursive (multi-trail attack)
    {
    printf("Stage 2 (multi-trail recursive procedure)\n");
    printf("=========================================\n");
    auto t0 = chrono::high_resolution_clock::now();
    full_attack_bitwise_recursive(
        trails,
        testvecs,
        attackC,
        64 // n_bits
    ); 
    auto t1 = chrono::high_resolution_clock::now();
    double time_cnt = \
        chrono::duration_cast<chrono::microseconds>(t1 - t0).count() / (double)1e6;
    printf("Multi-trail recurse time (T_cnt): %.6lfs\n", time_cnt);
    printf("\n");
    }
    return 0;    
}