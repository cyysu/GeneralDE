#ifndef UIPP_APP_SYSTEM_H
#define UIPP_APP_SYSTEM_H
#include "gdpp/app/System.hpp"
#include "uipp/sprite/System.hpp"
#include "render/cache/ui_cache_types.h"

class RGUIWindow;
class RGUIControl;
class RGUIEventArgs;
class RGUIListBoxAdvItem;

namespace UI { namespace App {

enum Language {
    LANGUAGE_EN = 1,
    LANGUAGE_CN = 2,
    LANGUAGE_TW = 3,
};

class Env;
class Device;
class Runing;
class Page;
class UICenter;
class UIPageProxy;
class UIPhase;
class UIPhaseNode;

class PageEvtSch;

template<typename T>
class PageGen;

}}

#endif
