# SHA-3 (Keccak) Hash Function in C

## Overview

This project implements the **SHA-3 (Keccak) hash function** in C using the **sponge construction**. It supports all standard SHA-3 variants: 224, 256, 384, and 512-bit hashes.  

The implementation is designed to be **clear, modular, and easy to integrate**, with separate modules for the sponge construction (`sha3.c`) and the Keccak permutation (`keccak_f.c`).

---

## Features

- SHA-3 hashing using the sponge construction
- Supports output lengths:
  - SHA3-224
  - SHA3-256
  - SHA3-384
  - SHA3-512
- File or `stdin` input
- Fully modular and portable C implementation
- Easy-to-read, well-commented code

---

## File Structure
├── main.c # CLI interface for hashing files or stdin
├── sha3.h # SHA-3 sponge interface
├── sha3.c # SHA-3 sponge implementation
├── keccak_f.h # Keccak-f permutation interface
├── keccak_f.c # Keccak-f permutation implementation
└── README.md # This file

---

## Compilation

Use `gcc` with optimization flags:

```bash
gcc main.c sha3.c keccak_f.c -o sha3 -lm -std=c11 -Wall -O2 -floop-nest-optimize -funroll-loops -fpeel-loops
```

---

## Usage

```bash
./sha3 <path/to/file> [-224|-256|-384|-512]
```
```bash
cat <path/to/file> | ./sha3 [-224|-256|-384|-512]
```

#### SHA3-256 hash of a file (SHA3-256 is default configuration)
```bash
./sha3 file.txt
```

#### SHA3-512 hash from stdin
```bash
cat file.txt | ./sha3 -512
```

### SHA3-224 hash of empty string
```bash
echo -n "" | ./sha3 -224
```

---

## API Overview

### Structures
```c
typedef struct {
    uint64_t state[25];  // Internal state of the sponge
    uint8_t buf[200];    // Partial input buffer
    size_t pos;          // Current buffer position
    size_t rate;         // Rate in bytes
    size_t out;          // Output length in bytes
} sponge_ctx;

typedef struct {
    const size_t rate;   // Rate of absorption (bytes)
    const size_t out;    // Output length (bytes)
} SHA_3;

extern const SHA_3 SHA3_224, SHA3_256, SHA3_384, SHA3_512;
```

### Functions

```c
// Initialize the sponge context with SHA-3 parameters
void sponge_init(sponge_ctx *ctx, const SHA_3 sha);

// Absorb input data into the sponge
void sponge_absorb(sponge_ctx *ctx, const uint8_t *data, size_t len);

// Apply SHA-3 padding and finalize absorption
void sponge_pad(sponge_ctx *ctx);

// Extract hash output from the sponge
void sponge_squeeze(uint8_t *hash, sponge_ctx *ctx);

// Keccak-f[1600] permutation on 25-lane state 
// (advanced,nonstandard functionality)
void keccak_f(uint64_t state[25]);
```

---

## Implementation Notes

The sponge construction absorbs input in blocks (rate bytes) and permutes the state using the Keccak-f function.

Keccak-f[1600] operates on a 5x5x64-bit state array, applying the standard rounds: θ (theta), ρ (rho), π (pi), χ (chi), ι (iota).

Padding uses the standard SHA-3 10*1 pattern (0x06 … 0x80).

The code is designed for clarity and modularity; memcpy and memset are used for efficient buffer handling.

---

## License

This project is released under the MIT License.