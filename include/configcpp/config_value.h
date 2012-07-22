/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_VALUE_H_
#define CONFIG_VALUE_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/variant_utils.h"
#include "configcpp/config_mergeable.h"

namespace config {

///
/// An immutable value, following the <a href="http://json.org">JSON</a> type
/// schema.
///
/// <p>
/// Because this object is immutable, it is safe to use from multiple threads and
/// there's no need for "defensive copies."
///
/// <p>
/// <em>Do not implement {@code ConfigValue}</em>; it should only be implemented
/// by the config library. Arbitrary implementations will not work because the
/// library internals assume a specific concrete implementation. Also, this
/// interface is likely to grow new methods over time, so third-party
/// implementations will break.
///
/// This class also holds some static factory methods for building {@link
/// ConfigValue} instances. See also {@link Config} which has methods for parsing
/// files and certain in-memory data structures.
///
class ConfigValue : public virtual ConfigMergeable {
public:
    /// The origin of the value (file, line number, etc.), for debugging and
    /// error messages.
    ///
    /// @return where the value came from
    virtual ConfigOriginPtr origin() = 0;

    /// The {@link ConfigValueType} of the value; matches the JSON type schema.
    ///
    /// @return value's type
    virtual ConfigValueType valueType() = 0;

    /// Returns the value, that is, a {@code std::string}, {@code int32_t},
    /// {@code int64_t}, {@code bool}, {@code MapVariant}, {@code VectorVariant},
    /// or {@code null}, matching the {@link #valueType()} of this
    /// {@code ConfigValue}. If the value is a {@link ConfigObject} or
    /// {@link ConfigList}, it is recursively unwrapped.
    virtual ConfigVariant unwrapped() = 0;

    /// Alternative to unwrapping the value to a ConfigVariant.
    template <typename T> T unwrapped() {
        return variant_get<T>(unwrapped());
    }

    /// Renders the config value as a HOCON string. This method is primarily
    /// intended for debugging, so it tries to add helpful comments and
    /// whitespace.
    ///
    /// <p>
    /// If the config value has not been resolved (see {@link Config#resolve}),
    /// it's possible that it can't be rendered as valid HOCON. In that case the
    /// rendering should still be useful for debugging but you might not be able
    /// to parse it.
    ///
    /// <p>
    /// This method is equivalent to
    /// {@code render(ConfigRenderOptions.defaults())}.
    ///
    /// @return the rendered value
    virtual std::string render() = 0;

    /// Renders the config value to a string, using the provided options.
    ///
    /// <p>
    /// If the config value has not been resolved (see {@link Config#resolve}),
    /// it's possible that it can't be rendered as valid HOCON. In that case the
    /// rendering should still be useful for debugging but you might not be able
    /// to parse it.
    ///
    /// <p>
    /// If the config value has been resolved and the options disable all
    /// HOCON-specific features (such as comments), the rendering will be valid
    /// JSON. If you enable HOCON-only features such as comments, the rendering
    /// will not be valid JSON.
    ///
    /// @param options
    ///            the rendering options
    /// @return the rendered value
    virtual std::string render(const ConfigRenderOptionsPtr& options) = 0;

    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& other) = 0;

    /// Creates a ConfigValue from a ConfigVariant value, which may be a
    /// bool, int32_t, int64_t, std::string, map, vector, or null/blank. A map
    /// must be a map from std::string to more ConfigVariant values that can be
    /// supplied to fromAnyRef(). A map will become a ConfigObject and an vector
    /// will become a ConfigList.
    ///
    /// <p>
    /// If a map passed to fromAnyRef(), the map's keys are plain keys, not path
    /// expressions. So if your map has a key "foo.bar" then you will get one
    /// object with a key called "foo.bar", rather than an object with a key
    /// "foo" containing another object with a key "bar".
    ///
    /// <p>
    /// The originDescription will be used to set the origin() field on the
    /// ConfigValue. It should normally be the name of the file the values came
    /// from, or something short describing the value such as "default settings".
    /// The originDescription is prefixed to error messages so users can tell
    /// where problematic values are coming from.
    ///
    /// <p>
    /// Supplying the result of ConfigValue.unwrapped() to this function is
    /// guaranteed to work and should give you back a ConfigValue that matches
    /// the one you unwrapped. The re-wrapped ConfigValue will lose some
    /// information that was present in the original such as its origin, but it
    /// will have matching values.
    ///
    /// <p>
    /// This function throws if you supply a value that cannot be converted to a
    /// ConfigValue, but supplying such a value is a bug in your program, so you
    /// should never handle the exception. Just fix your program (or report a bug
    /// against this library).
    ///
    /// @param object
    ///            object to convert to ConfigValue
    /// @param originDescription
    ///            name of origin file or brief description of what the value is
    /// @return a new value
    static ConfigValuePtr fromAnyRef(const ConfigVariant& object,
                                     const std::string& originDescription = "");

    /// See the fromAnyRef() documentation for details. This is a typesafe
    /// wrapper that only works on {@link MapVariant} and returns
    /// {@link ConfigObject} rather than {@link ConfigValue}.
    ///
    /// <p>
    /// If your map has a key "foo.bar" then you will get one object with a key
    /// called "foo.bar", rather than an object with a key "foo" containing
    /// another object with a key "bar". The keys in the map are keys; not path
    /// expressions. That is, the map corresponds exactly to a single
    /// {@code ConfigObject}. The keys will not be parsed or modified, and the
    /// values are wrapped in ConfigValue. To get nested {@code ConfigObject},
    /// some of the values in the map would have to be more maps.
    ///
    /// <p>
    /// See also {@link Config#parseMap(MapVariant,std::string)} which interprets
    /// the keys in the map as path expressions.
    ///
    /// @param values
    /// @param originDescription
    /// @return a new {@link ConfigObject} value
    static ConfigObjectPtr fromMap(const MapVariant& values,
                                   const std::string& originDescription = "");

    /// See the fromAnyRef() documentation for details. This is a typesafe
    /// wrapper that only works on {@link VectorVariant} and returns
    /// {@link ConfigList} rather than {@link ConfigValue}.
    ///
    /// @param values
    /// @param originDescription
    /// @return a new {@link ConfigList} value
    static ConfigListPtr fromVector(const VectorVariant& values,
                                    const std::string& originDescription = "");
};

}

#endif // CONFIG_VALUE_H_
