#ifndef UIPP_APP_UIPAGEPROXY_H
#define UIPP_APP_UIPAGEPROXY_H
#include "cpepp/dr/Meta.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIPageProxy {
public:
    virtual const char * name(void) const = 0;

    virtual UICenter & center(void) = 0;
    virtual UICenter const & center(void) const = 0;

    virtual Page & page(void) = 0;
    virtual Page const & page(void) const = 0;

    virtual void const * checkGetData(LPDRMETA meta) const = 0;
    virtual void setData(void const * data, size_t data_size, LPDRMETA meta) = 0;
    virtual void copyData(void * data, size_t data_capacity, LPDRMETA meta) const = 0;

    template<typename T>
    T const & data(void) const { return *(T const*)checkGetData(Cpe::Dr::MetaTraits<T>::META); }

	template<typename T>
	T copyData(void) const { 
		T r;
		copyData(&r, sizeof(r), Cpe::Dr::MetaTraits<T>::META);
		return r;
	}

    template<typename T>
    void setData(T const & o) { setData(&o, Cpe::Dr::MetaTraits<T>::data_size(o), Cpe::Dr::MetaTraits<T>::META); }

    virtual ~UIPageProxy();
};

}}

#endif
