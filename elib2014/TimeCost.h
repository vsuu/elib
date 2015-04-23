#ifndef _TIME_COST_H_RFT
#define _TIME_COST_H_RFT

#define TIME_COST

#ifdef TIME_COST
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <ostream>
#include <type_traits>
#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__
class TimeCost
{
public:
    using clock_type = std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
    TimeCost(const char *fun_name)
        :name_(fun_name), b_(clock_type::now())
    {
    }
    ~TimeCost()
    {
        auto d = clock_type::now() - b_;
        std::lock_guard<std::mutex> locker(mutex_);
        cost_[name_] += d;
    }
    static void Print(std::ostream &);
    TimeCost(const TimeCost &) = delete;
    TimeCost &operator=(const TimeCost &) = delete;
private:
    const char *name_;
    const clock_type::time_point b_;
    static std::unordered_map<const char *, clock_type::duration> cost_;
    static std::mutex mutex_;
};
__LIB_NAME_SPACE_END__
#define COUNT_TIME_COST   __LIB_NAME__::TimeCost timecost__tmp_object(__FUNCSIG__);
#define PRINT_TIME_COST(x) __LIB_NAME__::TimeCost::Print(x);
#else
#define COUNT_TIME_COST
#define PRINT_TIME_COST(x)
#endif

#endif 