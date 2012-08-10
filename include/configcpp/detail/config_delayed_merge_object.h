/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DELAYED_MERGE_OBJECT_H_
#define CONFIG_DELAYED_MERGE_OBJECT_H_

#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/unmergeable.h"
#include "configcpp/detail/replaceable_merge_stack.h"
#include "configcpp/detail/resolve_replacer.h"

namespace config {

///
/// This is just like ConfigDelayedMergeObject except we know statically
/// that it will turn out to be an object.
///
class ConfigDelayedMergeObject : public AbstractConfigObject, public virtual Unmergeable, public virtual ReplaceableMergeStack {
public:
    CONFIG_CLASS(ConfigDelayedMergeObject);

    ConfigDelayedMergeObject(const ConfigOriginPtr& origin,
                             const VectorAbstractConfigValue& stack);

    virtual AbstractConfigObjectPtr newCopy(ResolveStatus status,
                                            const ConfigOriginPtr& origin) override;
    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override;
    virtual ResolveReplacerPtr makeReplacer(uint32_t skipping) override;
    virtual ResolveStatus resolveStatus() override;
    virtual AbstractConfigValuePtr relativized(const PathPtr& prefix) override;

    virtual bool ignoresFallbacks() override;

protected:
    using AbstractConfigObject::mergedWithTheUnmergeable;
    virtual AbstractConfigValuePtr mergedWithTheUnmergeable(const UnmergeablePtr& fallback) override;

    using AbstractConfigObject::mergedWithObject;
    virtual AbstractConfigValuePtr mergedWithObject(const AbstractConfigObjectPtr& fallback) override;

    using AbstractConfigObject::mergedWithNonObject;
    virtual AbstractConfigValuePtr mergedWithNonObject(const AbstractConfigValuePtr& fallback) override;

public:
    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& other) override;

    virtual ConfigObjectPtr withOnlyKey(const std::string& key) override;
    virtual ConfigObjectPtr withoutKey(const std::string& key) override;
    virtual AbstractConfigObjectPtr withOnlyPathOrNull(const PathPtr& path) override;
    virtual AbstractConfigObjectPtr withOnlyPath(const PathPtr& path) override;
    virtual AbstractConfigObjectPtr withoutPath(const PathPtr& path) override;

    virtual VectorAbstractConfigValue unmergedValues() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

    virtual void render(std::string& s,
                        uint32_t indent,
                        const boost::optional<std::string>& atKey,
                        const ConfigRenderOptionsPtr& options) override;
    virtual void render(std::string& s,
                        uint32_t indent,
                        const ConfigRenderOptionsPtr& options) override;

private:
    static ConfigExceptionNotResolved notResolved();

public:
    virtual ConfigVariant unwrapped() override;
    virtual ConfigValuePtr get(const std::string& key) override;

    virtual MapConfigValue::const_iterator begin() const override;
    virtual MapConfigValue::const_iterator end() const override;
    virtual MapConfigValue::mapped_type operator[](const MapConfigValue::key_type& key) const override;
    virtual bool empty() const override;
    virtual MapConfigValue::size_type size() const override;
    virtual MapConfigValue::size_type count(const MapConfigValue::key_type& key) const override;
    virtual MapConfigValue::const_iterator find(const MapConfigValue::key_type& key) const override;

    virtual AbstractConfigValuePtr attemptPeekWithPartialResolve(const std::string& key) override;

private:
    VectorAbstractConfigValue stack;
};

class ConfigDelayedMergeObjectResolveReplacer : public virtual ResolveReplacer, public ConfigBase {
public:
    CONFIG_CLASS(ConfigDelayedMergeObjectResolveReplacer);

    ConfigDelayedMergeObjectResolveReplacer(const VectorAbstractConfigValue& stack,
                                            uint32_t skipping);

protected:
    virtual AbstractConfigValuePtr makeReplacement(const ResolveContextPtr& context) override;

private:
    const VectorAbstractConfigValue& stack;
    uint32_t skipping;
};

}

#endif // CONFIG_DELAYED_MERGE_OBJECT_H_
