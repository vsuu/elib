#include "Apdu.h"

__LIB_NAME_SPACE_BEGIN__

ApduCmd::ApduCmd(const BinData &data)
{
	if (data.size() < 4)
	{
		throw std::invalid_argument(ERR_WHERE "apdu数据长度小于4");
	}
	cla_ = data[0];
	ins_ = data[1];
	p1_ = data[2];
	p2_ = data[3];
	if (data.size() == 4)
	{
		le_ = -1;
		return;
	}
	int len = 0;
	size_t pos = 0;
	if (data[4] != 0)
	{
		len = data[4];
		pos = 5;
	}
	else if (data.size() == 5)
	{//兼容CCID格式
		le_ = 0;
		return;
	}
	else
	{
		if (data.size() < 7)
		{
			throw std::invalid_argument(ERR_WHERE "apdu格式错误");
		}
		len = data[5];
		len <<= 8;
		len += data[6];
		pos = 7;
	}
	if (data.size() == pos)
	{
		le_ = len;
		return;
	}
	if (data.size() < pos + len)
	{
		throw std::invalid_argument(ERR_WHERE "apdu格式错误");
	}
	data_.assign(data.data() + pos, data.data() + pos + len);
	pos += len;
	if (data.size() == pos)
	{
		le_ = -1;
		return;
	}
	len = data[pos];
	if (len == 0)
	{
		if (data.size() >= pos + 2)
		{
			len = data[pos + 1];
			len <<= 8;
			len += data[pos + 2];
		}
		else
		{//兼容CCID,不报错
		}
	}
	le_ = len;
}

__LIB_NAME_SPACE_END__