#include "BinData.h"
#include "strop.h"
#include <iterator>
using namespace std;

__LIB_NAME_SPACE_BEGIN__
BinData HexData::ToBin()const
{
    BinData ret;
    Hex2Bin(data(), back_inserter(ret));
    return ret;
}
HexData BinData::ToHex()const
{
    return HexData(Bin2Hex(data(), size()));
}
__LIB_NAME_SPACE_END__