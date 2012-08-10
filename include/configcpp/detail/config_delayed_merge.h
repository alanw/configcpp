/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of the Apache License (Version 2.0).
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DELAYED_MERGE_H_
#define CONFIG_DELAYED_MERGE_H_

#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/unmergeable.h"
#include "configcpp/detail/replaceable_merge_stack.h"
#include "configcpp/detail/resolve_replacer.h"

namespace config {

///
/// The issue here is that we want to first merge our stack of config files, and
/// then we want to evaluate substitutions. But if two substitutions both expand
/// to an object, we might need to merge those two objects. Thus, we can't ever
/// "override" a substitution when we do a merge; instead we have to save the
/// stack of values that should be merged, and resolve the merge when we evaluate
/// substitutions.
///
class ConfigDelayedMerge : public AbstractConfigValue, public virtual Unmergeable, public virtual ReplaceableMergeStack {
public:
    CONFIG_CLASS(ConfigDelayedMerge);

    ConfigDelayedMerge(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& stack);

    virtual ConfigValueType valueType() override;
    virtual ConfigVariant unwrapped() override;
    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context) override;

    /// Static method also used by ConfigDelayedMergeObject
    static AbstractConfigValuePtr resolveSubstitutions(const ReplaceableMergeStackPtr& replaceable,
                                                       const VectorAbstractConfigValue& stack,
                                                       const ResolveContextPtr& context);

    virtual ResolveReplacerPtr makeReplacer(uint32_t skipping) override;

    /// Static method also used by ConfigDelayedMergeObject
    static AbstractConfigValuePtr makeReplacement(const ResolveContextPtr& context,
                                                  const VectorAbstractConfigValue& stack,
                                                  uint32_t skipping);

    virtual ResolveStatus resolveStatus() override;
    virtual AbstractConfigValuePtr relativized(const PathPtr& prefix) override;

    /// Static utility shared with ConfigDelayedMergeObject
    static bool stackIgnoresFallbacks(const VectorAbstractConfigValue& stack);

    virtual bool ignoresFallbacks() override;

protected:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) override;

    using AbstractConfigValue::mergedWithTheUnmergeable;
    virtual AbstractConfigValuePtr mergedWithTheUnmergeable(const UnmergeablePtr& fallback) override;

    using AbstractConfigValue::mergedWithObject;
    virtual AbstractConfigValuePtr mergedWithObject(const AbstractConfigObjectPtr& fallback) override;

    using AbstractConfigValue::mergedWithNonObject;
    virtual AbstractConfigValuePtr mergedWithNonObject(const AbstractConfigValuePtr& fallback) override;

public:
    virtual VectorAbstractConfigValue unmergedValues() override;

protected:
    virtual bool canEqual(const ConfigVariant& other) override;

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;

protected:
    virtual void render(std::string& s,
                        uint32_t indent,
                        const boost::optional<std::string>& atKey,
                        const ConfigRenderOptionsPtr& options) override;

public:
    /// Static method also used by ConfigDelayedMergeObject.
    static void render(const VectorAbstractConfigValue& stack,
                       std::string& s,
                       uint32_t indent,
                       const boost::optional<std::string>& atKey,
                       const ConfigRenderOptionsPtr& options);

private:
    /// Earlier items in the stack win
    VectorAbstractConfigValue stack;
};

class ConfigDelayedMergeResolveReplacer : public virtual ResolveReplacer, public ConfigBase {
public:
    CONFIG_CLASS(ConfigDelayedMergeResolveReplacer);

    ConfigDelayedMergeResolveReplacer(const VectorAbstractConfigValue& stack, uint32_t skipping);

protected:
    virtual AbstractConfigValuePtr makeReplacement(const ResolveContextPtr& context) override;

private:
    const VectorAbstractConfigValue& stack;
    uint32_t skipping;
};

}

#endif // CONFIG_DELAYED_MERGE_H_

