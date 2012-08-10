/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_VALUE_TYPE_H_
#define CONFIG_VALUE_TYPE_H_

#include "configcpp/config_types.h"

namespace config {

///
/// The type of a configuration value (following the <a
/// href="http://json.org">JSON</a> type schema).
///
enum class ConfigValueType : uint32_t {
    OBJECT, LIST, NUMBER, BOOLEAN, NONE, STRING
};

class ConfigValueTypeEnum {
public:
    static std::string name(ConfigValueType valueType);
};

}

#endif // CONFIG_VALUE_TYPE_H_
