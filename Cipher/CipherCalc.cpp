#include "CipherCalc.h"
#include <openssl\evp.h>
#include "EVP_sm4.h"
#include <string>
#include <sstream>
#include <openssl\err.h>
#include <mutex>
#include <stdexcept>
#include "InitOpenSSL.h"

__LIB_NAME_SPACE_BEGIN__

const CipherCalc::Algo CipherCalc::Algo::sm4_ecb(NID_SM4_ecb);
const CipherCalc::Algo CipherCalc::Algo::sm4_cbc(NID_SM4_cbc);
const CipherCalc::Algo CipherCalc::Algo::des_cfb(NID_des_cfb64);
const CipherCalc::Algo CipherCalc::Algo::des_cfb1(NID_des_cfb1);
const CipherCalc::Algo CipherCalc::Algo::des_cfb8(NID_des_cfb8);
const CipherCalc::Algo CipherCalc::Algo::des_ede_cfb(NID_des_ede_cfb64);
const CipherCalc::Algo CipherCalc::Algo::des_ede3_cfb(NID_des_ede3_cfb64);
const CipherCalc::Algo CipherCalc::Algo::des_ede3_cfb1(NID_des_ede3_cfb1);
const CipherCalc::Algo CipherCalc::Algo::des_ede3_cfb8(NID_des_ede3_cfb8);
const CipherCalc::Algo CipherCalc::Algo::des_ofb(NID_des_ofb64);
const CipherCalc::Algo CipherCalc::Algo::des_ede_ofb(NID_des_ede_ofb64);
const CipherCalc::Algo CipherCalc::Algo::des_ede3_ofb(NID_des_ede3_ofb64);
const CipherCalc::Algo CipherCalc::Algo::desx_cbc(NID_desx_cbc);
const CipherCalc::Algo CipherCalc::Algo::des_cbc(NID_des_cbc);
const CipherCalc::Algo CipherCalc::Algo::des_ede_cbc(NID_des_ede_cbc);
const CipherCalc::Algo CipherCalc::Algo::des_ede3_cbc(NID_des_ede3_cbc);
const CipherCalc::Algo CipherCalc::Algo::des_ecb(NID_des_ecb);
const CipherCalc::Algo CipherCalc::Algo::des_ede(NID_des_ede_ecb);
const CipherCalc::Algo CipherCalc::Algo::des_ede3(NID_des_ede3_ecb);
const CipherCalc::Algo CipherCalc::Algo::rc4(NID_rc4);
const CipherCalc::Algo CipherCalc::Algo::rc4_40(NID_rc4_40);
const CipherCalc::Algo CipherCalc::Algo::rc4_hmac_md5(NID_rc4_hmac_md5);
const CipherCalc::Algo CipherCalc::Algo::idea_ecb(NID_idea_ecb);
const CipherCalc::Algo CipherCalc::Algo::idea_cfb(NID_idea_cfb64);
const CipherCalc::Algo CipherCalc::Algo::idea_ofb(NID_idea_ofb64);
const CipherCalc::Algo CipherCalc::Algo::idea_cbc(NID_idea_cbc);
const CipherCalc::Algo CipherCalc::Algo::seed_ecb(NID_seed_ecb);
const CipherCalc::Algo CipherCalc::Algo::seed_cfb(NID_seed_cfb128);
const CipherCalc::Algo CipherCalc::Algo::seed_ofb(NID_seed_ofb128);
const CipherCalc::Algo CipherCalc::Algo::seed_cbc(NID_seed_cbc);
const CipherCalc::Algo CipherCalc::Algo::rc2_ecb(NID_rc2_ecb);
const CipherCalc::Algo CipherCalc::Algo::rc2_cfb(NID_rc2_cfb64);
const CipherCalc::Algo CipherCalc::Algo::rc2_ofb(NID_rc2_ofb64);
const CipherCalc::Algo CipherCalc::Algo::rc2_cbc(NID_rc2_cbc);
const CipherCalc::Algo CipherCalc::Algo::rc2_40_cbc(NID_rc2_40_cbc);
const CipherCalc::Algo CipherCalc::Algo::rc2_64_cbc(NID_rc2_64_cbc);
const CipherCalc::Algo CipherCalc::Algo::bf_ecb(NID_bf_ecb);
const CipherCalc::Algo CipherCalc::Algo::bf_cfb(NID_bf_cfb64);
const CipherCalc::Algo CipherCalc::Algo::bf_ofb(NID_bf_ofb64);
const CipherCalc::Algo CipherCalc::Algo::bf_cbc(NID_bf_cbc);
const CipherCalc::Algo CipherCalc::Algo::cast5_ecb(NID_cast5_ecb);
const CipherCalc::Algo CipherCalc::Algo::cast5_cfb(NID_cast5_cfb64);
const CipherCalc::Algo CipherCalc::Algo::cast5_ofb(NID_cast5_ofb64);
const CipherCalc::Algo CipherCalc::Algo::cast5_cbc(NID_cast5_cbc);
const CipherCalc::Algo CipherCalc::Algo::rc5_32_12_16_ecb(NID_rc5_ecb);
const CipherCalc::Algo CipherCalc::Algo::rc5_32_12_16_cfb(NID_rc5_cfb64);
const CipherCalc::Algo CipherCalc::Algo::rc5_32_12_16_ofb(NID_rc5_ofb64);
const CipherCalc::Algo CipherCalc::Algo::rc5_32_12_16_cbc(NID_rc5_cbc);
const CipherCalc::Algo CipherCalc::Algo::aes_128_ecb(NID_aes_128_ecb);
const CipherCalc::Algo CipherCalc::Algo::aes_128_cbc(NID_aes_128_cbc);
const CipherCalc::Algo CipherCalc::Algo::aes_128_cfb(NID_aes_128_cfb128);
const CipherCalc::Algo CipherCalc::Algo::aes_128_cfb1(NID_aes_128_cfb1);
const CipherCalc::Algo CipherCalc::Algo::aes_128_cfb8(NID_aes_128_cfb8);
const CipherCalc::Algo CipherCalc::Algo::aes_128_ofb(NID_aes_128_ofb128);
const CipherCalc::Algo CipherCalc::Algo::aes_128_ctr(NID_aes_128_ctr);
const CipherCalc::Algo CipherCalc::Algo::aes_128_gcm(NID_aes_128_gcm);
const CipherCalc::Algo CipherCalc::Algo::aes_128_xts(NID_aes_128_xts);
const CipherCalc::Algo CipherCalc::Algo::aes_192_ecb(NID_aes_192_ecb);
const CipherCalc::Algo CipherCalc::Algo::aes_192_cbc(NID_aes_192_cbc);
const CipherCalc::Algo CipherCalc::Algo::aes_192_cfb(NID_aes_192_cfb128);
const CipherCalc::Algo CipherCalc::Algo::aes_192_cfb1(NID_aes_192_cfb1);
const CipherCalc::Algo CipherCalc::Algo::aes_192_cfb8(NID_aes_192_cfb8);
const CipherCalc::Algo CipherCalc::Algo::aes_192_ofb(NID_aes_192_ofb128);
const CipherCalc::Algo CipherCalc::Algo::aes_192_ctr(NID_aes_192_ctr);
const CipherCalc::Algo CipherCalc::Algo::aes_192_gcm(NID_aes_192_gcm);
const CipherCalc::Algo CipherCalc::Algo::aes_256_ecb(NID_aes_256_ecb);
const CipherCalc::Algo CipherCalc::Algo::aes_256_cbc(NID_aes_256_cbc);
const CipherCalc::Algo CipherCalc::Algo::aes_256_cfb(NID_aes_256_cfb128);
const CipherCalc::Algo CipherCalc::Algo::aes_256_cfb1(NID_aes_256_cfb1);
const CipherCalc::Algo CipherCalc::Algo::aes_256_cfb8(NID_aes_256_cfb8);
const CipherCalc::Algo CipherCalc::Algo::aes_256_ofb(NID_aes_256_ofb128);
const CipherCalc::Algo CipherCalc::Algo::aes_256_ctr(NID_aes_256_ctr);
const CipherCalc::Algo CipherCalc::Algo::aes_256_gcm(NID_aes_256_gcm);
const CipherCalc::Algo CipherCalc::Algo::aes_256_xts(NID_aes_256_xts);
const CipherCalc::Algo CipherCalc::Algo::camellia_128_ecb(NID_camellia_128_ecb);
const CipherCalc::Algo CipherCalc::Algo::camellia_128_cbc(NID_camellia_128_cbc);
const CipherCalc::Algo CipherCalc::Algo::camellia_128_cfb(NID_camellia_128_cfb128);
const CipherCalc::Algo CipherCalc::Algo::camellia_128_cfb1(NID_camellia_128_cfb1);
const CipherCalc::Algo CipherCalc::Algo::camellia_128_cfb8(NID_camellia_128_cfb8);
const CipherCalc::Algo CipherCalc::Algo::camellia_128_ofb(NID_camellia_128_ofb128);
const CipherCalc::Algo CipherCalc::Algo::camellia_192_ecb(NID_camellia_192_ecb);
const CipherCalc::Algo CipherCalc::Algo::camellia_192_cbc(NID_camellia_192_cbc);
const CipherCalc::Algo CipherCalc::Algo::camellia_192_cfb(NID_camellia_192_cfb128);
const CipherCalc::Algo CipherCalc::Algo::camellia_192_cfb1(NID_camellia_192_cfb1);
const CipherCalc::Algo CipherCalc::Algo::camellia_192_cfb8(NID_camellia_192_cfb8);
const CipherCalc::Algo CipherCalc::Algo::camellia_192_ofb(NID_camellia_192_ofb128);
const CipherCalc::Algo CipherCalc::Algo::camellia_256_ecb(NID_camellia_256_ecb);
const CipherCalc::Algo CipherCalc::Algo::camellia_256_cbc(NID_camellia_256_cbc);
const CipherCalc::Algo CipherCalc::Algo::camellia_256_cfb(NID_camellia_256_cfb128);
const CipherCalc::Algo CipherCalc::Algo::camellia_256_cfb1(NID_camellia_256_cfb1);
const CipherCalc::Algo CipherCalc::Algo::camellia_256_cfb8(NID_camellia_256_cfb8);
const CipherCalc::Algo CipherCalc::Algo::camellia_256_ofb(NID_camellia_256_ofb128);

const CipherCalc::ENCMODE CipherCalc::ENCMODE::ENCRYPT(1);
const CipherCalc::ENCMODE CipherCalc::ENCMODE::DECRYPT(0);
const CipherCalc::PADMODE CipherCalc::PADMODE::NO_PADDING(0);
const CipherCalc::PADMODE CipherCalc::PADMODE::OPENSSL_PADDING(1);

using namespace std;

static const EVP_CIPHER * GetCipher(CipherCalc::Algo algo)
{
	int alg_id = static_cast<int>(algo);

	if (alg_id == NID_SM4_ecb)
	{
		return EVP_SM4_ECB();
	}
	else if (alg_id == NID_SM4_cbc)
	{
		return EVP_SM4_CBC();
	}
	else
	{
		return EVP_get_cipherbynid(alg_id);
	}
}

class CipherCalc::Impl
{
public:
	Impl(Algo algo) :ctx_(EVP_CIPHER_CTX_new())
	{
		if (nullptr == ctx_)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		SetAlgo(algo);
	}
	~Impl()
	{
		EVP_CIPHER_CTX_free(ctx_);
	}
	Impl(const Impl &) = delete;
	Impl &operator=(const Impl &) = delete;
	void SetAlgo(Algo alg)
	{
		if (nullptr == (cipher_ = GetCipher(alg)))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}
	void Init(const unsigned char *key, const unsigned char *iv, int enc, int padding);
	void Update(const unsigned char *in, int inl, unsigned char *out, int *outl);
	void Final(unsigned char *out, int *outl);
private:
	EVP_CIPHER_CTX * ctx_ = nullptr;
	const EVP_CIPHER * cipher_;
	void CheckCtx()
	{
		if (nullptr == ctx_)
		{
			throw logic_error(ERR_WHERE "ÇëÏÈÖ´ÐÐCipherCalc::SetAlgo");
		}
	}
};

inline void CipherCalc::Impl::Update(const unsigned char *in, int inl, unsigned char *out, int *outl)
{
	if (0 == EVP_CipherUpdate(ctx_, out, outl, in, inl))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

inline void CipherCalc::Impl::Final(unsigned char *out, int *outl)
{
	if (0 == EVP_CipherFinal_ex(ctx_, out, outl))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

inline void  CipherCalc::Impl::Init(const unsigned char *key, const unsigned char *iv, int enc, int padding)
{
	if (0 == EVP_CipherInit_ex(ctx_, cipher_, NULL, key, iv, enc))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	EVP_CIPHER_CTX_set_padding(ctx_, padding);
}

inline CipherCalc::CipherCalc(Algo algo)
	:impl_(new Impl(algo))
{
}

inline CipherCalc::~CipherCalc()
{
	delete impl_;
}
inline void CipherCalc::SetAlgo(Algo algo)
{
	impl_->SetAlgo(algo);
}
inline void CipherCalc::Init(unsigned char *key, const unsigned char *iv, ENCMODE enc, PADMODE padding)
{
	impl_->Init(key, iv, enc, padding);
}
inline unsigned char * CipherCalc::Update(const unsigned char *in, int inl, unsigned char *out)
{
	int outl = 0;
	impl_->Update(in, inl, out, &outl);
	return out + outl;
}
inline unsigned char * CipherCalc::Final(unsigned char *out)
{
	int outl = 0;
	impl_->Final(out, &outl);
	return out + outl;
}

__LIB_NAME_SPACE_END__