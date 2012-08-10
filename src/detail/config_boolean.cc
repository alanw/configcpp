/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_boolean.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigBoolean::ConfigBoolean(const ConfigOriginPtr& origin, bool value) :
    AbstractConfigValue(origin),
    value(value) {
}

ConfigValueType ConfigBoolean::valueType() {
    return ConfigValueType::BOOLEAN;
}

ConfigVariant ConfigBoolean::unwrapped() {
    return value;
}

std::string ConfigBoolean::transformToString() {
    return value ? "true" : "false";
}

AbstractConfigValuePtr ConfigBoolean::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin, value);
}

}
