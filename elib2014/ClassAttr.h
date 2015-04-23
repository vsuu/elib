#ifndef _CLASS_ATTR_H_RFT
#define _CLASS_ATTR_H_RFT

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

class nocopyable
{
public:
    nocopyable() = default;
    nocopyable(const nocopyable &) = delete;
    nocopyable & operator=(const nocopyable &) = delete;
};

class nonewable
{
public:
    void * operator new(size_t) = delete;
};

__LIB_NAME_SPACE_END__

#endif