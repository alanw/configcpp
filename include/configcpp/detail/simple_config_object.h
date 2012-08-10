/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_CONFIG_OBJECT_H_
#define SIMPLE_CONFIG_OBJECT_H_

#include "configcpp/detail/abstract_config_object.h"

namespace config {

class SimpleConfigObject : public AbstractConfigObject {
public:
    CONFIG_CLASS(SimpleConfigObject);

    SimpleConfigObject(const ConfigOriginPtr& origin,
                       const MapAbstractConfigValue& value,
                       ResolveStatus status,
                       bool ignoresFallbacks);
    SimpleConfigObject(const ConfigOriginPtr& origin,
                       const MapAbstractConfigValue& value);

    virtual ConfigObjectPtr withOnlyKey(const std::string& key) override;
    virtual ConfigObjectPtr withoutKey(const std::string& key) override;

    /// Gets the object with only the path if the path
    /// exists, otherwise null if it doesn't. This ensures
    /// that if we have { a : { b : 42 } } and do
    /// withOnlyPath("a.b.c") that we don't keep an empty
    /// "a" object.
    virtual AbstractConfigObjectPtr withOnlyPathOrNull(const PathPtr& path) override;

    virtual AbstractConfigObjectPtr withOnlyPath(const PathPtr& path) override;
    virtual AbstractConfigObjectPtr withoutPath(const PathPtr& path) override;

    virtual ConfigObjectPtr withValue(const std::string& key, const ConfigValuePtr& value) override;
    virtual ConfigObjectPtr withValue(const PathPtr& path, const ConfigValuePtr& value) override;

    virtual AbstractConfigValuePtr attemptPeekWithPartialResolve(const std::string& key) override;

private:
    virtual AbstractConfigObjectPtr newCopy(ResolveStatus status,
                                            const ConfigOriginPtr& origin,
                                            bool newIgnoresFallbacks);

protected:
    virtual AbstractConfigObjectPtr newCopy(ResolveStatus status,
                                            const ConfigOriginPtr& origin) override;

protected:
    virtual AbstractConfigValuePtr withFallbacksIgnored() override;

public:
    virtual ResolveStatus resolveStatus() override;

    virtual bool ignoresFallbacks() override;

public:
    virtual ConfigVariant unwrapped() override;

    /// Alternative to unwrapping the value to a ConfigVariant.
    template <typename T> T unwrapped() {
        return variant_get<T>(unwrapped());
    }

protected:
    using AbstractConfigObject::mergedWithObject;
    virtual AbstractConfigValuePtr mergedWithObject(const AbstractConfigObjectPtr& fallback) override;

private:
    SimpleConfigObjectPtr modify(const NoExceptionsModifierPtr& modifier);
    SimpleConfigObjectPtr modifyMayThrow(const ModifierPtr& modifier);

public:
    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override;
    virtual AbstractConfigValuePtr relativized(const PathPtr& prefix) override;

protected:
    virtual void render(std::string& s,
                        uint32_t indent,
                        const ConfigRenderOptionsPtr& options) override;

public:
    virtual ConfigValuePtr get(const std::string& key) override;

private:
    static bool mapEquals(const MapConfigValue& a, const MapConfigValue& b);
    static uint32_t mapHash(const MapConfigValue& m);

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

public:
    virtual MapConfigValue::const_iterator begin() const override;
    virtual MapConfigValue::const_iterator end() const override;
    virtual MapConfigValue::mapped_type operator[](const MapConfigValue::key_type& key) const override;
    virtual bool empty() const override;
    virtual MapConfigValue::size_type size() const override;
    virtual MapConfigValue::size_type count(const MapConfigValue::key_type& key) const override;
    virtual MapConfigValue::const_iterator find(const MapConfigValue::key_type& key) const override;

public:
    static SimpleConfigObjectPtr makeEmpty();
    static SimpleConfigObjectPtr makeEmpty(const ConfigOriginPtr& origin);
    static SimpleConfigObjectPtr makeEmptyMissing(const ConfigOriginPtr& baseOrigin);

private:
    // this map should never be modified - assume immutable
    MapConfigValue value;
    bool resolved;
    bool ignoresFallbacks_;
};

class SimpleConfigObjectModifier : public virtual Modifier, public ConfigBase {
public:
    CONFIG_CLASS(SimpleConfigObjectModifier);

    SimpleConfigObjectModifier(const ResolveContextPtr& context);

    virtual AbstractConfigValuePtr modifyChildMayThrow(const std::string& keyOrNull,
                                                       const AbstractConfigValuePtr& v) override;

private:
    ResolveContextPtr context;
};

class SimpleConfigObjectNoExceptionsModifier : public NoExceptionsModifier {
public:
    CONFIG_CLASS(SimpleConfigObjectNoExceptionsModifier);

    SimpleConfigObjectNoExceptionsModifier(const PathPtr& prefix);

    virtual AbstractConfigValuePtr modifyChild(const std::string& keyOrNull,
                                               const AbstractConfigValuePtr& v) override;

private:
    PathPtr prefix;
};

}

#endif // SIMPLE_CONFIG_OBJECT_H_
