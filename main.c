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

#include "eigensha.h"

/* 
 * Parse command-line arguments for input filename and SHA-3 output size.
 *
 * index    - pointer to current argument index
 * length   - pointer to output hash length in bits (224, 256, 384, 512)
 * filename - buffer to store input filename if specified
 * argc     - number of command-line arguments
 * argv     - array of command-line arguments
 */
static void parameters(int* index, enum Sha *sha, char *filename, int argc, char** argv);

static const char *sha_to_string(enum Sha sha);

void print_help(const char *prog);

int main (int argc, char **argv) {

    /* Default hash output size and input filename */
    enum Sha sha = SHA_3_256;
    char filename[256] = {0};

    /* Parse up to first two command-line arguments */
    for (int i = 1; i < 3; i++){
        parameters(&i, &sha, filename, argc, argv);
    }

    /* Initialize sponge according to SHA-3 output size */
    eigensha_ctx ctx;
    eigensha_init(&ctx, sha);

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
        eigensha_update(&ctx, buffer, bytesRead);
    if (f != stdin) fclose(f);

    /* Finalize absorption and pad sponge */
    eigensha_finalize(&ctx);

    /* Squeeze hash output of specified length */
    uint8_t hash[512];
    eigensha_extract(hash, &ctx);
    eigensha_free(&ctx);

    /* Print hash in hexadecimal format */
    for (int i = 0; i < ctx.ops->hash_size; i++) printf("%02x", hash[i]);
    if (filename[0] != 0) printf("  %s\n", filename);
    else printf("  -\n");

    return 0;
}

#include <stdio.h>

void print_help(const char *prog) {
    printf("\n\tUsage:\n");
    printf("\t  %s <path/to/file> [OPTIONS]\n", prog);
    printf("\t  cat <path/to/file> | %s [OPTIONS]\n", prog);
    printf("\t  echo -n \"\" | %s\n\n", prog);

    printf("\tDescription:\n");
    printf("\t  Computes SHA-1, SHA-2, and SHA-3 hashes of input data.\n");
    printf("\t  Input can be provided via file or standard input (stdin).\n\n");

    printf("\tOptions:\n");
    printf("\t  -224        Use SHA-224\n");
    printf("\t  -256        Use SHA-256 (default)\n");
    printf("\t  -384        Use SHA-384\n");
    printf("\t  -512        Use SHA-512\n");
    printf("\t  -512/224    Use SHA-512/224\n");
    printf("\t  -512/256    Use SHA-512/256\n");
    printf("\t  -3_224      Use SHA3-224\n");
    printf("\t  -3_256      Use SHA3-256\n");
    printf("\t  -3_384      Use SHA3-384\n");
    printf("\t  -3_512      Use SHA3-512\n\n");

    printf("\tExamples:\n");
    printf("\t  %s file.txt -256\n", prog);
    printf("\t  %s file.txt -3_512\n", prog);
    printf("\t  cat file.txt | %s -512\n\n", prog);

    printf("\tNotes:\n");
    printf("\t  - If no file is provided, input is read from stdin.\n");
    printf("\t  - If no option is specified, SHA-256 is used by default.\n\n");
}

static const char *sha_to_string(enum Sha sha) {
    switch (sha) {
        case SHA_1:       return "SHA-1:       ";
        case SHA_224:     return "SHA-224:     ";
        case SHA_256:     return "SHA-256:     ";
        case SHA_384:     return "SHA-384:     ";
        case SHA_512:     return "SHA-512:     ";
        case SHA_512_224: return "SHA-512/224: ";
        case SHA_512_256: return "SHA-512/256: ";
        case SHA_3_224:   return "SHA3-224:    ";
        case SHA_3_256:   return "SHA3-256:    ";
        case SHA_3_384:   return "SHA3-384:    ";
        case SHA_3_512:   return "SHA3-512:    ";
        default:          return "UNKNOWN?!?:  ";
    }
}

/*
 * Parse a single command-line argument to determine filename or SHA-3 output length.
 *
 * If argument is "-h" or "--help", print usage and exit.
 * If argument is "-224", "-256", "-384", or "-512", set output length.
 * Otherwise, treat argument as input filename.
 */
static void parameters(int* index, enum Sha *sha, char *filename, int argc, char** argv) {
    if (argc > *index) {

        if (strcmp(argv[*index], "-h") == 0 || strcmp(argv[*index], "--help") == 0) {
            print_help(argv[0]);
            exit(1);
        } else if (strcmp(argv[*index], "-256") == 0) {
            *sha = SHA_256;
        } else if (strcmp(argv[*index], "-224") == 0) {
            *sha = SHA_224;
        } else if (strcmp(argv[*index], "-384") == 0) {
            *sha = SHA_384;
        } else if (strcmp(argv[*index], "-512") == 0) {
            *sha = SHA_512;
        } else if (strcmp(argv[*index], "-512/224") == 0) {
            *sha = SHA_512_224;
        } else if (strcmp(argv[*index], "-512/256") == 0) {
            *sha = SHA_512_256;
        } else if (strcmp(argv[*index], "-3_256") == 0) {
            *sha = SHA_3_256;
        } else if (strcmp(argv[*index], "-3_224") == 0) {
            *sha = SHA_3_224;
        } else if (strcmp(argv[*index], "-3_384") == 0) {
            *sha = SHA_3_384;
        } else if (strcmp(argv[*index], "-3_512") == 0) {
            *sha = SHA_3_512;
        } else if (strcmp(argv[*index], "-1") == 0) {
            *sha = SHA_1;
        } else if (argv[*index][0] == '-') {
            print_help(argv[0]);
            exit(1);
        } else {
            /* Copy argument to filename buffer (truncate if too long) */
            strncpy(filename, argv[*index], 255);
            filename[255] = '\0';
        }
    }
}