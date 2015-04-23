#include "TimeCost.h"


#ifdef TIME_COST

#include <algorithm>
#include <vector>
#include <ratio>
#include <iterator>

std::unordered_map<const char *, elib::TimeCost::clock_type::duration> elib::TimeCost::cost_;
std::mutex elib::TimeCost::mutex_;


void elib::TimeCost::Print(std::ostream &out)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::vector<std::pair<const char *, clock_type::duration>> tmp(cost_.begin(),cost_.end());
    lock.unlock();

    std::sort(tmp.begin(), tmp.end(), []
		(const std::pair<const char *, clock_type::duration> &l, const std::pair<const char *, clock_type::duration>&r)
		{
			return l.second > r.second;
		}
		);
		for (auto &x : tmp)
		{
			out << x.first << '\t' << std::chrono::duration_cast<std::chrono::nanoseconds>(x.second).count() << "ns" << std::endl;
		}
}
#endif