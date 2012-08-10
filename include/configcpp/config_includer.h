/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_INCLUDER_H_
#define CONFIG_INCLUDER_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Implement this interface and provide an instance to
/// {@link ConfigParseOptions#setIncluder ConfigParseOptions.setIncluder()} to
/// customize handling of {@code include} statements in config files. You may
/// also want to implement {@link ConfigIncluderFile}.
///
class ConfigIncluder {
public:
    /// Returns a new includer that falls back to the given includer. This is how
    /// you can obtain the default includer; it will be provided as a fallback.
    /// It's up to your includer to chain to it if you want to. You might want to
    /// merge any files found by the fallback includer with any objects you load
    /// yourself.
    ///
    /// It's important to handle the case where you already have the fallback
    /// with a "return this", i.e. this method should not create a new object if
    /// the fallback is the same one you already have. The same fallback may be
    /// added repeatedly.
    ///
    /// @param fallback
    /// @return a new includer
    virtual ConfigIncluderPtr withFallback(const ConfigIncluderPtr& fallback) = 0;

    /// Parses another item to be included. The returned object typically would
    /// not have substitutions resolved. You can throw a ConfigException here to
    /// abort parsing, or return an empty object.
    ///
    /// This method is used for a "heuristic" include statement that does not
    /// specify file resource. If the include statement does specify, then the
    /// same class implementing {@link ConfigIncluder} must also implement
    /// {@link ConfigIncluderFile} as needed, or a default includer will be used.
    ///
    /// @param context
    ///            some info about the include context
    /// @param what
    ///            the include statement's argument
    /// @return a non-null ConfigObject
    virtual ConfigObjectPtr include(const ConfigIncludeContextPtr& context,
                                    const std::string& what) = 0;
};

}

#endif // CONFIG_INCLUDER_H_
