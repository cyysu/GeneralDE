#include "gdpp/app/ModuleDef.hpp"
#include "uipp/app/Gen/PageGen.hpp"

namespace UI { namespace App {

class UIUniPage : public PageGen<UIUniPage> {
public:
    static cpe_hash_string_t NAME;

    UIUniPage(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg)
        : Base(app, cfg)
    {
    }
};

GDPP_APP_MODULE_DEF(UIUniPage, UIUniPage);

}}
