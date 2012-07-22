/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_ORIGIN_H_
#define CONFIG_ORIGIN_H_

#include "configcpp/config_types.h"

namespace config {

///
/// Represents the origin (such as filename and line number) of a
/// {@link ConfigValue} for use in error messages. Obtain the origin of a value
/// with {@link ConfigValue#origin}. Exceptions may have an origin, see
/// {@link ConfigException#origin}, but be careful because
/// <code>ConfigException.origin()</code> may return null.
///
/// <p>
/// It's best to use this interface only for debugging; its accuracy is
/// "best effort" rather than guaranteed, and a potentially-noticeable amount of
/// memory could probably be saved if origins were not kept around, so in the
/// future there might be some option to discard origins.
///
/// <p>
/// <em>Do not implement this interface</em>; it should only be implemented by
/// the config library. Arbitrary implementations will not work because the
/// library internals assume a specific concrete implementation. Also, this
/// interface is likely to grow new methods over time, so third-party
/// implementations will break.
///
class ConfigOrigin {
public:
    /// Returns a string describing the origin of a value or exception. This will
    /// never return null.
    ///
    /// @return string describing the origin
    virtual std::string description() = 0;

    /// Returns a filename describing the origin. This will return null if the
    /// origin was not a file.
    ///
    /// @return filename of the origin or null
    virtual std::string filename() = 0;

    /// Returns a line number where the value or exception originated. This will
    /// return -1 if there's no meaningful line number.
    ///
    /// @return line number or -1 if none is available
    virtual int32_t lineNumber() = 0;

    /// Returns any comments that appeared to "go with" this place in the file.
    /// Often an empty list, but never null. The details of this are subject to
    /// change, but at the moment comments that are immediately before an array
    /// element or object field, with no blank line after the comment, "go with"
    /// that element or field.
    ///
    /// @return any comments that seemed to "go with" this origin, empty list if
    ///         none
    virtual VectorString comments() = 0;
};

}

#endif // CONFIG_ORIGIN_H_
