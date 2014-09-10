#ifndef UIPP_APP_UIPAGEBINDINGS_H
#define UIPP_APP_UIPAGEBINDINGS_H
#include <string>
#include <list>
#include "System.hpp"

namespace UI { namespace App {

class UIPageBindings {
public:
    struct Binding {
        ::std::string m_control;
        ::std::string m_attr;
        ::std::string m_value;
    };

    bool loadBindings(Gd::App::Application & app, Cpe::Cfg::Node const & cfg);
    void setData(Gd::App::Application & app, RGUIControl * control, const char * msg, UICenter const & uiCenter);
    void setData(Gd::App::Application & app, RGUIControl * control, LPDRMETA meta, void const * data, size_t data_size, UICenter const & uiCenter);

    static int setControlValue(
        Gd::App::Application & app,
        RGUIControl * control, const char * control_name,
        const char * attr, const char * value, UICenter const & uiCenter);

private:
    ::std::list<Binding> m_bindins;
};

}}

#endif
