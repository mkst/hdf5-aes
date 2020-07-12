#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <hdf5.h>
#include <H5PLextern.h>

#include "hdf5_aes.h"

#define AES_BLOCK_SIZE 16
#define AES_256_KEY_LENGTH 32

#define ENV_HDF5_AES_KEY "HDF5_AES_KEY"

#define MIN(a,b) (((a)<(b))?(a):(b))

static size_t aes_filter(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf);

int encrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key);
int decrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key);
unsigned char* generate_random_block(unsigned char* block);

const H5Z_class2_t FILTER_TEMPLATE_CLASS[1] = {{
    H5Z_CLASS_T_VERS,
    HDF5_AES_FILTER,
    1,
    1,
    "HDF5 AES Filter",
    NULL,
    NULL,
    (H5Z_func_t)aes_filter,
}};

H5PL_type_t H5PLget_plugin_type(void) { return H5PL_TYPE_FILTER; }
const void *H5PLget_plugin_info(void) { srand(time(0)); return FILTER_TEMPLATE_CLASS; }

static size_t
aes_filter(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf)
{
    /* pointers */
    void *dest = NULL;
    void *src_ = NULL;
    unsigned char * key = NULL;

    char *env_var;
    int res;

    if(NULL == (env_var = getenv(ENV_HDF5_AES_KEY))) {
        printf("ERROR: %s VARIABLE NOT SET. DATA WILL NOT BE SAVED!\n", ENV_HDF5_AES_KEY);
        goto done;
    }

    key = (unsigned char *)H5allocate_memory(AES_256_KEY_LENGTH * sizeof(unsigned char), true);
    if (key == NULL)
        goto done;
    memcpy(key, env_var, MIN(AES_256_KEY_LENGTH, strlen(env_var)));

    if (flags & H5Z_FLAG_REVERSE)
    {
        /* decrypt */
        if (NULL == (dest = H5allocate_memory(nbytes, false)))
            goto done;
        if (AES_BLOCK_SIZE > (res = decrypt(nbytes, (unsigned char *)*buf, (unsigned char *)dest, key)))
            goto done;
        /* discard first block */
        memmove(dest, (char *)dest + AES_BLOCK_SIZE, res - AES_BLOCK_SIZE);
        res -= AES_BLOCK_SIZE;
    }
    else
    {
        /* encrypt */
        if (NULL == (src_ = H5allocate_memory(nbytes + AES_BLOCK_SIZE, false)))
            goto done;
        memcpy((char *)src_ + AES_BLOCK_SIZE, *buf, nbytes);
        /* additional room for (iv || chunk || padding) */
        if (NULL == (dest = H5allocate_memory(nbytes + 2*AES_BLOCK_SIZE, false)))
            goto done;
        if (!(res = encrypt(nbytes + AES_BLOCK_SIZE, (unsigned char *)src_, (unsigned char *)dest, key)))
            goto done;
    }

    /* repoint *buf to (en|de)crypted block */
    H5free_memory(*buf);
    *buf = dest;

done:
    H5free_memory(key);
    H5free_memory(src_);
    return res;
}

unsigned char* generate_random_block(unsigned char* block)
{
    for (int i = 0; i < AES_BLOCK_SIZE; i++)
    {
        block[i] = (unsigned char) rand() % 256;
    }
    return block;
}

int encrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key)
{
    EVP_CIPHER_CTX *ctx;
    unsigned char * iv = NULL;

    int len = 0;
    int ciphertext_len = 0;

    iv = (unsigned char*) H5allocate_memory(AES_BLOCK_SIZE * sizeof(unsigned char), false);
    if (iv == NULL)
        goto done;
    generate_random_block(iv);

    if (!(ctx = EVP_CIPHER_CTX_new()))
        goto done;
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        goto done;
    if(1 != EVP_EncryptUpdate(ctx, dest, &len, src, src_len))
        goto done;
    ciphertext_len = len;
    if(1 != EVP_EncryptFinal_ex(ctx, dest + len, &len))
        goto done;
    ciphertext_len += len;

done:
    H5free_memory(iv);
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int decrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key)
{
    /* iv is irrelevant as first block is discarded */
    unsigned char iv[AES_BLOCK_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EVP_CIPHER_CTX *ctx;

    int len;
    int plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        goto done;
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        goto done;
    if(1 != EVP_DecryptUpdate(ctx, dest, &len, src, src_len))
        goto done;
    plaintext_len = len;
    if(1 != EVP_DecryptFinal_ex(ctx, dest + len, &len))
        goto done;
    plaintext_len += len;

done:
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
