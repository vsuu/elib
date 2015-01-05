#ifndef _THREAD_POOL_H_RFT
#define _THREAD_POOL_H_RFT
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>
#include <vector>
#include <deque>
#include "libbase.h"
#include "Singleton.h"


__LIB_NAME_SPACE_BEGIN__

class ThreadPool;
class WorkThread
{
public:
	template<typename Fn, typename...Args>
	void work(Fn f, Args&&... args)
	{
		function_=std::function<void()>(f,std::forward<args>...);
		cv_.notify_one();
	}
	void join()
	{
		std::unique_lock<std::mutex> locker(mutex_);
		if (function_)
		{
			cv_j.wait(locker);
		}
	}
	std::thread::id get_id()const { return thread_->get_id(); }
private:
	WorkThread(const WorkThread &) = delete;
	WorkThread(WorkThread &&) = delete;
	WorkThread &operator=(const WorkThread &) = delete;
	WorkThread &operator=(WorkThread &&) = delete;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::condition_variable cv_j;
	std::function<void()> function_;
	bool quit_flag_ = false;
	std::thread *thread_;
	void ThreadFun();
	void init()
	{
		thread_ = new std::thread(std::mem_fn(&WorkThread::ThreadFun), this);
	}
	WorkThread() :thread_(nullptr){}
	~WorkThread()
	{
		if (nullptr == thread_)return;
		
		{
			std::lock_guard<std::mutex> locker(mutex_);
			quit_flag_ = true;
		}
		
		cv_.notify_one();
		thread_->join();
		delete thread_;
	}
	friend class ThreadPool;
};

class ThreadPool
{
public:
	WorkThread *AllocThread();
	void FreeThread(WorkThread *t)
	{
		std::lock_guard<std::mutex> lock(locker_);
		free_set_.push_back(t);
	}
	size_t ThreadCount()
	{
		std::lock_guard<std::mutex> locker(locker_);
		return All_set_.size();
	}
private:
	ThreadPool() = default;
	~ThreadPool()
	{
		std::lock_guard<std::mutex> lock(locker_);
		std::for_each(All_set_.begin(), All_set_.end(), [](WorkThread *p){delete p; });
	}
	ThreadPool(const ThreadPool &) = delete;
	ThreadPool(ThreadPool &&) = delete;
	ThreadPool &operator=(ThreadPool &&) = delete;
	ThreadPool &operator=(const ThreadPool &) = delete;
	std::vector<WorkThread *> free_set_;
	std::vector<WorkThread *> All_set_;
	std::mutex locker_;
	friend class Singleton<ThreadPool>;
};

class RecycThread
{
public:
	template<class Fn,class...Args>
	RecycThread(Fn f, Args&&... args) :thread_(Singleton<ThreadPool>::instance()->AllocThread())
	{
		thread_->work(f, std::forward<args>...);
	}
	~RecycThread()
	{
		Singleton<ThreadPool>::instance()->FreeThread(thread_);
	}
	RecycThread(RecycThread && r) :thread_(r.thread_)
	{
		r.thread_ = nullptr;
	}
	RecycThread(const RecycThread &) = delete;
	RecycThread &operator=(RecycThread &&r)
	{
		if(nullptr!=thread_)Singleton<ThreadPool>::instance()->FreeThread(thread_);
		thread_ = r.thread_;
		r.thread_ = nullptr;
	}
	RecycThread &operator=(const RecycThread &) = delete;
	std::thread::id get_id()const
	{
		return thread_->get_id();
	}
	void join()
	{
		return thread_->join();
	}
	void swap(RecycThread &b)_NOEXCEPT
	{
		std::swap(thread_, b.thread_);
	}
private:
	WorkThread * thread_;
};

inline void swap(RecycThread &a, RecycThread &b)_NOEXCEPT
{
	a.swap(b);
}








__LIB_NAME_SPACE_END__


#endif