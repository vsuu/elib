#ifndef _OBJECT_POOL_RFT
#define _OBJECT_POOL_RFT








#include "libbase.h"

#include <vector>
#include <deque>
#include <mutex>
__LIB_NAME_SPACE_BEGIN__

/*
template<class T, size_t N = 0>
class ObjectPool
{
public:
	typedef std::mutex mutex_type;
	ObjectPool() :all_(new T[N])
	{
		free_.reserve(N);
		for (int i = 0; i < N; ++i)
		{
			free_.push_back(all_ + i);
		}
	}
	~ObjectPool()
	{
		delete [] all_;
	}
	ObjectPool(const ObjectPool &) = delete;
	ObjectPool(ObjectPool &&r) = delete;
	ObjectPool &operator=(const ObjectPool &) = delete;
	ObjectPool &operator=(ObjectPool &&) = delete;
	T *alloc()
	{
		T *ret = nullptr;
		std::lock_guard<mutex_type> locker(mutex_);
		if (!free_.empty())
		{
			ret = free_.back();
			free_.pop_back();
		}
		return ret;
	}
	void free(T *ptr)
	{
		std::lock_guard<mutex_type> locker(mutex_);
		free_.push_back(ptr);
	}
	size_t size()
	{
		return N;
	}
private:
	T *all_;
	std::vector<T *> free_;
	mutex_type mutex_;
};

template<class T>
class ObjectPool<T, 0>
{
public:
	typedef std::mutex mutex_type;
	ObjectPool(size_t n) :all_(n)
	{
		free_.reserve(n);
		for (auto &x : all_)
		{
			free_.push_back(&x);
		}
	}
	ObjectPool() = default;
	~ObjectPool() = default;
	ObjectPool(const ObjectPool &) = delete;
	ObjectPool(ObjectPool &&r) = delete;
	ObjectPool &operator=(const ObjectPool &) = delete;
	ObjectPool &operator=(ObjectPool &&) = delete;
	T *alloc()
	{
		T *ret = nullptr;
		std::lock_guard<mutex_type> locker(mutex_);
		if (!free_.empty())
		{
			ret = free_.back();
			free_.pop_back();
		}
		else
		{
			all_.emplace_back();
			ret = &all_.back();
		}
		return ret;
	}
	void free(T *ptr)
	{
		std::lock_guard<mutex_type> locker(mutex_);
		free_.push_back(ptr);
	}
	size_t size()
	{
		std::lock_guard<mutex_type> locker(mutex_);
		return all_.size();
	}

private:
	std::deque<T> all_;
	std::vector<T *> free_;
	mutex_type mutex_;
};
*/
template<class T, bool = true,class Container = std::deque<T>>
class ObjectPool;

template<class T,class Container>
class ObjectPool<T,true,Container>
{
public:
    typedef std::mutex mutex_type;
    ObjectPool(size_t n) :all_(n)
    {
        free_.reserve(n);
        for (auto &x : all_)
        {
            free_.push_back(&x);
        }
    }
    ObjectPool() = default;
    ~ObjectPool() = default;
    ObjectPool(const ObjectPool &) = delete;
    ObjectPool(ObjectPool &&r) = delete;
    ObjectPool &operator=(const ObjectPool &) = delete;
    ObjectPool &operator=(ObjectPool &&) = delete;
    T *alloc()
    {
        T *ret = nullptr;
        std::lock_guard<mutex_type> locker(mutex_);
        if (!free_.empty())
        {
            ret = free_.back();
            free_.pop_back();
        }
        else
        {
            all_.emplace_back();
            ret = &all_.back();
        }
        return ret;
    }
    void free(T *ptr)
    {
        std::lock_guard<mutex_type> locker(mutex_);
        free_.push_back(ptr);
    }
    size_t size()
    {
        std::lock_guard<mutex_type> locker(mutex_);
        return all_.size();
    }

private:
    Container all_;
    std::vector<T *> free_;
    mutex_type mutex_;
};


template<class T,class Container>
class ObjectPool<T,false,Container>
{
public:
    ObjectPool(size_t n) :all_(n)
    {
        free_.reserve(n);
        for (auto &x : all_)
        {
            free_.push_back(&x);
        }
    }
    ObjectPool() = default;
    ~ObjectPool() = default;
    ObjectPool(const ObjectPool &) = delete;
    ObjectPool(ObjectPool &&r) = delete;
    ObjectPool &operator=(const ObjectPool &) = delete;
    ObjectPool &operator=(ObjectPool &&) = delete;
    T *alloc()
    {
        T *ret = nullptr;
        if (!free_.empty())
        {
            ret = free_.back();
            free_.pop_back();
        }
        else
        {
            all_.emplace_back();
            ret = &all_.back();
        }
        return ret;
    }
    void free(T *ptr)
    {
        free_.push_back(ptr);
    }
    size_t size()
    {
        return all_.size();
    }

private:
    Container all_;
    std::vector<T *> free_;
};

__LIB_NAME_SPACE_END__

#endif