/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_int64.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigInt64::ConfigInt64(const ConfigOriginPtr& origin, int64_t value, const std::string& originalText) :
    ConfigNumber(origin, originalText),
    value(value) {
}

ConfigValueType ConfigInt64::valueType() {
    return ConfigValueType::NUMBER;
}

ConfigVariant ConfigInt64::unwrapped() {
    return value;
}

std::string ConfigInt64::transformToString() {
    std::string s = ConfigNumber::transformToString();
    if (s.empty()) {
        return boost::lexical_cast<std::string>(value);
    }
    else {
        return s;
    }
}

int64_t ConfigInt64::int64Value() {
    return value;
}

double ConfigInt64::doubleValue() {
    return static_cast<double>(value);
}

AbstractConfigValuePtr ConfigInt64::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin, value, originalText);
}

}
