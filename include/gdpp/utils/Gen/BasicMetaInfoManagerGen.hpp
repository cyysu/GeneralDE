#ifndef GDPP_UTILS_GEN_BASICMETAINFOMANAGERGEN_H
#define GDPP_UTILS_GEN_BASICMETAINFOMANAGERGEN_H
#include <vector>
#include <algorithm>
#include <typeinfo>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_file.h"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Log.hpp"

namespace Gd { namespace Utils {

template<typename BaseT, typename ElementT, typename Compare>
class BasicMetaInfoManagerGen : public BaseT {
public:
    typedef BasicMetaInfoManagerGen Base;

    virtual void load(Cpe::Cfg::Node const & configNode) {
        size_t writeCount = m_elements.size();
        size_t readCount = 0;

        m_elements.resize(m_elements.size() + configNode.childCount());

        Cpe::Cfg::NodeConstIterator configNodes = configNode.childs();
        while(Cpe::Cfg::Node const * boxCfg = configNodes.next()) {
            if (!BaseT::META.try_load_from_cfg(m_elements[writeCount], *boxCfg)) {
                APP_ERROR("load %s fail, index="FMT_SIZE_T"!", typeid(ElementT).name(), readCount);
            }
            else {
                ++writeCount;
            }

            ++readCount;
        }

        m_elements.resize(writeCount);

        ::std::sort(m_elements.begin(), m_elements.end(), Compare());
    }

    virtual void dump(uint32_t level, write_stream_t stream) const {
        for(typename ElementContainer::const_iterator it = m_elements.begin();
            it != m_elements.end();
            ++it)
        {
            stream_putc_count(stream, ' ', level << 2);

            BaseT::META.dump_data(stream, (void const *)&*it);

            stream_putc(stream, '\n');
        }
    }

    virtual size_t count(void) const {
        return m_elements.size();
    }

    virtual ElementT const & at(size_t pos) const {
        return m_elements[pos];
    }

    virtual void clear(void) { m_elements.clear(); }

protected:
    typedef ::std::vector<ElementT> ElementContainer;

    bool _is_equal(ElementT const & l, ElementT const & r) const {
        return !Compare()(l, r) && !Compare()(r, l);
    }

    ::std::pair<ElementT const *, ElementT const *>
    find_range(ElementT const & key) const {
        typename ElementContainer::const_iterator begin = 
            ::std::lower_bound(m_elements.begin(), m_elements.end(), key, Compare());
        typename ElementContainer::const_iterator end = 
            ::std::upper_bound(m_elements.begin(), m_elements.end(), key, Compare());

        return ::std::pair<ElementT const *, ElementT const *>(&*begin, &*end);
    }


    ElementT const * find_first(ElementT const & key) const {
        typename ElementContainer::const_iterator pos = 
            ::std::lower_bound(m_elements.begin(), m_elements.end(), key, Compare());
        if (pos != m_elements.end() && _is_equal(*pos, key)) {
            return &*pos;
        }
        else {
            return 0;
        }
    }

    ElementT const * upper_bound(ElementT const & key) const {
        typename ElementContainer::const_iterator pos = 
            ::std::upper_bound(m_elements.begin(), m_elements.end(), key, Compare());
        if (pos != m_elements.end()) {
            return &*pos;
        }
        else {
            return end();
        }
    }

    template<typename CmpT>
    ElementT const * lineerFind(ElementT const & key, CmpT const & cmp = CmpT()) const {
        for(typename ElementContainer::const_iterator pos = m_elements.begin();
            pos != m_elements.end();
            ++pos)
        {
            if (cmp(*pos, key)) return &*pos;
        }

        return 0;
    }


    ElementT const * begin(void) const { return &m_elements[0]; }
    ElementT const * end(void) const { return begin() + m_elements.size(); }

    ElementContainer m_elements;
};

}}

#endif
