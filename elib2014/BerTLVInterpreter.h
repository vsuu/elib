#ifndef BERTLV_INTERPRETER_H_RFT
#define BERTLV_INTERPRETER_H_RFT

#include <string>
#include "libbase.h"
#include <cstdint>

__LIB_NAME_SPACE_BEGIN__

class BerTLV;

class TLVInterpreter_;
class TLVInterpreter//获取BerTLV相关介绍
{
public:
    TLVInterpreter(TLVInterpreter_ *);
    ~TLVInterpreter();
    std::string GetDescription(uint16_t tag)const;
    //获取本TLV数据表示的信息
    std::string GetMeaning(const BerTLV &tlv)const;
private:
    TLVInterpreter_ *impl_;
};

extern TLVInterpreter TLVInterpreter_VISA;
extern TLVInterpreter TLVInterpreter_JCB;
extern TLVInterpreter TLVInterpreter_MASTERCARD;
extern TLVInterpreter TLVInterpreter_PBOC;

__LIB_NAME_SPACE_END__

#endif