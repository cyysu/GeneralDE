#include "RControlNode.hpp"
#include "gdpp/app/Log.hpp"
#include "RRTTI.h"
#include "uipp/sprite_np/NpUtils.hpp"

namespace UI { namespace Sprite { namespace R {
RControlNode::RControlNode(const std::string & res)
	: m_control(NULL)
    , m_pan(NULL)
{
	RGUIControl* pControl =  RDynamicCast(RGUIControl, RGUIPanel::sGetTemplate("dumyPan"));
	m_control = new ControlNode();
	m_control->CloneFrom(pControl);

	RGUIPanel* pPanelTemplate	 = RDynamicCast(RGUIPanel, RGUIPanel::sGetTemplate(res));
	m_pan = new RGUIPanel();
	m_pan->CloneFrom(pPanelTemplate);
	m_control->AddChild(m_pan, true);	
}

RControlNode::RControlNode(RGUIControl * ctrl)
	: m_control(NULL)
    , m_pan(NULL)
{
	m_control = RDynamicCast(ControlNode, ctrl);
}

RControlNode::~RControlNode(){

}

void RControlNode::Render(void) {
    RNode::Render();
	if(m_control){
		UpdateGUI();
		m_control->RecalcRecur();
		m_control->Render();
	}
}

void	RControlNode::Update			( npf32 deltaTime ){
	RNode::Update(deltaTime);
	if(m_control){
		m_control->Update(deltaTime);
	}
}

bool RControlNode::WasRuning(){
	if(m_control){
		return m_control->WasRuning();
	}
	return false;
}
void RControlNode::UpdateGUI(void) {
	 if (m_pan == NULL) 
		 return;

    RVector3 trans = GetParent()->GetFinalT();
	RVector3 const & scale = GetParent()->GetFinalS();	

	npu08 alignVert = m_pan->GetAlignVert();
	npu08 alignHorz = m_pan->GetAlignHorz();

	RVector2 const & size = m_pan->GetRenderRealSZ();
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

	m_pan->SetRenderRealPT(RVector2(trans.x, trans.y));
	m_pan->SetScale(RVector2(scale.x, scale.y));
}

int RControlNode::setAttr(const char * ctrl_name, const char * attr, const char * value) {
	RGUIControl * control = ctrl_name[0] ? findChild(m_control, ctrl_name) : m_control;
    if (control == NULL) {
        APP_ERROR("RControl(%s) not exist!", ctrl_name);
        return -1;
    }

	return UI::Sprite::R::NpUtils::setAttr(control, attr, value);
}

RGUIControl * RControlNode::findChild(RGUIControl * control, const char * name) {
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
