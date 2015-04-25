#ifndef _STROP_H_RFT
#define _STROP_H_RFT

#include<string>
#include<iterator>

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

extern unsigned char char2hex_table[];
//int char2hex(char c, unsigned char&r);
inline bool char2hex(char c, unsigned char&r)
{
    r = char2hex_table[c];
    return r == 0xFF;
}

extern char hex2char_table[];

inline char hex2char(unsigned char c)
{
    return hex2char_table[c];
}

template<typename OutputIter>
OutputIter Hex2Bin(const char *str, OutputIter out)
{
    if (nullptr == str)return out;

    const char *beg_it = str;
    unsigned char tmp1, tmp2;

    do
    {
        if (char2hex(*beg_it++, tmp1) || char2hex(*beg_it++, tmp2))
        {
            break;
        }
        *out++ = (tmp1 << 4) | tmp2;
    } while (true);

    return out;
}

template<typename OutputIter>
inline OutputIter Hex2Bin(const std::string &str, OutputIter out)
{
    return Hex2Bin(str.c_str(), out);
}

template<typename InputIer, typename OutputIter>
OutputIter Bin2Hex(InputIer b, InputIer e, OutputIter out)
{
    unsigned char c;
    while (b != e)
    {
        c = *b++;
        *out++ = hex2char(c >> 4);
        *out++ = hex2char(c & 0x0F);
    }
    return out;
}

//std::string wstr2str(const wchar_t *);
std::string wstr2str(const std::wstring& wstr);

//std::wstring str2wstr(const char *);
std::wstring str2wstr(const std::string &str);

__LIB_NAME_SPACE_END__

#endif // _STROP_H_RFT
