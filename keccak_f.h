
/******************************************************************************
 * keccak_f.h
 *
 * Keccak-f[1600] permutation function for SHA-3 (Keccak) hash implementation.
 *
 * Author: eigensmite
 * Date: 2026-03-14
 *
 * Provides the low-level permutation applied to the 1600-bit sponge state.
 * This is the core transformation of the SHA-3 sponge construction.
 *****************************************************************************/

#ifndef KECCAK_F_H
#define KECCAK_F_H

#include <stdio.h>
#include <stdint.h>

/*
 * Apply the Keccak-f[1600] permutation to the given state.
 *
 * state - 25-element array representing the 1600-bit sponge state
 *
 * This function updates the state in-place according to the Keccak-f[1600]
 * specification as defined in FIPS 202.
 */
void keccak_f(uint64_t state[25]);

#endif