#ifndef _CIPHERCALC_H_RFT
#define _CIPHERCALC_H_RFT

#include "libbase.h"
#include <stdexcept>
#include "BinData.h"

__LIB_NAME_SPACE_BEGIN__

class CipherCalc
{
public:
	//表示进行加密运算还是解密运算
	class ENCMODE
	{
		const int enc_;
		explicit ENCMODE(int x) :enc_(x){}
	public:
		static const ENCMODE ENCRYPT;//加密
		static const ENCMODE DECRYPT;//解密
		operator int()
		{
			return enc_;
		}
	};

	//表示数据的填充方式
	class PADMODE
	{
		const int padding_;
		explicit PADMODE(int x) :padding_(x){}
	public:
		static const PADMODE NO_PADDING;//用户自己填充数据，保证数据长度为分组长度的整数倍，否则运算出错。
		static const PADMODE OPENSSL_PADDING;//openssl填充算法，填充数据至分组长度的整数倍。如果数据本身的长度就是分组
		//长度的整数倍，就追加一个分组。填充的所有字节值都等于追加的数据的长度。
		operator int()
		{
			return padding_;
		}
	};

	//表示使用哪种对称算法
	class Algo
	{
		explicit Algo(int id) :id_(id){}
		const int id_;
	public:
		operator int()
		{
			return id_;
		}
		static const Algo sm4_ecb;//国密SM4 ECB模式
		static const Algo sm4_cbc;//国密SM4 CBC模式
		static const Algo des_cfb;
		static const Algo des_cfb1;
		static const Algo des_cfb8;
		static const Algo des_ede_cfb;
		static const Algo des_ede3_cfb;
		static const Algo des_ede3_cfb1;
		static const Algo des_ede3_cfb8;
		static const Algo des_ofb;
		static const Algo des_ede_ofb;
		static const Algo des_ede3_ofb;
		static const Algo desx_cbc;
		static const Algo des_cbc;		//单DES CBC模式
		static const Algo des_ede_cbc;  //3DES CBC模式
		static const Algo des_ede3_cbc;
		static const Algo des_ecb;		//单DES ECB模式
		static const Algo des_ede;
		static const Algo des_ede3;     //3DES ECB模式
		static const Algo rc4;
		static const Algo rc4_40;
		static const Algo rc4_hmac_md5;
		static const Algo idea_ecb;
		static const Algo idea_cfb;
		static const Algo idea_ofb;
		static const Algo idea_cbc;
		static const Algo seed_ecb;
		static const Algo seed_cfb;
		static const Algo seed_ofb;
		static const Algo seed_cbc;
		static const Algo rc2_ecb;
		static const Algo rc2_cfb;
		static const Algo rc2_ofb;
		static const Algo rc2_cbc;
		static const Algo rc2_40_cbc;
		static const Algo rc2_64_cbc;
		static const Algo bf_ecb;
		static const Algo bf_cfb;
		static const Algo bf_ofb;
		static const Algo bf_cbc;
		static const Algo cast5_ecb;
		static const Algo cast5_cfb;
		static const Algo cast5_ofb;
		static const Algo cast5_cbc;
		static const Algo rc5_32_12_16_ecb;
		static const Algo rc5_32_12_16_cfb;
		static const Algo rc5_32_12_16_ofb;
		static const Algo rc5_32_12_16_cbc;
		static const Algo aes_128_ecb;
		static const Algo aes_128_cbc;
		static const Algo aes_128_cfb;
		static const Algo aes_128_cfb1;
		static const Algo aes_128_cfb8;
		static const Algo aes_128_ofb;
		static const Algo aes_128_ctr;
		static const Algo aes_128_gcm;
		static const Algo aes_128_xts;
		static const Algo aes_192_ecb;
		static const Algo aes_192_cbc;
		static const Algo aes_192_cfb;
		static const Algo aes_192_cfb1;
		static const Algo aes_192_cfb8;
		static const Algo aes_192_ofb;
		static const Algo aes_192_ctr;
		static const Algo aes_192_gcm;
		static const Algo aes_256_ecb;
		static const Algo aes_256_cbc;
		static const Algo aes_256_cfb;
		static const Algo aes_256_cfb1;
		static const Algo aes_256_cfb8;
		static const Algo aes_256_ofb;
		static const Algo aes_256_ctr;
		static const Algo aes_256_gcm;
		static const Algo aes_256_xts;
		static const Algo camellia_128_ecb;
		static const Algo camellia_128_cbc;
		static const Algo camellia_128_cfb;
		static const Algo camellia_128_cfb1;
		static const Algo camellia_128_cfb8;
		static const Algo camellia_128_ofb;
		static const Algo camellia_192_ecb;
		static const Algo camellia_192_cbc;
		static const Algo camellia_192_cfb;
		static const Algo camellia_192_cfb1;
		static const Algo camellia_192_cfb8;
		static const Algo camellia_192_ofb;
		static const Algo camellia_256_ecb;
		static const Algo camellia_256_cbc;
		static const Algo camellia_256_cfb;
		static const Algo camellia_256_cfb1;
		static const Algo camellia_256_cfb8;
		static const Algo camellia_256_ofb;
	};
public:
	explicit CipherCalc(Algo);
	CipherCalc(const CipherCalc &) = delete;
	CipherCalc &operator=(const CipherCalc &) = delete;
	~CipherCalc();
	void SetAlgo(Algo);
	void Init(unsigned char *key, const unsigned char *iv, ENCMODE enc, PADMODE padding = PADMODE::NO_PADDING);
	unsigned char * Update(const unsigned char *in, int inl, unsigned char *out);
	unsigned char * Final(unsigned char *out);
private:
	class Impl;
	Impl *impl_;
};

__LIB_NAME_SPACE_END__

#endif