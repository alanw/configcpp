/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_UTIL_H_
#define CONFIG_UTIL_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Contains static utility methods.
///
class ConfigUtil {
private:
    ConfigUtil();

public:
    /// Quotes and escapes a string, as in the JSON specification.
    ///
    /// @param s
    ///            a string
    /// @return the string quoted and escaped
    static std::string quoteString(const std::string& s);

    /// Converts a list of strings to a path expression, by quoting the path
    /// elements as needed and then joining them separated by a period. A path
    /// expression is usable with a {@link Config}, while individual path
    /// elements are usable with a {@link ConfigObject}.
    ///
    /// @param elements
    ///            the keys in the path
    /// @return a path expression
    /// @throws ConfigException
    ///             if the list is empty
    static std::string joinPath(const VectorString& elements = VectorString());

    /// Converts a path expression into a list of keys, by splitting on period
    /// and unquoting the individual path elements. A path expression is usable
    /// with a {@link Config}, while individual path elements are usable with a
    /// {@link ConfigObject}.
    ///
    /// @param path
    ///            a path expression
    /// @return the individual keys in the path
    /// @throws ConfigException
    ///             if the path expression is invalid
    static VectorString splitPath(const std::string& path);
};

}

#endif // CONFIG_UTIL_H_
