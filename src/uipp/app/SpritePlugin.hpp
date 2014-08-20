#ifndef UIPP_APP_SPRITEEXTERN_H
#define UIPP_APP_SPRITEEXTERN_H
#include "System.hpp"

namespace UI { namespace App {

class SpritePlugin {
public:
    virtual ~SpritePlugin();

    static ::std::auto_ptr<SpritePlugin> create(EnvExt & env);
};

}}

#endif


