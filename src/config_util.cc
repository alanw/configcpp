/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_util.h"
#include "configcpp/detail/config_impl_util.h"

namespace config {

ConfigUtil::ConfigUtil() {
    // private
}

std::string ConfigUtil::quoteString(const std::string& s) {
    return ConfigImplUtil::renderJsonString(s);
}

std::string ConfigUtil::joinPath(const VectorString& elements) {
    return ConfigImplUtil::joinPath(elements);
}

VectorString ConfigUtil::splitPath(const std::string& path) {
    return ConfigImplUtil::splitPath(path);
}

}
