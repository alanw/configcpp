/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_OBJECT_H_
#define CONFIG_OBJECT_H_

#include "configcpp/config_value.h"

namespace config {

///
/// Subtype of {@link ConfigValue} representing an object (dictionary, map)
/// value, as in JSON's <code>{ "a" : 42 }</code> syntax.
///
/// <p>
/// {@code ConfigObject} implements {@code MapConfigValue} so you can use it
/// like a regular std map. Or call {@link #unwrapped()} to unwrap the map to a
/// map with plain values rather than {@code ConfigValue}.
///
/// <p>
/// Like all {@link ConfigValue} subtypes, {@code ConfigObject} is immutable.
/// This makes it threadsafe and you never have to create "defensive copies".
///
/// <p>
/// The {@link ConfigValue#valueType} method on an object returns
/// {@link ConfigValueType#OBJECT}.
///
/// <p>
/// In most cases you want to use the {@link Config} interface rather than this
/// one. Call {@link #toConfig()} to convert a {@code ConfigObject} to a
/// {@code Config}.
///
/// <p>
/// The API for a {@code ConfigObject} is in terms of keys, while the API for a
/// {@link Config} is in terms of path expressions. Conceptually,
/// {@code ConfigObject} is a tree of maps from keys to values, while a
/// {@code Config} is a one-level map from paths to values.
///
/// <p>
/// Use {@link ConfigUtil#joinPath} and {@link ConfigUtil#splitPath} to convert
/// between path expressions and individual path elements (keys).
///
/// <p>
/// A {@code ConfigObject} may contain null values, which will have
/// {@link ConfigValue#valueType()} equal to {@link ConfigValueType#NULL}. If
/// {@code get()} returns empty then the key was not present in the parsed
/// file (or wherever this value tree came from). If {@code get()} returns a
/// {@link ConfigValue} with type {@code ConfigValueType#NULL} then the key was
/// set to null explicitly in the config file.
///
/// <p>
/// <em>Do not implement {@code ConfigObject}</em>; it should only be implemented
/// by the config library. Arbitrary implementations will not work because the
/// library internals assume a specific concrete implementation. Also, this
/// interface is likely to grow new methods over time, so third-party
/// implementations will break.
///
class ConfigObject : public virtual ConfigValue, public virtual MapConfigValue {
public:
    /// Converts this object to a {@link Config} instance, enabling you to use
    /// path expressions to find values in the object. This is a constant-time
    /// operation (it is not proportional to the size of the object).
    ///
    /// @return a {@link Config} with this object as its root
    virtual ConfigPtr toConfig() = 0;

    /// Recursively unwraps the object, returning a map from std::string to
    /// whatever plain values are unwrapped from boost:variant values.
    ///
    /// @return a {@link MapVariant} containing plain objects
    virtual ConfigVariant unwrapped() = 0;

    /// Alternative to unwrapping the value to a ConfigVariant.
    template <typename T> T unwrapped() {
        return variant_get<MapVariant>(unwrapped());
    }

    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& other) = 0;

    /// Gets a {@link ConfigValue} at the given key, or returns null if there is
    /// no value. The returned {@link ConfigValue} may have
    /// {@link ConfigValueType#NONE} or any other type, and the passed-in key
    /// must be a key in this object, rather than a path expression.
    ///
    /// @param key
    ///            key to look up
    ///
    /// @return the value at the key or null if none
    virtual ConfigValuePtr get(const std::string& key) = 0;

    /// Clone the object with only the given key (and its children) retained; all
    /// sibling keys are removed.
    ///
    /// @param key
    ///            key to keep
    /// @return a copy of the object minus all keys except the one specified
    virtual ConfigObjectPtr withOnlyKey(const std::string& key) = 0;

    /// Clone the object with the given key removed.
    ///
    /// @param key
    ///            key to remove
    /// @return a copy of the object minus the specified key
    virtual ConfigObjectPtr withoutKey(const std::string& key) = 0;

    /// Return set of key-value pairs.
    virtual SetConfigValue entrySet();

    virtual MapConfigValue::const_iterator begin() const;
    virtual MapConfigValue::const_iterator end() const;
    virtual MapConfigValue::mapped_type operator[](const MapConfigValue::key_type& key) const;
    virtual bool empty() const;
    virtual MapConfigValue::size_type size() const;
    virtual MapConfigValue::size_type count(const MapConfigValue::key_type& key) const;
    virtual MapConfigValue::const_iterator find(const MapConfigValue::key_type& key) const;

    virtual void clear();
    virtual MapConfigValue::size_type erase(const MapConfigValue::key_type& key);
    virtual void erase(const MapConfigValue::const_iterator& pos);
    virtual void insert(const MapConfigValue::value_type& val);
};

}

#endif // CONFIG_OBJECT_H_
