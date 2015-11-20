#ifndef BERTLV_INTERPRETER_H_RFT
#define BERTLV_INTERPRETER_H_RFT

#include <string>
#include "libbase.h"
#include <cstdint>
#include "ClassAttr.h"
#include <vector>
#include "TLVDescription.h"
#include "BinData.h"

__LIB_NAME_SPACE_BEGIN__


extern enum class TLV_VALUE_FORMAT :unsigned int;

class BerTLV;

class TLVInterpreter_;
class TLVInterpreter : public nocopyable //获取BerTLV相关介绍
{
public:
	explicit TLVInterpreter(TLVInterpreter_ *);
	~TLVInterpreter();
	std::string GetDescription(uint16_t tag)const;
	TLV_VALUE_FORMAT GetValueFomat(int tag)const;
	const std::vector<std::pair<std::string, std::string>> & GetDescriptionVec(uint16_t tag)const;
	//获取本TLV数据表示的信息
	std::string GetMeaning(const BerTLV &tlv)const;
	STANDARD Standard()const;
private:
	TLVInterpreter_ *impl_;
};

extern TLVInterpreter TLVInterpreter_VISA;
extern TLVInterpreter TLVInterpreter_JCB;
extern TLVInterpreter TLVInterpreter_MASTERCARD;
extern TLVInterpreter TLVInterpreter_PBOC;

const TLVInterpreter & GetTLVInterpreter(STANDARD);
const TLVInterpreter & GetTLVInterpreter(const BinData&AID );

__LIB_NAME_SPACE_END__

#endif