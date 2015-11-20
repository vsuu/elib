#ifndef RSA_CALC_H_RFT
#define RSA_CALC_H_RFT
#include "libbase.h"
#include "BinData.h"

struct rsa_st;

__LIB_NAME_SPACE_BEGIN__

class RsaPadding
{
public:
	static const RsaPadding PKCS1;
	static const RsaPadding NO_PADDING;
	static const RsaPadding SSLV23;
	static const RsaPadding PKCS1_OAEP;
	static const RsaPadding X931;
	operator int()
	{
		return id_;
	}
private:
	explicit RsaPadding(int id_);
	int id_;
};

class RsaKey
{
public:
	explicit RsaKey(rsa_st *nh) :native_handle_(nh)
	{}
	RsaKey();
	~RsaKey();
	RsaKey(const RsaKey &) = delete;
	RsaKey &operator=(const RsaKey &) = delete;
	RsaKey(RsaKey &&);
	RsaKey &operator=(RsaKey &&);
	operator rsa_st *()
	{
		return native_handle_;
	}
	//获取密钥信息
	size_t GetBitCount();
	void GetKeyInfo(HexData * n, HexData * e, HexData * d, HexData * p, HexData * q, HexData * dp, HexData * dq, HexData * qinv);
private:
	rsa_st * native_handle_;
};

RsaKey MakeRsaPubKey(const BinData &n, const BinData &e);
RsaKey MakeRsaPriKey(const BinData &n, const BinData &d);
RsaKey MakeRsaPriKey_CRT(const BinData &p, const BinData &q, const BinData &dp, const BinData &dq, const BinData &qinv);
RsaKey MakeRsaKey(const BinData &n, const BinData &e, const BinData &d);
RsaKey MakeRsaKey(const BinData &n, const BinData &e, const BinData &p, const BinData &q, const BinData &dp, const BinData &dq, const BinData &qinv);
RsaKey MakeRsaKey_Random(int bitcount, unsigned long e);

void RSA_PublicEncrypt(RsaKey &, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding);
void RSA_PrivateEncrypt(RsaKey &, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding);
void RSA_PublicDecrypt(RsaKey &, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding);
void RSA_PrivateDecrypt(RsaKey &, const unsigned char *in, int inl, unsigned char *out, RsaPadding padding);

__LIB_NAME_SPACE_END__

#endif