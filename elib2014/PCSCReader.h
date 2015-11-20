#ifndef PCSCREADER_H_RFT
#define PCSCREADER_H_RFT
#include "Reader.h"

__LIB_NAME_SPACE_BEGIN__

class PCSCReader : public SCardInterface
{
public:
	virtual std::vector<std::string> ListReaderName(std::function<bool(const char *)> filter = nullptr);
	virtual void UseReader(const std::string &name);//使用名称为name的读卡器
	PCSCReader();
	virtual ~PCSCReader();
	virtual void Connect();//连接卡片
	virtual void DisConnect();//断开与卡片的连接
	virtual void WaitForCardIn();//本函数阻塞当前线程直到读卡器中有卡片插入进来
	virtual void WaitForCardOut();
	virtual void CardReset();//对卡片进行热复位操作
	virtual void CardRestart();//对卡片进行冷复位操作
	virtual BinData GetATR();//获取卡片ATR
	virtual ApduRsp TransmitMsg(const ApduCmd&);//向卡片发送指令并返回卡片返回
	virtual void Cancel();
	virtual bool IsCardIn();
	virtual bool IsConnected();
private:
	class impl;
	impl *impl_;
};

__LIB_NAME_SPACE_END__

#endif