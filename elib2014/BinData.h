#ifndef BINDATA_H_RFT
#define BINDATA_H_RFT

#include <vector>
#include <string>

#include "libbase.h"
#include "strop.h"
#include <cassert>

__LIB_NAME_SPACE_BEGIN__

class HexData;
class BinData :public std::vector < unsigned char >
{
	//using std::vector<unsigned char>::vector;//vs2013 not support
	typedef BinData self;
public:
	BinData() = default;
	~BinData() = default;
	/*
	BinData(const BinData &o) :std::vector<unsigned char>::vector(o)
	{
	}
	BinData(BinData &&o) : std::vector<unsigned char>::vector(std::move(o))
	{
	}
	self &operator=(const BinData &o)
	{
	std::vector<unsigned char>::operator=(o);
	return *this;
	}
	self &operator=(BinData &&o)
	{
	std::vector<unsigned char>::operator=(std::move(o));
	return *this;
	}*/
	template <class InputIterator>
	BinData(InputIterator first, InputIterator last) :std::vector<unsigned char>::vector(first, last)
	{
	}
	BinData(size_t n, unsigned char x = 0x00) :std::vector<unsigned char>::vector(n, x)
	{}
	BinData(std::initializer_list<unsigned char> il) :std::vector<unsigned char>::vector(il)
	{}
	BinData SubData(size_t pos)const
	{
		return BinData(data() + pos, data() + size());
	}
	BinData SubData(size_t pos, size_t len)const
	{
		if (pos + len > size())throw std::range_error(ERR_WHERE);
		return BinData(data() + pos, data() + pos + len);
	}
	BinData BackData(size_t len)const
	{
		if (len > size())throw std::range_error(ERR_WHERE);
		return BinData(data()+size()-len, data() + size());
	}
	self operator+(const self &o)const
	{
		self ret(*this);
		return ret += o;
	}
	self &operator+=(const self &o)
	{
		insert(end(), o.begin(), o.end());
		return *this;
	}

	HexData ToHex()const;

	//slice

	/*class Slice_
	{
	public:
	Slice_(BinData::const_iterator beg, BinData::const_iterator end) :beg_(beg), end_(end)
	{}
	BinData::const_iterator begin()const
	{
	return beg_;
	}
	BinData::const_iterator end()const
	{
	return end_;
	}
	unsigned char operator[](size_t pos)const
	{
	return *next(beg_, pos);
	}
	HexData ToHex()const;

	BinData ToBin()const
	{
	return BinData(beg_, end_);
	}
	private:
	BinData::const_iterator beg_, end_;
	};

	Slice_ Slice(size_t pos)
	{
	return Slice_(next(begin(),pos), end());
	}
	Slice_ Slice(size_t pos, size_t len)
	{
	return Slice_(next(begin(), pos), next(begin(), pos + len));
	}
	Slice_ BackSlice(size_t pos)
	{
	return Slice_(begin(), prev(end(), pos));
	}
	Slice_ BackSlice(size_t pos, size_t len)
	{
	return Slice_(prev(end(), pos + len), prev(end(), pos));
	}*/
};

class HexData : public std::string
{
	typedef HexData self;
	//using std::string::basic_string;   vs2013 not support
public:
	HexData() = default;
	~HexData() = default;
	/*HexData(const HexData &o) :std::string::basic_string(o)
	{}
	HexData(HexData &&o) :std::string::basic_string(std::move(o))
	{}
	self & operator=(const self &o)
	{
	std::string::operator=(o);
	return *this;
	}

	self & operator=(const self &&o)
	{
	std::string::operator=(std::move(o));
	return *this;
	}*/
	HexData(const std::string& str) :std::string::basic_string(str)
	{}
	HexData(const char* s) :std::string::basic_string(s)
	{}
	template <class InputIterator>
	HexData(InputIterator first, InputIterator last) : std::string::basic_string(first, last)
	{}
	HexData(std::string&& str) : std::string::basic_string(std::move(str))
	{}

	BinData ToBin()const
	{
		BinData ret;
		Hex2Bin(*this, std::back_inserter(ret));
		return ret;
	}
};
//
inline BinData Hex2Bin(const char *s)
{
	BinData ret;
	Hex2Bin(s, std::back_inserter(ret));
	return ret;
}
inline BinData Hex2Bin(const std::string &s)
{
	return Hex2Bin(s.c_str());
}

template<typename InputIterator>
inline HexData Bin2Hex(InputIterator b, InputIterator e)
{
	HexData ret;
	Bin2Hex(b, e, std::back_inserter(ret));
	return ret;
}

inline HexData BinData::ToHex()const
{
	HexData ret;
	Bin2Hex(begin(), end(), std::back_inserter(ret));
	return ret;
}
//
//inline HexData BinData::Slice_::ToHex()const
//{
//	return Bin2Hex(beg_, end_);
//}

namespace {
	union
	{
		char c[4];
		int i;
	}endian_test{ { 'l', '?', '?', 'b' } };
}

#define ENDIANNESS ((char)endian_test.i)

template<class T>
BinData IntergerToBin(T i)
{
	static_assert(std::is_integral<T>::value, "±¾º¯Êý½öÊÊÓÃÕûÊýÀàÐÍ");
	unsigned char *p = reinterpret_cast<unsigned char*>(&i);
	BinData ret(p, p + sizeof(T));
	if (ENDIANNESS == 'l')
	{
		std::reverse(ret.begin(), ret.end());
	}
	return ret;
}
#undef ENDIANNESS

template<class Container>
Container::value_type &back(Container &container,size_t n)
{
	if(n>0 && n<=container.size())
	{
		return container[size()-n];
	}
	else
	{
		throw std::out_of_range("back");
	}
}

template<class Container>
const Container::value_type &back(const Container &container,size_t n)
{
	if(n>0 && n<=container.size())
	{
		return container[size()-n];
	}
	else
	{
		throw std::out_of_range("back");
	}
}

__LIB_NAME_SPACE_END__

#endif
