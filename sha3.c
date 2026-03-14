
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gmp.h>
#include <math.h>
#include <string.h>
#include <assert.h>

typedef struct SHA_3 {
    size_t r;
    size_t d;
} SHA_3;

void print_lanes(uint64_t A[5][5]) {
    printf("Xor'd state (as lanes of integers)\n");
    for (int y = 0; y < 5; y++) {
        for(int x = 0; x < 5; x++) {
            printf("\t[%d, %d] = %016lX\n", x, y, A[x][y]);
        }
    }
}

void print_state(uint64_t A[5][5]) {
    printf("\t");
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            for (int z = 0; z < 64; z+=8) {
                printf("%02lX ", (A[x][y] >> z) & 0xff);
            }
            if ((y+x) % 2 == 1) printf("\n\t");
        }
    }
    printf("\n\n");
}


typedef size_t(*pad_fn)(mpz_t, const mpz_t, const size_t);
typedef void(*perm_fn)(mpz_t, const mpz_t);


const static int OFFSET_TABLE[5][5] = {
    {   0,  36,   3, 105, 210 },
    {   1, 300,  10,  45,  66 },
    { 190,   6, 171,  15, 253 },
    {  28,  55, 153,  21, 120 },
    {  91, 276, 231, 136,  78 }
};

const static uint64_t RC_LFSR[4] = {0xeabe509fe178d01,0xa452a7767bf4cd46,0xb0fb7ae886c79cc5,0x8e25c0c93720adac};

static inline void _theta(uint64_t A[5][5], const size_t w) {
    uint64_t C[5] = {0};
    for (int x = 0; x < 5; x++) C[x] = A[x][0] ^ A[x][1] ^ A[x][2] ^ A[x][3] ^ A[x][4];
    for (int y = 0; y < 5; y++) 
        for (int x = 0; x < 5; x++)
            A[x][y] ^= C[(x+4)%5] ^ ((C[(x+1)%5] >> ((w-1)%w)) | C[(x+1)%5] << (1%w));
}

static inline void _rho(uint64_t A[5][5], const size_t w) {
    int offset;
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            if((offset = OFFSET_TABLE[x][y] % w) != 0) {
                A[x][y] = (A[x][y] << offset) | (A[x][y] >> (w - offset));
                if(w<64) A[x][y] &= (1ULL << w) - 1;
            }
        }
    }
}

static inline void _pi(uint64_t A[5][5]) {
    uint64_t prev = A[1][0], temp;
    int prev_x = 1, prev_y = 0;
    for (int t = 0; t < 24; t++) {
        int x = prev_y, y = (2 * prev_x + 3 * prev_y) % 5;
        temp = A[x][y], A[x][y] = prev, prev = temp;
        prev_x = x, prev_y = y;
    }
}

static inline void _chi(uint64_t A[5][5]) {
    uint64_t R[5];
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) R[x] = A[x][y];
        for (int x = 0; x < 5; x++) A[x][y] = R[x] ^ ((~R[(x+1)%5]) & R[(x+2)%5]);
    }
}

static inline void _iota(uint64_t A[5][5], const int i, const int l, const size_t w) {
    uint64_t RC = 0;
    for (int j = 0; j <= l; j++) {
        RC |= (((RC_LFSR[((j+7*i)%255)/64] >> (((j+7*i)%255)%64)) & 1) << ((1 << j) - 1));
    }
    A[0][0] ^= RC;
}

static inline void keccak_p(uint64_t state[25], const size_t b, const size_t n_r) {
    switch (b) {
        case 1600: case 800: case 400: case 200: case 100: case 50: case 25: break;
        default: 
            printf("keccak_p requires bit width b of size { 1600, 800, 400, 200, 100, 50, 25 }; you have b=%lu\n", b);
            return;
    }

    const size_t w = b/25;
    const size_t l = (size_t) log2(w);

    uint64_t A[5][5] = {{0}};

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if(w==64) A[x][y] = state[5*y + x];
            else A[x][y] = (state[w*(5*y+x)/64] >> ((w*(5*y+x))%64)) & ((1ULL << w) - 1);
        }
    }

    //print_state(A);

    for(int i = 12+2*l-n_r; i < 12+2*l; i++) {
        _theta(A, w);          // 𝜃
          _rho(A, w);         // 𝜌
           _pi(A);           // 𝜋
          _chi(A);          // 𝜒
        _iota(A, i, l, w); // 𝜄
    }

    for(int i = 0; i < (25 + 64/w - 1)*w/64; i+=1) {
        if(w==64) state[i] = A[i%5][i/5];
        else {
            for (int j = 1; j <= 64/w; j++) {
                state[i] <<= w;
                if((64/w-j+64/w*i)/(5) != 5) state[i] += A[(64/w-j+64/w*i)%5][(64/w-j+64/w*i)/(5)];
            }
        }
    }
}

static inline void keccak_f(uint64_t state[25]) {
    keccak_p(state, 1600, 24);
}

// r is always in bits
static void sponge_absorb_bytes(uint64_t state[25], uint8_t buffer[4096], size_t *preloaded_bytes, const size_t buffer_len, const struct SHA_3 sha){
    const size_t r = sha.r;
    if(r%8 != 0) {
        printf("r must be divisible by 8 because we are reversing endienness of bytes\n");
        assert(r%8 == 0);
    }

    //printf("%ld\n", buffer_len);

    uint8_t *buffer_start = buffer;

    size_t bytes_remain = buffer_len - (buffer - buffer_start);

    while (bytes_remain + *preloaded_bytes >= r/8) {
        for (int i = *preloaded_bytes; i < r/8; i++) {
            state[i/8] ^= ((uint64_t) buffer[0]) << (8*(i%8));
            buffer++;
        }
        keccak_f(state);
        *preloaded_bytes = 0;

        bytes_remain = buffer_len - (buffer - buffer_start);
    }

    for (int i = *preloaded_bytes; i < bytes_remain; i++) {
        state[i/8] ^= ((uint64_t) buffer[0]) << (8*(i%8));
        buffer++;
    } 
    *preloaded_bytes = (bytes_remain + *preloaded_bytes) % (r/8);
}


static void sponge_squeeze_hash(uint8_t *hash, uint64_t state[25], size_t *preloaded_bytes, const struct SHA_3 sha) {
    const size_t r = sha.r, d = sha.d;

    size_t len_pad = ((r - (4 + 8*(*preloaded_bytes)) % r) % r + 4)/8;
    state[(*preloaded_bytes)/8] ^= ((uint64_t) 0x06) << (8*((*preloaded_bytes)%8));
    state[((*preloaded_bytes)+len_pad-1)/8] ^= ((uint64_t) 0x80) << (8*(((*preloaded_bytes)+len_pad-1)%8));
    keccak_f(state);
    *preloaded_bytes = 0;

    memcpy(hash, state, sha.d/8);

}


static void parameters(int* index, int* length, char *filename, int argc, char** argv) {
    if (argc > *index) {
        if (strcmp(argv[*index], "-256") == 0) {
            *length = 256;
        } else if (strcmp(argv[*index], "-224") == 0) {
            *length = 224;
        } else if (strcmp(argv[*index], "-384") == 0) {
            *length = 384;
        } else if (strcmp(argv[*index], "-512") == 0) {
            *length = 512;
        } else {
            strncpy(filename, argv[*index], 255);
            // fprintf(stderr, "Usage: %s [-224|-254|-384|-512] -a\n", argv[0]);
            // exit(1);
        }
    }
}

const struct SHA_3 SHA3_224 = {.r=1152,.d=224};
const struct SHA_3 SHA3_256 = {.r=1088,.d=256};
const struct SHA_3 SHA3_384 = {.r=832, .d=384};
const struct SHA_3 SHA3_512 = {.r=576, .d=512};

int main (int argc, char **argv) {

    int bits = 256;
    char filename[256] = {0};

    for (int i = 1; i < 3; i++){
        parameters(&i, &bits, filename, argc, argv);
    }

    struct SHA_3 sha;
    switch (bits) {
        case 224: sha = SHA3_224; break;
        case 256: sha = SHA3_256; break;
        case 384: sha = SHA3_384; break;
        case 512: sha = SHA3_512; break;
        default: exit(1);
    }


    uint8_t buffer[4096];
    size_t bytesRead;

    FILE *f;
    if (filename[0] == 0) { f = stdin; }
    else {
        f = fopen(filename, "rb");
    }

    uint64_t state[25] = {0};
    size_t preloaded_bytes = 0;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0) {

        sponge_absorb_bytes(state, buffer, &preloaded_bytes, bytesRead, sha);

    }
    //printf("preloaded %ld\n", preloaded_bytes);
    //sponge_absorb_bytes(state, buffer, &preloaded_bytes, 0, r, 1);

    uint8_t hash[512];
    sponge_squeeze_hash(hash, state, &preloaded_bytes, sha);

    for (int i = 0; i < sha.d/8; i++) printf("%02x", hash[i]);
    printf("\n");
    //exit(1);
    
}