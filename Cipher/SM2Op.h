#ifndef _SM2OP_H
#define _SM2OP_H
#include <string>
#include <vector>
#include <exception>
#include "DigestCalc.h"
#include "libbase.h"
#include "BinData.h"

typedef struct ec_group_st EC_GROUP;
typedef struct ec_point_st EC_POINT;
typedef struct ec_key_st EC_KEY;
typedef struct bignum_st BIGNUM;
typedef struct bignum_ctx BN_CTX;

__LIB_NAME_SPACE_BEGIN__

class ECTYPE
{
	int type;
	explicit ECTYPE(int x) :type(x){}
public:
	bool operator==(const ECTYPE &x)const { return x.type == type; }
	bool operator!=(const ECTYPE &x)const { return x.type != type; }
	static const ECTYPE GFp; //素域曲线  椭圆曲线方程为： y^2 = x^3 + ax + b 的曲线
	static const ECTYPE GF2m;//二元扩域曲线  y^2 + x*y = x^3 + a*x^2 + b 的曲线
};

class SM2Key;

class SM2Group
{
public:
	SM2Group(ECTYPE ectype, const BinData &p, const BinData &a, const BinData &b, const BinData &n, const BinData &Gx, const BinData &Gy, const BinData &h = BinData(1, 0x01));
	SM2Group(const SM2Group&) = delete;
	~SM2Group();
	SM2Group &operator=(const SM2Group&)=delete;
	ECTYPE Type()const { return type_; }
	SM2Key GenerateRandomKey()const;//生成一个随机密钥
	SM2Key GenPublicKey(const BinData &Px, const BinData &Py)const;//根据参数生成一个公钥
	SM2Key GenPrivateKey(const BinData &Priv)const;//根据参数生成一个私钥
	SM2Key GenKey(const BinData &Px, const BinData &Py, const BinData &Priv)const;//根据参数生成一个公、私钥
private:
	friend class SM2key;
	const EC_GROUP *GetRawPointer()const { return raw_pointer_; }
private:
	EC_GROUP *raw_pointer_;
	ECTYPE type_;
	EC_KEY * GenEmptyKey()const;
	void SetPublicKey(EC_KEY *, const BinData &Px, const BinData &Py)const;
	void SetPrivateKey(EC_KEY *, const BinData &Priv)const;
};

const SM2Group & DefaultSM2Group();

class KDF
{
public:
	KDF(DigestCalc::Algo alg) : hasher_(alg)
	{}
	KDF(const KDF &) = delete;
	KDF &operator=(const KDF &) = delete;
	void GetK(unsigned char *out, unsigned int len);
	void Init(const BinData &Z)
	{
		Buff_.clear();
		Z_ = Z;
		ct_ = 1;
	}
private:
	DigestCalc hasher_;
	BinData Z_;
	BinData Buff_;
	unsigned int ct_ = 1;
};

class Rander
{
public:
	static void rand(const BIGNUM *n, BIGNUM *ret);
};

	class SM2Cipher;
	class SM2Sign;
	class SM2KeyExchangeBase;
	class SM2KeyExchangeInitiator;
	class SM2KeyExchangeResponder;

class SM2Key
{
	SM2Key(EC_KEY *pkey, ECTYPE ectype) :key_(pkey), type_(ectype)
	{}
public:
	void GetKeyInfo(HexData *Px, HexData *Py, HexData *D)const;//得到密钥的十六进制字符串
	~SM2Key();
	SM2Key(const SM2Key &);
	SM2Key &operator=(const SM2Key &);
private://内部使用
	const EC_GROUP *GetGroupPointer()const;
	const EC_KEY *GetRawPointer()const { return key_; }
	ECTYPE GetECType()const { return type_; }
	BinData CalcZa(const BinData&in, DigestCalc::Algo)const;
	//	void GenK(const BIGNUM *n,BIGNUM *k)const;
private:
	EC_KEY *key_;
	ECTYPE type_;
	friend class SM2Group;
	friend class SM2Cipher;
	friend class SM2Sign;
	friend class SM2KeyExchangeBase;
	friend class SM2KeyExchangeInitiator;
	friend class SM2KeyExchangeResponder;
};

	class SM2Sign
	{
	public:
		SM2Sign(const SM2Key&, const BinData &UsrID, DigestCalc::Algo = DigestCalc::Algo::sm3);
		~SM2Sign();
		void Init();
		void Update(const unsigned char *in, int inl)
		{
			hash_calc.Update(in, inl);
		}
		void Sign(unsigned char *r, int *r_len, unsigned char *s, int *s_len);//需要公私钥
		bool Verify(const unsigned char *r, int r_len, const unsigned char *s, int s_len);//需要公钥
	public://以下3函数测试用
		const BinData &GetZa()const { return Za_; }
		void SetK(const HexData &);
		HexData GetK()const;
	private:
		SM2Key sm2key_;
		DigestCalc hash_calc;
		DigestCalc::Algo hash_id;
		BinData Za_;
		SM2Sign(const SM2Sign &);
		SM2Sign &operator=(const SM2Sign &);
		BIGNUM *bn_k;
	};

	class SM2Cipher
	{
	public:
		//表示进行加密运算还是解密运算
		//class ENCMODE
		//{
		//	int enc;
		//	explicit ENCMODE(int x) :enc(x){}
		//public:
		//	static const ENCMODE ENCRYPT;//加密,需要公钥
		//	static const ENCMODE DECRYPT;//解密,需要私钥
		//public:
		//	bool operator==(ENCMODE x)const { return x.enc == enc; }
		//	bool operator!=(ENCMODE x)const { return x.enc != enc; }
		//};
	public:
		SM2Cipher(const SM2Key&, DigestCalc::Algo = DigestCalc::Algo::sm3);
		~SM2Cipher(void);
		//void Init(ENCMODE);
		void Init(bool encrypt);
		void Update(const unsigned char *in, int inl, unsigned char *out, int *outl);
		void Final(unsigned char *out, int *outl);
		int GetC1Len();   //加密结果长度= GetC1Len()+MessageLen+hashLen
		//for test
		void SetC1Format(int x){ c1format_ = x; }
		void SetK(const HexData &);
		HexData GetK()const;
	private:
		SM2Cipher(const SM2Cipher &);
		SM2Cipher & operator=(const SM2Cipher &);
		void Init_Encrypt();
		void DecryptStat1();
		void Encrypt_Init();
		void Init_Decrypt();
		void Update_Encrypt(const unsigned char *in, int inl, unsigned char *out, int *outl);
		void Final_Encrypt(unsigned char *out, int *outl);
		void Update_Decrypt(const unsigned char *in, int inl, unsigned char *out, int *outl);
		void Final_Decrypt(unsigned char *out, int *outl);
		BinData vec_x2;
		BinData vec_y2;
		BinData buff_;
		BinData C1buff_;
		BIGNUM * bn_c1;
		DigestCalc hasherC3;
		DigestCalc::Algo hash_alg;
		//ENCMODE _enc;
		bool enc_=true;
		int stat_;
		int c1format_;//for test
		SM2Key sm2key_;
		BIGNUM *bn_k;
		KDF kdf;
	};

	

	class SM2KeyExchangeBase
	{
	public:
		SM2KeyExchangeBase(const SM2Key &KeyOfInitiator, const BinData &UsrIDOfInitiator, const SM2Key &KeyOfResponder, const BinData &UsrIDOfResponder, DigestCalc::Algo = DigestCalc::Algo::sm3);
		~SM2KeyExchangeBase();
		void Init();
		//for test
		void SetPointFormat(int x){ _point_format = x; }
		void SetK(const HexData &);
		HexData GetK()const;
		const BinData &GetZOfInitiator()const { return _Za; }
		const BinData &GetZOfResponder()const { return _Zb; }
	protected:
		void GenRandomPointer(BinData &);
		SM2Key _KeyOfInitiator;
		SM2Key _KeyOfResponder;
		BinData _Za;
		BinData _Zb;
		void TransFormX(const BIGNUM *x, BIGNUM *x_);
		void CalcT(const BIGNUM *x, const SM2Key & key, BIGNUM *t);
		void CalcUV(const BIGNUM *t, const BIGNUM *x2_, const EC_POINT *Rb, const SM2Key &key, EC_POINT *UV);
		void GenKey(const BinData&X, const BinData &Y, unsigned char *key, int keylen);
		void GenS(int type, int flag, BinData &S);
		void GetAffine(const EC_POINT *Point, std::vector<unsigned char> &x, std::vector<unsigned char> &y);
		BinData _X;
		BinData _Y;
		void GenKey(const std::vector<unsigned char> &R, const SM2Key &Priv, const SM2Key &Pub, unsigned char *key, int keylen);
		BIGNUM *bn_k;
		BIGNUM *bn_x1;
		BIGNUM *bn_y1;
		BIGNUM *bn_x2;
		BIGNUM *bn_y2;
		BIGNUM *bn_n;
		BN_CTX *bn_ctx_obj;
		int stat_;
		int _point_format;//for test
		DigestCalc::Algo hash_alg;
		BIGNUM *bn_2w;
	private:
		SM2KeyExchangeBase(const SM2KeyExchangeBase&);
		SM2KeyExchangeBase &operator=(const SM2KeyExchangeBase&);
	};

	class SM2KeyExchangeInitiator : public SM2KeyExchangeBase
	{
	public:
		SM2KeyExchangeInitiator(const SM2Key &KeyOfInitiator, const std::string &UsrIDOfInitiator, const SM2Key &KeyOfResponder, const std::string &UsrIDOfResponder, DigestCalc::alg_id id = DigestCalc::alg_id::sm3)
			:SM2KeyExchangeBase(KeyOfInitiator, UsrIDOfInitiator, KeyOfResponder, UsrIDOfResponder, id)
		{
		}
		void GenInitiatorRP(BinData &R_Initiator){ GenRandomPointer(R_Initiator); }
		void GenKey(const BinData &R_Responde, unsigned char *key, int keyl);
		void GenKey(const BinData &R_Responde, const BinData &S_Responder, unsigned char *key, int keyl, BinData &S_Initiator);
	};

	class SM2KeyExchangeResponder : public SM2KeyExchangeBase
	{
	public:
		SM2KeyExchangeResponder(const SM2Key &KeyOfInitiator, const std::string &UsrIDOfInitiator, const SM2Key &KeyOfResponder, const std::string &UsrIDOfResponder, DigestCalc::alg_id id = DigestCalc::alg_id::sm3)
			:SM2KeyExchangeBase(KeyOfInitiator, UsrIDOfInitiator, KeyOfResponder, UsrIDOfResponder, id)
		{
		}
		void GenKey(const BinData &R_Initiator, unsigned char *key, int keyl, BinData &R_Responder);
		void GenKey(const BinData &R_Initiator, unsigned char *key, int keyl, BinData &R_Responder, BinData &S_Responder);
		bool CheckSa(const BinData &S_Initiator);
	private:
		std::vector<unsigned char> _s2;
	};

__LIB_NAME_SPACE_END__
#endif