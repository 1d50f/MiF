#pragma once

typedef unsigned short word_t;

const int ALPHA = 7;
const int BETA = 2;

const size_t NROUNDS = SPECK32_ATTACK;

#define ROTR(x,r) ((((x)>>(r)) | ((x)<<(16-(r))))&0xFFFF)
#define ROTL(x,r) ((((x)<<(r)) | ((x)>>(16-(r))))&0xFFFF)

#define ER(x,y,k) (x=ROTR(x,7), x+=y, x^=k, y=ROTL(y,2), y^=x)
#define DR(x,y,k) (y^=x, y=ROTR(y,2), x^=k, x-=y, x=ROTL(x,7))

vector<word_t> speckKS_Revert(const vector<word_t> &last_subkeys_rev, int nrounds);