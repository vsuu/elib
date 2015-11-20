#include "BerTLV.h"
#include "TLVDescription.h"
#include <algorithm>

using namespace std;

__LIB_NAME_SPACE_BEGIN__
//
//const unsigned char * BerTLV::ParseTag(const unsigned char *data, size_t len, TagType &tag)
//{
//    tag = 0;
//    auto taglen = min(2u, len);
//    const unsigned char *ptr = data;
//
//    for (size_t i = 0; i < taglen; ++i)
//    {
//        tag <<= 8;
//        tag |= *ptr;
//        if (((*ptr++) & 0x80) == 0x00)
//        {
//            return ptr;
//        }
//    }
//    throw invalid_argument("解析TAG出错，输入的数据是错误的");
//}
//const unsigned char * BerTLV::ParseLen(const unsigned char *data, size_t len, uint32_t &taglen)
//{
//    if (len < 1)
//    {
//        invalid_argument("解析TAG Len出错，输入的数据是错误的");
//    }
//    taglen = 0;
//    size_t llen = 1;
//    if ((*data & 0x80) == 0x00)
//    {
//        taglen = *data++;
//    }
//    else
//    {
//        llen += *data++ & 0x80;
//        if (len < llen)
//        {
//            invalid_argument("解析TAG Len出错，输入的数据是错误的");
//        }
//        llen--;
//        for (size_t i = 0; i < llen; ++i)
//        {
//            taglen <<= 8;
//            taglen |= *data++;
//        }
//    }
//    return data;
//}
//const unsigned char * BerTLV::ParseTLV(const unsigned char *data, size_t len, BerTLV &tlv)
//{
//    const unsigned char *ptr = data;
//    TagType tag = 0;
//    tlv.Clear();
//    ptr = ParseTag(ptr, len, tag);
//    tlv.SetTag(tag);
//    uint32_t tlen = 0;
//    ptr = ParseLen(ptr, len - (ptr - data), tlen);
//    if (tlen + (ptr - data) > len)
//    {
//        invalid_argument("解析TAG value出错，输入的数据是错误的");
//    }
//    if (!tlv.HasChildren())
//    {
//        tlv.value_.Reset(ptr, tlen);
//    }
//    else
//    {
//        ParseTLVList(ptr, tlen, tlv.children_);
//    }
//    return ptr + tlen;
//}
//
//void BerTLV::ParseTLVList(const unsigned char *data, size_t len, BerTLVList &list)
//{
//    list.clear();
//    const unsigned char *tmp = data;
//    while (len > 0)
//    {
//        list.emplace_back();
//        data = ParseTLV(data, len, list.back());
//        len -= data - tmp;
//        tmp = data;
//    }
//}

template
std::istreambuf_iterator<char> BerTLV::ParseTag(std::istreambuf_iterator<char> beg, std::istreambuf_iterator<char> end, TagType &);
template
std::istreambuf_iterator<char> BerTLV::ParseLen(std::istreambuf_iterator<char> beg, std::istreambuf_iterator<char> end, uint32_t &);
template
std::istreambuf_iterator<char> BerTLV::ParseTLV(std::istreambuf_iterator<char> beg, std::istreambuf_iterator<char> end, BerTLV &);
template
std::istreambuf_iterator<char> BerTLVList::ParseTLVList(std::istreambuf_iterator<char> beg, std::istreambuf_iterator<char> end, size_t len, BerTLVList &);

template
std::istream_iterator< char> BerTLV::ParseTag(std::istream_iterator< char> beg, std::istream_iterator< char> end, TagType &);
template
std::istream_iterator< char> BerTLV::ParseLen(std::istream_iterator< char> beg, std::istream_iterator< char> end, uint32_t &);
template
std::istream_iterator< char> BerTLV::ParseTLV(std::istream_iterator< char> beg, std::istream_iterator< char> end, BerTLV &);
template
std::istream_iterator< char> BerTLVList::ParseTLVList(std::istream_iterator< char> beg, std::istream_iterator< char> end, size_t len, BerTLVList &);

template
const unsigned char * BerTLV::ParseTag(const unsigned char * beg, const unsigned char * end, TagType &);
template
const unsigned char * BerTLV::ParseLen(const unsigned char * beg, const unsigned char * end, uint32_t &);
template
const unsigned char * BerTLV::ParseTLV(const unsigned char * beg, const unsigned char * end, BerTLV &);
template
const unsigned char * BerTLVList::ParseTLVList(const unsigned char * beg, const unsigned char * end, size_t len, BerTLVList &);
//template
//const unsigned char * BerTLV::ParseTLVList(const unsigned char * beg, const unsigned char* end, BerTLVList & tlv_list);

template
const   char * BerTLV::ParseTag(const   char * beg, const   char * end, TagType &);
template
const   char * BerTLV::ParseLen(const   char * beg, const   char * end, uint32_t &);
template
const   char * BerTLV::ParseTLV(const   char * beg, const   char * end, BerTLV &);
template
const   char * BerTLVList::ParseTLVList(const   char * beg, const   char * end, size_t len, BerTLVList &);
/*template
const   char * BerTLV::ParseTLVList(const   char * beg, const   char* end, BerTLVList & tlv_list)*/;

size_t BerTLV::LLen(uint32_t vlen)
{
	if (vlen < 0x80)
	{
		return 1;
	}
	else
	{
		size_t ret = 1;
		while (vlen > 0)
		{
			ret++;
			vlen >>= 8;
		}
		return ret;
	}
}
size_t BerTLV::VLen(const BerTLV &tlv)
{
	if (!tlv.IsStruct())
	{
		return tlv.value_.size();
	}
	else
	{
		return tlv.children_.Len();
	}
}

//
//BerTLV &BerTLV::operator=(BerTLV && other)
//{
//    if (this != &other)
//    {
//        tag_ = std::move(other.tag_);
//        value_ = std::move(other.value_);
//        children_ = std::move(other.children_);
//    }
//    return *this;
//}

void BerTLV::SetValue(const BinData &data)
{
	if (!IsStruct())
	{
		value_.assign(data.begin(), data.end());
	}
	else
	{
		children_.Clear();
		BerTLVList::ParseTLVList(data.begin(), data.end(), data.size(), children_);
	}
}

void BerTLV::SetValue(BinData &&data)
{
	if (!IsStruct())
	{
		value_ = std::move(data);
	}
	else
	{
		children_.Clear();
		BerTLVList::ParseTLVList(data.begin(), data.end(), data.size(), children_);
	}
}
//void BerTLV::SetValue(BinData &&data)
//{
//    if (!HasChildren())
//    {
//        value_ std::move(data);
//    }
//    else
//    {
//        ParseTLVList(data.Data(), data.ByteLen(), children_);
//    }
//}
//void BerTLV::SetChildren(const BerTLVList &list)
//{
//    if (IsStruct())
//    {
//        children_ = list;
//    }
//    else
//    {
//        throw logic_error("叶子结点不能SetChildren");
//    }
//}
//void BerTLV::SetChildren(BerTLVList &&list)
//{
//    if (IsStruct())
//    {
//        children_ = std::move(list);
//    }
//    else
//    {
//        throw logic_error("叶子结点不能SetChildren");
//    }
//}

BerTLV *BerTLVList::Child(TagType tag)
{
	for (auto &x : *this)
	{
		if (x.Tag() == tag)
		{
			return &x;
		}
	}

	return nullptr;
}
const BerTLV *BerTLVList::Child(TagType tag)const
{
	return const_cast<BerTLVList *>(this)->Child(tag);
}

BerTLV* BerTLVList::Find(TagType tag)
{
	BerTLV *ret = nullptr;

	for (auto &x : *this)
	{
		if (x.Tag() == tag)
		{
			return &x;
		}
		if ((ret = x.Find(tag)) != nullptr)
		{
			return ret;
		}
	}

	return nullptr;
}
const BerTLV * BerTLVList::Find(TagType tag)const
{
	return const_cast<BerTLVList *>(this)->Find(tag);
}
bool BerTLVList::DeleteChild(TagType tag)
{
	for (auto it = begin(); it != end(); ++it)
	{
		if (it->Tag() == tag)
		{
			erase(it);
			return true;
		}
		if (it->DeleteChild(tag))
		{
			return true;
		}
	}
	return false;
}

bool BerTLVList::InsertChild(TagType tag, const BerTLV &tlv)
{
	for (auto it = begin(); it != end(); ++it)
	{
		if (it->Tag() == tag)
		{
			++it;
			insert(it, tlv);
			return true;
		}
		if (it->InsertChild(tag, tlv))
		{
			return true;
		}
	}

	return false;
}
bool BerTLVList::InsertChild(TagType tag, BerTLV &&tlv)
{
	for (auto it = begin(); it != end(); ++it)
	{
		if (it->Tag() == tag)
		{
			++it;
			insert(it, std::move(tlv));
			return true;
		}
		if (it->InsertChild(tag, std::move(tlv)))
		{
			return true;
		}
	}

	return false;
}

BinData BerTLVList::ToBin()const
{
	BinData ret;
	for (auto &x : *this)
	{
		ret += x.ToBin();
	}
	return ret;
}
size_t BerTLVList::Len()const
{
	size_t ret = 0;
	for (auto &x : *this)
	{
		ret += x.Len();
	}
	return ret;
}

//BinData BerTLV::ToBin()const
//{
//    return TToBin() + LToBin() + VToBin();
//}
//BinData BerTLV::TToBin()const
//{
//    vector<unsigned char> tmp;
//
//    TagType tag = tag_;
//    while (tag_ > 0)
//    {
//        tmp.push_back(tag_ & 0xFF);
//        tag >>= 8;
//    }
//    reverse(tmp.begin(), tmp.end());
//    return BinData(tmp.data(), tmp.size());
//}
//BinData BerTLV::LToBin()const
//{
//    vector<unsigned char> tmp;
//    uint32_t len = LLen();
//    if (len == 0)
//    {
//        return BinData((unsigned char *)"\x00", 1);
//    }
//    while (len > 0)
//    {
//        tmp.push_back(len & 0xFF);
//        len >>= 8;
//    }
//    reverse(tmp.begin(), tmp.end());
//    return BinData(tmp.data(), tmp.size());
//}
//BinData BerTLV::VToBin()const
//{
//    if (!IsStruct())
//    {
//        return BinData(value_.data(), static_cast<int>(value_.size()));
//    }
//    else
//    {
//        BinData ret;
//        for (auto &x : children_)
//        {
//            ret += x.ToBin();
//        }
//        return ret;
//    }
//}
//BinData BerTLV::LVToBin()const
//{
//    return LToBin() + VToBin();
//}

__LIB_NAME_SPACE_END__