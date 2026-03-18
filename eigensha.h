#ifndef EIGENSHA_H
#define EIGENSHA_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sha_ops.h"

enum Sha {
    SHA_1, SHA_224, SHA_256, SHA_384, SHA_512, SHA_512_224, SHA_512_256, SHA_3_224, SHA_3_256, SHA_3_384, SHA_3_512, SHA_COUNT
};

typedef struct {
    const sha_ops *ops;
    void *ctx;
} eigensha_ctx;

void eigensha_init(eigensha_ctx *s, enum Sha sha);

void eigensha_free(eigensha_ctx *s);

void eigensha_update(eigensha_ctx *s, void *data, size_t len);

void eigensha_finalize(eigensha_ctx *s) ;

void eigensha_extract(uint8_t *hash, eigensha_ctx *s);

#define sha_init(ctx) _Generic((ctx), \
    sha1_ctx*: sha1_init, \
    sha224_ctx*: sha224_init, \
    sha256_ctx*: sha256_init, \
    sha384_ctx*: sha384_init, \
    sha512_ctx*: sha512_init, \
    sha512_224_ctx*: sha512_224_init, \
    sha512_256_ctx*: sha512_256_init \
)(ctx)

#define sha_update(ctx, data, len) _Generic((ctx), \
    eigensha_ctx*: eigensha_update, \
    sha1_ctx*: sha1_update, \
    sha224_ctx*: sha224_update, \
    sha256_ctx*: sha256_update, \
    sha384_ctx*: sha384_update, \
    sha512_ctx*: sha512_update, \
    sha512_224_ctx*: sha512_224_update, \
    sha512_256_ctx*: sha512_256_update \
)(ctx, data, len)

#define sha_finalize(ctx) _Generic((ctx), \
    eigensha_ctx*: eigensha_finalize, \
    sha1_ctx*: sha1_finalize, \
    sha224_ctx*: sha224_finalize, \
    sha256_ctx*: sha256_finalize, \
    sha384_ctx*: sha384_finalize, \
    sha512_ctx*: sha512_finalize, \
    sha512_224_ctx*: sha512_224_finalize, \
    sha512_256_ctx*: sha512_256_finalize \
)(ctx)

#define sha_extract(hash, ctx) _Generic((ctx), \
    eigensha_ctx*: eigensha_extract, \
    sha1_ctx*: sha1_extract, \
    sha224_ctx*: sha224_extract, \
    sha256_ctx*: sha256_extract, \
    sha384_ctx*: sha384_extract, \
    sha512_ctx*: sha512_extract, \
    sha512_224_ctx*: sha512_224_extract, \
    sha512_256_ctx*: sha512_256_extract \
)(hash, ctx)

#endif