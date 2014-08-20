#ifndef UIPP_SPRITE_NP_CONTROL_NODE_H
#define UIPP_SPRITE_NP_CONTROL_NODE_H
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite_np/System.hpp"
#include "NPGUIPanel.h"
#include "gd/app/app_log.h"
#include "NPGUIControl.h"

class ControlNode : public NPGUIControl {
public:
	ControlNode(void)
	: m_isRuning(true)
	{
	}

	ControlNode(NPGUIControl * ctrl)
	: m_isRuning(true)
	{
	}

	bool WasRuning(void)
	{
		return m_isRuning;
	}

protected:
	virtual void	OnEventAnimPlay					( NPGUIEventArgs& args )
	{
		NPGUIControl::OnEventAnimPlay(args);
		m_isRuning = true;
	}

	virtual void	OnEventAnimStop					( NPGUIEventArgs& args )
	{
		NPGUIControl::OnEventAnimStop(args);
		m_isRuning = false;
	}

private:
	bool m_isRuning;
};

namespace UI { namespace Sprite { namespace NP {
class NPControlNode : public NPNode {
public:
	NPControlNode(const std::string & res);
	NPControlNode(NPGUIControl * ctrl);
	~NPControlNode();
	void		Render	(void);
	void		Update	(npf32 deltaTime);

	void	setScale(NPGUIControl * control, NPVector3 const & scale);
	int		setAttr(const char * control, const char * attr, const char * value);
	bool	WasRuning();

private:
	NPGUIControl * findChild(NPGUIControl * control, const char * name);
	void UpdateGUI(void);


private:
	ControlNode * m_control;
	NPGUIPanel * m_pan;
};

}}}

#endif
