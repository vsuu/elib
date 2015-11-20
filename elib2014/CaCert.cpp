#include "CaCert.h"

#include <string>
#include <cassert>
#include <stdexcept>
#include <tuple>
#include "strop.h"
#include <fstream>
#include <iterator>
#include <cstdio>
#include <algorithm>
#include <memory>
#include "rapidxml\rapidxml.hpp"

using namespace std;

using namespace elib;

__LIB_NAME_SPACE_BEGIN__

class CaCert::impl
{
public:
	impl() = default;
	impl(const BinData&Rid, idx_type idx, CRYPTO_ALGO_TYPE type, const std::string &path, const std::tuple<BinData, BinData> &public_key_info) :
		RID_(Rid), idx_(idx), type_(type), path_(path), public_key_info_(public_key_info)
	{
	}
	BinData RID_;
	idx_type idx_;
	CRYPTO_ALGO_TYPE type_;
	std::string path_;
	std::tuple<BinData, BinData> public_key_info_;
};

const BinData &CaCert::RID()const
{
	return impl_->RID_;
}
idx_type CaCert::PKI()const
{
	return impl_->idx_;
}
CRYPTO_ALGO_TYPE CaCert::AlgoType()const
{
	return impl_->type_;
}
const std::string &CaCert::FilePath()const
{
	return impl_->path_;
}
const std::tuple<BinData, BinData> &CaCert::PublicKeyInfo()const
{
	return impl_->public_key_info_;
}

namespace {
	//return path,name,suffix

	void CopyFile(const char * FileSrc, const char *FileDst)
	{
		assert(FileSrc != nullptr);
		assert(FileDst != nullptr);
		ifstream ifs(FileSrc);
		if (!ifs)
		{
			throw runtime_error("文件打开失败,filename = " + string(FileSrc));
		}
		ofstream ofs(FileDst);
		if (!ofs)
		{
			throw runtime_error("文件创建失败,filename = " + string(FileDst));
		}

		copy(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>(), ostreambuf_iterator<char>(ofs));
	}

	BinData FileToBin(const string &filename)
	{
		ifstream ifs(filename.c_str(), ios_base::in | ios_base::binary);
		if (ifs)
		{
			istreambuf_iterator<char> first(ifs), last;
			return BinData(first, last);
		}
		else
		{
			throw runtime_error("打开文件失败,filename = " + filename);
		}
	}

	void ParseCaFile_MC(CaCert::impl *pcert)
	{
		BinData content(FileToBin(pcert->path_));
		if (content.size() < 9)
		{
			throw runtime_error(ERR_WHERE "解析MC证书失败，文件长度不够");
		}
		//RID
		pcert->RID_ = content.SubData(0, 5);
		pcert->idx_ = content[5];
		if (0x01 == content[6])
		{
			pcert->type_ = CRYPTO_ALGO_TYPE::RSA;
		}
		else
		{
			throw runtime_error(ERR_WHERE "解析证书失败" + pcert->path_);
		}
		size_t nlen = content[7];
		size_t elen = content[8];

		if (content.size() < 9 + nlen + elen)
		{
			throw runtime_error(ERR_WHERE "解析MC证书失败，文件长度不够");
		}

		pcert->public_key_info_ = make_tuple(content.SubData(9, nlen), content.SubData(9 + nlen, elen));
	}

	void ParseCaFile_PBOC(CaCert::impl *pcert)
	{
		BinData content(FileToBin(pcert->path_));
		if (content[0] == 0x20)//RSA证书
		{
			if (content.size() < 15)
			{
				throw runtime_error(ERR_WHERE "解析证书失败，文件长度不够");
			}
			//公钥模长
			size_t nlen = content[5];
			nlen <<= 8;
			nlen += content[6];
			//算法标识
			if (0x01 == content[7])
			{
				pcert->type_ = CRYPTO_ALGO_TYPE::RSA;
			}
			else
			{
				throw runtime_error(ERR_WHERE "算法标识非法,解析证书失败" + pcert->path_);
			}
			//公钥指数长度
			size_t elen = content[8];
			//RID
			pcert->RID_ = content.SubData(9, 5);
			//idx
			pcert->idx_ = content[14];
			//key info

			if (content.size() < 15 + nlen + elen)
			{
				throw runtime_error(ERR_WHERE "解析证书失败，文件长度不够");
			}
			pcert->public_key_info_ = make_tuple(content.SubData(15, nlen), content.SubData(15 + nlen, elen));
		}
		else if (content[0] == 0x31)//SM证书
		{
			if (content.size() < 16)
			{
				throw runtime_error(ERR_WHERE "解析证书失败，文件长度不够");
			}
			//RID
			pcert->RID_ = content.SubData(5, 5);
			//idx
			pcert->idx_ = content[10];
			//算法标识
			if (0x04 == content[13])
			{
				pcert->type_ = CRYPTO_ALGO_TYPE::SM2;
			}
			else
			{
				throw runtime_error(ERR_WHERE "算法标识非法,解析证书失败" + pcert->path_);
			}
			//公钥模长
			size_t klen = content[15];
			if (content.size() < 16 + klen)
			{
				throw runtime_error(ERR_WHERE "解析证书失败，文件长度不够");
			}

			//pcert->public_key_info_ = make_tuple(content.SubData(16, klen), BinData());
			pcert->public_key_info_ = make_tuple(content.SubData(16, klen / 2), content.SubData(16 + klen / 2, klen / 2));
		}
		else
		{
			throw runtime_error(ERR_WHERE "非法的PBOC CA证书");
		}
	}

	void ParseCaFile_VISA(CaCert::impl *pcert)
	{
		//BinData content(FileToBin(pcert->path_));
		ParseCaFile_PBOC(pcert);
	}

	void ParseCaFile_JCB(CaCert::impl *pcert)
	{
		string path, name;
		tie(path, name, ignore) = ParseFileName(pcert->path_);
		path += '\\';
		path += name;
		path += '.';
		path += "cpk";

		BinData content(FileToBin(path));//cpk文件
		content += FileToBin(pcert->path_);//sxx文件

		if (content.size() < 9)
		{
			throw runtime_error(ERR_WHERE "解析MC证书失败，文件长度不够");
		}

		//RID
		pcert->RID_ = content.SubData(0, 5);
		//idx
		pcert->idx_ = content[5];
		//
		if (0x01 == content[6])
		{
			pcert->type_ = CRYPTO_ALGO_TYPE::RSA;
		}
		else
		{
			throw runtime_error(ERR_WHERE "解析证书失败" + pcert->path_);
		}
		size_t nlen = content[7];
		size_t elen = content[8];

		if (content.size() < 9 + nlen + elen)
		{
			throw runtime_error(ERR_WHERE "解析MC证书失败，文件长度不够");
		}
		pcert->public_key_info_ = make_tuple(content.SubData(9, nlen), content.SubData(9 + nlen, elen));
	}

	void FillCaCert(CaCert::impl *pcert)
	{
		assert(pcert != nullptr);
		const string & filepath = pcert->path_;
		string suffix, name;
		tie(ignore, name, suffix) = ParseFileName(filepath);

		if (suffix.size() != 3)
		{
			throw invalid_argument(ERR_WHERE "CA证书文件后缀长度不为3,是否选择了错误的文件?");
		}

		if ((suffix == "sep") && (name.size() >= 5) && (name.substr(0, 3) == "MCI"))
		{//MC
			pcert->idx_ = static_cast<idx_type>((HexData(name.substr(name.size() - 2)).ToBin()).at(0));
			ParseCaFile_MC(pcert);
		}
		else if ((suffix.size() == 3) && (suffix[0] == 'C'))
		{//pboc
			pcert->idx_ = static_cast<idx_type>((HexData(suffix.substr(1)).ToBin()).at(0));
			ParseCaFile_PBOC(pcert);
		}
		else if ((suffix.size() == 3) && (suffix[0] == 'V'))
		{//visa
			pcert->idx_ = static_cast<idx_type>((HexData(suffix.substr(1)).ToBin()).at(0));
			ParseCaFile_VISA(pcert);
		}
		else if ((suffix.size() == 3) && (suffix[0] == 's'))
		{//jcb
			pcert->idx_ = static_cast<idx_type>((HexData(name).ToBin()).at(0));
			ParseCaFile_JCB(pcert);
		}
		else
		{//error
			throw invalid_argument(ERR_WHERE "错误或不支持的CA证书,filepath=" + filepath);
		}
	}
}

CaCert::CaCert(const std::string &path)
	:impl_(new impl)
{
	impl_->path_ = path;
	FillCaCert(impl_);
}

CaCert::CaCert(const BinData&Rid, idx_type idx, CRYPTO_ALGO_TYPE type, const std::string &path, const std::tuple<BinData, BinData> &public_key_info)
	:impl_(new impl(Rid, idx, type, path, public_key_info))
{
}

CaCert::CaCert(CaCert &&o) : impl_(o.impl_)
{
	o.impl_ = nullptr;
}

CaCert & CaCert::operator=(CaCert &&o)
{
	swap(impl_, o.impl_);
	return *this;
}
CaCert::~CaCert()
{
	if (nullptr != impl_)
	{
		delete impl_;
	}
}
CaCertManager::~CaCertManager()
{
	unique_ptr<XMLCfgFile> tmp(new XMLCfgFile);
	tmp->Create((path_ + "\\ca_cert.tmp").c_str());

	int i = 0;
	for (auto &x : CaCerts_)
	{
		if (x.FilePath().empty())
		{
			++i;
			tmp->Add<const char *>(("CERT" + to_string(i) + "/RID").c_str(), x.RID().ToHex().c_str());
			tmp->Add<int>(("CERT" + to_string(i) + "/IDX").c_str(), x.PKI());
			tmp->Add<int>(("CERT" + to_string(i) + "/ALGO").c_str(), static_cast<int>(x.AlgoType()));
			tmp->Add<const char *>(("CERT" + to_string(i) + "/N").c_str(), std::get<0>(x.PublicKeyInfo()).ToHex().c_str());
			tmp->Add<const char *>(("CERT" + to_string(i) + "/E").c_str(), std::get<1>(x.PublicKeyInfo()).ToHex().c_str());
		}
	}
	tmp->Add<int>("count", i);
	tmp.reset();
	remove((path_ + "\\ca_component.xml").c_str());
	rename((path_ + "\\ca_cert.tmp").c_str(), (path_ + "\\ca_component.xml").c_str());
}
void CaCertManager::DeleteCaCert(const char * FileName)
{
	for (auto it = CaCerts_.begin(); it != CaCerts_.end(); ++it)
	{
		if (it->FilePath() == FileName)
		{
			string name, suffix;
			tie(ignore, name, suffix) = ParseFileName(FileName);
			cfg_.Del("filename", (name + '.' + suffix).c_str());
			cfg_.Save();
			remove(FileName);
			//jcb
			if ((suffix.size() == 3) && (suffix != "seq") && (suffix[0] == 's'))
			{
				remove((path_ + '\\' + name + ".cpk").c_str());
			}

			CaCerts_.erase(it);

			break;
		}
	}
}
void CaCertManager::ImportCaCert(const char * FileName)
{//导入证书
	//解析、导入、顺带检查
	CaCert cert(FileName);
	if (find_if(CaCerts_.begin(), CaCerts_.end(), [&cert](const CaCert &l){return (l.RID() == cert.RID()) && (l.PKI() == cert.PKI()); })
		== CaCerts_.end()
		)
	{
		CaCerts_.push_back(std::move(cert));
	}
	else
	{
		throw runtime_error(ERR_WHERE "已存在相同RID及CA公钥索引的公钥证书");
	}

	string path, name, suffix;
	tie(path, name, suffix) = ParseFileName(FileName);

	//copy文件
	CopyFile(FileName, (path_ + '\\' + name + '.' + suffix).c_str());

	//jcb 2个文件，特殊处理
	if (CaCerts_.back().RID() == Hex2Bin("A000000065"))
		//if ((suffix.size() == 3) && (suffix != "seq") && (suffix[0] == 's'))
	{
		CopyFile((path + '\\' + name + ".cpk").c_str(), (path_ + '\\' + name + ".cpk").c_str());
	}

	cfg_.Add("filename", (name + '.' + suffix).c_str());
	cfg_.Save();
}
void CaCertManager::ExportCaCert(const char * SrcFileName, const char * DstPath)
{//导出证书
	string path, name, suffix;
	tie(path, name, suffix) = ParseFileName(SrcFileName);

	string strDstPath(DstPath);
	TrimRight(strDstPath, " \\/");
	CopyFile(SrcFileName, (strDstPath + '\\' + name + '.' + suffix).c_str());

	//jcb 2个文件，特殊处理
	if ((suffix.size() == 3) && (suffix != "seq") && (suffix[0] == 's'))
	{
		CopyFile((path + '\\' + name + ".cpk").c_str(), (strDstPath + '\\' + name + ".cpk").c_str());
	}
}
const CaCert & CaCertManager::GetCaCert(BinData RID, idx_type idx)
{
	for (auto &x : CaCerts_)
	{
		if ((x.RID() == RID)
			&& (x.PKI() == idx)
			)
		{
			return x;
		}
	}
	throw runtime_error(ERR_WHERE "未找到符合条件的CA公钥证书");
}

bool CaCertManager::IsCaCertExist(BinData RID, idx_type idx)
{
	for (auto &x : CaCerts_)
	{
		if ((x.RID() == RID)
			&& (x.PKI() == idx)
			)
		{
			return true;
		}
	}
	return false;
}

void CaCertManager::AddCaCertByComponent(const elib::BinData &rid, idx_type idx, CRYPTO_ALGO_TYPE algo_type, const std::tuple<BinData, BinData> & keyinfo)
{
	if (!IsCaCertExist(rid, idx))
	{
		CaCerts_.emplace_back(rid, idx, algo_type, string(), keyinfo);
	}
	else
	{
		throw runtime_error(ERR_WHERE "已存在相同RID及CA公钥索引的公钥证书");
	}
}

void CaCertManager::Init(const std::string &path)
{
	path_ = path;
	TrimRight(path_, " \\/");
	//读配置文件
	cfg_.Load((path_ + "\\ca.cfg").c_str());
	auto vec = cfg_.GetList<const char *>("filename");

	for (auto x : vec)
	{
		CaCerts_.push_back(CaCert(path_ + '\\' + x));
	}

	LoadCaCertByComponent();
}

void CaCertManager::LoadCaCertByComponent()
{
	//cert_set.xml
	XMLCfgFile  cfg_component_;
	cfg_component_.Load((path_ + "\\ca_component.xml").c_str());
	auto count = cfg_component_.Get<int>("count");
	for (int i = 0; i < count; ++i)
	{
		auto rid = cfg_component_.Get<const char *>(("CERT" + to_string(i + 1) + "/RID").c_str());
		auto idx = cfg_component_.Get<int>(("CERT" + to_string(i + 1) + "/IDX").c_str());
		auto algo = static_cast<CRYPTO_ALGO_TYPE>(cfg_component_.Get<int>(("CERT" + to_string(i + 1) + "/ALGO").c_str()));
		auto n = cfg_component_.Get<const char *>(("CERT" + to_string(i + 1) + "/N").c_str());
		auto e = cfg_component_.Get<const char *>(("CERT" + to_string(i + 1) + "/E").c_str());

		BinData Rid = Hex2Bin(std::string(rid));
		BinData N_Component = Hex2Bin(std::string(n));
		BinData E_Component = Hex2Bin(std::string(e));

		if (find_if(CaCerts_.begin(), CaCerts_.end(), [&Rid, &idx](const CaCert &l){return (l.RID() == Rid) && (l.PKI() == idx); })
			!= CaCerts_.end()
			)
		{
			throw runtime_error(ERR_WHERE "已存在相同RID及CA公钥索引的公钥证书");
		}

		CaCerts_.emplace_back(std::move(Rid), idx, algo, string(), make_tuple(std::move(N_Component), std::move(E_Component)));
	}
}

__LIB_NAME_SPACE_END__