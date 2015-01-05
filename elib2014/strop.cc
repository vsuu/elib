#include "strop.h"


__LIB_NAME_SPACE_BEGIN__

int char2hex(char c,unsigned char&r)
{
    if(c>='0' && c<='9')
    {
        r= c-'0';
    }
    else
    if(c>='A' && c<='F')
    {
        r= c-'A'+10;
    }
    else
    if(c>='a' && c<='f')
    {
        r=-'a'+10;
    }
    else
    {
        return 1;
    }
    return 0;
}

char hex2char_table[]="0123456789ABCDEF";

__LIB_NAME_SPACE_END__
