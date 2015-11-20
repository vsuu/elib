#ifndef MSG_H_RFT
#define MSG_H_RFT
#include "libbase.h"
#include "BinData.h"
#include <cstdint>
#include <list>

__LIB_NAME_SPACE_BEGIN__

class Message
{
public:
	explicit Message(uint32_t type) :msg_type_(type)
	{}
	explicit Message(const elib::BinData &);
	void AddField(int, const elib::BinData &);
	void AddField(int, elib::BinData &&);
	elib::BinData ToBin()const;
	bool IsFieldIn(int);
	const elib::BinData &Field(int);
	uint32_t Type()const
	{
		return msg_type_;
	}
private:
	std::list<std::pair<int, elib::BinData>> buff_;
	uint32_t msg_type_;
};

__LIB_NAME_SPACE_END__

#endif