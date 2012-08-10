/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/variant_utils.h"

namespace config {

ConfigBase::~ConfigBase() {
}

void ConfigBase::initialize() {
    // override
}

uint32_t ConfigBase::hashCode() {
    return (uint32_t)(uint64_t)this;
}

bool ConfigBase::equals(const ConfigVariant& other) {
    if (!instanceof<ConfigBasePtr>(other)) {
        return false;
    }
    return this == variant_get<ConfigBasePtr>(other).get();
}

std::string ConfigBase::toString() {
    return getClassName() + " " + boost::lexical_cast<std::string>(this);
}

std::string ConfigBase::getClassName() {
    return "ConfigBase";
}

}
