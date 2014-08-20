#include "NPControlNode.hpp"
#include "gdpp/app/Log.hpp"
#include "NPRTTI.h"
#include "uipp/sprite_np/NpUtils.hpp"

namespace UI { namespace Sprite { namespace NP {
NPControlNode::NPControlNode(const std::string & res)
	: m_control(NULL)
    , m_pan(NULL)
{
	NPGUIControl* pControl =  NPDynamicCast(NPGUIControl, NPGUIPanel::sGetTemplate("dumyPan"));
	m_control = new ControlNode();
	m_control->CloneFrom(pControl);

	NPGUIPanel* pPanelTemplate	 = NPDynamicCast(NPGUIPanel, NPGUIPanel::sGetTemplate(res));
	m_pan = new NPGUIPanel();
	m_pan->CloneFrom(pPanelTemplate);
	m_control->AddChild(m_pan, true);	
}

NPControlNode::NPControlNode(NPGUIControl * ctrl)
	: m_control(NULL)
    , m_pan(NULL)
{
	m_control = NPDynamicCast(ControlNode, ctrl);
}

NPControlNode::~NPControlNode(){

}

void NPControlNode::Render(void) {
    NPNode::Render();
	if(m_control){
		UpdateGUI();
		m_control->RecalcRecur();
		m_control->Render();
	}
}

void	NPControlNode::Update			( npf32 deltaTime ){
	NPNode::Update(deltaTime);
	if(m_control){
		m_control->Update(deltaTime);
	}
}

bool NPControlNode::WasRuning(){
	if(m_control){
		return m_control->WasRuning();
	}
	return false;
}
void NPControlNode::UpdateGUI(void) {
	 if (m_pan == NULL) 
		 return;

    NPVector3 trans = GetParent()->GetFinalT();
	NPVector3 const & scale = GetParent()->GetFinalS();	

	npu08 alignVert = m_pan->GetAlignVert();
	npu08 alignHorz = m_pan->GetAlignHorz();

	NPVector2 const & size = m_pan->GetRenderRealSZ();
	if(alignVert == AV_CENTER){
		trans.y -= size.y * scale.y / 2.0f;
	}
	else if(alignVert == AV_BOTTOM){
		trans.y -= size.y * scale.y;
	}

	if(alignHorz == AH_CENTER){
		trans.x -= size.x * scale.x / 2.0f;
	}
	else if(alignHorz == AH_RIGHT){
		trans.x -= size.x * scale.x;
	}

	m_pan->SetRenderRealPT(NPVector2(trans.x, trans.y));
	m_pan->SetScale(NPVector2(scale.x, scale.y));
}

int NPControlNode::setAttr(const char * ctrl_name, const char * attr, const char * value) {
	NPGUIControl * control = ctrl_name[0] ? findChild(m_control, ctrl_name) : m_control;
    if (control == NULL) {
        APP_ERROR("NPControl(%s) not exist!", ctrl_name);
        return -1;
    }

	return UI::Sprite::NP::NpUtils::setAttr(control, attr, value);
}

NPGUIControl * NPControlNode::findChild(NPGUIControl * control, const char * name) {
    while(const char * sep = strchr(name, '.')) {
        char buf[64];
        size_t len = sep - name;

        strncpy(buf, name, len);
        buf[len] = 0;

        control = control->RecChildByName(buf);
        if (control == NULL) {
            APP_ERROR("child control %s not exist!", buf);
            return NULL;
        }

        name = sep + 1;
    }

    control = control->RecChildByName(name);
    if (control == NULL) {
        APP_ERROR("child control %s not exist!", name);
        return NULL;
    }

    return control;
}
}}}
