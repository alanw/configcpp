/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_value_type.h"

namespace config {

std::string ConfigValueTypeEnum::name(ConfigValueType valueType) {
    typedef std::map<ConfigValueType, std::string> ConfigValueTypeName;
    static ConfigValueTypeName names = {
        {ConfigValueType::OBJECT, "OBJECT"},
        {ConfigValueType::LIST, "LIST"},
        {ConfigValueType::NUMBER, "NUMBER"},
        {ConfigValueType::BOOLEAN, "BOOLEAN"},
        {ConfigValueType::NONE, "NONE"},
        {ConfigValueType::STRING, "STRING"}
    };
    return names[valueType];
}

}
