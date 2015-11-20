#include "SM2Op.h"
#include <openssl\ec.h>
#include <stdexcept>
#include "InitOpenSSL.h"
#include "OnScopeExit.h"

using namespace std;

__LIB_NAME_SPACE_BEGIN__

const ECTYPE ECTYPE::GF2m(0);
const ECTYPE ECTYPE::GFp(1);

const SM2Cipher::ENCMODE SM2Cipher::ENCMODE::ENCRYPT(1);
const SM2Cipher::ENCMODE SM2Cipher::ENCMODE::DECRYPT(0);

static const SM2Group recommand_sm2_group(ECTYPE::GFp,
	Hex2Bin("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"),
	Hex2Bin("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC"),
	Hex2Bin("28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93"),
	Hex2Bin("FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123"),
	Hex2Bin("32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7"),
	Hex2Bin("BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0")
	);


const SM2Group & DefaultSM2Group()
{
	return recommand_sm2_group;
}



void KDF::GetK(unsigned char *out, unsigned int len)
{
	if (!Buff_.empty())
	{
		if (Buff_.size() <= len)
		{
			copy(Buff_.begin(), Buff_.end(), out);
			out += Buff_.size();
			len -= Buff_.size();
			Buff_.clear();
		}
		else
		{
			copy_n(Buff_.begin(), len, out);
			Buff_.erase(Buff_.begin(), Buff_.begin() + len);
			return;
		}
	}

	auto hashlen = hasher_.DigestLength();
	while (len != 0)
	{
		unsigned char net_ct[4];
		net_ct[0] = ct_ >> 24;
		net_ct[1] = ct_ >> 16;
		net_ct[2] = ct_ >> 8;
		net_ct[3] = ct_;

		hasher_.Init();
		hasher_.Update(Z_);
		hasher_.Update(net_ct, 4);

		if (len >= hashlen)
		{
			hasher_.Final(out);
			out += hashlen;
			len -= hashlen;
		}
		else
		{
			Buff_.resize(hashlen);
			hasher_.Final(Buff_.data());

			copy_n(Buff_.begin(), len, out);
			Buff_.erase(Buff_.begin(), Buff_.begin() + len);
			len = 0;
		}
		++ct_;
	}
}

void Rander::rand(const BIGNUM *bn_n, BIGNUM *bn_k)
{
	do
	{
		if (0 == BN_rand_range(bn_k, bn_n))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	} while (BN_is_zero(bn_k));
}
void BN2HEX(const BIGNUM *bn, HexData &hex)
{
	char *tmp = BN_bn2hex(bn);
	if (NULL == tmp)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	hex.assign(tmp);
	OPENSSL_free(tmp);
}

//SM2Group::SM2Group(const SM2Group & other) :raw_pointer_(other.raw_pointer_ ? EC_GROUP_dup(other.raw_pointer_) : NULL), type_(other.type_)
//{
//	if (NULL != other.raw_pointer_ && NULL == raw_pointer_)
//	{
//		throw SM2Exception(GetOpensslErrInfo());
//	}
//}
SM2Group::~SM2Group()
{
	if (raw_pointer_)
	{
		EC_GROUP_free(raw_pointer_);
	}
}
//SM2Group &SM2Group::operator=(const SM2Group& other)
//{
//	if (this != &other)
//	{
//		type_ = other.type_;
//
//		if (NULL != other.raw_pointer_)
//		{
//			if (NULL != raw_pointer_)
//			{
//				if (0 == EC_GROUP_copy(raw_pointer_, other.raw_pointer_))
//				{
//					throw SM2Exception(GetOpensslErrInfo());
//				}
//			}
//			else
//			{
//				raw_pointer_ = EC_GROUP_dup(other.raw_pointer_);
//				if (NULL == raw_pointer_)
//				{
//					throw SM2Exception(GetOpensslErrInfo());
//				}
//			}
//		}
//		else
//		{
//			if (NULL != raw_pointer_)
//			{
//				EC_GROUP_free(raw_pointer_);
//				raw_pointer_ = NULL;
//			}
//		}
//	}
//	return *this;
//}
namespace
{
	inline BIGNUM * Bin2BN(BN_CTX *bn_ctx,const BinData &data){
		BIGNUM *ret = BN_CTX_get(bn_ctx);
		if (ret && (0 != BN_bin2bn(data.data(), data.size(), ret)))
		{
			return ret;
		}
		else
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	};

	inline BIGNUM * GetBN_Safe(BN_CTX *bn_ctx)
	{
		auto ret = BN_CTX_get(bn_ctx);
		if (ret)
		{
			return ret;
		}
		else
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	};
}

SM2Group::SM2Group(ECTYPE ectype, const BinData &p, const BinData &a, const BinData &b, const BinData &n, const BinData &Gx, const BinData &Gy, const BinData &h)
	:type_(ectype), raw_pointer_(nullptr)
{
	BN_CTX *bn_ctx = BN_CTX_new();
	if (NULL == bn_ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([&]{BN_CTX_free(bn_ctx);});

	BIGNUM *bn_p = Bin2BN(bn_ctx,p);
	BIGNUM *bn_a = Bin2BN(bn_ctx, a);
	BIGNUM *bn_b = Bin2BN(bn_ctx, b);
	BIGNUM *bn_Gx = Bin2BN(bn_ctx, Gx);
	BIGNUM *bn_Gy = Bin2BN(bn_ctx, Gy);
	BIGNUM *bn_n = Bin2BN(bn_ctx, n);
	BIGNUM *bn_h = Bin2BN(bn_ctx, h);
	
	do
	{
		if (ectype == ECTYPE::GFp)
		{
			raw_pointer_ = EC_GROUP_new_curve_GFp(bn_p, bn_a, bn_b, bn_ctx);
		}
		else
		{
			raw_pointer_ = EC_GROUP_new_curve_GF2m(bn_p, bn_a, bn_b, bn_ctx);
		}

		if (nullptr == raw_pointer_)
		{
			break;
		}

		ScopeGuard raw_pointer_releaser([&]{EC_GROUP_free(raw_pointer_); raw_pointer_ = nullptr; });

		EC_POINT *_G = EC_POINT_new(raw_pointer_);
		if (nullptr == _G)
		{
			break;
		}

		ON_SCOPE_EXIT([&_G]{EC_POINT_free(_G); });

		if (ectype == ECTYPE::GFp)
		{
			if (0 == EC_POINT_set_affine_coordinates_GFp(raw_pointer_, _G, bn_Gx, bn_Gy, bn_ctx))
			{
				break;
			}
		}
		else
		{
			if (0 == EC_POINT_set_affine_coordinates_GF2m(raw_pointer_, _G, bn_Gx, bn_Gy, bn_ctx))
			{
				break;
			}
		}

		if (0 == EC_GROUP_set_generator(raw_pointer_, _G, bn_n, bn_h))//设置G及n、h
		{
			break;
		}

		if (0 == EC_GROUP_check(raw_pointer_, bn_ctx))
		{
			break;
		}

		raw_pointer_releaser.dismiss();
		return;
	}while(false);

	throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
}

SM2Key SM2Group::GenerateRandomKey()const//生成一个随机密钥
{
	SM2Key ret(GenEmptyKey(), type_);
	if (0 != EC_KEY_generate_key(ret.key_))
	{
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

EC_KEY * SM2Group::GenEmptyKey()const
{
	EC_KEY *ret = EC_KEY_new();
	if (nullptr != ret && 0 != EC_KEY_set_group(ret, raw_pointer_))
	{
		return ret;
	}
	else
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

void SM2Group::SetPublicKey(EC_KEY *key, const BinData &Px, const BinData &Py)const
{
	BN_CTX *bn_ctx = BN_CTX_new();
	if (nullptr == bn_ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([bn_ctx]{BN_CTX_free(bn_ctx); });
	BIGNUM *bn_px = Bin2BN(bn_ctx, Px);
	BIGNUM *bn_py=Bin2BN(bn_ctx,Py);

	EC_POINT *_P = EC_POINT_new(raw_pointer_);
	if (nullptr == _P)
	{
		throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([_P]{EC_POINT_free(_P); });

	if (type_ == ECTYPE::GFp)
	{
		if (0 == EC_POINT_set_affine_coordinates_GFp(raw_pointer_, _P, bn_px, bn_py, bn_ctx))
		{
			throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
		}
	}
	else
	{
		if (0 == EC_POINT_set_affine_coordinates_GF2m(raw_pointer_, _P, bn_px, bn_py, bn_ctx))
		{
			throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
		}
	}

	if (0 == EC_KEY_set_public_key(key, _P))
	{
		throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
	}
}
void SM2Group::SetPrivateKey(EC_KEY *key_, const BinData &Priv)const
{
	BIGNUM *bn_D = BN_new();
	if (nullptr == bn_D)
	{
		throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([bn_D]{BN_free(bn_D); });
	if (0 == BN_bin2bn(Priv.data(), Priv.size(), bn_D))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	if (0 == EC_KEY_set_private_key(key_, bn_D))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}

SM2Key SM2Group::GenPublicKey(const BinData &Px, const BinData &Py)const
{
	SM2Key ret(GenEmptyKey(), type_);
	SetPublicKey(ret.key_, Px, Py);
	return ret;
}
SM2Key SM2Group::GenPrivateKey(const BinData &Priv)const
{
	SM2Key ret(GenEmptyKey(), type_);
	SetPrivateKey(ret.key_, Priv);
	return ret;
}
SM2Key SM2Group::GenKey(const BinData &Px, const BinData &Py, const BinData &Priv)const
{
	SM2Key ret(GenEmptyKey(), type_);
	SetPublicKey(ret.key_, Px, Py);
	SetPrivateKey(ret.key_, Priv);
	return ret;
}

inline const EC_GROUP *SM2Key::GetGroupPointer()const
{
	return EC_KEY_get0_group(key_);
}

SM2Key::~SM2Key()
{
	if (nullptr != key_)
	{
		EC_KEY_free(key_);
	}
}/*
SM2Key::SM2Key(const SM2Key &other) :type_(other.type_), key_(other.key_ ? EC_KEY_dup(other.key_) : NULL)
{
	if (NULL != other.key_ && NULL == key_)
	{
		throw SM2Exception(GetOpensslErrInfo());
	}
}
SM2Key &SM2Key::operator=(const SM2Key &other)
{
	if (this != &other)
	{
		type_ = other.type_;
		if (NULL != other.key_)
		{
			if (NULL != key_)
			{
				if (NULL == EC_KEY_copy(key_, other.key_))
				{
					throw SM2Exception(GetOpensslErrInfo());
				}
			}
			else
			{
				key_ = EC_KEY_dup(other.key_);
				if (NULL == key_)
				{
					throw SM2Exception(GetOpensslErrInfo());
				}
			}
		}
		else
		{
			if (NULL != key_)
			{
				EC_KEY_free(key_);
				key_ = NULL;
			}
		}
	}
	return *this;
}*/
void SM2Key::GetKeyInfo(HexData *Px, HexData *Py, HexData *D)const
{
	if (nullptr == key_)
	{
		return;
	}

	const BIGNUM * priv = EC_KEY_get0_private_key(key_);
	if (nullptr != priv && nullptr != D)
	{
		BN2HEX(priv, *D);
	}

	const EC_POINT *_P = EC_KEY_get0_public_key(key_);
	if (nullptr != _P && (nullptr != Px || nullptr != Py))
	{
		BIGNUM *bn_px, *bn_py;
		bn_px = BN_new();
		if (nullptr == bn_px)
		{
			throw runtime_error(ERR_WHERE+ GetOpensslErrInfo());
		}
		ON_SCOPE_EXIT([bn_px]{BN_free(bn_px); });
		bn_py = BN_new();
		if (nullptr == bn_py)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		ON_SCOPE_EXIT([bn_py]{BN_free(bn_py); });
		if (type_ == ECTYPE::GFp)
		{
			if (0 == EC_POINT_get_affine_coordinates_GFp(EC_KEY_get0_group(key_), _P, bn_px, bn_py, nullptr))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
		}
		else
		{
			if (0 == EC_POINT_get_affine_coordinates_GF2m(GetGroupPointer(), _P, bn_px, bn_py, nullptr))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
		}

		if (nullptr != Px)
		{
			BN2HEX(bn_px, *Px);
		}

		if (nullptr != Py)
		{
			BN2HEX(bn_py, *Py);
			
		}
	}
}

BinData SM2Key::CalcZa(const BinData &in, DigestCalc::Algo hash_id)const
{
	const EC_POINT *_P = EC_KEY_get0_public_key(key_);
	if (NULL == _P)
	{
		throw runtime_error("所用的密钥不是公钥,无法计算Z值！");
	}

	const EC_GROUP *_group = GetGroupPointer();

	BN_CTX *bn_ctx = BN_CTX_new();
	if (NULL == bn_ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{BN_CTX_free(bn_ctx); });

	const EC_POINT *_G = EC_GROUP_get0_generator(_group);
	if (NULL == _G)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	

	BIGNUM *p = GetBN_Safe(bn_ctx);
	BIGNUM *a = GetBN_Safe(bn_ctx);
	BIGNUM *b = GetBN_Safe(bn_ctx);
	BIGNUM *Gx = GetBN_Safe(bn_ctx);
	BIGNUM *Gy = GetBN_Safe(bn_ctx);
	BIGNUM *Px = GetBN_Safe(bn_ctx);
	BIGNUM *Py = GetBN_Safe(bn_ctx);
	

	if (type_ == ECTYPE::GFp)
	{
		if (0 == EC_GROUP_get_curve_GFp(_group, p, a, b, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}
	else
	{
		if (0 == EC_GROUP_get_curve_GF2m(_group, p, a, b, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}

	if (type_ == ECTYPE::GFp)
	{
		if (0 == EC_POINT_get_affine_coordinates_GFp(_group, _G, Gx, Gy, bn_ctx) || 0 == EC_POINT_get_affine_coordinates_GFp(_group, _P, Px, Py, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}
	else
	{
		if (0 == EC_POINT_get_affine_coordinates_GF2m(_group, _G, Gx, Gy, bn_ctx) || 0 == EC_POINT_get_affine_coordinates_GF2m(_group, _P, Px, Py, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}

	//计算Za
	BinData buff;
	unsigned int user_id_bits = in.size() << 3;
	buff.push_back(user_id_bits >> 8);
	buff.push_back(user_id_bits & 0xFF);
	buff += in;

	int len = 0;
	unsigned char tmp[2048];
	if ((EC_GROUP_get_degree(_group) >> 3) > 2048)
	{
		throw runtime_error("密钥长度太长，支持不了！");
	}
	if (type_ == ECTYPE::GFp)
	{
		len = BN_bn2bin(a, tmp);
		buff.insert(buff.end(), tmp, tmp + len);
		len = BN_bn2bin(b, tmp);
		buff.insert(buff.end(), tmp, tmp + len);
		len = BN_bn2bin(Gx, tmp);
		buff.insert(buff.end(), tmp, tmp + len);
		len = BN_bn2bin(Gy, tmp);
		buff.insert(buff.end(), tmp, tmp + len);
		len = BN_bn2bin(Px, tmp);
		buff.insert(buff.end(), tmp, tmp + len);
		len = BN_bn2bin(Py, tmp);
		buff.insert(buff.end(), tmp, tmp + len);
	}
	else
	{
		vector<unsigned char> GF2mBuff(BN_num_bytes(p));
		len = BN_bn2bin(a, tmp);
		memset(GF2mBuff.data(), 0x00, GF2mBuff.size());
		memcpy(GF2mBuff.data() + GF2mBuff.size() - len, tmp, len);
		buff.insert(buff.end(), GF2mBuff.begin(), GF2mBuff.end());
		len = BN_bn2bin(b, tmp);
		memset(GF2mBuff.data(), 0x00, GF2mBuff.size());
		memcpy(GF2mBuff.data() + GF2mBuff.size() - len, tmp, len);
		buff.insert(buff.end(), GF2mBuff.begin(), GF2mBuff.end());
		len = BN_bn2bin(Gx, tmp);
		memset(GF2mBuff.data(), 0x00, GF2mBuff.size());
		memcpy(GF2mBuff.data() + GF2mBuff.size() - len, tmp, len);
		buff.insert(buff.end(), GF2mBuff.begin(), GF2mBuff.end());
		len = BN_bn2bin(Gy, tmp);
		memset(GF2mBuff.data(), 0x00, GF2mBuff.size());
		memcpy(GF2mBuff.data() + GF2mBuff.size() - len, tmp, len);
		buff.insert(buff.end(), GF2mBuff.begin(), GF2mBuff.end());
		len = BN_bn2bin(Px, tmp);
		memset(GF2mBuff.data(), 0x00, GF2mBuff.size());
		memcpy(GF2mBuff.data() + GF2mBuff.size() - len, tmp, len);
		buff.insert(buff.end(), GF2mBuff.begin(), GF2mBuff.end());
		len = BN_bn2bin(Py, tmp);
		memset(GF2mBuff.data(), 0x00, GF2mBuff.size());
		memcpy(GF2mBuff.data() + GF2mBuff.size() - len, tmp, len);
		buff.insert(buff.end(), GF2mBuff.begin(), GF2mBuff.end());
	}

	DigestCalc hasher(hash_id);
	hasher.Update(buff);
	return hasher.Final();
}

/*
void SM2Key::GenK(const BIGNUM *bn_n,BIGNUM *bn_k)const
{
do
{
if(0==BN_rand_range(bn_k,bn_n))
{
throw SM2Exception(GetOpensslErrInfo());
}
}while(BN_is_zero(bn_k));
}*/

SM2Sign::~SM2Sign()
{
	if (bn_k)
	{
		BN_free(bn_k);
	}
}

SM2Sign::SM2Sign(const SM2Key& key, const BinData &UsrID, DigestCalc::Algo id) 
	:sm2key_(key), hash_id(id), bn_k(BN_new()), Za_(key.CalcZa(UsrID, id)), hash_calc(id)
{
	if (nullptr == bn_k)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
void SM2Sign::Init()
{
	BN_zero(bn_k);
	hash_calc.Init();
	hash_calc.Update(Za_);
}

void SM2Sign::Sign(unsigned char *r, int *r_len, unsigned char *s, int *s_len)
{
	const BIGNUM *priv = EC_KEY_get0_private_key(sm2key_.GetRawPointer());
	if (NULL == priv)
	{
		throw runtime_error("密钥不是私钥，不能签名!");
	}

	BN_CTX *bn_ctx = BN_CTX_new();
	if (NULL == bn_ctx)
	{
		throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{BN_CTX_free(bn_ctx); });

	bool ret = false;
	BIGNUM *bn_e = GetBN_Safe(bn_ctx);
	BIGNUM *bn_x1 = GetBN_Safe(bn_ctx);
	BIGNUM *bn_r = GetBN_Safe(bn_ctx);
	BIGNUM *bn_s = GetBN_Safe(bn_ctx);
	BIGNUM *dA1 = GetBN_Safe(bn_ctx);
	BIGNUM *bn_tmp = GetBN_Safe(bn_ctx);
	BIGNUM *bn_n = GetBN_Safe(bn_ctx);
	

	if (0 == EC_GROUP_get_order(sm2key_.GetGroupPointer(), bn_n, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	EC_POINT *kG = EC_POINT_new(sm2key_.GetGroupPointer());
	if (NULL == kG)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(kG); });


	
		auto e = hash_calc.Final();
		BN_bin2bn(e.data(), e.size(), bn_e);

		do
		{
			if (BN_is_zero(bn_k))
			{
				//sm2key_.GenK(bn_n,bn_k);
				Rander::rand(bn_n, bn_k);
			}

			if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), kG, bn_k, NULL, NULL, bn_ctx))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
			if (sm2key_.GetECType() == ECTYPE::GFp)
			{
				if (0 == EC_POINT_get_affine_coordinates_GFp(sm2key_.GetGroupPointer(), kG, bn_x1, NULL, bn_ctx))
				{
					throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
				}
			}
			else
			{
				if (0 == EC_POINT_get_affine_coordinates_GF2m(sm2key_.GetGroupPointer(), kG, bn_x1, NULL, bn_ctx))
				{
					throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
				}
			}
			// r = (e+x1)mod n
			//if( 0== BN_mod_add(bn_r, bn_e, bn_x1, bn_n, bn_ctx))
			if (0 == BN_mod_add_quick(bn_r, bn_e, bn_x1, bn_n))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
			//检查r==0
			if (BN_is_zero(bn_r))
			{
				continue;
			}
			//检查 n==k+r
			BN_uadd(bn_tmp, bn_r, bn_k);
			if (0 == BN_ucmp(bn_tmp, bn_n))
			{
				continue;
			}

			// (1+dA)^-1 mod n
			if (1 != BN_mod_add_quick(dA1, priv, BN_value_one(), bn_n))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}

			if (NULL == BN_mod_inverse(bn_s, dA1, bn_n, bn_ctx))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}

			// (r*da)mod n
			if (0 == BN_mod_mul(bn_tmp, bn_r, priv, bn_n, bn_ctx))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
			//(k-r*da)mod n
			if (0 == BN_mod_sub_quick(bn_tmp, bn_k, bn_tmp, bn_n))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
			//s
			if (0 == BN_mod_mul(bn_s, bn_s, bn_tmp, bn_n, bn_ctx))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
			if (BN_is_zero(bn_s))
			{
				continue;
			}

			*r_len = BN_bn2bin(bn_r, r);
			*s_len = BN_bn2bin(bn_s, s);
		} while (false);
	
}
bool SM2Sign::Verify(const unsigned char *r, int r_len, const unsigned char *s, int s_len)
{
	BN_CTX *bn_ctx = BN_CTX_new();
	if (NULL == bn_ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{BN_CTX_free(bn_ctx); });

	const EC_POINT *_P = EC_KEY_get0_public_key(sm2key_.GetRawPointer());


	BIGNUM *bn_r1 = GetBN_Safe(bn_ctx);
	BIGNUM *bn_s1 = GetBN_Safe(bn_ctx);
	BIGNUM *bn_e1 = GetBN_Safe(bn_ctx);
	BIGNUM *bn_x1 = GetBN_Safe(bn_ctx);
	BIGNUM *bn_t = GetBN_Safe(bn_ctx);
	BIGNUM *bn_R = GetBN_Safe(bn_ctx);
	BIGNUM *bn_n = GetBN_Safe(bn_ctx);

	if (0 == EC_GROUP_get_order(sm2key_.GetGroupPointer(), bn_n, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	BN_bin2bn(r, r_len, bn_r1);
	BN_bin2bn(s, s_len, bn_s1);

	if (BN_ucmp(bn_r1, BN_value_one()) < 0)
	{
		//throw runtime_error("验签失败: r<1");
		return false;
	}

	if (BN_ucmp(bn_r1, bn_n) >= 0)
	{
		//throw runtime_error("验签失败: r>=n");
		return false;
	}
	if (BN_ucmp(bn_s1, BN_value_one()) < 0)
	{
		//throw runtime_error("验签失败: s<1");
		return false;
	}
	if (BN_ucmp(bn_s1, bn_n) >= 0)
	{
		//throw runtime_error("验签失败: s>=n");
		return false;
	}
	
		auto e = hash_calc.Final();
		
		BN_bin2bn(e.data(), e.size(), bn_e1);

		if (1 != BN_mod_add_quick(bn_t, bn_r1, bn_s1, bn_n))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		if (BN_is_zero(bn_t))
		{
			//throw runtime_error("验签失败: t==0");
			return false;
		}

		EC_POINT *point = EC_POINT_new(sm2key_.GetGroupPointer());
		if (NULL == point)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		ON_SCOPE_EXIT([=]{EC_POINT_free(point); });
		if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), point, bn_s1, _P, bn_t, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		if (sm2key_.GetECType() == ECTYPE::GFp)
		{
			if (0 == EC_POINT_get_affine_coordinates_GFp(sm2key_.GetGroupPointer(), point, bn_x1, NULL, bn_ctx))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
		}
		else
		{
			if (0 == EC_POINT_get_affine_coordinates_GF2m(sm2key_.GetGroupPointer(), point, bn_x1, NULL, bn_ctx))
			{
				throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
			}
		}
		
		if (1 != BN_mod_add_quick(bn_R, bn_e1, bn_x1, bn_n))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	
		return 0 == BN_ucmp(bn_R, bn_r1);
}

void SM2Sign::SetK(const HexData &k)
{
	if (0 >= BN_hex2bn(&bn_k, k.c_str()))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
HexData SM2Sign::GetK()const
{
	HexData ret;
	BN2HEX(bn_k,ret);
	return ret;
}

SM2Cipher::SM2Cipher(const SM2Key& key, DigestCalc::Algo id) :c1format_(2), sm2key_(key), hash_alg(id), stat_(0)
, bn_c1(BN_new()), bn_k(BN_new()), hasherC3(id), kdf(id)
{
	if (NULL == bn_k || NULL == bn_c1)
	{
		if (NULL != bn_k) BN_free(bn_k);
		if (NULL != bn_c1) BN_free(bn_c1);
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
}
SM2Cipher::~SM2Cipher(void)
{
	BN_free(bn_c1);
	BN_free(bn_k);
}

void SM2Cipher::Init(bool enc)
{
	enc_ = enc;
	if(enc_)
	{
		Init_Encrypt();
	}
	else
	{
		Init_Decrypt();
	}
}
void SM2Cipher::Update(const unsigned char *in, int inl, unsigned char *out, int *outl)
{
	if (enc_)
	{
		Update_Encrypt(in, inl, out, outl);
	}
	else
	{
		Update_Decrypt(in, inl, out, outl);
	}
}
void SM2Cipher::Final(unsigned char *out, int *outl)
{
	if (enc_)
	{
		Final_Encrypt(out, outl);
	}
	else
	{
		Final_Decrypt(out, outl);
	}
}
int	 SM2Cipher::GetC1Len()
{
	int l = (EC_GROUP_get_degree(sm2key_.GetGroupPointer()) - 1) / 8 + 1;

	switch (c1format_)
	{
	case POINT_CONVERSION_COMPRESSED:
	case POINT_CONVERSION_COMPRESSED + 1:
		l += 1;
		break;
	case POINT_CONVERSION_HYBRID:
	case POINT_CONVERSION_HYBRID + 1:
	case POINT_CONVERSION_UNCOMPRESSED:
		l = 2 * l + 1;
		break;
	default:
		throw runtime_error("不支持的C1压缩格式 ");
		l = 0;
		break;
	}

	return l;
}

void SM2Cipher::SetK(const HexData &k)
{
	if (0 >= BN_hex2bn(&bn_k, k.c_str()))
	{
		throw runtime_error(GetOpensslErrInfo());
	}
}

HexData SM2Cipher::GetK()const
{
	HexData ret;
	BN2HEX(bn_k, ret);
	return ret;
}

void SM2Cipher::Encrypt_Init()
{
	BN_CTX *bn_ctx = BN_CTX_new();
	if (NULL == bn_ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{BN_CTX_free(bn_ctx); });

	BIGNUM *bn_n = GetBN_Safe(bn_ctx);
	BIGNUM *bn_h = GetBN_Safe(bn_ctx);
	BIGNUM *x2 = GetBN_Safe(bn_ctx);
	BIGNUM *y2 = GetBN_Safe(bn_ctx);
	
	if (0 == EC_GROUP_get_order(sm2key_.GetGroupPointer(), bn_n, bn_ctx) || 0 == EC_GROUP_get_cofactor(sm2key_.GetGroupPointer(), bn_h, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	const EC_POINT *_P = EC_KEY_get0_public_key(sm2key_.GetRawPointer());

	if (BN_is_zero(bn_k))
	{
		Rander::rand(bn_k, bn_n);
	}

	EC_POINT *C1 = EC_POINT_new(sm2key_.GetGroupPointer());
	if (NULL == C1)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(C1); });

	if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), C1, bn_k, NULL, NULL, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	if (NULL == EC_POINT_point2bn(sm2key_.GetGroupPointer(), C1, (point_conversion_form_t)c1format_, bn_c1, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	C1buff_.resize(BN_num_bytes(bn_c1));
	BN_bn2bin(bn_c1, C1buff_.data());

	EC_POINT * sP = EC_POINT_new(sm2key_.GetGroupPointer());
	if (NULL == sP)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(sP); });

	if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), sP, NULL, _P, bn_h, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	if (EC_POINT_is_at_infinity(sm2key_.GetGroupPointer(), sP))
	{
		throw runtime_error("h[P]是无穷远点！");
	}

	//kP
	EC_POINT * kP = EC_POINT_new(sm2key_.GetGroupPointer());
	if (NULL == kP)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(kP); });

	if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), kP, NULL, _P, bn_k, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	vec_x2.clear();
	vec_y2.clear();
	if (sm2key_.GetECType() == ECTYPE::GFp)
	{
		if (0 == EC_POINT_get_affine_coordinates_GFp(sm2key_.GetGroupPointer(), kP, x2, y2, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		unsigned char tmp[2048];
		int len = 0;
		len = BN_bn2bin(x2, tmp);
		vec_x2.assign(tmp, tmp + len);
		len = BN_bn2bin(y2, tmp);
		vec_y2.assign(tmp, tmp + len);
	}
	else
	{
		if (0 == EC_POINT_get_affine_coordinates_GF2m(sm2key_.GetGroupPointer(), kP, x2, y2, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		unsigned char tmp[2048];
		int len = 0;
		vec_x2.resize((EC_GROUP_get_degree(sm2key_.GetGroupPointer()) - 1) / 8 + 1);
		len = BN_bn2bin(x2, tmp);
		memcpy(vec_x2.data() + vec_x2.size() - len, tmp, len);

		vec_y2.resize((EC_GROUP_get_degree(sm2key_.GetGroupPointer()) - 1) / 8 + 1);
		len = BN_bn2bin(y2, tmp);
		memcpy(vec_y2.data() + vec_y2.size() - len, tmp, len);
	}

	kdf.Init(vec_x2 + vec_y2);
	hasherC3.Init();
	hasherC3.Update(vec_x2);
	buff_.clear();
}

inline void SM2Cipher::Init_Encrypt()
{
	stat_ = 1;
	BN_zero(bn_k);
}
void SM2Cipher::Init_Decrypt()
{
	stat_ = 1;
	buff_.clear();
}
void SM2Cipher::Update_Encrypt(const unsigned char *in, int inl, unsigned char *out, int *outl)
{
	if (0 == stat_)
	{
		throw runtime_error(ERR_WHERE "还未调用init");
	}
	if (1 == stat_)
	{
		Encrypt_Init();
		stat_ = 2;
	}

	if (NULL == out || NULL == outl)
	{
		return;
	}

	if (2 == stat_)
	{
		//输出C1
		out = copy(C1buff_.begin(), C1buff_.end(), out);
		*outl += C1buff_.size();
		stat_ = 3;
	}

	if (NULL != in)
	{
		hasherC3.Update(in, inl);

		vector<unsigned char> tmp(inl);

		kdf.GetK(tmp.data(), inl);

		for (int i = 0; i < inl; ++i)
		{
			out[i] = in[i] ^ tmp[i];
		}
		*outl += inl;
	}
}
void SM2Cipher::Final_Encrypt(unsigned char *out, int *outl)
{
	if (0 == stat_)
	{
		throw runtime_error(ERR_WHERE "还未调用init");
	}

	if (NULL == out || NULL == outl)
	{
		throw runtime_error(ERR_WHERE "out==NULL || outl==NULL");
	}

	if (1 == stat_)
	{
		Encrypt_Init();
		stat_ = 2;
	}

	if (2 == stat_)
	{
		//输出C1
		out = copy(C1buff_.begin(), C1buff_.end(), out);
		*outl += C1buff_.size();
		stat_ = 3;
	}

	//c3
	hasherC3.Update(vec_y2);
	hasherC3.Final(out);
	*outl += hasherC3.DigestLength();
}

void SM2Cipher::DecryptStat1()
{
	BN_CTX *bn_ctx = BN_CTX_new();
	if (NULL == bn_ctx)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{BN_CTX_free(bn_ctx); });

	unsigned int l = (EC_GROUP_get_degree(sm2key_.GetGroupPointer()) - 1) / 8 + 1;

	switch (buff_[0])
	{
	case POINT_CONVERSION_COMPRESSED:
	case POINT_CONVERSION_COMPRESSED + 1:
		l += 1;
		break;
	case POINT_CONVERSION_HYBRID:
	case POINT_CONVERSION_HYBRID + 1:
	case POINT_CONVERSION_UNCOMPRESSED:
		l = 2 * l + 1;
		break;
	default:
		throw runtime_error(ERR_WHERE "不支持的压缩格式!");
	}

	if (buff_.size() < l)
	{
		return;
	}

	//解出C1并验证
	BIGNUM *c1 = GetBN_Safe(bn_ctx);

	if (NULL == BN_bin2bn(buff_.data(), l, c1))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	EC_POINT *pc1 = EC_POINT_new(sm2key_.GetGroupPointer());
	if (NULL == pc1)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(pc1); });

	if (NULL == EC_POINT_bn2point(sm2key_.GetGroupPointer(), c1, pc1, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	//验证c1是否在曲线上

	if (0 == EC_POINT_is_on_curve(sm2key_.GetGroupPointer(), pc1, bn_ctx))
	{
		throw runtime_error(ERR_WHERE "c1不在曲线上!");
	}

	EC_POINT *s = EC_POINT_new(sm2key_.GetGroupPointer());
	if (NULL == s)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(s); });

	BIGNUM *bn_h = BN_CTX_get(bn_ctx);
	if (NULL == bn_h || NULL == EC_GROUP_get_cofactor(sm2key_.GetGroupPointer(), bn_h, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), s, NULL, pc1, bn_h, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	if (EC_POINT_is_at_infinity(sm2key_.GetGroupPointer(), s))
	{
		throw runtime_error(ERR_WHERE "s不在曲线上!");
	}

	//生成x2,y2
	const BIGNUM * bn_D = EC_KEY_get0_private_key(sm2key_.GetRawPointer());

	if (0 == EC_POINT_mul(sm2key_.GetGroupPointer(), s, NULL, pc1, bn_D, bn_ctx))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	BIGNUM *x2 = GetBN_Safe(bn_ctx);
	BIGNUM *y2 = GetBN_Safe(bn_ctx);

	if (sm2key_.GetECType() == ECTYPE::GFp)
	{
		if (0 == EC_POINT_get_affine_coordinates_GFp(sm2key_.GetGroupPointer(), s, x2, y2, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		unsigned char tmp[2048];
		int len = 0;
		len = BN_bn2bin(x2, tmp);
		vec_x2.assign(tmp, tmp + len);
		len = BN_bn2bin(y2, tmp);
		vec_y2.assign(tmp, tmp + len);
	}
	else
	{
		if (0 == EC_POINT_get_affine_coordinates_GF2m(sm2key_.GetGroupPointer(), s, x2, y2, bn_ctx))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		unsigned char tmp[2048];
		int len = 0;
		vec_x2.resize((EC_GROUP_get_degree(sm2key_.GetGroupPointer()) - 1) / 8 + 1);
		len = BN_bn2bin(x2, tmp);
		memcpy(vec_x2.data() + vec_x2.size() - len, tmp, len);

		vec_y2.resize((EC_GROUP_get_degree(sm2key_.GetGroupPointer()) - 1) / 8 + 1);
		len = BN_bn2bin(y2, tmp);
		memcpy(vec_y2.data() + vec_x2.size() - len, tmp, len);
	}

	buff_.erase(buff_.begin(), buff_.begin() + l);
	hasherC3.Init();
	hasherC3.Update(vec_x2);
	kdf.Init(vec_x2 + vec_y2);

	stat_ = 2;
}
void SM2Cipher::Update_Decrypt(const unsigned char *in, int inl, unsigned char *out, int *outl)
{
	if (0 == stat_)
	{
		throw runtime_error(ERR_WHERE "未调用init!");
	}
	if (NULL != in)
	{
		buff_.insert(buff_.end(), in, in + inl);
	}
	if (NULL == out || NULL == outl)
	{
		return;
	}

	if ((1 == stat_) && (!buff_.empty()))
	{
		DecryptStat1();
	}

	if (2 == stat_)
	{
		//解密
		if (buff_.size() > hasherC3.DigestLength())
		{
			int len = buff_.size() - hasherC3.DigestLength();
			vector<unsigned char> tmp(len);
			kdf.GetK(tmp.data(), len);

			for (int i = 0; i < len; ++i)
			{
				out[i] = buff_[i] ^ tmp[i];
			}
			*outl += len;
			buff_.erase(buff_.begin(), buff_.begin() + len);

			hasherC3.Update(out, len);
		}
	}
}
void SM2Cipher::Final_Decrypt(unsigned char *out, int *outl)
{
	if (0 == stat_)
	{
		throw runtime_error(ERR_WHERE "未调用init!");
	}

	if ((1 == stat_) && (!buff_.empty()))
	{
		DecryptStat1();
	}

	if (2 != stat_)
	{
		throw runtime_error(ERR_WHERE "没有足够的数据解出C1");
	}

	//解密
	if (buff_.size() < hasherC3.DigestLength())
	{
		throw runtime_error(ERR_WHERE "没有足够的数据解出C3");
	}

	if (buff_.size() > hasherC3.DigestLength())
	{
		if (NULL == out || NULL == outl)
		{
			throw runtime_error(ERR_WHERE "out==NULL || outl==NULL，无法存放明文");
		}

		int len = buff_.size() - hasherC3.DigestLength();

		vector<unsigned char> tmp(len);
		kdf.GetK(tmp.data(), len);

		for (int i = 0; i < len; ++i)
		{
			out[i] = buff_[i] ^ tmp[i];
		}
		*outl += len;
		buff_.erase(buff_.begin(), buff_.begin() + len);

		hasherC3.Update(out, len);
	}

	//验证C3
	hasherC3.Update(vec_y2);
	if (buff_ != hasherC3.Final())
	{
		throw runtime_error(ERR_WHERE "C3验证失败！");
	}
}

SM2KeyExchangeBase::SM2KeyExchangeBase(const SM2Key &KeyOfInitiator, const BinData &UsrIDOfInitiator, const SM2Key &KeyOfResponder, const BinData &UsrIDOfResponder, DigestCalc::Algo alg)
	:_KeyOfInitiator(KeyOfInitiator), _KeyOfResponder(KeyOfResponder), stat_(0), hash_alg(alg), _point_format(2)
{
	_Za=_KeyOfInitiator.CalcZa(UsrIDOfInitiator,alg);
	_Zb = _KeyOfResponder.CalcZa(UsrIDOfResponder, alg);

	bn_ctx_obj = BN_CTX_new();
	if (NULL == bn_ctx_obj)
	{
		throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
	}

	if (EC_GROUP_cmp(_KeyOfInitiator.GetGroupPointer(), _KeyOfResponder.GetGroupPointer(), bn_ctx_obj))
	{
		BN_CTX_free(bn_ctx_obj);
		throw runtime_error(ERR_WHERE "两把密钥不属于同一个曲线上");
	}

	bn_k = BN_CTX_get(bn_ctx_obj);
	bn_x1 = BN_CTX_get(bn_ctx_obj);
	bn_y1 = BN_CTX_get(bn_ctx_obj);
	bn_x2 = BN_CTX_get(bn_ctx_obj);
	bn_y2 = BN_CTX_get(bn_ctx_obj);
	bn_n = BN_CTX_get(bn_ctx_obj);
	bn_2w = BN_CTX_get(bn_ctx_obj);
	if (NULL == bn_k || NULL == bn_x1 || NULL == bn_x2 || NULL == bn_y1 || NULL == bn_y2 || NULL == bn_n || NULL == bn_2w)
	{
		BN_CTX_free(bn_ctx_obj);
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	BN_zero(bn_k);
	if (0 == EC_GROUP_get_order(_KeyOfInitiator.GetGroupPointer(), bn_n, bn_ctx_obj))
	{
		BN_CTX_free(bn_ctx_obj);
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	int w = BN_num_bits(bn_n) / 2 - 1;
	BN_set_bit(bn_2w, w);
}

SM2KeyExchangeBase::~SM2KeyExchangeBase()
{
	BN_CTX_free(bn_ctx_obj);
}

void SM2KeyExchangeBase::SetK(const HexData &k)
{
	if (0 >= BN_hex2bn(&bn_k, k.c_str()))
	{
		throw runtime_error(ERR_WHERE+GetOpensslErrInfo());
	}
}

HexData SM2KeyExchangeBase::GetK()const
{
	HexData k;
	BN2HEX(bn_k, k);
	return k;
}

void SM2KeyExchangeBase::GenRandomPointer(BinData &ret)
{
	if (BN_is_zero(bn_k))
	{
		Rander::rand(bn_n, bn_k);
	}
	EC_POINT *kG = EC_POINT_new(_KeyOfInitiator.GetGroupPointer());
	if (NULL == kG)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(kG); });
	BN_CTX_start(bn_ctx_obj);
	ON_SCOPE_EXIT([=]{BN_CTX_end(bn_ctx_obj); });

	if (0 == EC_POINT_mul(_KeyOfInitiator.GetGroupPointer(), kG, bn_k, NULL, NULL, bn_ctx_obj))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	if (_KeyOfInitiator.GetECType() == ECTYPE::GFp)
	{
		if (0 == EC_POINT_get_affine_coordinates_GFp(_KeyOfInitiator.GetGroupPointer(), kG, bn_x1, bn_y1, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}
	else
	{
		if (0 == EC_POINT_get_affine_coordinates_GF2m(_KeyOfInitiator.GetGroupPointer(), kG, bn_x1, bn_y1, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
	}

	BIGNUM *bn_tmp = BN_CTX_get(bn_ctx_obj);
	if (NULL == bn_tmp)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	if (NULL == EC_POINT_point2bn(_KeyOfInitiator.GetGroupPointer(), kG, (point_conversion_form_t)_point_format, bn_tmp, bn_ctx_obj))
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}

	ret.resize(BN_num_bytes(bn_tmp));
	BN_bn2bin(bn_tmp, ret.data());
}

void SM2KeyExchangeBase::TransFormX(const BIGNUM *x, BIGNUM *x_)
{
	BN_CTX_start(bn_ctx_obj);
	BIGNUM *bn_tmp = BN_CTX_get(bn_ctx_obj);
	if (NULL == bn_tmp)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		BN_CTX_end(bn_ctx_obj);
	}
	BN_nnmod(bn_tmp, x, bn_2w, bn_ctx_obj);
	BN_uadd(x_, bn_tmp, bn_2w);
	BN_CTX_end(bn_ctx_obj);
}

void SM2KeyExchangeBase::CalcT(const BIGNUM *x, const SM2Key & key, BIGNUM *t)
{
	const BIGNUM *priv = EC_KEY_get0_private_key(key.GetRawPointer());
	if (NULL == priv)
	{
		throw runtime_error(ERR_WHERE "不具备私钥，无法计算");
	}

	BN_CTX_start(bn_ctx_obj);
	ON_SCOPE_EXIT([=]{BN_CTX_end(bn_ctx_obj); });
	
		BIGNUM *bn_x_ = BN_CTX_get(bn_ctx_obj);
		BIGNUM *bn_tmp = BN_CTX_get(bn_ctx_obj);
		if (NULL == bn_tmp || NULL == bn_x_)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		TransFormX(x, bn_x_);
		if (0 == BN_mod_mul(bn_tmp, bn_k, bn_x_, bn_n, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		if (0 == BN_mod_add_quick(t, priv, bn_tmp, bn_n))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
}

void SM2KeyExchangeBase::CalcUV(const BIGNUM *t, const BIGNUM *x, const EC_POINT *Rb, const SM2Key &key, EC_POINT *UV)
{
	const EC_POINT * Pb = EC_KEY_get0_public_key(key.GetRawPointer());
	EC_POINT *tmp_point = EC_POINT_new(_KeyOfInitiator.GetGroupPointer());
	if (NULL == tmp_point)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(tmp_point); });
	EC_POINT *tmp_point1 = EC_POINT_new(_KeyOfInitiator.GetGroupPointer());
	if (NULL == tmp_point)
	{
		throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
	}
	ON_SCOPE_EXIT([=]{EC_POINT_free(tmp_point1); });
	BN_CTX_start(bn_ctx_obj);
	ON_SCOPE_EXIT([=]{BN_CTX_end(bn_ctx_obj); });
	
		BIGNUM *x2_ = BN_CTX_get(bn_ctx_obj);
		if (NULL == x2_)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		TransFormX(x, x2_);
		if (0 == EC_POINT_mul(_KeyOfInitiator.GetGroupPointer(), tmp_point, NULL, Rb, x2_, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}

		if (0 == EC_POINT_add(_KeyOfInitiator.GetGroupPointer(), tmp_point1, Pb, tmp_point, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}

		BIGNUM *bn_h = BN_CTX_get(bn_ctx_obj);
		BIGNUM *bn_tmp = BN_CTX_get(bn_ctx_obj);
		if (NULL == bn_h || NULL == bn_tmp)
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
		if (0 == EC_GROUP_get_cofactor(_KeyOfInitiator.GetGroupPointer(), bn_h, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}

		if (0 == BN_mul(bn_tmp, bn_h, t, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}

		if (0 == EC_POINT_mul(_KeyOfInitiator.GetGroupPointer(), UV, NULL, tmp_point1, bn_tmp, bn_ctx_obj))
		{
			throw runtime_error(ERR_WHERE + GetOpensslErrInfo());
		}
}

void SM2KeyExchangeBase::GenKey(const BinData &X, const BinData &Y, unsigned char *key, int keylen)
{
	if (NULL == key || keylen <= 0)
	{
		throw runtime_error(ERR_WHERE "输出缓冲区指针为空或 keylen<=0!");
	}

	KDF kdf(hash_alg);
	kdf.Init(X + Y + _Za + _Zb);
	kdf.GetK(key, keylen);
}

void SM2KeyExchangeBase::GenS(int type, int flag, BinData &S)
{
	BIGNUM *x1, *y1, *x2, *y2;
	if (flag)
	{
		x1 = bn_x1;
		y1 = bn_y1;
		x2 = bn_x2;
		y2 = bn_y2;
	}
	else
	{
		x1 = bn_x2;
		y1 = bn_y2;
		x2 = bn_x1;
		y2 = bn_y1;
	}

	DigestCalc hasher(hash_alg);
	hasher.Init();
	hasher.Update(_X);
	hasher.Update(_Za);
	hasher.Update(_Zb);

	unsigned char tmp[1024];
	int len;

	if (_KeyOfInitiator.GetECType() == ECTYPE::GFp)
	{
		len = BN_bn2bin(x1, tmp);
		hasher.Update(tmp, len);
		len = BN_bn2bin(y1, tmp);
		hasher.Update(tmp, len);
		len = BN_bn2bin(x2, tmp);
		hasher.Update(tmp, len);
		len = BN_bn2bin(y2, tmp);
		hasher.Update(tmp, len);
	}
	else
	{
		len = (EC_GROUP_get_degree(_KeyOfInitiator.GetGroupPointer()) - 1) / 8 + 1;
		unsigned char tmp1[1024];
		int len1;
		len1 = BN_bn2bin(x1, tmp1);
		memcpy(tmp + len - len1, tmp1, len1);
		hasher.Update(tmp, len);
		len1 = BN_bn2bin(y1, tmp1);
		memcpy(tmp + len - len1, tmp1, len1);
		hasher.Update(tmp, len);
		len1 = BN_bn2bin(x2, tmp1);
		memcpy(tmp + len - len1, tmp1, len1);
		hasher.Update(tmp, len);
		len1 = BN_bn2bin(y2, tmp1);
		memcpy(tmp + len - len1, tmp1, len1);
		hasher.Update(tmp, len);
	}

	auto vectmp = hasher.Final();

	unsigned char arrtype[1];
	arrtype[0] = type;
	hasher.Init();
	hasher.Update(arrtype, 1);
	hasher.Update(_Y);
	hasher.Update(vectmp);

	S=hasher.Final();
}

void SM2KeyExchangeBase::Init()
{
	BN_zero(bn_k);
}

void SM2KeyExchangeBase::GetAffine(const EC_POINT *Point, std::vector<unsigned char> &x, std::vector<unsigned char> &y)
{
	BN_CTX_start(bn_ctx_obj);
	try
	{
		BIGNUM *bn_x = BN_CTX_get(bn_ctx_obj);
		if (NULL == bn_x) throw SM2Exception(GetOpensslErrInfo());
		BIGNUM *bn_y = BN_CTX_get(bn_ctx_obj);
		if (NULL == bn_y) throw SM2Exception(GetOpensslErrInfo());
		if (_KeyOfInitiator.GetECType() == ECTYPE::GFp)
		{
			if (0 == EC_POINT_get_affine_coordinates_GFp(_KeyOfInitiator.GetGroupPointer(), Point, bn_x, bn_y, bn_ctx_obj))
			{
				throw SM2Exception(GetOpensslErrInfo());
			}
			unsigned char tmp[1024];
			int len = 0;
			len = BN_bn2bin(bn_x, tmp);
			x.assign(tmp, tmp + len);
			len = BN_bn2bin(bn_y, tmp);
			y.assign(tmp, tmp + len);
		}
		else
		{
			if (0 == EC_POINT_get_affine_coordinates_GF2m(_KeyOfInitiator.GetGroupPointer(), Point, bn_x, bn_y, bn_ctx_obj))
			{
				throw SM2Exception(GetOpensslErrInfo());
			}
			unsigned char tmp[1024];
			int len = 0;
			x.resize((EC_GROUP_get_degree(_KeyOfInitiator.GetGroupPointer()) - 1) / 8 + 1, 0);
			len = BN_bn2bin(bn_x, tmp);
			memcpy(x.data() + x.size() - len, tmp, len);

			y.resize(x.size(), 0);
			len = BN_bn2bin(bn_y, tmp);
			memcpy(y.data() + y.size() - len, tmp, len);
		}
	}
	catch (const SM2Exception &)
	{
		BN_CTX_end(bn_ctx_obj);
		throw;
	}
	BN_CTX_end(bn_ctx_obj);
}
void SM2KeyExchangeInitiator::GenKey(const BinData &R_Responde, unsigned char *key, int keyl)
{
	SM2KeyExchangeBase::GenKey(R_Responde, _KeyOfInitiator, _KeyOfResponder, key, keyl);
}
void SM2KeyExchangeInitiator::GenKey(const BinData &R_Responde, const BinData &S_Responder, unsigned char *key, int keyl, BinData &S_Initiator)
{
	
	SM2KeyExchangeBase::GenKey(R_Responde, _KeyOfInitiator, _KeyOfResponder, key, keyl);
	vector<unsigned char> s1;
	GenS(0x02, 1, s1);
	if (S_Responder != s1)
	{
		throw SM2Exception("S1!=Sb!");
	}

	GenS(0x03, 1, S_Initiator);
}

void SM2KeyExchangeResponder::GenKey(const BinData &R_Initiator, unsigned char *key, int keyl, BinData &R_Responder)
{
	GenRandomPointer(R_Responder);
	SM2KeyExchangeBase::GenKey(R_Initiator, _KeyOfResponder, _KeyOfInitiator, key, keyl);
}
void SM2KeyExchangeResponder::GenKey(const BinData &R_Initiator, unsigned char *key, int keyl, BinData &R_Responder, BinData &S_Responder)
{
	GenRandomPointer(R_Responder);
	SM2KeyExchangeBase::GenKey(R_Initiator, _KeyOfResponder, _KeyOfInitiator, key, keyl);

	GenS(0x02, 0, S_Responder);
	GenS(0x03, 0, _s2);
}
bool SM2KeyExchangeResponder::CheckSa(const BinData &S_Initiator)
{
	return _s2 == S_Initiator;
}

void SM2KeyExchangeBase::GenKey(const std::vector<unsigned char> &R, const SM2Key &Priv, const SM2Key &Pub, unsigned char *key, int keylen)
{
	EC_POINT *PR = EC_POINT_new(Priv.GetGroupPointer());
	if (NULL == PR)
	{
		throw SM2Exception(GetOpensslErrInfo());
	}
	EC_POINT *UV = EC_POINT_new(Priv.GetGroupPointer());
	if (NULL == UV)
	{
		EC_POINT_free(PR);
		throw SM2Exception(GetOpensslErrInfo());
	}
	BN_CTX_start(bn_ctx_obj);
	try
	{
		BIGNUM *bn_R = BN_CTX_get(bn_ctx_obj);
		if (NULL == bn_R)
		{
			throw SM2Exception(GetOpensslErrInfo());
		}
		if (NULL == BN_bin2bn(R.data(), R.size(), bn_R))
		{
			throw SM2Exception(GetOpensslErrInfo());
		}
		if (NULL == EC_POINT_bn2point(_KeyOfInitiator.GetGroupPointer(), bn_R, PR, bn_ctx_obj))
		{
			throw SM2Exception(GetOpensslErrInfo());
		}
		if (0 == EC_POINT_is_on_curve(_KeyOfInitiator.GetGroupPointer(), PR, bn_ctx_obj))
		{
			throw SM2Exception("R不在曲线上!");
		}

		if (_KeyOfInitiator.GetECType() == ECTYPE::GFp)
		{
			if (0 == EC_POINT_get_affine_coordinates_GFp(_KeyOfInitiator.GetGroupPointer(), PR, bn_x2, bn_y2, bn_ctx_obj))
			{
				throw SM2Exception(GetOpensslErrInfo());
			}
		}
		else
		{
			if (0 == EC_POINT_get_affine_coordinates_GF2m(_KeyOfInitiator.GetGroupPointer(), PR, bn_x2, bn_y2, bn_ctx_obj))
			{
				throw SM2Exception(GetOpensslErrInfo());
			}
		}
		BIGNUM *bn_t = BN_CTX_get(bn_ctx_obj);
		if (NULL == bn_t)
		{
			throw SM2Exception(GetOpensslErrInfo());
		}
		CalcT(bn_x1, Priv, bn_t);
		CalcUV(bn_t, bn_x2, PR, Pub, UV);
		if (EC_POINT_is_at_infinity(Priv.GetGroupPointer(), UV))
		{
			throw SM2Exception("U/V是无穷远点");
		}
		GetAffine(UV, _X, _Y);
		SM2KeyExchangeBase::GenKey(_X, _Y, key, keylen);
	}
	catch (const SM2Exception &)
	{
		EC_POINT_free(UV);
		EC_POINT_free(PR);
		BN_CTX_end(bn_ctx_obj);
		throw;
	}
	EC_POINT_free(UV);
	EC_POINT_free(PR);
	BN_CTX_end(bn_ctx_obj);
}

__LIB_NAME_SPACE_END__