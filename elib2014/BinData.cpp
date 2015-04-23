#include "BinData.h"
#include "strop.h"
#include <iterator>
using namespace std;

__LIB_NAME_SPACE_BEGIN__
BinData HexData::ToBin()const
{
    BinData ret;
    Hex2Bin(data_.data(), back_inserter(ret.data_));
    return ret;
}
HexData BinData::ToHex()const
{
    HexData ret;
    ret.data_ = Bin2Hex(data_.data(), data_.size());
    return ret;
}
__LIB_NAME_SPACE_END__