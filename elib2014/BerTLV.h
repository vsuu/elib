#ifndef BERTLV_H_RFT
#define BERTLV_H_RFT
#include "BinData.h"
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <istream>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <iostream>

#include "BerTLVInterpreter.h"

__LIB_NAME_SPACE_BEGIN__

class BerTLV;
using TagType = uint16_t;
//using BerTLVList = std::vector < BerTLV > ;

class BerTLVList : private std::vector < BerTLV >
{
public:
	template<typename OutputIterator>
	static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapTLVList(const BerTLVList &, OutputIterator out);
	template<typename OutputIterator>
	typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapTLVList(OutputIterator out)const
	{
		return EncapTLVList(*this, out);
	}
	static std::ostream& EncapTLVList(const BerTLVList &tlv_list, std::ostream& os)
	{
		EncapTLVList(tlv_list, std::ostreambuf_iterator<char>(os));
		return os;
	}
	std::ostream & EncapTLVList(std::ostream & os)const
	{
		EncapTLVList(*this, os);
		return os;
	}

	template<typename InputIterator>
	static InputIterator ParseTLVList(InputIterator beg, InputIterator end, size_t len, BerTLVList &);
	static std::istream & ParseTLVList(std::istream &is, size_t len, BerTLVList & tlv_list)
	{
		ParseTLVList(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), len, tlv_list);
		return is;
	}
	BinData ToBin()const;
	size_t Len()const;

	BerTLV *Child(TagType);
	const BerTLV *Child(TagType)const;
	BerTLV* Find(TagType);
	const BerTLV * Find(TagType)const;
	bool DeleteChild(TagType tag);
	void AppendChild(const BerTLV& tlv)
	{
		push_back(tlv);
	}
	void AppendChild(BerTLV&&tlv)
	{
		push_back(std::move(tlv));
	}
	bool InsertChild(TagType tag, const BerTLV &);
	bool InsertChild(TagType tag, BerTLV &&);
	void Clear()
	{
		clear();
	}
	size_t Count()const
	{
		return size();
	}
	bool Empty()const
	{
		return empty();
	}
	using std::vector<BerTLV>::begin;
	using std::vector<BerTLV>::end;
};

class BerTLV
{
public:
	BerTLV() = default;
	~BerTLV() = default;
	template<class InputIterator>
	BerTLV(InputIterator beg, InputIterator end)
	{
		ParseTLV(beg, end, *this);
	}
	explicit BerTLV(std::istream &is)
	{
		ParseTLV(is, *this);
	}

	explicit BerTLV(TagType  tag) :tag_(tag)
	{
	}

	BerTLV(TagType tag, const BerTLVList &list) : tag_(tag), children_(list)
	{
		if (!IsStruct())
		{
			throw std::logic_error(ERR_WHERE "叶子结点不能有子结点");
		}
	}
	BerTLV(TagType tag, BerTLVList &&list) :tag_(tag), children_(std::move(list))
	{
		if (!IsStruct())
		{
			throw std::logic_error(ERR_WHERE "叶子结点不能有子结点");
		}
	}

	explicit BerTLV(const BinData &data)
	{
		ParseTLV(data.begin(), data.end(), *this);
	}
	BerTLV::BerTLV(TagType tag, const BinData &data) :tag_(tag)
	{
		if (!IsStruct())
		{
			value_.assign(data.begin(), data.end());
		}
		else
		{
			BerTLVList::ParseTLVList(data.begin(), data.end(), data.size(), children_);
		}
	}
	BerTLV::BerTLV(TagType tag, const BinData &&data) :tag_(tag)
	{
		if (!IsStruct())
		{
			value_ = std::move(data);
		}
		else
		{
			BerTLVList::ParseTLVList(data.begin(), data.end(), data.size(), children_);
		}
	}

	TagType Tag()const
	{
		return tag_;
	}
	size_t TLen()const
	{
		return TLen(tag_);
	}
	size_t LLen()const
	{
		return LLen(VLen());
	}
	size_t VLen()const
	{
		return VLen(*this);
	}
	size_t Len()const
	{
		return TLVLen(*this);
	}

	//   BerTLV &operator=(const BerTLV &) = default;
	//   BerTLV &operator=(BerTLV &&);

	bool IsStruct()const
	{
		return IsStructTag(tag_);
	}
	void Clear()
	{
		tag_ = 0;
		value_.clear();
		children_.Clear();
	}
	const BinData &LeafValue()const
	{
		assert(!IsStruct());
		return value_;
	}
	const BerTLVList &Children()const
	{
		assert(IsStruct());
		return children_;
	}
	BinData Value()const
	{
		/*if (!IsStruct())
		{
		return value_;
		}
		else
		{
		BinData ret;
		for (auto & x : children_)
		{
		ret += x.ToBin();
		}
		return ret;
		}*/
		return VToBin();
	}
	//void GetValue(BinData &data)
	//{
	//	assert(!IsStruct());
	//	data.assign(value_.begin(), value_.end());
	//}
	//void GetValue(BerTLVList &list)
	//{
	//	assert(IsStruct());
	//	list = children_;
	//}
	void SetValue(const BinData &);
	void SetValue(BinData &&);

	void SetValue(const BerTLVList &list)
	{
		if (IsStruct())
		{
			children_ = list;
		}
		else
		{
			throw std::logic_error(ERR_WHERE "叶子结点不能调用本函数");
		}
	}
	void SetValue(BerTLVList && list)
	{
		if (IsStruct())
		{
			children_ = std::move(list);
		}
		else
		{
			throw std::logic_error(ERR_WHERE "叶子结点不能调用本函数");
		}
	}
	BerTLV *Child(TagType tag)
	{
		if (IsStruct())
		{
			return children_.Child(tag);
		}
		return nullptr;
	}
	const BerTLV *Child(TagType tag)const
	{
		return const_cast<BerTLV &>(*this).Child(tag);
	}
	BerTLV* Find(TagType tag)
	{
		if (IsStruct())
		{
			return children_.Find(tag);
		}
		return nullptr;
	}
	const BerTLV * Find(TagType tag)const
	{
		return const_cast<BerTLV &>(*this).Find(tag);
	}
	bool DeleteChild(TagType tag)
	{
		return children_.DeleteChild(tag);
	}
	void AppendChild(const BerTLV& tlv)
	{
		if (IsStruct())
		{
			children_.AppendChild(tlv);
		}
		else
		{
			throw std::runtime_error(ERR_WHERE "叶子结点不能添加孩子结点");
		}
	}
	void AppendChild(BerTLV&& tlv)
	{
		if (IsStruct())
		{
			children_.AppendChild(std::move(tlv));
		}
		else
		{
			throw std::runtime_error(ERR_WHERE "叶子结点不能添加孩子结点");
		}
	}
	bool InsertChild(TagType tag, const BerTLV &tlv)
	{
		if (IsStruct())
		{
			return children_.InsertChild(tag, tlv);
		}
		else
		{
			throw std::runtime_error(ERR_WHERE "叶子结点不能添加孩子结点");
		}
	}
	bool InsertChild(TagType tag, BerTLV && tlv)
	{
		if (IsStruct())
		{
			return children_.InsertChild(tag, std::move(tlv));
		}
		else
		{
			throw std::runtime_error(ERR_WHERE "叶子结点不能添加孩子结点");
		}
	}

	template <typename OutputIterator>
	typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapTag(OutputIterator  out)const
	{
		return EncapTag(tag_, out);
	}

	template<typename OutputIterator>
	typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapLen(OutputIterator out)const
	{
		return EncapLen(VLen(), out);
	}

	template<typename OutputIterator>
	typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapValue(OutputIterator out)const
	{
		return EncapValue(*this, out);
	}

	template<typename OutputIterator>
	typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapTLV(OutputIterator out)const
	{
		return EncapTLV(*this, out);
	}

	/*template<typename OutputIterator>
	typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
	EncapTLVList(OutputIterator out)const
	{
	return EncapTLVList(children_, out);
	}*/

	std::ostream & EncapTag(std::ostream & os)const
	{
		EncapTag(tag_, os);
		return os;
	}

	std::ostream & EncapLen(std::ostream & os)const
	{
		EncapLen(VLen(), os);
		return os;
	}

	std::ostream & EncapValue(std::ostream & os)const
	{
		EncapValue(*this, os);
		return os;
	}

	std::ostream & EncapTLV(std::ostream & os)const
	{
		EncapTLV(*this, os);
		return os;
	}

	//std::ostream & EncapTLVList(std::ostream & os)const
	//{
	//	EncapTLVList(children_, os);
	//	return os;
	//}

	//tobin
	BinData ToBin()const
	{
		BinData ret;
		EncapTLV(std::back_inserter(ret));
		return ret;
	}
	BinData TToBin()const
	{
		BinData ret;
		EncapTag(std::back_inserter(ret));
		return ret;
	}
	BinData LToBin()const
	{
		BinData ret;
		EncapLen(std::back_inserter(ret));
		return ret;
	}
	BinData VToBin()const
	{
		BinData ret;
		EncapValue(std::back_inserter(ret));
		return ret;
	}
	BinData LVToBin()const
	{
		BinData ret;
		EncapLen(std::back_inserter(ret));
		EncapValue(std::back_inserter(ret));
		return ret;
	}

	//parse
	template<typename InputIterator>
	static InputIterator ParseTag(InputIterator beg, InputIterator end, TagType &);
	template<typename InputIterator>
	static InputIterator ParseLen(InputIterator beg, InputIterator end, uint32_t &);
	template<typename InputIterator>
	static InputIterator ParseTLV(InputIterator beg, InputIterator end, BerTLV &);
	/*template<typename InputIterator>
	static InputIterator ParseTLVList(InputIterator beg, InputIterator end, size_t len, BerTLVList &);
	template<typename T>
	static const T* ParseTLVList(const T * beg, const T* end, BerTLVList & tlv_list);*/

	static std::istream & ParseTag(std::istream &is, TagType & tag)
	{
		ParseTag(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), tag);
		return is;
	}

	static std::istream & ParseLen(std::istream &is, uint32_t & len)
	{
		ParseLen(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), len);
		return is;
	}
	static std::istream & ParseTLV(std::istream &is, BerTLV &tlv)
	{
		ParseTLV(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), tlv);
		return is;
	}
	/*static std::istream & ParseTLVList(std::istream &is, size_t len, BerTLVList & tlv_list)
	{
	ParseTLVList(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), len, tlv_list);
	return is;
	}*/

	//encap
	template<typename OutputIterator>
	static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapTag(TagType tag, OutputIterator out);
	template<typename OutputIterator>
	static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapLen(uint32_t len, OutputIterator out);
	template<typename OutputIterator>
	static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapValue(const BerTLV &, OutputIterator out);
	template<typename OutputIterator>
	static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
		EncapTLV(const BerTLV &, OutputIterator out);
	/*template<typename OutputIterator>
	static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
	EncapTLVList(const BerTLVList &, OutputIterator out);*/

	static std::ostream& EncapTag(TagType tag, std::ostream& os)
	{
		EncapTag(tag, std::ostreambuf_iterator<char>(os));
		return os;
	}
	static std::ostream& EncapLen(uint32_t len, std::ostream& os)
	{
		EncapLen(len, std::ostreambuf_iterator<char>(os));
		return os;
	}

	static std::ostream& EncapValue(const BerTLV &tlv, std::ostream& os)
	{
		EncapValue(tlv, std::ostreambuf_iterator<char>(os));
		return os;
	}
	static std::ostream& EncapTLV(const BerTLV &tlv, std::ostream& os)
	{
		EncapTLV(tlv, std::ostreambuf_iterator<char>(os));
		return os;
	}
	/*static std::ostream& EncapTLVList(const BerTLVList &tlv_list, std::ostream& os)
	{
	EncapTLVList(tlv_list, std::ostreambuf_iterator<char>(os));
	return os;
	}*/

	//struct
	static bool IsStructTag(TagType tag)
	{
		if (tag & 0xFF00)
		{
			return (tag & 0x2000) != 0;
		}
		return (tag & 0x20) != 0;
	}

	//len
	static size_t TLen(TagType tag)
	{
		if (tag > 0xFF)return 2;
		return 1;
	}
	static size_t LLen(uint32_t);
	static size_t VLen(const BerTLV &);
	static size_t TLVLen(const BerTLV &tlv)
	{
		size_t vlen = VLen(tlv);
		return TLen(tlv.tag_) + LLen(vlen) + vlen;
	}

	//meaning
	static std::string GetDescription(TagType tag, const TLVInterpreter & interpret)
	{
		return interpret.GetDescription(tag);
	}
	static const std::vector<std::pair<std::string, std::string>> & GetDescriptionVec(TagType tag, const TLVInterpreter & interpret)
	{
		return interpret.GetDescriptionVec(tag);
	}
	static std::string GetMeaning(const BerTLV &tlv, const TLVInterpreter & interpret)
	{
		return interpret.GetMeaning(tlv);
	}
	static TLV_VALUE_FORMAT GetValueFomat(const BerTLV &tlv, const TLVInterpreter & interpret)
	{
		return interpret.GetValueFomat(tlv.tag_);
	}

	std::string GetDescription(const TLVInterpreter & interpret)const
	{
		return GetDescription(tag_, interpret);
	}

	const std::vector<std::pair<std::string, std::string>> & GetDescriptionVec(const TLVInterpreter & interpret)const
	{
		return GetDescriptionVec(tag_, interpret);
	}
	std::string GetMeaning(const TLVInterpreter & interpret)const
	{
		return GetMeaning(*this, interpret);
	}

	TLV_VALUE_FORMAT GetValueFomat(const TLVInterpreter & interpret)const
	{
		return GetValueFomat(*this, interpret);
	}
private:
	TagType tag_;
	BinData value_;
	BerTLVList children_;
};

template<typename InputIterator>
static InputIterator BerTLV::ParseTag(InputIterator beg, InputIterator end, TagType & tag)
{
	static_assert(sizeof(std::iterator_traits<InputIterator>::value_type) == 1, "ParseTag,InputIterator 必须指向字节流");
	if (beg == end)
	{
		throw std::invalid_argument(ERR_WHERE "ParseTag 出错，输入为空");
	}
	tag = static_cast<unsigned char>(*beg++);
	if ((tag & 0x1F) != 0x1F)
	{
		return beg;
	}

	int i = 1;
	while (beg != end)
	{
		if (++i > 2)
		{
			break;
		}
		auto data = *beg++;
		tag <<= 8;
		tag |= static_cast<unsigned char>(data);
		if ((static_cast<unsigned char>(data)& 0x80) == 0x00)
		{
			return beg;
		}
	}
	throw std::invalid_argument(ERR_WHERE "ParseTag 出错");
}
template<typename InputIterator>
InputIterator BerTLV::ParseLen(InputIterator beg, InputIterator end, uint32_t & len)
{
	static_assert(sizeof(std::iterator_traits<InputIterator>::value_type) == 1, "InputIterator 必须指向字节流");
	if (beg == end)
	{
		throw std::invalid_argument(ERR_WHERE "ParseLen 出错,位置1");//输入为空
	}
	auto data = *beg++;
	len = static_cast<unsigned char>(data);
	if (len < 0x80)
	{
		return beg;
	}
	size_t llen = len & 0x7F;
	if (llen > 4)
	{
		throw std::invalid_argument(ERR_WHERE "ParseLen 出错,位置2");//长度数据超过4字节
	}

	len = 0;
	int i = 0;
	while (beg != end)
	{
		data = *beg++;

		len <<= 8;
		len |= static_cast<unsigned char>(data);
		if (++i == llen)
		{
			return beg;
		}
	}
	throw std::invalid_argument(ERR_WHERE "ParseLen 出错,位置3");//数据不够
}
template<typename InputIterator>
InputIterator BerTLV::ParseTLV(InputIterator beg, InputIterator end, BerTLV & tlv)
{
	static_assert(sizeof(std::iterator_traits<InputIterator>::value_type) == 1, "InputIterator 必须指向字节流");
	tlv.Clear();
	beg = ParseTag(beg, end, tlv.tag_);
	uint32_t len = 0;
	beg = ParseLen(beg, end, len);

	if (0 == len)
	{
		return beg;
	}

	if (!tlv.IsStruct())
	{
		for (size_t i = 0; i < len; ++i)
		{
			if (beg == end)
			{
				throw std::invalid_argument(ERR_WHERE "ParseTLV 出错，位置1");//数据不够
			}
			tlv.value_.push_back(static_cast<unsigned char>(*beg++));
		}
		return beg;
	}
	else
	{
		return BerTLVList::ParseTLVList(beg, end, len, tlv.children_);
	}
}

template<typename InputIterator>
InputIterator BerTLVList::ParseTLVList(InputIterator beg, InputIterator end, size_t len, BerTLVList & tlv_list)
{
	static_assert(sizeof(std::iterator_traits<InputIterator>::value_type) == 1, "InputIterator 必须指向字节流");
	size_t i = 0;
	while (i < len)
	{
		tlv_list.emplace_back();
		beg = BerTLV::ParseTLV(beg, end, tlv_list.back());
		i += tlv_list.back().Len();
	}
	if (i == len)
	{
		return beg;
	}
	else
	{
		throw std::invalid_argument(ERR_WHERE "ParseTLVList 出错");//children 长度与 len不一致。
	}
}

//template<typename T>
//const T* BerTLV::ParseTLVList(const T * beg, const T* end, BerTLVList & tlv_list)
//{
//    static_assert(sizeof(T) == 1, "ParseTLVList,InputIterator必须指向字节流");
//    while (true)
//    {
//        tlv_list.emplace_back();
//        beg = ParseTLV(beg, end, tlv_list.back());
//        if (beg == end)
//        {
//            break;
//        }
//    }
//    return beg;
//}

template<typename OutputIterator>
typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
BerTLV::EncapTag(TagType tag, OutputIterator out)
{
	if (tag > 0xFF)
	{
		*out++ = static_cast<unsigned char>((tag >> 8) & 0xFF);
	}
	*out++ = static_cast<unsigned char>(tag & 0xFF);
	return out;
}
template<typename OutputIterator>
typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
BerTLV::EncapLen(uint32_t len, OutputIterator out)
{
	if (len < 0x80)
	{
		*out++ = static_cast<unsigned char>(len & 0xFF);
		return out;
	}
	else
	{
		std::vector<unsigned char> tmp;
		while (len > 0)
		{
			tmp.push_back(static_cast<unsigned char>(len & 0xFF));
			len >>= 8;
		}
		*out++ = static_cast<unsigned char>(0x80 | tmp.size());
		return std::reverse_copy(begin(tmp), end(tmp), out);
	}
}

template<typename OutputIterator>
typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
BerTLV::EncapValue(const BerTLV &tlv, OutputIterator out)
{
	if (!tlv.IsStruct())
	{
		return copy(begin(tlv.value_), end(tlv.value_), out);
	}
	else
	{
		return BerTLVList::EncapTLVList(tlv.children_, out);
	}
}

template<typename OutputIterator>
typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
BerTLV::EncapTLV(const BerTLV &tlv, OutputIterator out)
{
	out = EncapTag(tlv.tag_, out);
	out = EncapLen(tlv.VLen(), out);
	return EncapValue(tlv, out);
}

template<typename OutputIterator>
typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
BerTLVList::EncapTLVList(const BerTLVList &tlv_list, OutputIterator out)
{
	for (auto &x : tlv_list)
	{
		out = BerTLV::EncapTLV(x, out);
	}
	return out;
}

__LIB_NAME_SPACE_END__

#endif