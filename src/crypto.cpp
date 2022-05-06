#include "crypto.h"
#include "errors.h"

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>


int genHash(const char hashFunc[], void *msg, size_t size, HashSHA3_256 *digest){
    EVP_MD_CTX *ctx = NULL;
    EVP_MD *func = NULL;
    unsigned int len = 0;
    /* Create a context for the digest operation */
    ctx = EVP_MD_CTX_new();
    if (ctx == NULL){
        /* Clean up all the resources we allocated */
        EVP_MD_free(func);
        EVP_MD_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        error("Context returned NULL");
        return -1;
    }

    /*
     * Fetch the SHA256 algorithm implementation for doing the digest. We're
     * using the "default" library context here (first NULL parameter), and
     * we're not supplying any particular search criteria for our SHA256
     * implementation (second NULL parameter). Any SHA256 implementation will
     * do.
     */
    func = EVP_MD_fetch(NULL, hashFunc, NULL);
    if (func == NULL){
        /* Clean up all the resources we allocated */
        EVP_MD_free(func);
        EVP_MD_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        error("Function returned NULL");
        return -1;
    }

   /* Initialise the digest operation */
   if (!EVP_DigestInit_ex(ctx, func, NULL)){
       /* Clean up all the resources we allocated */
       EVP_MD_free(func);
       EVP_MD_CTX_free(ctx);
       ERR_print_errors_fp(stderr);
       error("Could not initialise the digest operation");
       return -1;
   }

    /*
     * Pass the message to be digested. This can be passed in over multiple
     * EVP_DigestUpdate calls if necessary
     */
    if (!EVP_DigestUpdate(ctx, (char *) msg, size)){
        /* Clean up all the resources we allocated */
        EVP_MD_free(func);
        EVP_MD_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        error("Could not pass message to be digested");
        return -1;
    }

    // Zero out digest structure
    memset(digest, 0, sizeof(HashSHA3_256));

    /* Now calculate the digest itself */
    if (!EVP_DigestFinal_ex(ctx, (unsigned char *) digest, &len)){
        /* Clean up all the resources we allocated */
        EVP_MD_free(func);
        EVP_MD_CTX_free(ctx);
        ERR_print_errors_fp(stderr);
        error("Could not calcualte digest");
        return -1;
    }
    return 0;
}
