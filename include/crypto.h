#ifndef CRYPTO_H
#define CRYPTO_H

#define SHA256_LEN_BYTES 32

typedef struct HashSHA256{
    char hash[SHA256_LEN_BYTES];
} HashSHA256;

int genHash();

#endif
