#ifndef ONSCOPEEXIT_H_RFT
#define ONSCOPEEXIT_H_RFT

#include <functional>

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

class _ScopeGuard
{
public:
	explicit _ScopeGuard(std::function<void()> &&fun) :OnExitScope_(std::move(fun))
	{
	}
	explicit _ScopeGuard(const std::function<void()> &fun) :OnExitScope_(fun)
	{
	}
	~_ScopeGuard()
	{
		OnExitScope_();
	}
private:
	_ScopeGuard(const _ScopeGuard &) = delete;
	_ScopeGuard &operator=(const _ScopeGuard &) = delete;
	std::function<void()> OnExitScope_;
};

class ScopeGuard
{
public:
	explicit ScopeGuard(std::function<void()> &&fun) :OnExitScope_(std::move(fun))
	{
	}
	explicit ScopeGuard(const std::function<void()> &fun) :OnExitScope_(fun)
	{
	}
	~ScopeGuard()
	{
		if (!dismissed_)
			OnExitScope_();
	}

	void release()
	{
		if (!dismissed_)
		{
			OnExitScope_();
			dismiss();
		}
	}

	void dismiss()
	{
		dismissed_ = true;
	}
private:
	bool dismissed_ = false;
	std::function<void()> OnExitScope_;
	ScopeGuard(const ScopeGuard &) = delete;
	ScopeGuard &operator=(const ScopeGuard &) = delete;
};

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)

#define ON_SCOPE_EXIT(callback) __LIB_NAME__::_ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

__LIB_NAME_SPACE_END__

#endif // ONSCOPEEXIT_H_INCLUDED
