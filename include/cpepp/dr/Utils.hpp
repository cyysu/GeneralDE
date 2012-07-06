#ifndef CPEPP_DR_UTILS_H
#define CPEPP_DR_UTILS_H
#include "Meta.hpp"

namespace Cpe { namespace Dr {

template<class T1, typename T2>
inline void copy_same_entries(T1 & target, T2 const & src, int policy = 0, error_monitor_t em = 0) {
    MetaTraits<T1>::META.copy_same_entries(
        &target, sizeof(target),
        &src, MetaTraits<T2>::META,
        sizeof(src), policy, em);
}

}}

#endif

