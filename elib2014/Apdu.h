#ifndef APDU_H_RFT
#define APDU_H_RFT

#include "libbase.h"

#include "BinData.h"
#include <cstdint>
#include <type_traits>
#include <ostream>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <iterator>
__LIB_NAME_SPACE_BEGIN__

class ApduCmd
{
    typedef uint8_t byte;
public:
    ApduCmd(int cla, int ins, int p1, int p2)
        :ApduCmd(cla, ins, p1, p2, -1)
    {}
    ApduCmd(int cla, int ins, int p1, int p2, int le)
        :cla_(cla), ins_(ins), p1_(p1), p2_(p2), le_(le)
    {}
    ApduCmd(int cla, int ins, int p1, int p2, const BinData &data)
        :ApduCmd(cla, ins, p1, p2, data, -1)
    {}
    ApduCmd(int cla, int ins, int p1, int p2, const BinData &data, int le)
        :cla_(cla), ins_(ins), p1_(p1), p2_(p2), data_(data), le_(le)
    {
        assert(data_.size() < 0x10000);
    }
    ApduCmd(int cla, int ins, int p1, int p2, BinData &&data)
        :ApduCmd(cla, ins, p1, p2, std::move(data), -1)
    {}
    ApduCmd(int cla, int ins, int p1, int p2, BinData &&data, int le)
        :cla_(cla), ins_(ins), p1_(p1), p2_(p2), data_(std::move(data)), le_(le)
    {
        assert(data_.size() < 0x10000);
    }

    byte &Cla()
    {
        return cla_;
    }
    const byte &Cla()const
    {
        return cla_;
    }
    byte &Ins()
    {
        return ins_;
    }
    const byte &Ins()const
    {
        return ins_;
    }
    byte &P1()
    {
        return p1_;
    }
    const byte &P1()const
    {
        return p1_;
    }
    byte &P2()
    {
        return p2_;
    }
    const byte &P2()const
    {
        return p2_;
    }
    BinData &Data()
    {
        return data_;
    }
    const BinData &Data()const
    {
        return data_;
    }
    BinData ToBin()const
    {
        BinData ret;
        EncapApdu(*this, std::back_inserter(ret));
        return ret;
    }
    int &Le() //-1表示没有le
    {
        return le_;
    }
    const int &Le()const
    {
        return le_;
    }

    template<class OutputIterator>
    static  OutputIterator  EncapApdu(int cla, int ins, int p1, int p2, OutputIterator out)
    {
        *out++ = static_cast<byte>(cla);
        *out++ = static_cast<byte>(ins);
        *out++ = static_cast<byte>(p1);
        *out++ = static_cast<byte>(p2);
        return out;
    }
    template<class OutputIterator>
    static  OutputIterator  EncapApdu(int cla, int ins, int p1, int p2, int le, OutputIterator out)
    {
        out = EncapApdu(cla, ins, p1, p2, out);
        out = EncapLen(le, out);
        return out;
    }
    template<class OutputIterator>
    static  OutputIterator  EncapApdu(int cla, int ins, int p1, int p2, const BinData &data, OutputIterator out)
    {
        assert(data.size() < 0x10000);
        out = EncapApdu(cla, ins, p1, p2, out);
        out = EncapData(data, out);
        return out;
    }
    template<class OutputIterator>
    static  OutputIterator  EncapApdu(int cla, int ins, int p1, int p2, const BinData &data, int le, OutputIterator out)
    {
        out = EncapApdu(cla, ins, p1, p2, out);
        out = EncapData(data, out);
        out = EncapLen(le, out);
        return out;
    }
    template<class OutputIterator>
    static OutputIterator EncapApdu(const ApduCmd &cmd, OutputIterator out)
    {
        return EncapApdu(cmd.cla_, cmd.ins_, cmd.p1_, cmd.p2_, cmd.data_, cmd.le_, out);
    }
private:
    byte cla_;
    byte ins_;
    byte p1_;
    byte p2_;
    int le_;
    BinData data_;
private:
    template<class OutputIterator>
    static OutputIterator EncapLen(int len, OutputIterator out)
    {
        if (len >= 0)
        {
            if (len > static_cast<int>(0xFF))
            {
                *out++ = static_cast<byte>(0x00);
                *out++ = static_cast<byte>((len >> 8) & 0xFF);
            }
            *out++ = static_cast<byte>(len & 0xFF);
        }
        return out;
    }
    template<class OutputIterator>
    static OutputIterator EncapData(const BinData &data, OutputIterator out)
    {
        if (!data.empty())
        {
            out = EncapLen(data.size(), out);
            return std::copy(begin(data), end(data), out);
        }
        return out;
    }
};

class ApduRsp
{
    typedef uint8_t byte;
    typedef uint16_t word;
public:
    template<typename InputIterator>
    ApduRsp(InputIterator first, InputIterator last) :data_(first, last)
    {
        if (data_.size() < 2)
        {
            throw std::invalid_argument(ERR_WHERE "ApduRsp输入长度不能小于2");
        }
        sw_ = data_.back();
        data_.pop_back();
        sw_ |= (data_.back() << 8) & 0xFF00;
        data_.pop_back();
    }
    word Sw()const
    {
        return sw_;
    }
    byte Sw1()const
    {
        return (sw_ >> 8) & 0xFF;
    }
    byte Sw2()const
    {
        return sw_ & 0xFF;
    }
    const BinData &Data()const
    {
        return data_;
    }
private:
    BinData data_;
    word sw_;
};

__LIB_NAME_SPACE_END__

#endif