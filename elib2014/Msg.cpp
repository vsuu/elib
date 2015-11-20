#include "Msg.h"
#include <stdexcept>

using namespace std;

__LIB_NAME_SPACE_BEGIN__

BinData ParseFiled2(const BinData &data, size_t &pos)
{
	char tmp[4] = {};
	memcpy(tmp, &data[pos], 2);
	int len = atoi(tmp);
	BinData ret = data.SubData(pos, len + 2);
	pos += len + 2;
	return ret;
}

BinData ParseFiled55(const BinData &data, size_t &pos)
{
	char tmp[4] = {};
	memcpy(tmp, &data[pos], 3);
	int len = atoi(tmp);
	BinData ret = data.SubData(pos, len + 3);
	pos += len + 3;
	return ret;
}

BinData ParseField(int field_id, const BinData &data, size_t &pos)
{
	BinData ret;
	switch (field_id)
	{
	case 2:
		return ParseFiled2(data, pos);
		break;
	case 23:
		pos += 3;
		return data.SubData(pos - 3, 3);
		break;
	case 52:
		pos += 8;
		return data.SubData(pos - 8, 8);
		break;
	case 55:
		return ParseFiled55(data, pos);
		break;
	default:
		throw runtime_error(ERR_WHERE "该域还不支持解析" + to_string(field_id));
		break;
	}

	return ret;
}

Message::Message(const elib::BinData &msg) :msg_type_(0)
{
	//获取报文类型
	size_t pos = 46;
	for (size_t i = 0; i < 4; ++i)
	{
		msg_type_ <<= 8;
		msg_type_ += msg[pos];
	}
	pos += 4;
	//获取位图
	BinData BitMap;
	if (msg[pos] & 0x01)
	{
		BitMap = msg.SubData(pos, 16);
		pos += 16;
	}
	else
	{
		BitMap = msg.SubData(pos, 8);
		pos += 8;
	}
	int field_id;
	for (size_t i = 0; i < BitMap.size(); ++i)
	{
		for (size_t j = 0; j < 8; ++j)
		{
			if (BitMap[i] & (0x01 << j))
			{
				field_id = i * 8 + j + 1;
				buff_.emplace_back(field_id, ParseField(field_id, msg, pos));
			}
		}
	}
}

void CheckField(int field_id)
{
	if ((field_id < 2) || (field_id == 65))
	{
		throw runtime_error(ERR_WHERE "非法的位域");
	}
}
void Message::AddField(int field_id, const elib::BinData & data)
{
	CheckField(field_id);

	for (auto it = buff_.begin(); it != buff_.end(); ++it)
	{
		if (it->first < field_id)
		{
			continue;
		}
		else if (it->first > field_id)
		{
			buff_.emplace(it, field_id, data);
		}
		else
		{
			throw runtime_error(ERR_WHERE "这个域已经存在!");
		}
	}
	buff_.emplace_back(field_id, data);
}

void Message::AddField(int field_id, elib::BinData && data)
{
	CheckField(field_id);

	for (auto it = buff_.begin(); it != buff_.end(); ++it)
	{
		if (it->first < field_id)
		{
			continue;
		}
		else if (it->first > field_id)
		{
			buff_.emplace(it, field_id, std::move(data));
			return;
		}
		else
		{
			throw runtime_error(ERR_WHERE "这个域已经存在!");
		}
	}
	buff_.emplace_back(field_id, std::move(data));
}

elib::BinData Message::ToBin()const
{
	//报文头
	BinData Head(46);
	Head[0] = 45;
	//报文类型
	BinData MsgType(4);
	MsgType[0] = (msg_type_ >> 24) & 0xFF;
	MsgType[1] = (msg_type_ >> 16) & 0xFF;
	MsgType[2] = (msg_type_ >> 8) & 0xFF;
	MsgType[3] = msg_type_ & 0xFF;
	//位图
	BinData BitMap(8);
	//域
	BinData data;
	for (auto &x : buff_)
	{
		data += x.second;
		//修改位图
		if (x.first > 64 && BitMap.size() < 8)
		{
			BitMap.resize(16);
			BitMap[0] |= 0x01;
		}
		BitMap[(x.first - 1) / 8] |= 0x01 << ((x.first - 1) % 8);
	}
	char tmp[6] = {};
	sprintf(tmp, "%04d", data.size() + 46 + 4 + BitMap.size());
	memcpy(Head.data() + 2, tmp, 4);

	return Head + MsgType + BitMap + data;
}

bool Message::IsFieldIn(int field_id)
{
	for (auto &x : buff_)
	{
		if (x.first == field_id)
		{
			return true;
		}
	}
	return false;
}

const elib::BinData &Message::Field(int field_id)
{
	for (auto &x : buff_)
	{
		if (x.first == field_id)
		{
			return x.second;
		}
	}
	throw runtime_error(ERR_WHERE "报文中不存在这个域:" + to_string(field_id));
}

__LIB_NAME_SPACE_END__