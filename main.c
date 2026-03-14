/******************************************************************************
 * main.c
 *
 * Example SHA-3 hashing program.
 *
 * Author: eigensmite
 * Date: 2026-03-14
 *
 * Usage:
 *   - Hash a file:
 *       ./sha3 <path/to/file> [-224|-256|-384|-512]
 *   - Hash stdin input:
 *       cat <path/to/file> | ./sha3 [-224|-256|-384|-512]
 *       echo -n "" | ./sha3 [-224|-256|-384|-512]
 *
 * This program demonstrates:
 *   - Initializing a SHA-3 sponge context
 *   - Absorbing input data from file or stdin
 *   - Finalizing with SHA-3 padding
 *   - Squeezing the hash output
 *   - Printing the hash in hexadecimal
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "sha3.h"

/* 
 * Parse command-line arguments for input filename and SHA-3 output size.
 *
 * index    - pointer to current argument index
 * length   - pointer to output hash length in bits (224, 256, 384, 512)
 * filename - buffer to store input filename if specified
 * argc     - number of command-line arguments
 * argv     - array of command-line arguments
 */
static void parameters(int* index, int* length, char *filename, int argc, char** argv);


int main (int argc, char **argv) {

    /* Default hash output size and input filename */
    int bits = 256;
    char filename[256] = {0};

    /* Parse up to first two command-line arguments */
    for (int i = 1; i < 3; i++){
        parameters(&i, &bits, filename, argc, argv);
    }

    /* Initialize sponge according to SHA-3 output size */
    sponge_ctx ctx;
    switch (bits) {
        case 224: sponge_init(&ctx, SHA3_224); break;
        case 256: sponge_init(&ctx, SHA3_256); break;
        case 384: sponge_init(&ctx, SHA3_384); break;
        case 512: sponge_init(&ctx, SHA3_512); break;
        default: exit(1);
    }

    /* Open file or default to stdin if no filename provided */
    FILE *f;
    if (filename[0] == 0) { 
        f = stdin; 
    } else {
        f = fopen(filename, "rb");
        if (!f) {
            perror("Failed to open file");
            exit(1);
        }
    }

    /* Read data from file or stdin and absorb into sponge */
    uint8_t buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0)
        sponge_absorb(&ctx, buffer, bytesRead);
    if (f != stdin) fclose(f);

    /* Finalize absorption and pad sponge */
    sponge_pad(&ctx);

    /* Squeeze hash output of specified length */
    uint8_t hash[512];
    sponge_squeeze(hash, &ctx);

    /* Print hash in hexadecimal format */
    for (int i = 0; i < ctx.out; i++) printf("%02x", hash[i]);
    if (filename[0] != 0) printf("  %s\n", filename);
    else printf("  -\n");

    return 0;
}

/*
 * Parse a single command-line argument to determine filename or SHA-3 output length.
 *
 * If argument is "-h" or "--help", print usage and exit.
 * If argument is "-224", "-256", "-384", or "-512", set output length.
 * Otherwise, treat argument as input filename.
 */
static void parameters(int* index, int* length, char *filename, int argc, char** argv) {
    if (argc > *index) {

        if (strcmp(argv[*index], "-h") == 0 || strcmp(argv[*index], "--help") == 0) {
            fprintf(stderr, "Usage: \n\t%s <path/to/file> [-224|-256|-384|-512]\n\tcat <path/to/file> | %s [-224|-256|-384|-512]\n\techo -n \"\" | %s\n", argv[0], argv[0], argv[0]);
            exit(1);
        } else if (strcmp(argv[*index], "-256") == 0) {
            *length = 256;
        } else if (strcmp(argv[*index], "-224") == 0) {
            *length = 224;
        } else if (strcmp(argv[*index], "-384") == 0) {
            *length = 384;
        } else if (strcmp(argv[*index], "-512") == 0) {
            *length = 512;
        } else {
            /* Copy argument to filename buffer (truncate if too long) */
            strncpy(filename, argv[*index], 255);
            filename[255] = '\0';
        }
    }
}