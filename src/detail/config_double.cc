/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_double.h"
#include "configcpp/config_value_type.h"

namespace config {

ConfigDouble::ConfigDouble(const ConfigOriginPtr& origin, double value, const std::string& originalText) :
    ConfigNumber(origin, originalText),
    value(value) {
}

ConfigValueType ConfigDouble::valueType() {
    return ConfigValueType::NUMBER;
}

ConfigVariant ConfigDouble::unwrapped() {
    return value;
}

std::string ConfigDouble::transformToString() {
    std::string s = ConfigNumber::transformToString();
    if (s.empty()) {
        return boost::lexical_cast<std::string>(value);
    }
    else {
        return s;
    }
}

int64_t ConfigDouble::int64Value() {
    return static_cast<int64_t>(value);
}

double ConfigDouble::doubleValue() {
    return value;
}

AbstractConfigValuePtr ConfigDouble::newCopy(const ConfigOriginPtr& origin) {
    return make_instance(origin, value, originalText);
}

}
