/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_INCLUDER_FILE_H_
#define CONFIG_INCLUDER_FILE_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Implement this <em>in addition to</em> {@link ConfigIncluder} if you want to
/// support inclusion of files with the {@code include file("filename")} syntax.
/// If you do not implement this but do implement {@link ConfigIncluder},
/// attempts to load files will use the default includer.
///
class ConfigIncluderFile {
public:
    /// Parses another item to be included. The returned object typically would
    /// not have substitutions resolved. You can throw a ConfigException here to
    /// abort parsing, or return an empty object, but may not return null.
    ///
    /// @param context
    ///            some info about the include context
    /// @param what
    ///            the include statement's argument
    /// @return a non-null ConfigObject
    virtual ConfigObjectPtr includeFile(const ConfigIncludeContextPtr& context,
                                        const std::string& file) = 0;
};

}

#endif // CONFIG_INCLUDER_FILE_H_
