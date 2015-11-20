#include "EVP_SM4.h"

#include "sm4.h"
#include "string.h"




const int NID_SM4_ecb = 1002;
const int NID_SM4_cbc = 1003;


const int SM4_BLOCK_SIZE=16;
const int SM4_KEY_LEN=16;
const int SM4_IV_LEN=16;


static int SM4_ECB_init(EVP_CIPHER_CTX *ctx, const unsigned char *key,
		    const unsigned char *iv, int enc)
{
	sm4_context * context = (sm4_context *)(ctx->cipher_data);
	if(0!=enc)
	{
		sm4_setkey_enc(context,const_cast<unsigned char *>(key));
	}
	else
	{
		sm4_setkey_dec(context,const_cast<unsigned char *>(key));
	}
	return 1;
}

static int SM4_ECB_do_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
			 const unsigned char *in, size_t inl)
{
	sm4_crypt_ecb((sm4_context *)(ctx->cipher_data),inl,const_cast<unsigned char *>(in),out);
	return inl;
}
/*
static int SM4_ECB_cleanup(EVP_CIPHER_CTX *ctx)
{
	return 1;
}*/


static int SM4_CBC_init(EVP_CIPHER_CTX *ctx, const unsigned char *key,
		    const unsigned char *iv, int enc)
{
	sm4_context * context = (sm4_context *)(ctx->cipher_data);
	if(0!=enc)
	{
		sm4_setkey_enc(context,const_cast<unsigned char *>(key));
	}
	else
	{
		sm4_setkey_dec(context,const_cast<unsigned char *>(key));
	}
//	memcpy(ctx->iv,iv,SM4_IV_LEN); //EVP已经封装处理了
	return 1;
}

static int SM4_CBC_do_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
			 const unsigned char *in, size_t inl)
{
	sm4_crypt_cbc((sm4_context *)(ctx->cipher_data),inl,ctx->iv,const_cast<unsigned char *>(in),out);
	return inl;
}

/*
static int SM4_CBC_cleanup(EVP_CIPHER_CTX *ctx)
{
	return 1;
}*/

static const EVP_CIPHER sm4_ecb={
	NID_SM4_ecb,
	SM4_BLOCK_SIZE,
	SM4_KEY_LEN,		
	0,
	EVP_CIPH_ECB_MODE,
	SM4_ECB_init,
	SM4_ECB_do_cipher,
	NULL,//SM4_ECB_cleanup,
	sizeof(sm4_context),
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

static const EVP_CIPHER sm4_cbc={
	NID_SM4_cbc,
	SM4_BLOCK_SIZE,
	SM4_KEY_LEN,		
	SM4_IV_LEN,
	EVP_CIPH_CBC_MODE,
	SM4_CBC_init,
	SM4_CBC_do_cipher,
	NULL,//SM4_CBC_cleanup,
	sizeof(sm4_context),
	nullptr,
	nullptr,
	nullptr,
	nullptr
};



const EVP_CIPHER * EVP_SM4_ECB()
{
	return &sm4_ecb;
}

const EVP_CIPHER * EVP_SM4_CBC()
{
	return &sm4_cbc;
}