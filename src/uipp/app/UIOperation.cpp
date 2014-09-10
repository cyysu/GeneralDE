#include "UIOperation.hpp"

namespace UI { namespace App {

UIOperation::~UIOperation() {
}

::std::auto_ptr<UIOperation>
UIOperation::create(Cpe::Cfg::Node const & config) {
    throw 0;
}

}}

