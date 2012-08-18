#ifndef USFPP_LOGIC_USE_DYNDATA_H
#define USFPP_LOGIC_USE_DYNDATA_H
#include <cassert>
#include <algorithm>
#include "cpepp/utils/IntTypeSelect.hpp"
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/dr/Meta.hpp"
#include "usfpp/logic/LogicOpData.hpp"
#include "usfpp/logic/LogicOpContext.hpp"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "System.hpp"

namespace Usf { namespace Logic {

template<typename DataT, int fix_count>
class LogicOpDynData {
public:
    typedef typename Cpe::Dr::MetaTraits<DataT>::dyn_element_type element_type;
    typedef typename Cpe::Dr::MetaTraits<DataT>::dyn_size_type size_type;

    LogicOpDynData(LogicOpContext & context) {
        m_data = context.checkCreateData<DataT>();
    }

    LogicOpDynData(LogicOpStackNode & stackNode) {
        m_data = stackNode.checkCreateData<DataT>();
    }

    LogicOpDynData(LogicOpRequire & require) {
        m_data = require.checkCreateData<DataT>();
    }

    DataT & data(void) { return *(DataT*) m_data.data(); }
    DataT const & data(void) const { return *(DataT const *) m_data.data(); }

    size_type count(void) const { return *size_i(); }
    size_type capacity(void) const { return (size_type)fix_count; }

    element_type & operator[] (size_type pos) { return data_i()[pos]; }
    element_type const & operator[] (size_type pos) const { return data_i()[pos]; }

    element_type & at(size_type pos) { 
        assert(pos < count());
        return data_i()[pos];
    }

    element_type const & at (size_type pos) const {
        assert(pos < count());

        return data_i()[pos];
    }

    void append(element_type const & e) {
        assert(count() + 1 < capacity());

        size_type pos = (*size_i())++;

        operator[](pos) = e;
    }

    void erase(size_type pos) {
        size_type count = this->count();
        assert(pos < count);

        if (pos + 1 < count) {
            memcpy(&data_i()[pos], &data_i()[count - 1], sizeof(element_type));
        }

        --(*size_i());
    }

    void clear(void) {
        (*size_i()) = 0;
    }

    template<typename CmpT>
    void sort(CmpT const & cmp = CmpT()) {
        element_type * begin = data_i();
        ::std::sort(begin, begin + count(), cmp);
    }

    template<typename CmpT>
    ::std::pair<element_type const *, element_type const *>
    find_range(element_type const & key, CmpT const & cmp) const {
        element_type const * begin = data_i();

        return ::std::pair<element_type const *, element_type const *>(
            ::std::lower_bound(begin, begin + count(), cmp),
            ::std::upper_bound(begin, begin + count(), cmp));
    }

    template<typename CmpT>
    ::std::pair<element_type *, element_type *>
    find_range(element_type const & key, CmpT const & cmp) {
        element_type * begin = data_i();

        return ::std::pair<element_type *, element_type *>(
            ::std::lower_bound(begin, begin + count(), cmp),
            ::std::upper_bound(begin, begin + count(), cmp));
    }


private:
    size_type * size_i(void) { return (size_type *)(((char *)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_refer_start_pos); }
    size_type const * size_i(void) const { return (size_type const *)(((char*)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_refer_start_pos); }

    element_type * data_i(void) { return (element_type*)(((char * *)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_data_start_pos); }
    element_type const * data_i(void) const { return (element_type const *)(((char const *)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_data_start_pos); }

    Cpe::Utils::ObjRef<LogicOpData> m_data;
};

template<typename DataT>
class LogicOpDynData<DataT, 0> {
public:
    typedef typename Cpe::Dr::MetaTraits<DataT>::dyn_element_type element_type;
    typedef typename Cpe::Dr::MetaTraits<DataT>::dyn_size_type size_type;

    LogicOpDynData(
        LogicOpContext & context,
        size_t capacity = 8)
        : m_holder_type(0)
        , m_holder(&context)
    {
        checkCreateData(Cpe::Dr::MetaTraits<DataT>::META, capacity);
    }

    LogicOpDynData(
        LogicOpStackNode & node,
        size_t capacity = 8)
        : m_holder_type(1)
        , m_holder(&node)
    {
        checkCreateData(Cpe::Dr::MetaTraits<DataT>::META, capacity);
    }

    LogicOpDynData(
        LogicOpRequire & require,
        size_t capacity = 8)
        : m_holder_type(2)
        , m_holder(&require)
    {
        checkCreateData(Cpe::Dr::MetaTraits<DataT>::META, capacity);
    }


    DataT & data(void) { return *(DataT*) m_data.data(); }
    DataT const & data(void) const { return *(DataT const *) m_data.data(); }

    size_type count(void) const { return *size_i(); }
    size_type capacity(void) const { return ((m_data.get().capacity() - sizeof(DataT)) / sizeof(element_type)) + 1; }

    element_type & operator[] (size_type pos) { return data_i()[pos]; }
    element_type const & operator[] (size_type pos) const { return data_i()[pos]; }

    element_type & at(size_type pos) { 
        assert(pos < count());
        return data_i()[pos];
    }

    element_type const & at (size_type pos) const {
        assert(pos < count());

        return data_i()[pos];
    }

    void append(element_type const & e) {
        if (count() + 1 >= capacity()) {
            size_type inc_capacity = capacity();
            if (inc_capacity < 16) inc_capacity = 16;
            checkCreateData(m_data.get().meta(), capacity() + inc_capacity);
        }

        assert(count() + 1 < capacity());

        size_type pos = (*size_i())++;

        operator[](pos) = e;
    }

    void erase(size_type pos) {
        size_type count = this->count();
        assert(pos < count);

        if (pos + 1 < count) {
            memcpy(&data_i()[pos], &data_i()[count - 1], sizeof(element_type));
        }

        -- (*size_i());
    }

    void clear(void) {
        (*size_i()) = 0;
    }

    template<typename CmpT>
    void sort(CmpT const & cmp = CmpT()) {
        element_type * begin = data_i();
        ::std::sort(begin, begin + count(), cmp);
    }

    template<typename CmpT>
    ::std::pair<element_type const *, element_type const *>
    find_range(element_type const & key, CmpT const & cmp) const {
        element_type const * begin = data_i();

        return ::std::pair<element_type const *, element_type const *>(
            ::std::lower_bound(begin, begin + count(), cmp),
            ::std::upper_bound(begin, begin + count(), cmp));
    }

    template<typename CmpT>
    ::std::pair<element_type *, element_type *>
    find_range(element_type const & key, CmpT const & cmp) {
        element_type * begin = data_i();

        return ::std::pair<element_type *, element_type *>(
            ::std::lower_bound(begin, begin + count(), cmp),
            ::std::upper_bound(begin, begin + count(), cmp));
    }


private:
    size_type * size_i(void) { return (size_type *)(((char *)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_refer_start_pos); }
    size_type const * size_i(void) const { return (size_type const *)(((char*)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_refer_start_pos); }

    element_type * data_i(void) { return (element_type*)(((char * *)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_data_start_pos); }
    element_type const * data_i(void) const { return (element_type const *)(((char const *)m_data.get().data()) + Cpe::Dr::MetaTraits<DataT>::dyn_data_start_pos); }

    void checkCreateData(LPDRMETA meta, size_t capacity) {
        assert(capacity > 0);
        assert(m_holder_type < 3);

        if (m_holder_type == 0){
            m_data = ((LogicOpContext *)m_holder)->checkCreateData(meta, sizeof(element_type) * (capacity - 1) + sizeof(DataT));
        }
        else if (m_holder_type == 1) {
            m_data = ((LogicOpStackNode *)m_holder)->checkCreateData(meta, sizeof(element_type) * (capacity - 1) + sizeof(DataT));
        }
        else {
            m_data = ((LogicOpRequire *)m_holder)->checkCreateData(meta, sizeof(element_type) * (capacity - 1) + sizeof(DataT));
        }

        assert(m_data.valid());
        if (!m_data.valid()) throw ::std::bad_alloc();
    }

    Cpe::Utils::ObjRef<LogicOpData> m_data;
    int m_holder_type;
    void * m_holder;
};

template<typename DataT>
class LogicOpDynData<DataT, 1>;

}}

#endif
