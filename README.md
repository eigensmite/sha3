# EigenSHA - Polymorphic Interface for SHA Hashing

## Overview

EigenSHA is a modular C library providing a **unified polymorphic interface** for the SHA-1, SHA-2, and SHA-3 families.

It supports **two layers of polymorphism**:

### 1. Runtime Polymorphism (Dynamic Dispatch)

* Uses `eigensha_ctx` + `enum Sha`
* Works across **all SHA-1, SHA-2, and SHA-3 variants**
* Backed by a virtual function table (`sha_ops`)

### 2. Compile-time Polymorphism (C11 `_Generic`)

* Provides uniform macros: `sha_init`, `sha_update`, etc.
* Works for **SHA-1 and SHA-2 only**
* SHA-3 uses the **sponge interface** (`sha3.h`) directly

---

## Features

### SHA-1 / SHA-2 (Merkle–Damgård + Compression Function)

* Built on the Merkle–Damgård construction
* Uses algorithm-specific compression functions

Supports:

* SHA-1
* SHA-224
* SHA-256
* SHA-384
* SHA-512
* SHA-512/224
* SHA-512/256

---

### SHA-3 (Keccak Sponge Construction)

* Built on the sponge construction
* Uses the Keccak-f permutation
* Implemented via a dedicated sponge engine (`sha3.c`)

Supports:

* SHA3-224
* SHA3-256
* SHA3-384
* SHA3-512

---

## File Structure

```
├── main.c        # CLI interface
├── eigensha.h    # Unified polymorphic interface
├── sha_ops.h     # VTable definitions and wrappers

├── sha1.h/.c     # SHA-1 implementation
├── sha256.h/.c   # SHA-224 / SHA-256
├── sha512.h/.c   # SHA-384 / SHA-512 family

├── sha3.h/.c     # Sponge construction
├── keccak_f.h/.c # Keccak-f permutation

└── README.md
```
---

## Compilation

```bash
make
```

Optional test suite:

```bash
make test
```

---

## Usage

### CLI Syntax

```bash
./eigensha <file> [option]
cat <file> | ./eigensha [option]
```

### Algorithm Flags

| Flag       | Algorithm          |
| ---------- | ------------------ |
| `-1`       | SHA-1              |
| `-224`     | SHA-224            |
| `-256`     | SHA-256            |
| `-384`     | SHA-384            |
| `-512`     | SHA-512            |
| `-512_224` | SHA-512/224        |
| `-512_256` | SHA-512/256        |
| `-3_224`   | SHA3-224           |
| `-3_256`   | SHA3-256 (default) |
| `-3_384`   | SHA3-384           |
| `-3_512`   | SHA3-512           |

---

### Examples

#### Default (SHA3-256)

```bash
./eigensha file.txt
```

#### SHA-512

```bash
./eigensha file.txt -512
```

#### SHA3-512 from stdin

```bash
cat file.txt | ./eigensha -3_512
```

#### SHA3-224 empty string

```bash
echo -n "" | ./eigensha -3_224
```

---

## API Overview

### Runtime Polymorphic Interface

#### Context

```c
typedef struct {
const sha_ops *ops;
void *ctx;
} eigensha_ctx;
```

#### Core Functions

```c
void eigensha_init(eigensha_ctx *s, enum Sha sha);
void eigensha_free(eigensha_ctx *s);

void eigensha_update(eigensha_ctx *s, void *data, size_t len);
void eigensha_finalize(eigensha_ctx *s);

void eigensha_extract(uint8_t *hash, eigensha_ctx *s);
size_t eigensha_get_hash_len(eigensha_ctx *s);
```

#### Utility

```c
void eigensha_hash_to_string(char *out, uint8_t *hash, size_t len);
```

---

### Compile-time Polymorphic API (SHA-1 / SHA-2 Only)

```c
sha_init(ctx);
sha_update(ctx, data, len);
sha_finalize(ctx);
sha_extract(hash, ctx);
```

* Implemented via `_Generic`
* Works with:

  * `sha1_ctx`
  * `sha224_ctx`, `sha256_ctx`
  * `sha384_ctx`, `sha512_ctx`, etc.
* **Not valid for `eigensha_ctx` or SHA-3**

---

### SHA-3 Sponge API (Separate)

SHA-3 is accessed via the sponge interface:

```c
sponge_init(...)
sponge_absorb(...)
sponge_pad(...)
sponge_squeeze(...)
```

This separation reflects the difference between:

* Merkle–Damgård (SHA-1/2)
* Sponge construction (SHA-3)

---

## Implementation Notes

* **Dynamic dispatch**

  * Implemented via `sha_ops` (VTable)
  * Each algorithm defines wrappers to unify function signatures

* **SHA-1 / SHA-2**

  * Block-based processing
  * Compression function per round
  * Fixed internal state evolution

* **SHA-3**

  * Absorb → Permute → Squeeze model
  * Uses Keccak-f[1600] permutation
  * 5×5×64-bit state array
  * Round steps: θ, ρ, π, χ, ι

* **Padding**

  * SHA-1 / SHA-2: standard Merkle–Damgård padding
  * SHA-3: `10*1` pattern (`0x06 ... 0x80`)

---

## Design Notes

* `eigensha_ctx` provides **runtime algorithm switching**
* `_Generic` macros provide **zero-overhead static dispatch**
* SHA-3 is intentionally **separated from SHA-2-style APIs** to preserve architectural clarity

---

## License

MIT License
