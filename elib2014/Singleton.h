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
	class destroyer_
	{
	public:
		destroyer_(T * &pointer) :pointer_(pointer)
		{}
		~destroyer_()
		{
			if (pointer_)
			{
				delete pointer_;
				pointer_ = nullptr;
			}
		}
	private:
		T * &pointer_;
	};
    static void init()
    {
        value_ = new T();
    }
    static T * value_;
	static std::once_flag once_flag_;
	static destroyer_ destroyer;
    Singleton()=delete;
    Singleton(const Singleton &) = delete;
};

template<typename T>
T * Singleton<T>::value_ = nullptr;
template<typename T>
std::once_flag Singleton<T>::once_flag_;
template<typename T>
typename Singleton<T>::destroyer_ Singleton<T>::destroyer(Singleton<T>::value_);

__LIB_NAME_SPACE_END__

#endif