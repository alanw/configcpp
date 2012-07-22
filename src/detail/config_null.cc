/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_null.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigNull::ConfigNull(const ConfigOriginPtr& origin) :
    AbstractConfigValue(origin) {
}

ConfigValueType ConfigNull::valueType() {
    return ConfigValueType::NONE;
}

ConfigVariant ConfigNull::unwrapped() {
    return null();
}

std::string ConfigNull::transformToString() {
    return "null";
}

AbstractConfigValuePtr ConfigNull::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin);
}

}
