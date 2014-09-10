#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "UIPopupPageDef.hpp"

namespace UI { namespace App {

UIPopupPageDef::UIPopupPageDef(Gd::App::Application & app, Cpe::Cfg::Node const & cfg)
	: m_name(cfg.name())
	, m_resource(cfg["load-from"].asString(""))
	, m_duration(cfg["duration"].dft(0.0f))
{
    if (m_resource.empty()) {
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "popup page %s: load-from not configured!", m_name.c_str());
    }

    if (!loadBindings(app, cfg["bindings"])) {
        APP_CTX_THROW_EXCEPTION(
            app, ::std::runtime_error,
            "popup page %s: bindings fail!", m_name.c_str());
    }
}

}}

