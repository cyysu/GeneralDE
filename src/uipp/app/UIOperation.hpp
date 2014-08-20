#ifndef UIPP_APP_UIOPERATION_H
#define UIPP_APP_UIOPERATION_H
#include "System.hpp"

namespace UI { namespace App {

class UIOperation {
public:
    virtual void execute(EnvExt & env, UIPageProxyExt & page, NPGUIControl & control) = 0;
    virtual ~UIOperation();

    static ::std::auto_ptr<UIOperation> create(Cpe::Cfg::Node const & config);
};

}}

#endif


