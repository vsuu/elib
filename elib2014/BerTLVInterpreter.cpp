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

class TLVInterpreter_//��ȡBerTLV��ؽ���
{
public:
    TLVInterpreter_(STANDARD s) :s_(s){}
    ~TLVInterpreter_();
    std::string GetDescription(uint16_t tag)const
    {
        return TLVDescription::GetTagDescription(static_cast<int>(tag), s_);
    }
    //��ȡ��TLV���ݱ�ʾ����Ϣ
    std::string GetMeaning(const BerTLV &tlv)const
    {
        BinData tmp;
        tlv.EncapValue(back_inserter(tmp));
        return TLVDescription::GetTagInfo(static_cast<int>(tlv.Tag()), tmp.ToHex().c_str(), s_);
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
//��ȡ��TLV���ݱ�ʾ����Ϣ
std::string TLVInterpreter::GetMeaning(const BerTLV &tlv)const
{
    return impl_->GetMeaning(tlv);
}

TLVInterpreter TLVInterpreter_VISA(new TLVInterpreter_(STANDARD::VISA));
TLVInterpreter TLVInterpreter_JCB(new TLVInterpreter_(STANDARD::JCB));
TLVInterpreter TLVInterpreter_MASTERCARD(new TLVInterpreter_(STANDARD::MASTERCARD));
TLVInterpreter TLVInterpreter_PBOC(new TLVInterpreter_(STANDARD::PBOC));

__LIB_NAME_SPACE_END__