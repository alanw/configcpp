/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_LIST_H_
#define CONFIG_LIST_H_

#include "configcpp/config_value.h"

namespace config {

///
/// Subtype of {@link ConfigValue} representing a list value, as in JSON's
/// {@code [1,2,3]} syntax.
///
/// <p>
/// {@code ConfigList} implements {@code VectorConfigValue} so you can
/// use it like a regular vector. Or call {@link #unwrapped()} to unwrap the
/// list elements into plain values.
///
/// <p>
/// Like all {@link ConfigValue} subtypes, {@code ConfigList} is immutable. This
/// makes it threadsafe and you never have to create "defensive copies.".
///
/// <p>
/// The {@link ConfigValue#valueType} method on a list returns
/// {@link ConfigValueType#LIST}.
///
/// <p>
/// <em>Do not implement {@code ConfigList}</em>; it should only be implemented
/// by the config library. Arbitrary implementations will not work because the
/// library internals assume a specific concrete implementation. Also, this
/// interface is likely to grow new methods over time, so third-party
/// implementations will break.
///
class ConfigList : public virtual VectorConfigValue, public virtual ConfigValue {
public:
    /// Recursively unwraps the list, returning a list of plain values such
    /// as int32_t or std::string or whatever is in the list.
    virtual ConfigVariant unwrapped() = 0;

    /// Alternative to unwrapping the value to a ConfigVariant.
    template <typename T> T unwrapped() {
        return variant_get<T>(unwrapped());
    }

    virtual VectorConfigValue::const_iterator begin() const;
    virtual VectorConfigValue::const_iterator end() const;
    virtual VectorConfigValue::const_reference at(VectorConfigValue::size_type n) const;
    virtual VectorConfigValue::const_reference front() const;
    virtual VectorConfigValue::const_reference back() const;
    virtual VectorConfigValue::const_reference operator[](VectorConfigValue::size_type n) const;
    virtual bool empty() const;
    virtual VectorConfigValue::size_type size() const;
    virtual void clear();
    virtual void pop_back();
    virtual void resize(VectorConfigValue::size_type n,
                        const VectorConfigValue::value_type& val = nullptr);
    virtual VectorConfigValue::const_iterator erase(VectorConfigValue::const_iterator pos);
    virtual VectorConfigValue::const_iterator insert(VectorConfigValue::const_iterator pos,
                                                     const VectorConfigValue::value_type& val);
};

}

#endif // CONFIG_LIST_H_
