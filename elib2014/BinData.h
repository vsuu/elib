#ifndef BINDATA_H_RFT
#define BINDATA_H_RFT

#include <vector>
#include <string>

#include "libbase.h"
#include "strop.h"

__LIB_NAME_SPACE_BEGIN__

class HexData;
class BinData :public std::vector < unsigned char >
{
    //using std::vector<unsigned char>::vector;//vs2013 not support
    typedef BinData self;
public:
    BinData() = default;
    ~BinData() = default;
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
    }
    template <class InputIterator>
    BinData(InputIterator first, InputIterator last) :std::vector<unsigned char>::vector(first, last)
    {
    }
    BinData(size_t n) :std::vector<unsigned char>::vector(n)
    {}

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
};

class HexData : public std::string
{
    typedef HexData self;
    //using std::string::basic_string;   vs2013 not support
public:
    HexData() = default;
    ~HexData() = default;
    HexData(const HexData &o) :std::string::basic_string(o)
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
    }
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

__LIB_NAME_SPACE_END__

#endif