/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_PARSEABLE_H_
#define CONFIG_PARSEABLE_H_

#include "configcpp/config_types.h"

namespace config {

///
/// An opaque handle to something that can be parsed, obtained from
/// {@link ConfigIncludeContext}.
///
/// <p>
/// <em>Do not implement this interface</em>; it should only be implemented by
/// the config library. Arbitrary implementations will not work because the
/// library internals assume a specific concrete implementation. Also, this
/// interface is likely to grow new methods over time, so third-party
/// implementations will break.
///
class ConfigParseable {
public:
    /// Parse whatever it is. The options should come from
    /// {@link ConfigParseable#options options()} but you could tweak them if
    /// you like.
    ///
    /// @param options
    ///            parse options, should be based on the ones from
    ///            {@link ConfigParseable#options options()}
    virtual ConfigObjectPtr parse(const ConfigParseOptionsPtr& options) = 0;

    /// Returns a {@link ConfigOrigin} describing the origin of the parseable
    /// item.
    virtual ConfigOriginPtr origin() = 0;

    /// Get the initial options, which can be modified then passed to parse().
    /// These options will have the right description, includer, and other
    /// parameters already set up.
    virtual ConfigParseOptionsPtr options() = 0;
};

}

#endif // CONFIG_PARSEABLE_H_
