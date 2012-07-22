/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_SYNTAX_H_
#define CONFIG_SYNTAX_H_

namespace config {

///
/// The syntax of a character stream, <a href="http://json.org">JSON</a>, <a
/// href="https://github.com/typesafehub/config/blob/master/HOCON.md">HOCON</a>
/// aka ".conf".
///
enum class ConfigSyntax : uint32_t {
    /// Null value.
    NONE,

    /// Pedantically strict <a href="http://json.org">JSON</a> format; no
    /// comments, no unexpected commas, no duplicate keys in the same object.
    JSON,

    /// The JSON-superset <a
    /// href="https://github.com/typesafehub/config/blob/master/HOCON.md"
    /// >HOCON</a> format.
    CONF
};

}

#endif // CONFIG_SYNTAX_H_
