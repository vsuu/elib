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
	virtual void UseReader(const std::string &name)//ʹ������Ϊname�Ķ�����
	{
		reader_name_ = name;
	}
	SCardInterface() = default;
	virtual ~SCardInterface() = 0
	{}
	virtual void Connect() = 0;//���ӿ�Ƭ
	virtual void DisConnect() = 0;//�Ͽ��뿨Ƭ������
	const std::string & GetReaderNameInUse()const//��ѯ������ǰ����ʹ�õĶ�����������
	{
		return reader_name_;
	}
	virtual void WaitForCardIn() = 0;//������������ǰ�߳�ֱ�����������п�Ƭ�������
	virtual void WaitForCardOut() = 0;//������������ǰ�߳�ֱ�����������п�Ƭ�γ�
	virtual void CardReset() = 0;//�Կ�Ƭ�����ȸ�λ����
	virtual void CardRestart() = 0;//�Կ�Ƭ�����临λ����
	virtual bool IsCardIn() = 0;
	virtual bool IsConnected() = 0;
	virtual BinData GetATR() = 0;//��ȡ��ƬATR
	virtual ApduRsp TransmitMsg(const ApduCmd&) = 0;//��Ƭ����ָ����ؿ�Ƭ����
	virtual void Cancel() = 0;//ȡ�����ڽ��еĲ���
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