#ifndef TERMINAL_H_RFT
#define TERMINAL_H_RFT

#include "libbase.h"
#include "BerTLV.h"
#include <unordered_map>
#include <stdexcept>
#include "Log.h"
#include <functional>
#include "Reader.h"
#include <memory>
#include <vector>

//namespace std
//{
//	template<>
//	struct hash < elib::BerTLV > :public unary_function < elib::BerTLV, size_t >
//	{
//		size_t operator()(const elib::BerTLV & tlv)
//		{
//			return hash<elib::TagType>()(tlv.Tag());
//		}
//	};
//	template<>
//	struct equal_to < elib::BerTLV > :public binary_function < elib::BerTLV, elib::BerTLV, bool >
//	{
//		bool operator()(const elib::BerTLV & tlv1, const elib::BerTLV & tlv2)
//		{
//			return tlv1.Tag() == tlv2.Tag();
//		}
//	};
//}

__LIB_NAME_SPACE_BEGIN__

class Storage
{
public:
	virtual ~Storage() = default;
	virtual void Clear()
	{
		sad_record_.clear();
		map_.clear();
	}
	virtual BerTLV &Tag(TagType tag)
	{
		auto it = map_.find(tag);
		if (it != map_.end())
		{
			return it->second;
		}
		else
		{
			throw std::runtime_error(ERR_WHERE "不存在此TAG：0X" + BerTLV(tag).TToBin().ToHex());
		}
	}
	virtual bool IsTagIn(TagType tag)const
	{
		return map_.find(tag) != map_.end();
	}
	virtual void AddTag(BerTLV &&tlv)
	{
		if (!map_.emplace(tlv.Tag(), std::move(tlv)).second)
		{
			ReportTagRepeated(tlv.Tag());
		}
	}
	virtual void AddTag(const BerTLV &tlv)
	{
		if (!map_.emplace(tlv.Tag(), tlv).second)
		{
			ReportTagRepeated(tlv.Tag());
		}
	}
	virtual void AddSADRecord(int sfi, BerTLV && record)
	{
		sad_record_.emplace_back(sfi, std::move(record));
	}
	virtual const std::vector<std::pair<int, BerTLV>> &GetSADRecordList()const
	{
		return sad_record_;
	}
private:
	std::unordered_map<TagType, BerTLV> map_;
	void ReportTagRepeated(TagType tag)
	{
		throw std::runtime_error(ERR_WHERE "已存在这个TAG:" + BerTLV(tag).TToBin().ToHex());
	}
	std::vector<std::pair<int, BerTLV>> sad_record_;
};
//
//class TerminalDataBase : public nocopyable
//{
//public:
//	TerminalDataBase() = default;
//	virtual ~TerminalDataBase() = 0
//	{}
//	virtual bool test(size_t pos)const = 0;
//	virtual bool any()const = 0;
//	virtual bool none()const = 0;
//	virtual bool all()const = 0;
//	virtual void set() = 0;
//	virtual void set(size_t pos) = 0;
//	virtual void reset() = 0;
//	virtual void reset(size_t pos) = 0;
//	virtual void flip() = 0;
//	virtual void flip(size_t pos) = 0;
//	virtual unsigned char getbyte(size_t pos_byte)const = 0;
//	virtual void setbyte(size_t pos_byte, unsigned char) = 0;
//	virtual elib::BinData ToBin()const = 0;
//};

class TerminalDataShower : public nocopyable
{
public:
	virtual ~TerminalDataShower() = 0
	{}
	virtual void Update(const BinData&) = 0;
	virtual void Init() = 0;
};

typedef struct _CardAIDInfo
{
	elib::BinData terminal_code;//终端上的应用名称
	elib::BinData code;//应用名称
	elib::BinData lable;//应用标签
	unsigned char pri;//应用优先级
	bool need_confirm;//需要确认
}CardAIDInfo;

class EMV_Terminal
{
public:
	EMV_Terminal() :pStorage_(new Storage), CurAidPos_(-1), CDA_flag_(false)
	{
	}
	~EMV_Terminal()
	{
		delete pStorage_;
	}

	void clear()
	{
		pStorage_->Clear();
		/*if (pTvr_)
		{
		pTvr_->Init();
		}
		if (pTsi_)
		{
		pTsi_->Init();
		}
		*/
		IssuerScripts2_.clear();
		CDA_flag_ = false;
		Online_flag_ = false;
		OnlinePin_.clear();
	}
	void SetTVR(size_t BytePos, size_t BitPos)
	{
		BinData & V95 = const_cast<BinData &>(pStorage_->Tag(0x95).LeafValue());
		V95[BytePos] |= (0x01 << BitPos);
		//pTvr_->Update(V95);
		if (CallbackFunForTvrChange_)
		{
			CallbackFunForTvrChange_(V95);
		}
	}
	void SetTSI(size_t BytePos, size_t BitPos)
	{
		BinData & V9B = const_cast<BinData &>(pStorage_->Tag(0x9B).LeafValue());
		V9B[BytePos] |= (0x01 << BitPos);
		//pTsi_->Update(V9B);
		if (CallbackFunForTsiChange_)
		{
			CallbackFunForTsiChange_(V9B);
		}
	}
	Storage* pStorage_;
	std::unique_ptr<SCardInterface> pReader_;
	//TerminalDataBase *pTvr_;
	//TerminalDataBase *pTsi_;
	//std::unique_ptr<TerminalDataShower> pTvr_;
	//std::unique_ptr<TerminalDataShower> pTsi_;
	std::vector<CardAIDInfo> CardAidList_;
	std::vector<CardAIDInfo> CardAidList_Locked_;
	//std::vector<elib::ApduCmd> IssuerScripts1_;//before GAC2
	std::vector<elib::ApduCmd> IssuerScripts2_;//after GAC2
	std::function<bool(std::string &)> GetOnlinePinFun_;
	std::function<bool(std::string &)> GetOfflinePinFun_;
	std::function<void(const BinData &)> CallbackFunForTvrChange_;
	std::function<void(const BinData &)> CallbackFunForTsiChange_;
	size_t CurAidPos_;
	Logger log_;
	std::string OnlinePin_;
	bool CDA_flag_;
	bool ExApp_flag_;
	bool Online_flag_;
};

__LIB_NAME_SPACE_END__

#endif