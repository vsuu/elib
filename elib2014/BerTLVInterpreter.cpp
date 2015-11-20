#include "BerTLVInterpreter.h"

#include <string>
#include <iterator>
#include "BerTLV.h"
#include "TLVDescription.h"
#include "strop.h"
#include "BinData.h"
using std::string;
using std::back_inserter;

__LIB_NAME_SPACE_BEGIN__

class TLVInterpreter_//获取BerTLV相关介绍
{
public:
	TLVInterpreter_(STANDARD s) :s_(s){}
	~TLVInterpreter_() = default;
	std::string GetDescription(uint16_t tag)const
	{
		return TLVDescription::GetTagDescription(static_cast<int>(tag), s_);
	}
	TLV_VALUE_FORMAT GetValueFomat(int tag)const
	{
		return TLVDescription::GetValueFomat(static_cast<int>(tag), s_);
	}
	//获取本TLV数据表示的信息
	std::string GetMeaning(const BerTLV &tlv)const
	{
		return TLVDescription::GetTagInfo(static_cast<int>(tlv.Tag()), tlv.Value().ToHex().c_str(), s_);
	}
	const std::vector<std::pair<std::string, std::string>> & GetDescriptionVec(uint16_t tag)const
	{
		return TLVDescription::GetTagDescriptionVec(static_cast<int>(tag), s_);
	}
	STANDARD Standard()const
	{
		return s_;
	}
private:
	STANDARD s_;
};

TLVInterpreter::TLVInterpreter(TLVInterpreter_ *impl) :impl_(impl)
{
}
TLVInterpreter::~TLVInterpreter()
{
	delete impl_;
}
std::string TLVInterpreter::GetDescription(uint16_t tag)const
{
	return impl_->GetDescription(tag);
}

TLV_VALUE_FORMAT TLVInterpreter::GetValueFomat(int tag)const
{
	return impl_->GetValueFomat(tag);
}
const std::vector<std::pair<std::string, std::string>> & TLVInterpreter::GetDescriptionVec(uint16_t tag)const
{
	return impl_->GetDescriptionVec(tag);
}
//获取本TLV数据表示的信息
std::string TLVInterpreter::GetMeaning(const BerTLV &tlv)const
{
	return impl_->GetMeaning(tlv);
}
STANDARD TLVInterpreter::Standard()const
{
	return impl_->Standard();
}

TLVInterpreter TLVInterpreter_VISA(new TLVInterpreter_(STANDARD::VISA));
TLVInterpreter TLVInterpreter_JCB(new TLVInterpreter_(STANDARD::JCB));
TLVInterpreter TLVInterpreter_MASTERCARD(new TLVInterpreter_(STANDARD::MASTERCARD));
TLVInterpreter TLVInterpreter_PBOC(new TLVInterpreter_(STANDARD::PBOC));



const TLVInterpreter & GetTLVInterpreter(STANDARD s)
{
	switch (s)
	{
	case STANDARD::PBOC:
		return TLVInterpreter_PBOC;
		break;
	case STANDARD::VISA:
		return TLVInterpreter_VISA;
		break;
	case STANDARD::MASTERCARD:
		return TLVInterpreter_MASTERCARD;
		break;
	case STANDARD::JCB:
		return TLVInterpreter_JCB;
		break;
	default:
		throw std::invalid_argument(ERR_WHERE);
	}
}

namespace
{
	BinData RID_PBOC{0xA0,0x00,0x00,0x03,0x33};
	BinData RID_VISA{ 0xA0, 0x00, 0x00, 0x00, 0x03 };
	BinData RID_MC{ 0xA0, 0x00, 0x00, 0x00, 0x04 };
	BinData RID_JCB{ 0xA0, 0x00, 0x00, 0x00, 0x65 };
}
const TLVInterpreter & GetTLVInterpreter(const BinData&AID)
{
	auto rid = AID.SubData(0, 5);
	if (rid == RID_PBOC)
	{
		return TLVInterpreter_PBOC;
	}
	if (rid == RID_VISA)
	{
		return TLVInterpreter_VISA;
	}
	if (rid == RID_MC)
	{
		return TLVInterpreter_MASTERCARD;
	}
	if (rid == RID_JCB)
	{
		return TLVInterpreter_JCB;
	}
	throw std::invalid_argument(ERR_WHERE + AID.ToHex());
}

__LIB_NAME_SPACE_END__