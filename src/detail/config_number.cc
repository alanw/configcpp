/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_number.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/detail/config_int.h"
#include "configcpp/detail/config_int64.h"
#include "configcpp/detail/config_double.h"
#include "configcpp/config_exception.h"

namespace config {

ConfigNumber::ConfigNumber(const ConfigOriginPtr& origin, const std::string& originalText) :
    AbstractConfigValue(origin),
    originalText(originalText) {
}

std::string ConfigNumber::transformToString() {
    return originalText;
}

int32_t ConfigNumber::intValueRangeChecked(const std::string& path) {
    int64_t l = int64Value();

    if (l < std::numeric_limits<int32_t>::min() || l > std::numeric_limits<int32_t>::max()) {
        throw ConfigExceptionWrongType(origin(), path, "32-bit integer", "out-of-range value " + boost::lexical_cast<std::string>(l));
    }
    return static_cast<int32_t>(l);
}

bool ConfigNumber::isWhole() {
    int64_t asInt64 = int64Value();
    return asInt64 == doubleValue();
}

bool ConfigNumber::canEqual(const ConfigVariant& other) {
    return instanceof<ConfigNumber>(other);
}

bool ConfigNumber::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<ConfigNumber>(other)) {
        return canEqual(other) &&
                (this->valueType() == dynamic_get<ConfigValue>(other)->valueType()) &&
                ConfigImplUtil::equalsHandlingNull(this->unwrapped(), dynamic_get<ConfigValue>(other)->unwrapped());
    }
    else {
        return false;
    }
}

uint32_t ConfigNumber::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    int64_t asInt64;
    if (isWhole()) {
        asInt64 = int64Value();
    }
    else {
        const uint64_t DOUBLE_EXPONENT_MASK = 0x7ff0000000000000LL;
        const uint64_t DOUBLE_MANTISSA_MASK = 0x000fffffffffffffLL;
        const uint64_t DOUBLE_NAN_BITS = DOUBLE_EXPONENT_MASK | 0x0008000000000000LL;

        double value = doubleValue();
        std::memcpy(&asInt64, &value, sizeof(double));

        // replacement for Double.doubleToInt64Bits(doubleValue())
        if ((asInt64 & DOUBLE_EXPONENT_MASK) == DOUBLE_EXPONENT_MASK) {
            if (asInt64 & DOUBLE_MANTISSA_MASK) {
                asInt64 = DOUBLE_NAN_BITS;
            }
        }
    }
    return static_cast<uint32_t>(asInt64 ^ (asInt64 >> 32));
}

ConfigNumberPtr ConfigNumber::newNumber(const ConfigOriginPtr& origin, int64_t number, const std::string& originalText) {
    if (number <= std::numeric_limits<int32_t>::max() && number >= std::numeric_limits<int32_t>::min()) {
        return ConfigInt::make_instance(origin, static_cast<int32_t>(number), originalText);
    }
    else {
        return ConfigInt64::make_instance(origin, number, originalText);
    }
}

ConfigNumberPtr ConfigNumber::newNumber(const ConfigOriginPtr& origin, double number, const std::string& originalText) {
    int64_t asInt64 = static_cast<int64_t>(number);
    if (asInt64 == number) {
        return newNumber(origin, asInt64, originalText);
    }
    else {
        return ConfigDouble::make_instance(origin, number, originalText);
    }
}

}
