#ifndef PCSCREADER_H_RFT
#define PCSCREADER_H_RFT
#include "Reader.h"

__LIB_NAME_SPACE_BEGIN__

class PCSCReader : public SCardInterface
{
public:
	virtual std::vector<std::string> ListReaderName(std::function<bool(const char *)> filter = nullptr);
	virtual void UseReader(const std::string &name);//ʹ������Ϊname�Ķ�����
	PCSCReader();
	virtual ~PCSCReader();
	virtual void Connect();//���ӿ�Ƭ
	virtual void DisConnect();//�Ͽ��뿨Ƭ������
	virtual void WaitForCardIn();//������������ǰ�߳�ֱ�����������п�Ƭ�������
	virtual void WaitForCardOut();
	virtual void CardReset();//�Կ�Ƭ�����ȸ�λ����
	virtual void CardRestart();//�Կ�Ƭ�����临λ����
	virtual BinData GetATR();//��ȡ��ƬATR
	virtual ApduRsp TransmitMsg(const ApduCmd&);//��Ƭ����ָ����ؿ�Ƭ����
	virtual void Cancel();
	virtual bool IsCardIn();
	virtual bool IsConnected();
private:
	class impl;
	impl *impl_;
};

__LIB_NAME_SPACE_END__

#endif