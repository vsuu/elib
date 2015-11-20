#ifndef EVP_SM4_H
#define EVP_SM4_H

#include <openssl\evp.h>



const EVP_CIPHER * EVP_SM4_ECB();
const EVP_CIPHER * EVP_SM4_CBC();

extern const int NID_SM4_ecb;
extern const int NID_SM4_cbc;



#endif