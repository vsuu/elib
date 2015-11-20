#include "RsaCalc.h"
#include "openssl\rsa.h"
#include <stdexcept>
#include "InitOpenSSL.h"
#include "OnScopeExit.h"

using namespace std;

__LIB_NAME_SPACE_BEGIN__

const RsaPadding RsaPadding::PKCS1(1);
const RsaPadding RsaPadding::SSLV23(2);
const RsaPadding RsaPadding::NO_PADDING(3);
const RsaPadding RsaPadding::PKCS1_OAEP(4);
const RsaPadding RsaPadding::X931(5);

RsaKey::RsaKey() :native_handle_(RSA_new_method(nullptr))
{
	if (nullptr == native_handle_)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

RsaKey::~RsaKey()
{
	if (native_handle_)
	{
		RSA_free(native_handle_);
	}
}

RsaKey::RsaKey(RsaKey &&o) :native_handle_(o.native_handle_)
{
	o.native_handle_ = nullptr;
}

RsaKey & RsaKey::operator=(RsaKey &&o)
{
	swap(o.native_handle_, native_handle_);
	return *this;
}

inline BIGNUM * BN_new_Safe()
{
	auto * ret = BN_new();
	if (ret)
	{
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

RsaKey MakeRsaPubKey(const BinData &n, const BinData &e)
{
	RsaKey ret;
	rsa_st * nh = static_cast<rsa_st*>(ret);
	nh->n = BN_new_Safe();
	nh->e = BN_new_Safe();

	if ((nullptr != BN_bin2bn(n.data(), n.size(), nh->n))
		&& (nullptr != BN_bin2bn(e.data(), e.size(), nh->e))
		)
	{
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
RsaKey MakeRsaPriKey(const BinData &n, const BinData &d)
{
	RsaKey ret;
	rsa_st * nh = static_cast<rsa_st*>(ret);
	nh->n = BN_new_Safe();
	nh->d = BN_new_Safe();

	if ((nullptr != BN_bin2bn(n.data(), n.size(), nh->n))
		&& (0 != BN_bin2bn(d.data(), d.size(), nh->d))
		)
	{
		nh->flags |= RSA_FLAG_NO_BLINDING;//取消时间攻击保护，此模式需要e,而这里没有
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
RsaKey MakeRsaPriKey_CRT(const BinData &p, const BinData &q, const BinData &dp, const BinData &dq, const BinData &qinv)
{
	RsaKey ret;
	rsa_st * nh = static_cast<rsa_st*>(ret);
	nh->n = BN_new_Safe();
	nh->p = BN_new_Safe();
	nh->q = BN_new_Safe();
	nh->dmp1 = BN_new_Safe();
	nh->dmq1 = BN_new_Safe();
	nh->iqmp = BN_new_Safe();

	BN_CTX *ctx = BN_CTX_new();
	if (nullptr == ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([&]{BN_CTX_free(ctx); });

	if ((0 != BN_bin2bn(p.data(), p.size(), nh->p))
		&& (0 != BN_bin2bn(q.data(), q.size(), nh->q))
		&& (0 != BN_bin2bn(dp.data(), dp.size(), nh->dmp1))
		&& (0 != BN_bin2bn(dq.data(), dq.size(), nh->dmq1))
		&& (0 != BN_bin2bn(qinv.data(), qinv.size(), nh->iqmp))
		&& (1 == BN_mul(nh->n, nh->p, nh->q, ctx))
		)
	{
		nh->flags |= RSA_FLAG_NO_BLINDING;//取消时间攻击保护，此模式需要e
		nh->flags |= RSA_FLAG_EXT_PKEY;//开启crt模式
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
RsaKey MakeRsaKey(const BinData &n, const BinData &e, const BinData &d)
{
	RsaKey ret;
	rsa_st * nh = static_cast<rsa_st*>(ret);
	nh->n = BN_new_Safe();
	nh->d = BN_new_Safe();
	nh->e = BN_new_Safe();

	if ((0 != BN_bin2bn(n.data(), n.size(), nh->n))
		&& (0 != BN_bin2bn(d.data(), d.size(), nh->d))
		&& (0 != BN_bin2bn(e.data(), e.size(), nh->e))
		)
	{
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
RsaKey MakeRsaKey(const BinData &n, const BinData &e, const BinData &p, const BinData &q, const BinData &dp, const BinData &dq, const BinData &qinv)
{
	RsaKey ret;
	rsa_st * nh = static_cast<rsa_st*>(ret);
	nh->n = BN_new_Safe();
	nh->e = BN_new_Safe();
	nh->p = BN_new_Safe();
	nh->q = BN_new_Safe();
	nh->dmp1 = BN_new_Safe();
	nh->dmq1 = BN_new_Safe();
	nh->iqmp = BN_new_Safe();

	if ((0 != BN_bin2bn(n.data(), n.size(), nh->n))
		&& (0 != BN_bin2bn(e.data(), e.size(), nh->e))
		&& (0 != BN_bin2bn(p.data(), p.size(), nh->p))
		&& (0 != BN_bin2bn(q.data(), q.size(), nh->q))
		&& (0 != BN_bin2bn(dp.data(), dp.size(), nh->dmp1))
		&& (0 != BN_bin2bn(dq.data(), dq.size(), nh->dmq1))
		&& (0 != BN_bin2bn(qinv.data(), qinv.size(), nh->iqmp))
		)
	{
		nh->flags |= RSA_FLAG_EXT_PKEY;//开启crt模式
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
RsaKey MakeRsaKey_Random(int bitcount, unsigned long e)
{
	auto nh = RSA_generate_key(bitcount, e, nullptr, nullptr);
	if (nh)
	{
		return RsaKey(nh);
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

void RSA_PublicEncrypt(RsaKey &key, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding)
{
	if (-1 == RSA_public_encrypt(inl, in, out, static_cast<rsa_st*>(key), static_cast<int>(padding)))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
void RSA_PrivateEncrypt(RsaKey &key, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding)
{
	if (-1 == RSA_private_encrypt(inl, in, out, static_cast<rsa_st*>(key), static_cast<int>(padding)))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
void RSA_PublicDecrypt(RsaKey &key, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding)
{
	if (-1 == RSA_public_decrypt(inl, in, out, static_cast<rsa_st*>(key), static_cast<int>(padding)))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
void RSA_PrivateDecrypt(RsaKey &key, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding)
{
	if (-1 == RSA_private_decrypt(inl, in, out, static_cast<rsa_st*>(key), static_cast<int>(padding)))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

void GetRsaKeyInfo_Fill(HexData *x, BIGNUM *y)
{
	if (nullptr != x)
	{
		if (nullptr != y)
		{
			char *tmp = BN_bn2hex(y);
			x->assign(tmp);
			OPENSSL_free(tmp);
		}
		else
		{
			x->clear();
		}
	}
}

size_t RsaKey::GetBitCount()
{
	if (nullptr == native_handle_)
	{
		throw runtime_error(ERR_WHERE "不存在密钥");
	}
	return RSA_size(native_handle_);
}

void RsaKey::GetKeyInfo(HexData * n, HexData * e, HexData * d, HexData * p, HexData * q, HexData * dp, HexData * dq, HexData * qinv)
{
	if (nullptr == native_handle_)
	{
		throw runtime_error(ERR_WHERE "不存在密钥");
	}
	auto nh = native_handle_;

	GetRsaKeyInfo_Fill(n, nh->n);
	GetRsaKeyInfo_Fill(e, nh->e);
	GetRsaKeyInfo_Fill(d, nh->d);
	GetRsaKeyInfo_Fill(p, nh->p);
	GetRsaKeyInfo_Fill(q, nh->q);
	GetRsaKeyInfo_Fill(dp, nh->dmp1);
	GetRsaKeyInfo_Fill(dq, nh->dmq1);
	GetRsaKeyInfo_Fill(qinv, nh->iqmp);
}
__LIB_NAME_SPACE_END__