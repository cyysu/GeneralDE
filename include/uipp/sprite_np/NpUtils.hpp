#ifndef UIPP_SPRITE_NP_NPUTILS_H
#define UIPP_SPRITE_NP_NPUTILS_H
#include "System.hpp"
#include "NPRectangle.h"

namespace UI { namespace Sprite { namespace NP {

class NpUtils{
public:
	 static  uint32_t getFileID(const char * typeName, const char * path, const char * file);
	 static  uint32_t getFileID(const char * typeName, uint32_t	resFile);

	 static  void	  loadByType(const char * type_name, char * arg, int resId);
	 static  void	  loadByResName(char * res, int resId);
};

}}}

#endif
