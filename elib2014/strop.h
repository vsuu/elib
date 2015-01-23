#ifndef _STROP_H_RFT
#define _STROP_H_RFT

#include<string>
#include<iterator>

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__


int char2hex(char c, unsigned char&r);

extern char hex2char_table[];

inline char hex2char(unsigned char c)
{
    return hex2char_table[c];
}

template<typename OutputIter>
int Hex2Bin(const char *str,OutputIter out)
{
    if(nullptr==str)return 0;

    const char *beg_it=str;
    unsigned char tmp1,tmp2;

    do
    {
        if(char2hex(*beg_it,tmp1) || char2hex(*(beg_it+1),tmp2))
        {
            break;
        }
        *out++=(tmp1<<4)|tmp2;
		beg_it += 2;
    }
    while(true);

    return (beg_it - str)/2;
}

template<typename OutputIter>
inline int Hex2Bin(const std::string &str,OutputIter out)
{
    return Hex2Bin(str.c_str(),out);
}

inline std::string Hex2Bin(const std::string &str)
{
    std::string ret;
    Hex2Bin(str,std::back_inserter(ret));
    return ret;
}

template<typename OutputIter>
int Bin2Hex(const void *b,const void *e,OutputIter out)
{
    if(nullptr==b || nullptr==e) return 0;
    const unsigned char *begp=static_cast<const unsigned char *>(b);
    const unsigned char *endp=static_cast<const unsigned char *>(e);
    while(begp!=endp)
    {
        *out++=hex2char(*begp>>4);
        *out++=hex2char(*begp&0x0F);
		++begp;
    }
    return (endp-static_cast<const unsigned char *>(b))*2;
}
template<typename OutputIter>
inline int Bin2Hex(const void *b,const unsigned int l,OutputIter out)
{
    return Bin2Hex(b,(const unsigned char *)b+l,out);
}
inline std::string Bin2Hex(const void *b,const void *e)
{
    std::string ret;
    Bin2Hex(b,e,std::back_inserter(ret));
    return ret;
}
inline std::string Bin2Hex(const void *b,unsigned int l)
{
    return Bin2Hex(b,static_cast<const char *>(b)+l);
}

//std::string wstr2str(const wchar_t *);
std::string wstr2str(const std::wstring& wstr);

//std::wstring str2wstr(const char *);
std::wstring str2wstr(const std::string &str);


__LIB_NAME_SPACE_END__










#endif // _STROP_H_RFT
