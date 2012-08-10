/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/simple_include_context.h"
#include "configcpp/detail/parseable.h"
#include "configcpp/detail/simple_includer.h"

namespace config {

SimpleIncludeContext::SimpleIncludeContext(const ParseablePtr& parseable) :
    parseable(parseable) {
}

SimpleIncludeContextPtr SimpleIncludeContext::withParseable(const ParseablePtr& parseable) {
    if (parseable == this->parseable) {
        return shared_from_this();
    }
    else {
        return make_instance(parseable);
    }
}

ConfigParseablePtr SimpleIncludeContext::relativeTo(const std::string& filename) {
    if (parseable) {
        return parseable->relativeTo(filename);
    }
    else {
        return nullptr;
    }
}

ConfigParseOptionsPtr SimpleIncludeContext::parseOptions() {
    return SimpleIncluder::clearForInclude(parseable->options());
}

}
