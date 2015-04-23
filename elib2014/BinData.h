#ifndef BINDATA_H_RFT
#define BINDATA_H_RFT

#include <vector>
#include <string>

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

class HexData;
class BinData
{
    typedef BinData self;
public:
    BinData() = default;
    BinData(const unsigned char *data, int len) :data_(data, data + len)
    {}
    BinData(const BinData &other) :data_(other.data_)
    {}
    BinData(BinData &&other) :data_(std::move(other.data_))
    {}
    void Reset(const unsigned char *data, int len)
    {
        data_.assign(data, data + len);
    }
    void Clear()
    {
        data_.clear();
    }
    const unsigned char *Data()const
    {
        return data_.data();
    }
    size_t ByteLen()const
    {
        return data_.size();
    }
    self & operator=(const self &o)
    {
        data_ = o.data_;
        return *this;
    }
    self & operator=(self &&o)
    {
        data_ = std::move(o.data_);
        return *this;
    }
    self operator+(const self &o)const
    {
        self ret(*this);
        return ret += o;
    }
    self &operator+=(const self &o)
    {
        data_.insert(data_.end(), o.data_.begin(), o.data_.end());
        return *this;
    }
    HexData ToHex()const;

    friend class HexData;
private:
    std::vector < unsigned char > data_;
};

class HexData
{
    typedef HexData self;
public:
    HexData() = default;
    explicit HexData(char *str) :data_(str)
    {}
    HexData(const HexData &o) :data_(o.data_)
    {}
    HexData(HexData &&o) :data_(std::move(o.data_))
    {}
    void Reset(const char *str)
    {
        data_.assign(str);
    }
    void Clear()
    {
        data_.clear();
    }
    const char * Data()const
    {
        return data_.data();
    }
    size_t ByteLen()const
    {
        return data_.size() >> 1;
    }
    self & operator=(const self &o)
    {
        data_ = o.data_;
        return *this;
    }
    self & operator=(self &&o)
    {
        data_ = std::move(o.data_);
        return *this;
    }
    self operator+(const self &o)const
    {
        self ret(*this);
        return ret += o;
    }
    self& operator+=(const self &o)
    {
        data_ += o.data_;
        return *this;
    }
    BinData ToBin()const;
    friend class BinData;
private:
    std::string data_;
};

__LIB_NAME_SPACE_END__

#endif