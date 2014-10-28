#ifndef UIPP_APP_RUNING_H
#define UIPP_APP_RUNING_H
#include "System.hpp"

namespace UI { namespace App {

class Runing {
public:
	virtual float fps(void) const = 0;
	virtual float runingFps(void) const = 0;
    virtual ~Runing();
};

}}

#endif
