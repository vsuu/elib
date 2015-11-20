#ifndef _DIGESTCALC_H_RFT
#define _DIGESTCALC_H_RFT
#include "libbase.h"
#include <stdexcept>
#include "BinData.h"

__LIB_NAME_SPACE_BEGIN__

class DigestCalc
{
public:
	class Algo
	{
	public:
		static const Algo sm3;
		static const Algo sha;
		static const Algo sha1;
		static const Algo sha224;
		static const Algo sha256;
		static const Algo sha384;
		static const Algo sha512;
		static const Algo md2;
		static const Algo md4;
		static const Algo md5;
		static const Algo rc2;
		static const Algo rc4;
		static const Algo rc5;
		static const Algo dsa;
		static const Algo mdc2;
		static const Algo ecdsa;
		operator int()const
		{
			return id_;
		}
	private:
		explicit Algo(int id) :id_(id)
		{}
		int id_;
	};

	explicit DigestCalc(Algo);
	DigestCalc(const DigestCalc &) = delete;
	DigestCalc &operator=(const DigestCalc &) = delete;
	~DigestCalc();
	void SetAlgo(Algo);
	void Init();
	void Update(const unsigned char * in, size_t inl);
	void Final(unsigned char *);
	void Update(const BinData&data)
	{
		Update(data.data(), data.size());
	}
	BinData Final()
	{
		BinData ret(DigestLength());
		Final(ret.data());
		return ret;
	}
	size_t DigestLength()const;
private:
	class Impl;
	Impl *impl_;
};

BinData CalcDigest(DigestCalc::Algo algo, const BinData &data);
size_t CalcDigest(DigestCalc::Algo algo, const BinData &data, unsigned char *out);
BinData CalcDigest(DigestCalc::Algo algo, const unsigned char *in, int inl);
size_t CalcDigest(DigestCalc::Algo algo, const unsigned char *in, int inl, unsigned char *out);
size_t GetDigestLength(DigestCalc::Algo);

__LIB_NAME_SPACE_END__

#endif