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
using BerTLVList = std::vector < BerTLV > ;
using TagType = uint16_t;

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
    BerTLV(std::istream &is)
    {
        ParseTLV(is, *this);
    }

    BerTLV(TagType  tag) :tag_(tag)
    {
    }

    BerTLV(TagType tag, const BerTLVList &list) : tag_(tag), children_(list)
    {
        if (!IsStruct())
        {
            throw std::logic_error(ERR_WHERE "叶子结点不能有子结点");
        }
    }

    BerTLV(const BinData &);
    BerTLV(TagType tag, const BinData &data) :tag_(tag)
    {
        if (!IsStruct())
        {
            value_.assign(data.begin(), data.end());
        }
        else
        {
            ParseTLVList(data.begin(), data.end(), data.size(), children_);
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

    BerTLV &operator=(const BerTLV &) = default;
    BerTLV &operator=(BerTLV &&);

    bool IsStruct()const
    {
        return IsStructTag(tag_);
    }
    void Clear()
    {
        tag_ = 0;
        value_.clear();
        children_.clear();
    }
    void GetValue(BinData &data)
    {
        assert(!IsStruct());
        data.assign(value_.begin(), value_.end());
    }
    void GetValue(BerTLVList &list)
    {
        assert(IsStruct());
        list = children_;
    }
    void SetValue(const BinData &);
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

    BerTLV* FindChild(TagType);
    const BerTLV * FindChild(TagType)const;
    bool DeleteChild(TagType tag);
    void AppendChild(const BerTLV&);
    void AppendChild(BerTLV&&);
    bool InsertChild(TagType tag, const BerTLV &);
    bool InsertChild(TagType tag, BerTLV &&);

    template <typename OutputIterator>
    typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type  EncapTag(OutputIterator  out)const
    {
        return EncapTag(tag_, out);
    }

    template<typename OutputIterator>
    typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type EncapLen(OutputIterator out)const
    {
        return EncapLen(VLen(), out);
    }

    template<typename OutputIterator>
    typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type EncapValue(OutputIterator out)const
    {
        return EncapValue(*this, out);
    }

    template<typename OutputIterator>
    typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type EncapTLV(OutputIterator out)const
    {
        return EncapTLV(*this, out);
    }

    template<typename OutputIterator>
    typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type EncapTLVList(OutputIterator out)const
    {
        return EncapTLVList(children_, out);
    }

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

    std::ostream & EncapTLVList(std::ostream & os)const
    {
        EncapTLVList(children_, os);
        return os;
    }

    /* BinData ToBin()const;
     BinData TToBin()const;
     BinData LToBin()const;
     BinData VToBin()const;
     BinData LVToBin()const;*/

    /* static const unsigned char * ParseTag(const unsigned char *, size_t len, TagType &);
     static const unsigned char * ParseLen(const unsigned char *, size_t len, uint32_t &);
     static const unsigned char * ParseTLV(const unsigned char *, size_t len, BerTLV &);
     static void ParseTLVList(const unsigned char *, size_t len, BerTLVList &);*/

    //parse
    template<typename InputIterator>
    static InputIterator ParseTag(InputIterator beg, InputIterator end, TagType &);
    template<typename InputIterator>
    static InputIterator ParseLen(InputIterator beg, InputIterator end, uint32_t &);
    template<typename InputIterator>
    static InputIterator ParseTLV(InputIterator beg, InputIterator end, BerTLV &);
    template<typename InputIterator>
    static InputIterator ParseTLVList(InputIterator beg, InputIterator end, size_t len, BerTLVList &);
    template<typename T>
    static const T* ParseTLVList(const T * beg, const T* end, BerTLVList & tlv_list);

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
    static std::istream & ParseTLVList(std::istream &is, size_t len, BerTLVList & tlv_list)
    {
        ParseTLVList(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), len, tlv_list);
        return is;
    }

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
    template<typename OutputIterator>
    static typename std::enable_if<!std::is_base_of<std::ostream, OutputIterator>::value, OutputIterator>::type
        EncapTLVList(const BerTLVList &, OutputIterator out);

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
    static std::ostream& EncapTLVList(const BerTLVList &tlv_list, std::ostream& os)
    {
        EncapTLVList(tlv_list, std::ostreambuf_iterator<char>(os));
        return os;
    }

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
    static size_t TLen(TagType);
    static size_t LLen(uint32_t);
    static size_t VLen(const BerTLV &);
    static size_t TLVLen(const BerTLV &);

    //meaning
    static std::string GetDescription(TagType tag, const TLVInterpreter & interpret)
    {
        return interpret.GetDescription(tag);
    }
    static std::string GetMeaning(const BerTLV &tlv, const TLVInterpreter & interpret)
    {
        return interpret.GetMeaning(tlv);
    }

    std::string GetDescription(const TLVInterpreter & interpret)
    {
        return GetDescription(tag_, interpret);
    }
    std::string GetMeaning(const TLVInterpreter & interpret)
    {
        return GetMeaning(*this, interpret);
    }

private:
    TagType tag_;
    std::vector<unsigned char> value_;
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
        return ParseTLVList(beg, end, len, tlv.children_);
    }
}

template<typename InputIterator>
InputIterator BerTLV::ParseTLVList(InputIterator beg, InputIterator end, size_t len, BerTLVList & tlv_list)
{
    static_assert(sizeof(std::iterator_traits<InputIterator>::value_type) == 1, "InputIterator 必须指向字节流");
    size_t i = 0;
    while (i < len)
    {
        tlv_list.emplace_back();
        beg = ParseTLV(beg, end, tlv_list.back());
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
        return EncapTLVList(tlv.children_, out);
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
BerTLV::EncapTLVList(const BerTLVList &tlv_list, OutputIterator out)
{
    for (auto &x : tlv_list)
    {
        out = EncapTLV(x, out);
    }
    return out;
}

//template
//std::istream_iterator<unsigned char> BerTLV::ParseTag(std::istream_iterator<unsigned char> beg, std::istream_iterator<unsigned char> end, TagType &);
//template
//std::istream_iterator<unsigned char> BerTLV::ParseLen(std::istream_iterator<unsigned char> beg, std::istream_iterator<unsigned char> end, uint32_t &);
//template
//std::istream_iterator<unsigned char> BerTLV::ParseTLV(std::istream_iterator<unsigned char> beg, std::istream_iterator<unsigned char> end, BerTLV &);
//template
//std::istream_iterator<unsigned char> BerTLV::ParseTLVList(std::istream_iterator<unsigned char> beg, std::istream_iterator<unsigned char> end, size_t len, BerTLVList &);
//
//template
//std::istream_iterator< char> BerTLV::ParseTag(std::istream_iterator< char> beg, std::istream_iterator< char> end, TagType &);
//template
//std::istream_iterator< char> BerTLV::ParseLen(std::istream_iterator< char> beg, std::istream_iterator< char> end, uint32_t &);
//template
//std::istream_iterator< char> BerTLV::ParseTLV(std::istream_iterator< char> beg, std::istream_iterator< char> end, BerTLV &);
//template
//std::istream_iterator< char> BerTLV::ParseTLVList(std::istream_iterator< char> beg, std::istream_iterator< char> end, size_t len, BerTLVList &);
//
//template
//const unsigned char * BerTLV::ParseTag(const unsigned char * beg, const unsigned char * end, TagType &);
//template
//const unsigned char * BerTLV::ParseLen(const unsigned char * beg, const unsigned char * end, uint32_t &);
//template
//const unsigned char * BerTLV::ParseTLV(const unsigned char * beg, const unsigned char * end, BerTLV &);
//template
//const unsigned char * BerTLV::ParseTLVList(const unsigned char * beg, const unsigned char * end, size_t len, BerTLVList &);
//
//extern template
//const   char * BerTLV::ParseTag(const   char * beg, const   char * end, TagType &);
//extern template
//const   char * BerTLV::ParseLen(const   char * beg, const   char * end, uint32_t &);
//extern template
//const   char * BerTLV::ParseTLV(const   char * beg, const   char * end, BerTLV &);
//extern template
//const   char * BerTLV::ParseTLVList(const   char * beg, const   char * end, size_t len, BerTLVList &);

__LIB_NAME_SPACE_END__

#endif