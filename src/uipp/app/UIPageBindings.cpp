#include "RRTTI.h"
#include "RGUIProgressBar.h"
#include "RGUILabel.h"
#include "cpe/utils/stream_mem.h"
#include "cpepp/cfg/Node.hpp"
#include "uipp/sprite_render/NpUtils.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/app/Page.hpp"
#include "UIPageBindings.hpp"

namespace UI { namespace App {

bool UIPageBindings::loadBindings(Gd::App::Application & app, Cpe::Cfg::Node const & cfg) {
    Cpe::Cfg::NodeConstIterator binding_node_it = cfg.childs();

    while(Cpe::Cfg::Node const * binding_node = binding_node_it.next()) {
        const char * control_name = (*binding_node)["control"].asString(NULL);
        if (control_name == NULL) {
            APP_CTX_ERROR(app, "binding control not configured!");
            return false;
        }

        Cpe::Cfg::NodeConstIterator attr_node_it = binding_node->childs();
        while(Cpe::Cfg::Node const * attr_node = attr_node_it.next()) {
            const char * attr_name = attr_node->name();
            if (strcmp(attr_name, "control") == 0) continue;

            const char * value;
            char value_buf[64];

            value = cfg_as_string(*attr_node, NULL);
            if (value == NULL) {
                struct write_stream_mem s = CPE_WRITE_STREAM_MEM_INITIALIZER(value_buf, sizeof(value_buf));
                cfg_print_inline(*attr_node, (write_stream_t)&s);
                stream_putc((write_stream_t)&s, 0);
                value = value_buf;
            }

            Binding binding;
            binding.m_control = control_name;
            binding.m_attr = attr_name;
            binding.m_value = value;

            m_bindins.push_back(binding);
        }
    }

    return true;
}

void UIPageBindings::setData(Gd::App::Application & app, RGUIControl * root, const char * msg, UICenter const & uiCenter) {
    for( ::std::list<Binding>::iterator it = m_bindins.begin();
         it != m_bindins.end();
         ++it)
    {
        if (it->m_value.empty()) continue;

        if (it->m_value[0] != '@') {
            setControlValue(app, root, it->m_control.c_str(), it->m_attr.c_str(), it->m_value.c_str(), uiCenter);
            continue;
        }

        if (strcmp(it->m_value.c_str() + 1, "message") == 0) {
            setControlValue(app, root, it->m_control.c_str(), it->m_attr.c_str(), msg, uiCenter);
        }
        else {
            setControlValue(app, root, it->m_control.c_str(), it->m_attr.c_str(), "", uiCenter);
        }
    }
}

void UIPageBindings::setData(
    Gd::App::Application & app, RGUIControl * root, LPDRMETA meta, void const * data, size_t data_size, UICenter const & uiCenter) {
}

int UIPageBindings::setControlValue(
    Gd::App::Application & app,
    RGUIControl * root, const char * ctrl_name, const char * attr, const char * value, UICenter const & uiCenter)
{
	RGUIControl * control = ctrl_name[0] ? Page::findChild(root, ctrl_name, uiCenter) : root;
    if (control == NULL) {
        APP_CTX_ERROR(app, "RControl(%s) not exist!", ctrl_name);
        return -1;
    }

	return UI::Sprite::R::NpUtils::setAttr(control, attr, value);
}

}}
