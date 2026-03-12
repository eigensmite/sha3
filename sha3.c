
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gmp.h>
#include <math.h>
#include <string.h>

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

size_t pad10star1(mpz_t out, const mpz_t in, const size_t r) {
    
    size_t len_in = mpz_sizeinbase(in, 2) - 1;

    mpz_fdiv_r_2exp(out, in, len_in); // copy in to out


    // printf("in size %ld\n", len_in);
    // gmp_printf("in  %Zx\n", in);
    // gmp_printf("out %Zx\n", out);

    // calculate length of 10*1 pad
    // yield x = -(4 + len_in) (mod r), 0 <= x < r
    size_t len_pad = (r - (4 + len_in) % r) % r + 4;

    mpz_t pad;
    mpz_init(pad);  // init pad

    mpz_set_ui(pad, 1);                   // pad = 1;
    mpz_mul_2exp(pad, pad, len_pad - 1); // pad <<= len_pad-1;
    mpz_add_ui(pad, pad, 6);            // pad += 6;
    mpz_mul_2exp(pad, pad, len_in);    // pad <<= len_in
    mpz_add(out, out, pad);           // out += pad;
    
    mpz_clear(pad); // clear pad

    size_t len_out = len_pad + len_in;

    return len_out;
}

// // natural order endian version
// size_t pad10star1(mpz_t out, const mpz_t in, const size_t r) {
    
//     size_t len_in = mpz_sizeinbase(in, 2) - 1;

//     mpz_fdiv_r_2exp(out, in, len_in); // copy in to out


//     printf("in size %ld\n", len_in);
//     gmp_printf("in  %Zx\n", in);
//     gmp_printf("out %Zx\n", out);

//     // calculate length of 10*1 pad
//     // yield x = -(4 + len_in) (mod r), 0 <= x < r
//     size_t len_pad = (r - (4 + len_in) % r) % r + 4;

//     mpz_t pad;
//     mpz_init(pad);  // init pad

//     // mpz_set_ui(pad, 1);                   // pad = 1;
//     // mpz_mul_2exp(pad, pad, len_pad - 1); // pad <<= len_pad-1;
//     // mpz_add_ui(pad, pad, 6);            // pad += 6;
//     // mpz_mul_2exp(pad, pad, len_in);    // pad <<= len_in
//     // mpz_add(out, out, pad);           // out += pad;


//     mpz_mul_2exp(out, out, 4);
//     mpz_add_ui(out, out, 6);
//     mpz_mul_2exp(out, out, len_pad - 4);
//     mpz_add_ui(out, out, 1);
    
    
//     mpz_clear(pad); // clear pad

//     size_t len_out = len_pad + len_in;

//     return len_out;
// }


// uint8_t get_bit(uint64_t A[5][5], const int x, const int y, const int z) {
//     return (A[x][y] >> z) & 1;
// }

// void set_bit(uint64_t A[5][5], const int x, const int y, const int z, const _Bool b) {
//     A[x][y] &= ~((uint64_t)1 << z);
//     A[x][y] |= ((uint64_t)b << z);
// }

// static inline _Bool _theta_C(uint64_t A[5][5], const int x, const int z) {
//     return get_bit(A, x, 0, z) ^ 
//            get_bit(A, x, 1, z) ^ 
//            get_bit(A, x, 2, z) ^ 
//            get_bit(A, x, 3, z) ^ 
//            get_bit(A, x, 4, z) ; 
// }

// uint8_t _theta_D(uint64_t *C, const int x, const int z, const size_t w) {
//     //return (C[(x-1) % 5] >> z) ^ (C[(x+1) % 5] >> ((z-1) % w));
//     // return 1 & (((C[(x+4) % 5] >> z) & 1) ^ ((C[(x+1) % 5] >> ((z+w-1) % w)) & 1));
//     return ((C[(x+4) % 5] >> z) ^ (C[(x+1) % 5] >> ((z+w-1) % w))) & 1;
// }


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


static inline void keccak_p(mpz_t out, const mpz_t in, const size_t b, const size_t n_r) {

    switch (b) {
        case 1600: case 800: case 400: case 200: case 100: case 50: case 25: break;
        default: 
            printf("keccak_p requires bit width b of size { 1600, 800, 400, 200, 100, 50, 25 }; you have b=%lu\n", b);
            return;
    }

    const size_t w = b/25;
    const size_t l = (size_t) log2(w); // cast to size_t safe because w is always a power of 2 (enforced by switch above)

    uint64_t A[5][5] = {{0}};

    mpz_t tmp;
    mpz_init(tmp);
    for(int x = 0; x < 5; x++) {
        for(int y = 0; y < 5; y++) {
            mpz_tdiv_q_2exp(tmp, in, w*(5*y + x));         // align desired bits to lowest 64               
            A[x][y] = mpz_get_ui(tmp);                     // load lanes
            if(w < 64) A[x][y] &= ((1ULL << w) - 1);       // mask for <64 lane length
        }
    }
    mpz_clear(tmp);

    for(int i = 12+2*l-n_r; i < 12+2*l; i++) {
        _theta(A, w);          // 𝜃
          _rho(A, w);         // 𝜌
           _pi(A);           // 𝜋
          _chi(A);          // 𝜒
        _iota(A, i, l, w); // 𝜄
    }

    mpz_set_ui(out, 0);
    for (int y = 4; y >= 0; y--) {
        for (int x = 4; x >= 0; x--) {
            mpz_mul_2exp(out, out, w);
            mpz_add_ui(out, out, A[x][y]);
        }
    }
}

static inline void keccak_f(mpz_t out, const mpz_t in) {keccak_p(out, in, 1600, 24);}

static void sponge(mpz_t out, const mpz_t in, pad_fn pad, perm_fn perm, size_t r, size_t d) {
    mpz_t P, Pr, S, Sr;
    mpz_inits(P, Pr, S, Sr, NULL);
    
    // const size_t b can be commented out due to 0 bit 
    // padding being trivial/leading bits
    // size of b is determined by perm function exclusively
    // const size_t b = 1600;

    size_t p_len = pad(P, in, r); // pad input N

    // find number of r-bit pieces
    // P_0, P_1, ... , P_{n-1}.
    if (p_len % r != 0) {
        printf("Padding function didn't yield P of num bits divisible by r\n");
        exit(1);
    } 
    size_t n = p_len /= r; // get num of pieces

    // absorb all pieces into sponge
    for(size_t i = 0; i < n; i++) {

        // extract i-th piece
        mpz_fdiv_q_2exp(Pr, P, r*i); // shift right r*i bits
        mpz_fdiv_r_2exp(Pr, Pr, r);   // take r bits

        // extension by c = b - r bits is automatic
        // leading 0 bits are incipient in mpz_t

        mpz_xor(S, S, Pr); // XOR state with (trivially) extended P

        perm(S, S); // apply b-bit block permutation, yielding S_{i+1}
    }
    //exit(1);
   
    size_t len_out = 0;
    do {
        // extract first r bits from S
        mpz_fdiv_r_2exp(Sr, S, r); // take r bits

        // append those bits to Z
        mpz_mul_2exp(out, out, r);
        mpz_ior(out, out, Sr);
        len_out += r;

        if (len_out < d) perm(S, S);
    } while (len_out < d);

    mpz_fdiv_r_2exp(out, out, d); // truncate to d bits  
    
    mpz_clears(P, Pr, S, Sr, NULL);
}

void sha3_224(mpz_t out, const mpz_t in) {sponge(out, in, pad10star1, keccak_f, 1152, 224);}

void sha3_256(mpz_t out, const mpz_t in) {sponge(out, in, pad10star1, keccak_f, 1088, 256);}

void sha3_384(mpz_t out, const mpz_t in) {sponge(out, in, pad10star1, keccak_f, 832, 384);}

void sha3_512(mpz_t out, const mpz_t in) {sponge(out, in, pad10star1, keccak_f, 576, 512);}


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

int main (int argc, char **argv) {

    // time_t start_time = time(NULL);
    // time_t build_bitstream_time;
    // time_t sha3_time;

    int bits = 256;
    char filename[256] = {0};

    for (int i = 1; i < 3; i++){
        parameters(&i, &bits, filename, argc, argv);
    }

    mpz_t out;
    mpz_init(out);

    mpz_t value; 
    mpz_init(value);

    unsigned char buffer[200];
    size_t bytesRead;
    size_t totalBytes = 0;

    mpz_t chunk;
    mpz_init(chunk);

    FILE *f;
    if (filename[0] == 0) { f = stdin; }
    else {
        f = fopen(filename, "rb");
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        mpz_set_ui(chunk, 0);

        // Import chunk in little-endian
        mpz_import(chunk, bytesRead, -1, 1, 0, 0, buffer);

        // Shift the chunk left by totalBytes*8 so it is positioned correctly
        mpz_mul_2exp(chunk, chunk, totalBytes * 8);

        // Add chunk to the total number
        mpz_add(value, value, chunk);

        // Update total bytes read so far
        totalBytes += bytesRead;
    }
    mpz_clear(chunk);
    


    //mpz_import(value, strlen(str), -1, sizeof(char), 1, 0, str);
    mpz_setbit(value, totalBytes * 8); // need sentinel bit for padding func to know when to start

    if (bits == 224) {
    unsigned char buffer_224[224/8];
    sha3_224(out, value);
    mpz_export(buffer_224, NULL, -1, 1, -1, 0, out);
    for(int i = 0; i < 224/8; i++) {
        printf("%02x", buffer_224[i]);
    }
    printf("\n");
} else if (bits == 256) {

    unsigned char buffer_256[256/8];
    sha3_256(out, value);
    mpz_export(buffer_256, NULL, -1, 1, -1, 0, out);
    for(int i = 0; i < 256/8; i++) {
        printf("%02x", buffer_256[i]);
    }
    printf("\n");
} else if (bits == 384) {

    unsigned char buffer_384[384/8];
    sha3_384(out, value);
    mpz_export(buffer_384, NULL, -1, 1, -1, 0, out);
    for(int i = 0; i < 384/8; i++) {
        printf("%02x", buffer_384[i]);
    }
    printf("\n");

} else if (bits == 512) {
    unsigned char buffer_512[512/8];
    sha3_512(out, value);
    mpz_export(buffer_512, NULL, -1, 1, -1, 0, out);
    for(int i = 0; i < 512/8; i++) {
        printf("%02x", buffer_512[i]);
    }
    printf("\n");
} else printf("uhhh what happened");






    mpz_clear(value);


    // sha3_384(out, in);
    // gmp_printf("%ZX\n", out);
    // sha3_512(out, in);
    // gmp_printf("%ZX\n", out);


    
}