#include <stdlib.h>
#include <string.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <hdf5.h>
#include <H5PLextern.h>

#include "hdf5_aes.h"

#define AES_BLOCK_SIZE 16
#define AES_256_KEY_LENGTH 32

#define ENV_HDF5_AES_IV "HDF5_AES_IV"
#define ENV_HDF5_AES_KEY "HDF5_AES_KEY"

#define MIN(a,b) (((a)<(b))?(a):(b))

static size_t aes_filter(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf);

int encrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key, unsigned char * iv);
int decrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key, unsigned char * iv);
int on_error(EVP_CIPHER_CTX *ctx);

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
const void *H5PLget_plugin_info(void) { return FILTER_TEMPLATE_CLASS; }

static size_t
aes_filter(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf)
{
    int res;
    void *dest = NULL;
    dest = H5allocate_memory(nbytes + AES_BLOCK_SIZE, true); /* Add 16 bytes for padding */
    char *env_var;

    if (NULL == (env_var = getenv(ENV_HDF5_AES_IV))) {
        printf("HDF5_AES_IV NOT SET\n");
        return 0;
    }
    unsigned char * iv = (unsigned char *)calloc(AES_BLOCK_SIZE, sizeof(unsigned char));
    memcpy(iv, env_var, MIN(AES_BLOCK_SIZE, strlen(env_var)));

    if(NULL == (env_var = getenv(ENV_HDF5_AES_KEY))) {
        printf("HDF5_AES_KEY NOT SET\n");
        return 0;
    }
    unsigned char * key = (unsigned char *)calloc(AES_256_KEY_LENGTH, sizeof(unsigned char));
    memcpy(key, env_var, MIN(AES_256_KEY_LENGTH, strlen(env_var)));

    if (flags & H5Z_FLAG_REVERSE)
    {
        res = decrypt(nbytes, (unsigned char *)*buf, (unsigned char *)dest, key, iv);
    }
    else
    {
        res = encrypt(nbytes, (unsigned char *)*buf, (unsigned char *)dest, key, iv);
    }

    if (res)
    {
        free(*buf);
        *buf = dest;
    }

    return res;
}

int on_error(EVP_CIPHER_CTX *ctx)
{
  if (ctx != NULL)
  {
      EVP_CIPHER_CTX_free(ctx);
  }
  return 0;
}

int encrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key, unsigned char * iv)
{
    int len = 0;
    int ciphertext_len = 0;

    EVP_CIPHER_CTX *ctx;

    if (!(ctx = EVP_CIPHER_CTX_new()))
    {
        return on_error(ctx);
    }
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        return on_error(ctx);
    }
    if(1 != EVP_EncryptUpdate(ctx, dest, &len, src, src_len))
    {
        return on_error(ctx);
    }
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, dest + len, &len))
    {
        return on_error(ctx);
    }
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int decrypt(size_t src_len, unsigned char *src, unsigned char *dest,
            unsigned char * key, unsigned char * iv)
{
    int len;
    int plaintext_len;

    EVP_CIPHER_CTX *ctx;
    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        printf("EVP_CIPHER_CTX_new\n");
        return on_error(ctx);
    }
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        printf("EVP_DecryptInit_ex\n");
        return on_error(ctx);
    }
    if(1 != EVP_DecryptUpdate(ctx, dest, &len, src, src_len))
    {
        printf("EVP_DecryptUpdate\n");
        return on_error(ctx);
    }
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, dest + len, &len))
    {
        printf("EVP_DecryptFinal_ex\n");
        ERR_print_errors_fp(stderr);
        return on_error(ctx);
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
