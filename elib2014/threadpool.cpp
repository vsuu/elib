#include "threadpool.h"
#include <vector>
#include <stdexcept>

__LIB_NAME_SPACE_BEGIN__


void WorkThread::ThreadFun()
{
	std::unique_lock<std::mutex> locker(mutex_);
	if (quit_flag_)
	{
		return;
	}
	if (function_)
	{
		function_();
		function_ = nullptr;
		cv_j.notify_one();
	}
	do
	{
		cv_.wait(locker);
		if (quit_flag_)
		{
			break;
		}
		function_();
		function_ = nullptr;
		cv_j.notify_one();
	} while (true);
}




WorkThread *ThreadPool::AllocThread()
{
	std::lock_guard<std::mutex> lock(locker_);
	WorkThread * ret = nullptr;
	if (free_set_.empty())
	{
		ret = new WorkThread();
		ret->init();
		All_set_.push_back(ret);
	}
	else
	{
		ret = free_set_.back();
		free_set_.pop_back();
	}
	return ret;
}


__LIB_NAME_SPACE_END__