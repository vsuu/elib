#ifndef _RANGE_H_RFT
#define _RANGE_H_RFT
#include <iterator>
#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

namespace detail {
    template<class T>
    class range
    {
    public:
        range(T b, T e, T step) :b_(b), e_(e), step_(step)
        {}
        ~range() = default;
        class itetype : public std::iterator<std::forward_iterator_tag, T>
        {
        public:
            itetype(T v, T e, T step) :v_(v), step_(step), e_(e)
            {}
            T &operator++()
            {
                v_ += step_;
                if (v_ >= e_)v_ = e_;
                return v_;
            }
            T operator++(int)
            {
                T ret = v_;
                v_ += step_;
                if (v_ >= e_)v = e_;
                return ret;
            }
            T &operator*()
            {
                return v_;
            }
            bool operator!=(const itetype &r)
            {
                return v_ != r.v_;
            }
            bool operator==(const itetype &)
            {
                return v_ == r.v_;
            }
        private:
            T v_;
            T e_;
            T step_;
        };
        itetype begin()
        {
            return itetype(b_, e_, step_);
        }
        itetype end()
        {
            return itetype(e_, e_, step_);
        }
        T b_;
        T e_;
        T step_;
    };
}

/*
template<class T>
typename detail::range<T>::itetype begin(const detail::range<T> &r)
{
    return detail::range<T>::itetype(r.b_, r.e_, r.step_);
}

template<class T>
typename detail::range<T>::itetype end(const detail::range<T> &r)
{
    return detail::range<T>::itetype(r.e_, r.e_, step_);
}
*/

template<class T>
typename detail::range<T> make_range(T b, T e, T step)
{
    return detail::range<T>(b, e, step);
}



__LIB_NAME_SPACE_END__



#endif