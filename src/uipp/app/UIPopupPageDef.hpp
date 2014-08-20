#ifndef UIPP_APP_UITIPPAGEDEF_H
#define UIPP_APP_UITIPPAGEDEF_H
#include "UIPageBindings.hpp"

namespace UI { namespace App {

class UIPopupPageDef : public UIPageBindings {
public:
    UIPopupPageDef(Gd::App::Application & app, Cpe::Cfg::Node const & cfg);

    const char * name(void) const { return m_name.c_str(); }
    const char * res(void) const { return m_resource.c_str(); }
    float duration(void) const { return m_duration; }

private:
    ::std::string m_name;
    ::std::string m_resource;
    float m_duration;
};

}}

#endif
