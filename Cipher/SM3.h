#ifndef _SM3_H
#define _SM3_H

#include <openssl\evp.h>

const EVP_MD * EVP_SM3();

extern const int NID_SM3;


#endif