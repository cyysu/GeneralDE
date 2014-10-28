#ifndef UIPP_SPRITE_R_CONTROL_NODE_H
#define UIPP_SPRITE_R_CONTROL_NODE_H
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite_render/System.hpp"
#include "RGUIPanel.h"
#include "gd/app/app_log.h"
#include "RGUIControl.h"

class ControlNode : public RGUIControl {
public:
	ControlNode(void)
	: m_isRuning(true)
	{
	}

	ControlNode(RGUIControl * ctrl)
	: m_isRuning(true)
	{
	}

	bool WasRuning(void)
	{
		return m_isRuning;
	}

protected:
	virtual void	OnEventAnimPlay					( RGUIEventArgs& args )
	{
		RGUIControl::OnEventAnimPlay(args);
		m_isRuning = true;
	}

	virtual void	OnEventAnimStop					( RGUIEventArgs& args )
	{
		RGUIControl::OnEventAnimStop(args);
		m_isRuning = false;
	}

private:
	bool m_isRuning;
};

namespace UI { namespace Sprite { namespace R {
class RControlNode : public RNode {
public:
	RControlNode(const std::string & res);
	RControlNode(RGUIControl * ctrl);
	~RControlNode();
	void		Render	(void);
	void		Update	(npf32 deltaTime);

	void	setScale(RGUIControl * control, RVector3 const & scale);
	int		setAttr(const char * control, const char * attr, const char * value);
	bool	WasRuning();

private:
	RGUIControl * findChild(RGUIControl * control, const char * name);
	void UpdateGUI(void);


private:
	ControlNode * m_control;
	RGUIPanel * m_pan;
};

}}}

#endif
