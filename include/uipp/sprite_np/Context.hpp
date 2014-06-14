#ifndef UIPP_SPRITE_B2_CONTEXT_H
#define UIPP_SPRITE_B2_CONTEXT_H
#include "NP2DS.h"
#include "uipp/sprite/WorldRes.hpp"
#include "System.hpp"
#include "NPGUIControl.h"

namespace UI { namespace Sprite { namespace NP {

class Context {
public:
    virtual WorldRes & worldRes(void) = 0;
    virtual WorldRes const & worldRes(void) const = 0;

    virtual World & world(void) = 0;
    virtual World const & world(void) const = 0;

    virtual Layer & defaultLayer(void) = 0;
    virtual Layer const & defaultLayer(void) const = 0;

    virtual Layer * findLayer(const char * name) = 0;
    virtual Layer const * findLayer(const char * name) const = 0;

    virtual Layer & layer(const char * name) = 0;
    virtual Layer const & layer(const char * name) const = 0;

    virtual Layer & createLayer(const char * name, Layer * before = NULL) = 0;

	virtual void registerExternCtrl(const char * name, NPGUIControl * ctrl) = 0;
	virtual NPGUIControl * findExternCtrl(const char * name) = 0;
	virtual void unregisterExternCtrl(NPGUIControl * ctrl) = 0;
	virtual void clearExternCtrls(void) = 0;

    virtual ~Context();

    static const char * NAME;

    static Context & install(World & world);
};

}}}

#endif
