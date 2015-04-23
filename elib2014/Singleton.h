#ifndef _SINGLETON_H_RFT
#define _SINGLETON_H_RFT

#include <mutex>

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

template<typename T>
class Singleton
{
public:
    static T *instance()
    {
        std::call_once(once_flag_, init);
        return value_;
    }
private:
    static void init()
    {
        value_ = new T();
        ::atexit(destroy);
    }
    static void destroy()
    {
        delete value_;
    }
    static T * value_;
    static std::once_flag once_flag_;
    Singleton();
    Singleton(const Singleton &) = delete;
};

template<typename T>
T * Singleton<T>::value_ = nullptr;
template<typename T>
std::once_flag Singleton<T>::once_flag_;

__LIB_NAME_SPACE_END__

#endif