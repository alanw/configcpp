/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef INSTANCE_UTILS_H_
#define INSTANCE_UTILS_H_

#include "configcpp/config_types.h"

namespace config {

/// Return whether given object is of a specified type
template <typename T, typename O>
bool instanceof(const O& object) {
    return !!std::dynamic_pointer_cast<T>(object);
}

/// Return whether given variant is of a specified type
template <typename T>
typename std::enable_if<std::is_scalar<T>::value, bool>::type instanceof(const ConfigVariant& var) {
    return boost::get<T>(&var);
}

/// Return whether given variant is of a specified type
template <typename T>
typename std::enable_if<std::is_class<T>::value, bool>::type instanceof(const ConfigVariant& var) {
    if (var.type() == typeid(ConfigBasePtr)) {
        auto obj = boost::get<ConfigBasePtr>(var);
        return !obj || instanceof<T>(obj);
    }
    return boost::get<T>(&var);
}

}

#endif // INSTANCE_UTILS_H_
