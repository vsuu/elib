#ifndef READER_H_RFT
#define READER_H_RFT

#include "libbase.h"
#include "ClassAttr.h"
#include <string>
#include <vector>
#include <functional>
#include "BinData.h"
#include "Apdu.h"
#include <stdexcept>
#include <stdint.h>

__LIB_NAME_SPACE_BEGIN__

class SCardInterface :public  nocopyable
{
public:
	virtual std::vector<std::string> ListReaderName(std::function<bool(const char *)> filter = nullptr) = 0;
	virtual void UseReader(const std::string &name)//使用名称为name的读卡器
	{
		reader_name_ = name;
	}
	SCardInterface() = default;
	virtual ~SCardInterface() = 0
	{}
	virtual void Connect() = 0;//连接卡片
	virtual void DisConnect() = 0;//断开与卡片的连接
	const std::string & GetReaderNameInUse()const//查询本程序当前正在使用的读卡器的名称
	{
		return reader_name_;
	}
	virtual void WaitForCardIn() = 0;//本函数阻塞当前线程直到读卡器中有卡片插入进来
	virtual void WaitForCardOut() = 0;//本函数阻塞当前线程直到读卡器中有卡片拔出
	virtual void CardReset() = 0;//对卡片进行热复位操作
	virtual void CardRestart() = 0;//对卡片进行冷复位操作
	virtual bool IsCardIn() = 0;
	virtual bool IsConnected() = 0;
	virtual BinData GetATR() = 0;//获取卡片ATR
	virtual ApduRsp TransmitMsg(const ApduCmd&) = 0;//向卡片发送指令并返回卡片返回
	virtual void Cancel() = 0;//取消正在进行的操作
private:
	std::string reader_name_;
};

class ScardError : public std::runtime_error
{
public:
	ScardError(int code, const char *err) :std::runtime_error(err), code_(code)
	{}
	ScardError(int code, const std::string &err) : std::runtime_error(err), code_(code)
	{}
	bool IsCancel()const;
private:
	int code_;
};

class ScardNoCard : public std::runtime_error
{
public:
	ScardNoCard(const char *err) :std::runtime_error(err)
	{}
	ScardNoCard(const std::string &err) : std::runtime_error(err)
	{}
};

class ScardNoReader : public std::runtime_error
{
public:
	ScardNoReader(const char *err) :std::runtime_error(err)
	{}
	ScardNoReader(const std::string &err) : std::runtime_error(err)
	{}
};

__LIB_NAME_SPACE_END__

#endif