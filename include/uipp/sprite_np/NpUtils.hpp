#ifndef UIPP_SPRITE_NP_NPUTILS_H
#define UIPP_SPRITE_NP_NPUTILS_H
#include "System.hpp"
#include "NPRectangle.h"
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite/System.hpp"
#include "NPGUIControl.h"

namespace UI { namespace Sprite { namespace NP {

struct RectNode  
{  
	std::string name;
	UI::Sprite::P2D::Rect rect;
	RectNode()
		: name(""){
	}
};

class NpUtils{
public:
	 static  uint32_t getFileID(const char * typeName, const char * path, const char * file);
	 static  uint32_t getFileID(const char * typeName, uint32_t	resFile);

	 static  void	  loadByType(const char * type_name, char * arg, int resId);
	 static  void	  loadByResName(char * res, int resId);
	 static	 void	  preloadResources(UI::Sprite::Entity & entity);
	 static  int	  getRectNodeList(std::vector<RectNode> & rectNodeVer,  char * resName, UI::Sprite::Entity & entity);
	 static  void	  setMerge(char * resName, UI::Sprite::World & world, UI::Sprite::Entity & entity);
	 static  void     setBackFrame(NPGUIControl * control, const char * resource);
	 static  bool	  readResId(char * res, int & id);
	 static  int	  setAttr(NPGUIControl * control, const char * attr, const char * value);
};

}}}

#endif
