#include "common.h"

static inline word_t speckKS_RecoverOne(word_t A1, word_t A2) {
    word_t B2 = ROTL(A1, BETA) ^ A2;
    return B2;
}

vector<word_t> speckKS_Revert(const vector<word_t> &last_subkeys_rev, int nrounds) {
    assert(last_subkeys_rev.size() == 4); // currently supported
    vector<word_t> subkeys = last_subkeys_rev;

    nrounds--;
    
    word_t A1, A2, B, C, D;
    A2 = subkeys[0];
    
    A1 = subkeys[1];
    D = speckKS_RecoverOne(A1, A2);
    DR(D, A2, --nrounds);
    assert(A2 == A1);
    
    A1 = subkeys[2];
    C = speckKS_RecoverOne(A1, A2);
    DR(C, A2, --nrounds);
    assert(A2 == A1);

    A1 = subkeys[3];
    B = speckKS_RecoverOne(A1, A2);
    DR(B, A2, --nrounds);
    assert(A2 == A1);

    word_t A = A2;
    while (nrounds >= 1) {
        DR(D, A, --nrounds);
        subkeys.push_back(A);

        if (!nrounds) break;

        DR(C, A, --nrounds);
        subkeys.push_back(A);

        if (!nrounds) break;

        DR(B, A, --nrounds);
        subkeys.push_back(A);
    }
    reverse(subkeys.begin(), subkeys.end());
    return subkeys;
}