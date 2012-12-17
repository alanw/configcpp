/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_string.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/config_value_type.h"
#include "configcpp/config_render_options.h"

namespace config {

ConfigString::ConfigString(const ConfigOriginPtr& origin, const std::string& value) :
    AbstractConfigValue(origin),
    value(value) {
}

ConfigValueType ConfigString::valueType() {
    return ConfigValueType::STRING;
}

ConfigVariant ConfigString::unwrapped() {
    return value;
}

std::string ConfigString::transformToString() {
    return value;
}

void ConfigString::render(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) {
    std::string rendered;
    if (options->getJson()) {
        rendered = ConfigImplUtil::renderJsonString(value);
    }
    else {
        rendered = ConfigImplUtil::renderStringUnquotedIfPossible(value);
    }
    s += rendered;
}

AbstractConfigValuePtr ConfigString::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin, value);
}

}
