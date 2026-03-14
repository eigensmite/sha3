
/******************************************************************************
 * sha3.c
 *
 * SHA-3 (Keccak) hash function implementation using the sponge construction.
 *
 * Author: eigensmite
 * Date: 2026-03-14
 * 
 * Provides:
 *   - sponge_ctx: internal state and buffer for the sponge
 *   - SHA_3: SHA-3 parameter sets (rate and output length)
 *   - Predefined SHA3-224, SHA3-256, SHA3-384, SHA3-512 parameter sets
 *   - Functions for initialization, absorption, padding, and squeezing
 *
 * Reference:
 *   - FIPS 202: SHA-3 Standard (Keccak)
 *****************************************************************************/

#include "sha3.h"

/* Predefined SHA-3 parameter sets */
const SHA_3 SHA3_224 = {.rate=144, .out=28};
const SHA_3 SHA3_256 = {.rate=136, .out=32};
const SHA_3 SHA3_384 = {.rate=104, .out=48};
const SHA_3 SHA3_512 = {.rate=72,  .out=64};


/*
 * Initialize the sponge context with the specified SHA-3 parameters.
 * Resets the state and buffer.
 */
void sponge_init(sponge_ctx *ctx, const SHA_3 sha) {
    for (int i = 0; i < 25; i++) ctx->state[i] = 0;
    for (int i = 0; i < 200; i++) ctx->buf[i] = 0;
    ctx->pos  = 0;
    ctx->rate = sha.rate;
    ctx->out  = sha.out;
}


/*
 * Absorb input bytes into the sponge state.
 * Fills the internal buffer, XORs into state when full, and applies
 * the Keccak-f permutation.
 *
 * ctx  - sponge context
 * data - input data buffer
 * len  - number of bytes to absorb
 */
void sponge_absorb(sponge_ctx *ctx, const uint8_t *data, size_t len) {
    while (len > 0) {

        /* Number of bytes we can take into buffer */
        size_t take = ctx->rate - ctx->pos;
        if (take > len) take = len;

        /* Copy data into buffer */
        memcpy(ctx->buf + ctx->pos, data, take);
        ctx->pos += take;
        data += take;
        len -= take;

        /* If buffer is full, XOR into state and permute */
        if (ctx->pos == ctx->rate) {
            for (size_t i = 0; i < ctx->rate/8; i++)
                ctx->state[i] ^= ((uint64_t *)ctx->buf)[i];
            keccak_f(ctx->state);
            ctx->pos = 0;
        }
    }
}


/*
 * Apply SHA-3 padding (10*1) and finalize the sponge state.
 *
 * ctx - sponge context
 */
void sponge_pad(sponge_ctx *ctx) {

    /* Zero pad the remaining buffer */
    memset(ctx->buf + ctx->pos, 0x00, ctx->rate - ctx->pos);

    /* Insert 10*1 padding (0x06 ... 0x80) */
    ctx->buf[ctx->pos]       |= 0x06;
    ctx->buf[ctx->rate - 1] |= 0x80;

    /* XOR buffer into state and permute */
    for (size_t i = 0; i < ctx->rate/8; i++)
        ctx->state[i] ^= ((uint64_t *)ctx->buf)[i];
    keccak_f(ctx->state);
    ctx->pos = 0;
}


/*
 * Squeeze the final hash output from the sponge.
 *
 * hash - output buffer (length ctx->out)
 * ctx  - sponge context
 */
void sponge_squeeze(uint8_t *hash, sponge_ctx *ctx) {
    memcpy(hash, ctx->state, ctx->out);
}