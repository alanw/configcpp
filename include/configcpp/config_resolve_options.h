/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_RESOLVE_OPTIONS_H_
#define CONFIG_RESOLVE_OPTIONS_H_

#include "configcpp/detail/config_base.h"

namespace config {

///
/// A set of options related to resolving substitutions. Substitutions use the
/// <code>${foo.bar}</code> syntax and are documented in the <a
/// href="https://github.com/typesafehub/config/blob/master/HOCON.md">HOCON</a>
/// spec.
/// <p>
/// This object is immutable, so the "setters" return a new object.
/// <p>
/// Here is an example of creating a custom {@code ConfigResolveOptions}:
///
/// <pre>
///     auto options = ConfigResolveOptions::defaults()
///         ->setUseSystemEnvironment(false)
/// </pre>
/// <p>
/// In addition to {@link ConfigResolveOptions#defaults}, there's a prebuilt
/// {@link ConfigResolveOptions#noSystem} which avoids looking at any system
/// environment variables or other external system information. (Right now,
/// environment variables are the only example.)
///
class ConfigResolveOptions : public ConfigBase {
public:
    CONFIG_CLASS(ConfigResolveOptions);

    ConfigResolveOptions(bool useSystemEnvironment);

    /// Returns the default resolve options.
    ///
    /// @return the default resolve options
    static ConfigResolveOptionsPtr defaults();

    /// Returns resolve options that disable any reference to "system" data
    /// (currently, this means environment variables).
    ///
    /// @return the resolve options with env variables disabled
    static ConfigResolveOptionsPtr noSystem();

    /// Returns options with use of environment variables set to the given value.
    ///
    /// @param value
    ///            true to resolve substitutions falling back to environment
    ///            variables.
    /// @return options with requested setting for use of environment variables
    ConfigResolveOptionsPtr setUseSystemEnvironment(bool value);

    /// Returns whether the options enable use of system environment variables.
    /// This method is mostly used by the config lib internally, not by
    /// applications.
    ///
    /// @return true if environment variables should be used
    bool getUseSystemEnvironment();

private:
    bool useSystemEnvironment;
};

}

#endif // CONFIG_RESOLVE_OPTIONS_H_
