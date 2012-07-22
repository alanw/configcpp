/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/config_value.h"
#include "configcpp/config_object.h"
#include "configcpp/config_list.h"
#include "configcpp/detail/config_impl.h"

namespace config {

ConfigValuePtr ConfigValue::fromAnyRef(const ConfigVariant& object, const std::string& originDescription) {
    return ConfigImpl::fromAnyRef(object, originDescription);
}

ConfigObjectPtr ConfigValue::fromMap(const MapVariant& values, const std::string& originDescription) {
    return std::dynamic_pointer_cast<ConfigObject>(fromAnyRef(values, originDescription));
}

ConfigListPtr ConfigValue::fromVector(const VectorVariant& values, const std::string& originDescription) {
    return std::dynamic_pointer_cast<ConfigList>(fromAnyRef(values, originDescription));
}

}
