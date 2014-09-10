#ifndef UIPP_SPRITE_R_RUTILS_H
#define UIPP_SPRITE_R_RUTILS_H
#include "System.hpp"
#include "RRectangle.h"
#include "uipp/sprite_2d/System.hpp"
#include "uipp/sprite/System.hpp"
#include "RGUIControl.h"

namespace UI { namespace Sprite { namespace R {

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
	 static  void     setBackFrame(RGUIControl * control, const char * resource);
	 static  bool	  readResId(char * res, int & id);
	 static  int	  setAttr(RGUIControl * control, const char * attr, const char * value);
};

}}}

#endif
