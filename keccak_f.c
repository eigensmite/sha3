
#include "keccak_f.h"

const static int OFFSET_TABLE[5][5] = {
    {  0,  1, 62, 28, 27 },
    { 36, 44,  6, 55, 20 },
    {  3, 10, 43, 25, 39 },
    { 41, 45, 15, 21,  8 },
    { 18,  2, 61, 56, 14 }
};

const static uint64_t RC[24] = { 
    0x0000000000000001, 0x0000000000008082, 0x800000000000808A, 0x8000000080008000, 0x000000000000808B, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009, 
    0x000000000000008A, 0x0000000000000088, 0x0000000080008009, 0x000000008000000A, 0x000000008000808B, 0x800000000000008B, 0x8000000000008089, 0x8000000000008003, 
    0x8000000000008002, 0x8000000000000080, 0x000000000000800A, 0x800000008000000A, 0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008
};

static inline void _theta(uint64_t A[5][5]) {
    uint64_t C[5] = {0};
    for (int x = 0; x < 5; x++) C[x] = A[0][x] ^ A[1][x] ^ A[2][x] ^ A[3][x] ^ A[4][x];
    for (int y = 0; y < 5; y++) for (int x = 0; x < 5; x++)
        A[y][x] ^= C[(x+4)%5] ^ ((C[(x+1)%5] >> 63) | (C[(x+1)%5] << 1));
}

static inline void _rho(uint64_t A[5][5]) {
    for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++) 
        A[y][x] = (A[y][x] << OFFSET_TABLE[y][x]) | (A[y][x] >> (64 - OFFSET_TABLE[y][x]));     
}

static inline void _pi(uint64_t A[5][5]) {
    uint64_t prev = A[0][1], temp;
    int prev_x = 1, prev_y = 0;
    for (int t = 0; t < 24; t++) {
        int x = prev_y, y = (2 * prev_x + 3 * prev_y) % 5;
        temp = A[y][x], A[y][x] = prev, prev = temp;
        prev_x = x, prev_y = y;
    }
}

static inline void _chi(uint64_t A[5][5]) {
    uint64_t R[5];
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) R[x] = A[y][x];
        for (int x = 0; x < 5; x++) A[y][x] = R[x] ^ ((~R[(x+1)%5]) & R[(x+2)%5]);
    }
}

static inline void _iota(uint64_t A[5][5], const int i) {
    A[0][0] ^= RC[i];
}

void keccak_f(uint64_t state[25]) {
    uint64_t (*A)[5] = (uint64_t (*)[5])state;
    for(int i = 0; i < 24; i++) {
        _theta(A);        // 𝜃
          _rho(A);       // 𝜌
           _pi(A);      // 𝜋
          _chi(A);     // 𝜒
        _iota(A, i);  // 𝜄
    }
}