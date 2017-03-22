#include <sys/time.h>
#include <stdexcept>

#ifndef _TIME_INLINES_HPP
#define _TIME_INLINES_HPP

inline struct timeval xgettimeofday()
{
    struct timeval tv;

    int err = gettimeofday(&tv, NULL);
    if (err)
	throw std::runtime_error("gettimeofday failed");

    return tv;
}

inline double secs_between(const struct timeval &tv1, const struct timeval &tv2)
{
    return (tv2.tv_sec - tv1.tv_sec) + 1.0e-6*(tv2.tv_usec - tv1.tv_usec);
}

inline double usecs_between(const struct timeval &tv1, const struct timeval &tv2)
{
    return 1.0e6*(tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec);
}

#endif  // _TIME_INLINES_HPP
