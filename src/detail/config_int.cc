/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_int.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigInt::ConfigInt(const ConfigOriginPtr& origin, int32_t value, const std::string& originalText) :
    ConfigNumber(origin, originalText),
    value(value) {
}

ConfigValueType ConfigInt::valueType() {
    return ConfigValueType::NUMBER;
}

ConfigVariant ConfigInt::unwrapped() {
    return value;
}

std::string ConfigInt::transformToString() {
    std::string s = ConfigNumber::transformToString();
    if (s.empty()) {
        return boost::lexical_cast<std::string>(value);
    }
    else {
        return s;
    }
}

int64_t ConfigInt::int64Value() {
    return static_cast<int64_t>(value);
}

double ConfigInt::doubleValue() {
    return static_cast<double>(value);
}

AbstractConfigValuePtr ConfigInt::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin, value, originalText);
}

}
