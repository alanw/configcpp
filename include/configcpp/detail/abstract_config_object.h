/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef ABSTRACT_CONFIG_OBJECT_H_
#define ABSTRACT_CONFIG_OBJECT_H_

#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/config_object.h"

namespace config {

class AbstractConfigObject : public AbstractConfigValue, public virtual ConfigObject {
public:
    CONFIG_CLASS(AbstractConfigObject);

    AbstractConfigObject(const ConfigOriginPtr& origin);

    virtual void initialize() override;

    virtual ConfigPtr toConfig() override;

    virtual ConfigValuePtr toFallbackValue() override;

    virtual ConfigObjectPtr withOnlyKey(const std::string& key) = 0;
    virtual ConfigObjectPtr withoutKey(const std::string& key) = 0;

    virtual ConfigObjectPtr withValue(const std::string& key, const ConfigValuePtr& value) = 0;

    virtual AbstractConfigObjectPtr withOnlyPathOrNull(const PathPtr& path) = 0;
    virtual AbstractConfigObjectPtr withOnlyPath(const PathPtr& path) = 0;
    virtual AbstractConfigObjectPtr withoutPath(const PathPtr& path) = 0;

    virtual ConfigObjectPtr withValue(const PathPtr& path, const ConfigValuePtr& value) = 0;

    /// This looks up the key with no transformation or type conversion of any
    /// kind, and returns null if the key is not present. The object must be
    /// resolved; use attemptPeekWithPartialResolve() if it is not.
    ///
    /// @param key
    /// @return the unmodified raw value or null
    virtual AbstractConfigValuePtr peekAssumingResolved(const std::string& key,
                                                        const PathPtr& originalPath);

    /// Look up the key on an only-partially-resolved object, with no
    /// transformation or type conversion of any kind; if 'this' is not resolved
    /// then try to look up the key anyway if possible.
    ///
    /// @param key
    ///            key to look up
    /// @return the value of the key, or null if known not to exist
    /// @throws ConfigExceptionNotResolved
    ///             if can't figure out key's value or can't know whether it
    ///             exists
    virtual AbstractConfigValuePtr attemptPeekWithPartialResolve(const std::string& key) = 0;

    /// Looks up the path with no transformation, type conversion, or exceptions
    /// (just returns null if path not found). Does however resolve the path, if
    /// resolver != null.
    ///
    /// @throws NotPossibleToResolve
    ///             if context is not null and resolution fails
    virtual AbstractConfigValuePtr peekPath(const PathPtr& path,
                                            const ResolveContextPtr& context);

public:
    /// Looks up the path. Doesn't do any resolution, will throw if any is
    /// needed.
    virtual AbstractConfigValuePtr peekPath(const PathPtr& path);

private:
    static AbstractConfigValuePtr peekPath(const AbstractConfigObjectPtr& self,
                                           const PathPtr& path,
                                           const ResolveContextPtr& context);

public:
    virtual ConfigValueType valueType() override;

protected:
    virtual AbstractConfigObjectPtr newCopy(ResolveStatus status,
                                            const ConfigOriginPtr& origin) = 0;

    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

    virtual AbstractConfigValuePtr constructDelayedMerge(const ConfigOriginPtr& origin,
                                                         const VectorAbstractConfigValue& stack) override;

    virtual AbstractConfigValuePtr mergedWithObject(const AbstractConfigObjectPtr& fallback) = 0;

public:
    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& other) override;

public:
    static ConfigOriginPtr mergeOrigins(const VectorAbstractConfigValue& stack);
    static ConfigOriginPtr mergeOrigins(const VectorAbstractConfigObject& stack = VectorAbstractConfigObject());

    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override = 0;
    virtual AbstractConfigValuePtr relativized(const PathPtr& prefix) override = 0;

protected:
    virtual void render(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) = 0;

private:
    static ConfigExceptionUnsupportedOperation weAreImmutable(const std::string& method);

public:
    MapConfigValue::const_iterator begin() const;
    MapConfigValue::const_iterator end() const;
    MapConfigValue::mapped_type operator[](const MapConfigValue::key_type& key) const;
    bool empty() const;
    MapConfigValue::size_type size() const;
    MapConfigValue::size_type count(const MapConfigValue::key_type& key) const;
    MapConfigValue::const_iterator find(const MapConfigValue::key_type& key) const;

    void clear();
    MapConfigValue::size_type erase(const MapConfigValue::key_type&);
    void erase(const MapConfigValue::const_iterator&);
    void insert(const MapConfigValue::value_type&);

private:
    SimpleConfigPtr config;
};

}

#endif // ABSTRACT_CONFIG_OBJECT_H_
