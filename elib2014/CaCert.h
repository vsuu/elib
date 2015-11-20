#ifndef CACERT_H_RFT
#define CACERT_H_RFT
#include "libbase.h"
#include "Singleton.h"
#include "ClassAttr.h"
#include <string>
#include <tuple>
#include "BinData.h"
#include <cstdint>
#include <vector>
#include "CfgBase.h"

__LIB_NAME_SPACE_BEGIN__

enum class CRYPTO_ALGO_TYPE { RSA, SM2 };

typedef  uint8_t idx_type;

class CaCert
{
public:
	CaCert(const std::string &path);
	CaCert(const BinData&Rid, idx_type idx, CRYPTO_ALGO_TYPE type, const std::string &path, const std::tuple<BinData, BinData> &public_key_info);
	~CaCert();
	CaCert(const CaCert &) = delete;
	CaCert(CaCert &&);
	CaCert & operator=(const CaCert &) = delete;
	CaCert & operator=(CaCert &&);
	const BinData &RID()const;

	idx_type PKI()const;

	CRYPTO_ALGO_TYPE AlgoType()const;

	const std::string &FilePath()const;

	const std::tuple<BinData, BinData> &PublicKeyInfo()const;
	class impl;
private:
	impl * impl_;
};

class CaCertManager :public nocopyable  //单例模式
{
public:
	void Init(const std::string &path);
	void AddCaCertByComponent(const elib::BinData &, idx_type idx, CRYPTO_ALGO_TYPE algo_type, const std::tuple<BinData, BinData> & keyinfo);
	void ImportCaCert(const char * FileName);
	void LoadCaCertByComponent();
	void DeleteCaCert(const char * FileName);
	void ExportCaCert(const char *SrcFileName, const char * DstPath);
	const CaCert & GetCaCert(BinData RID, idx_type idx);
	bool IsCaCertExist(BinData RID, idx_type idx);
	typedef std::vector<CaCert>::iterator iterator;
	iterator begin()
	{
		return CaCerts_.begin();
	}
	iterator end()
	{
		return CaCerts_.end();
	}
private:
	CaCertManager() = default;
	~CaCertManager();
	friend class Singleton < CaCertManager >;
	std::vector<CaCert> CaCerts_;
	std::string path_;//证书文件所在目录
	XMLCfgFile  cfg_;
};
__LIB_NAME_SPACE_END__
#endif