
/******************************************************************************
 * keccak_f.c
 *
 * Keccak-f[1600] permutation implementation for SHA-3 (Keccak) sponge construction.
 *
 * Author: eigensmite
 * Date: 2026-03-14
 *
 * Provides:
 *   - keccak_f(): applies the Keccak-f[1600] permutation on a 1600-bit state
 *   - Internal helper functions for the Theta, Rho, Pi, Chi, and Iota steps
 *   - Rotation offsets, Pi mapping, and round constants
 *
 * Reference:
 *   - FIPS 202: SHA-3 Standard (Keccak)
 *****************************************************************************/

#include "keccak_f.h"

/* Rotation offsets for the Rho step (ρ) */
const static int RHO_Z[5][5] = {
    {  0,  1, 62, 28, 27 },
    { 36, 44,  6, 55, 20 },
    {  3, 10, 43, 25, 39 },
    { 41, 45, 15, 21,  8 },
    { 18,  2, 61, 56, 14 }
};

/* Pi step Y-coordinate mapping (π) for lane permutation */
const static int PI_Y[25] = { 2, 1, 2, 3, 3, 0, 1, 3, 1, 4, 4, 0, 3, 4, 3, 2, 2, 0, 4, 2, 4, 1, 1, 0 };

/* Round constants for the Iota step (ι) */
const static uint64_t IOTA_RC[24] = { 
    0x0000000000000001, 0x0000000000008082, 0x800000000000808A, 0x8000000080008000, 
    0x000000000000808B, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009, 
    0x000000000000008A, 0x0000000000000088, 0x0000000080008009, 0x000000008000000A, 
    0x000000008000808B, 0x800000000000008B, 0x8000000000008089, 0x8000000000008003, 
    0x8000000000008002, 0x8000000000000080, 0x000000000000800A, 0x800000008000000A, 
    0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

/*
 * Theta step (θ)
 * Mixes columns by XORing each bit with a parity of two neighboring columns.
 */
static inline void _theta(uint64_t A[5][5]) {
    uint64_t C[5];
    for (int x = 0; x < 5; x++)
        C[x] = A[0][x] ^ A[1][x] ^ A[2][x] ^ A[3][x] ^ A[4][x];
    for (int y = 0; y < 5; y++) 
        for (int x = 0; x < 5; x++)
            A[y][x] ^= C[(x+4)%5] ^ ((C[(x+1)%5] >> 63) | (C[(x+1)%5] << 1));
}

/*
 * Rho and Pi steps (ρ, π)
 * Rotate each lane and permute positions according to the specification.
 */
static inline void _rho_pi(uint64_t A[5][5]) {
    uint64_t prev = (A[0][1] << RHO_Z[0][1]) | (A[0][1] >> (64 - RHO_Z[0][1]));
    uint64_t temp;
    int prev_y = 0;
    for (int t = 0; t < 24; t++) {
        int x = prev_y, y = PI_Y[t];
        temp = (A[y][x] << RHO_Z[y][x]) | (A[y][x] >> (64 - RHO_Z[y][x]));
        A[y][x] = prev;
        prev = temp;
        prev_y = y;
    }
}

/*
 * Chi step (χ)
 * Non-linear step: each lane is XORed with the negation of the next lane AND the following lane.
 */
static inline void _chi(uint64_t A[5][5]) {
    uint64_t R[5];
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) R[x] = A[y][x];
        for (int x = 0; x < 5; x++) 
            A[y][x] = R[x] ^ ((~R[(x+1)%5]) & R[(x+2)%5]);
    }
}

/*
 * Iota step (ι)
 * XORs a round-dependent constant into the first lane.
 */
static inline void _iota(uint64_t A[5][5], const int i) {
    A[0][0] ^= IOTA_RC[i];
}

/*
 * Keccak-f[1600] permutation
 *
 * state - 25-element array representing the 1600-bit sponge state
 *
 * Applies 24 rounds of θ, ρ, π, χ, and ι steps in sequence.
 */
void keccak_f(uint64_t state[25]) {
    uint64_t (*A)[5] = (uint64_t (*)[5])state;  /* Treat 1D state as 5x5 lanes */
    for (int i = 0; i < 24; i++) {
              _theta(A);  // 𝜃 
            _rho_pi(A);  // 𝜌𝜋
          _chi(A);      // 𝜒
        _iota(A, i);   // 𝜄
    }
}





// void print_lanes(uint64_t A[5][5]) {
//     printf("Xor'd state (as lanes of integers)\n");
//     for (int y = 0; y < 5; y++) {
//         for(int x = 0; x < 5; x++) {
//             printf("\t[%d, %d] = %016lX\n", y, x, A[y][x]);
//         }
//     }
// }

// static void print_state(uint64_t A[5][5]) {
//     printf("\t");
//     for (int y = 0; y < 5; y++) {
//         for (int x = 0; x < 5; x++) {
//             for (int z = 0; z < 64; z+=8) {
//                 printf("%02lX ", (A[y][x] >> z) & 0xff);
//             }
//             if ((y+x) % 2 == 1) printf("\n\t");
//         }
//     }
//     printf("\n\n");
// }
