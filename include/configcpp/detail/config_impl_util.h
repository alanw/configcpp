/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_IMPL_UTIL_H_
#define CONFIG_IMPL_UTIL_H_

#include "configcpp/config_types.h"

namespace config {

class ConfigImplUtil {
public:
    static bool equalsHandlingNull(const ConfigVariant& a, const ConfigVariant& b);

    /// This is public ONLY for use by the "config" package, DO NOT USE this ABI
    /// may change.
    static std::string renderJsonString(const std::string& s);

    static std::string renderStringUnquotedIfPossible(const std::string& s);

    /// This is public ONLY for use by the "config" package, DO NOT USE this ABI
    /// may change. You can use the version in ConfigUtil instead.
    static std::string joinPath(const VectorString& elements = VectorString());

    /// This is public ONLY for use by the "config" package, DO NOT USE this ABI
    /// may change. You can use the version in ConfigUtil instead.
    static VectorString splitPath(const std::string& path);
};

}

#endif // CONFIG_IMPL_UTIL_H_
