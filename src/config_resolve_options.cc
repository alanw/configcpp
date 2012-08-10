/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_resolve_options.h"

namespace config {

ConfigResolveOptions::ConfigResolveOptions(bool useSystemEnvironment) :
    useSystemEnvironment(useSystemEnvironment) {
}

ConfigResolveOptionsPtr ConfigResolveOptions::defaults() {
    return make_instance(true);
}

ConfigResolveOptionsPtr ConfigResolveOptions::noSystem() {
    return make_instance(false);
}

ConfigResolveOptionsPtr ConfigResolveOptions::setUseSystemEnvironment(bool value) {
    return make_instance(value);
}

bool ConfigResolveOptions::getUseSystemEnvironment() {
    return useSystemEnvironment;
}

}
