/*
 * Use default SPECK implementation from 
 * https://nsacyber.github.io/simon-speck/implementations/ImplementationGuide1.1.pdf
 * to get a sense of processing time on a machine
 *
 * gcc speckTime32.c -lm -o speck -O3 -DNROUNDS=11 && ./speck
 * gcc speckTime32.c -lm -o speck -O3 -DNROUNDS=12 && ./speck
 */

/****************************************************************************/
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//32 aprox. 30s
//32 aprox. 16min
#define NENC (1ull<<32)

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define ROTL16(x,r) (((x)<<(r)) | (x>>(16-(r))))
#define ROTR16(x,r) (((x)>>(r)) | ((x)<<(16-(r))))
#define ROTL32(x,r) (((x)<<(r)) | (x>>(32-(r))))
#define ROTR32(x,r) (((x)>>(r)) | ((x)<<(32-(r))))
#define ROTL64(x,r) (((x)<<(r)) | (x>>(64-(r))))
#define ROTR64(x,r) (((x)>>(r)) | ((x)<<(64-(r)))

#define ER16(x,y,k) (x=ROTR16(x,7), x+=y, x^=k, y=ROTL16(y,2), y^=x)
#define DR16(x,y,k) (y^=x, y=ROTR16(y,2), x^=k, x-=y, x=ROTL16(x,7))
/****************************************************************************/

int main(int argc, char **argv) {
    printf("Rounds: %u\n", NROUNDS);

    u16 k[4] = {0, 1, 2, 3};
    u16 *rk = malloc((NROUNDS+4) * sizeof(u16));

    struct timespec start, end;

__asm__ __volatile__("":::"memory");
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
__asm__ __volatile__("":::"memory");

    u16 j;
    u16 L = 0, R = 0;
    for(u64 i=0; i<NENC; i++){
        k[0]++;  // change key
        u16 D = k[3], C = k[2], B = k[1], A = k[0];

        for (j = 0; j < NROUNDS;) {
            ER16(R, L, A);
            if (j + 1 >= NROUNDS) break;
            ER16(B, A, j++);

            ER16(R, L, A);
            if (j + 1 >= NROUNDS) break;
            ER16(C, A, j++);

            ER16(R, L, A);
            if (j + 1 >= NROUNDS) break;
            ER16(D, A, j++);
        }
    }
__asm__ __volatile__("":::"memory");
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
__asm__ __volatile__("":::"memory");

    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

    printf("output %04x %04x\n", L, R);
    printf("elapsed time = %ld\n", delta_us);
    printf("Speck32-64: 2^%.3f s^-1\n", log2 ( (float)(NENC)/(delta_us)*1000000.0) );
    free(rk);
 
    return 0;
}
