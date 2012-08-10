/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_PARSE_OPTIONS_H_
#define CONFIG_PARSE_OPTIONS_H_

#include "configcpp/detail/config_base.h"

namespace config {

///
/// A set of options related to parsing.
///
/// <p>
/// This object is immutable, so the "setters" return a new object.
///
/// <p>
/// Here is an example of creating a custom {@code ConfigParseOptions}:
///
/// <pre>
///     auto options = ConfigParseOptions::defaults()
///         ->setSyntax(ConfigSyntax::JSON)
///         ->setAllowMissing(false)
/// </pre>
///
class ConfigParseOptions : public ConfigBase {
public:
    CONFIG_CLASS(ConfigParseOptions);

    ConfigParseOptions(ConfigSyntax syntax,
                       const std::string& originDescription,
                       bool allowMissing,
                       const ConfigIncluderPtr& includer);

    static ConfigParseOptionsPtr defaults();

    /// Set the file format. If set to null, try to guess from any available
    /// filename extension; if guessing fails, assume {@link ConfigSyntax#CONF}.
    ///
    /// @param syntax
    ///            a syntax or {@code ConfigSyntax::NONE} for best guess
    /// @return options with the syntax set
    ConfigParseOptionsPtr setSyntax(ConfigSyntax syntax);

    ConfigSyntax getSyntax();

    /// Set a description for the thing being parsed. In most cases this will be
    /// set up for you to something like the filename, but if you provide just an
    /// input stream you might want to improve on it. Set to null to allow the
    /// library to come up with something automatically. This description is the
    /// basis for the {@link ConfigOrigin} of the parsed values.
    ///
    /// @param originDescription
    /// @return options with the origin description set
    ConfigParseOptionsPtr setOriginDescription(const std::string& originDescription);

    std::string getOriginDescription();

private:
    ConfigParseOptionsPtr withFallbackOriginDescription(const std::string& originDescription);

public:
    /// Set to false to throw an exception if the item being parsed (for example
    /// a file) is missing. Set to true to just return an empty document in that
    /// case.
    ///
    /// @param allowMissing
    /// @return options with the "allow missing" flag set
    ConfigParseOptionsPtr setAllowMissing(bool allowMissing);

    bool getAllowMissing();

    /// Set a ConfigIncluder which customizes how includes are handled.
    ///
    /// @param includer
    /// @return new version of the parse options with different includer
    ConfigParseOptionsPtr setIncluder(const ConfigIncluderPtr& includer);

    ConfigParseOptionsPtr prependIncluder(const ConfigIncluderPtr& includer);
    ConfigParseOptionsPtr appendIncluder(const ConfigIncluderPtr& includer);

    ConfigIncluderPtr getIncluder();

private:
    ConfigSyntax syntax;
    std::string originDescription;
    bool allowMissing;
    ConfigIncluderPtr includer;
};

}

#endif // CONFIG_PARSE_OPTIONS_H_
