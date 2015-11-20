#include "DigestCalc.h"
#include <openssl\evp.h>
#include "SM3.h"
#include <openssl\err.h>
#include <mutex>
#include "BinData.h"
#include "OnScopeExit.h"
#include "InitOpenSSL.h"

__LIB_NAME_SPACE_BEGIN__

using namespace std;

const DigestCalc::Algo DigestCalc::Algo::sha(NID_sha);
const DigestCalc::Algo DigestCalc::Algo::sha1(NID_sha1);
const DigestCalc::Algo DigestCalc::Algo::sha224(NID_sha224);
const DigestCalc::Algo DigestCalc::Algo::sha256(NID_sha256);
const DigestCalc::Algo DigestCalc::Algo::sha384(NID_sha384);
const DigestCalc::Algo DigestCalc::Algo::sha512(NID_sha512);
const DigestCalc::Algo DigestCalc::Algo::md2(NID_md2);
const DigestCalc::Algo DigestCalc::Algo::md4(NID_md4);
const DigestCalc::Algo DigestCalc::Algo::md5(NID_md5);
const DigestCalc::Algo DigestCalc::Algo::sm3(NID_SM3);
const DigestCalc::Algo DigestCalc::Algo::mdc2(NID_mdc2);
const DigestCalc::Algo DigestCalc::Algo::rc4(NID_rc4);
const DigestCalc::Algo DigestCalc::Algo::dsa(NID_dsa);
const DigestCalc::Algo DigestCalc::Algo::ecdsa(NID_ecdsa_with_SHA1);

inline static const EVP_MD *GetMD(DigestCalc::Algo algo)
{
	int ialg = static_cast<int>(algo);
	return  ialg == NID_SM3 ? EVP_SM3() : EVP_get_digestbynid(ialg);
}

class DigestCalc::Impl
{
public:
	Impl(Algo algo) :ctx_(EVP_MD_CTX_create())
	{
		if (nullptr == ctx_)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		SetAlgo(algo);
	}
	~Impl()
	{
		EVP_MD_CTX_destroy(ctx_);
	}
	Impl(const Impl&) = delete;
	Impl &operator=(const Impl&) = delete;
	void SetAlgo(Algo);
	void Init();
	void Update(const unsigned char * beg, size_t len);
	void Final(unsigned char *);
	size_t DigestLength()const
	{
		return md_->md_size;
	}
private:
	const EVP_MD *md_ = nullptr;
	EVP_MD_CTX *ctx_ = nullptr;
};

inline void DigestCalc::Impl::SetAlgo(Algo algo)
{
	if (nullptr == (md_ = GetMD(algo)))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

inline void DigestCalc::Impl::Init()
{
	if (0 == EVP_DigestInit_ex(ctx_, md_, nullptr))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

void DigestCalc::Impl::Update(const unsigned char * in, size_t inl)
{
	if (nullptr == in)
	{
		throw invalid_argument(ERR_WHERE);
	}
	if (0 == EVP_DigestUpdate(ctx_, in, inl))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

void DigestCalc::Impl::Final(unsigned char *out)
{
	if (nullptr == out)
	{
		throw invalid_argument(ERR_WHERE);
	}
	if (0 == EVP_DigestFinal_ex(ctx_, out, nullptr))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

inline DigestCalc::DigestCalc(Algo algo) : impl_(new Impl(algo))
{
}

inline DigestCalc::~DigestCalc()
{
	delete impl_;
}
inline void DigestCalc::SetAlgo(Algo algo)
{
	impl_->SetAlgo(algo);
}
inline void DigestCalc::Init()
{
	impl_->Init();
}

inline void DigestCalc::Update(const unsigned char * in, size_t inl)
{
	impl_->Update(in, inl);
}

inline void DigestCalc::Final(unsigned char *out)
{
	impl_->Final(out);
}
inline size_t DigestCalc::DigestLength()const
{
	return impl_->DigestLength();
}

BinData CalcDigest(DigestCalc::Algo algo, const BinData &data)
{
	DigestCalc calc(algo);
	calc.Init();
	calc.Update(data);
	return calc.Final();
}

size_t CalcDigest(DigestCalc::Algo algo, const BinData &data, unsigned char *out)
{
	DigestCalc calc(algo);
	calc.Init();
	calc.Update(data);
	calc.Final(out);
	return calc.DigestLength();
}

BinData CalcDigest(DigestCalc::Algo algo, const unsigned char *in, int inl)
{
	DigestCalc calc(algo);
	calc.Init();
	calc.Update(in, inl);
	return calc.Final();
}

size_t CalcDigest(DigestCalc::Algo algo, const unsigned char *in, int inl, unsigned char *out)
{
	DigestCalc calc(algo);
	calc.Init();
	calc.Update(in, inl);
	calc.Final(out);
	return calc.DigestLength();
}

size_t GetDigestLength(DigestCalc::Algo algo)
{
	return GetMD(algo)->md_size;
}

__LIB_NAME_SPACE_END__