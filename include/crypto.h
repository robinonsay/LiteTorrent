#ifndef CRYPTO_H
#define CRYPTO_H
#define SHA_256_BYTES 32

#include <stdio.h>
#include <string.h>

typedef struct HashSHA3_256{
    unsigned char hash[SHA_256_BYTES];
} HashSHA3_256;

int genHash(const char hashFunc[], void *msg, size_t size, HashSHA3_256 *digest);

#endif
