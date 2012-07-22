/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#ifndef ABSTRACT_CONFIG_VALUE_H_
#define ABSTRACT_CONFIG_VALUE_H_

#include "configcpp/detail/config_base.h"
#include "configcpp/detail/resolve_status.h"
#include "configcpp/detail/mergeable_value.h"
#include "configcpp/config_exception.h"
#include "configcpp/config_value.h"

namespace config {

///
/// This exception means that a value is inherently not resolveable, at the
/// moment the only known cause is a cycle of substitutions. This is a
/// checked exception since it's internal to the library and we want to be
/// sure we handle it before passing it out to public API. This is only
/// supposed to be thrown by the target of a cyclic reference and it's
/// supposed to be caught by the ConfigReference looking up that reference,
/// so it should be impossible for an outermost resolve() to throw this.
///
/// Contrast with ConfigExceptionNotResolved which just means nobody called
/// resolve().
///
class NotPossibleToResolve : public ConfigException {
public:
    EXCEPTION_CLASS(NotPossibleToResolve)

    NotPossibleToResolve(const ResolveContextPtr& context);

    std::string traceString();

private:
    std::string traceString_;
};

class Modifier {
public:
    /// keyOrNull is null for non-objects
    virtual AbstractConfigValuePtr modifyChildMayThrow(const std::string& keyOrNull,
                                                       const AbstractConfigValuePtr& v) = 0;
};

class NoExceptionsModifier : public virtual Modifier, public ConfigBase {
public:
    CONFIG_CLASS(NoExceptionsModifier);

    virtual AbstractConfigValuePtr modifyChildMayThrow(const std::string& keyOrNull,
                                                       const AbstractConfigValuePtr& v) override;
    virtual AbstractConfigValuePtr modifyChild(const std::string& keyOrNull,
                                               const AbstractConfigValuePtr& v) = 0;
};

///
/// Trying very hard to avoid a parent reference in config values; when you have
/// a tree like this, the availability of parent() tends to result in a lot of
/// improperly-factored and non-modular code. Please don't add parent().
///
class AbstractConfigValue : public virtual ConfigValue, public virtual MergeableValue, public ConfigBase {
public:
    CONFIG_CLASS(AbstractConfigValue);

    AbstractConfigValue(const ConfigOriginPtr& origin);

    virtual ConfigOriginPtr origin() override;

    /// Called only by ResolveContext::resolve().
    ///
    /// @param context
    ///            state of the current resolve
    /// @return a new value if there were changes, or this if no changes
    virtual AbstractConfigValuePtr resolveSubstitutions(const ResolveContextPtr& context);

    virtual ResolveStatus resolveStatus();

    /// This is used when including one file in another; the included file is
    /// relativized to the path it's included into in the parent file. The point
    /// is that if you include a file at foo.bar in the parent, and the included
    /// file as a substitution ${a.b.c}, the included substitution now needs to
    /// be ${foo.bar.a.b.c} because we resolve substitutions globally only after
    /// parsing everything.
    ///
    /// @param prefix
    /// @return value relativized to the given path or the same value if nothing
    ///         to do
    virtual AbstractConfigValuePtr relativized(const PathPtr& prefix);

    virtual ConfigValuePtr toFallbackValue() override;

protected:
    virtual AbstractConfigValuePtr newCopy(const ConfigOriginPtr& origin) = 0;

public:
    /// This is virtualized rather than a field because only some subclasses
    /// really need to store the boolean, and they may be able to pack it
    /// with another boolean to save space.
    virtual bool ignoresFallbacks();

protected:
    virtual AbstractConfigValuePtr withFallbacksIgnored();

    /// The withFallback() implementation is supposed to avoid calling
    /// mergedWith* if we're ignoring fallbacks.
    virtual void requireNotIgnoringFallbacks();

    virtual AbstractConfigValuePtr constructDelayedMerge(const ConfigOriginPtr& origin,
                                                         const VectorAbstractConfigValue& stack);
    virtual AbstractConfigValuePtr mergedWithTheUnmergeable(const VectorAbstractConfigValue& stack,
                                                            const UnmergeablePtr& fallback);
    virtual AbstractConfigValuePtr delayMerge(const VectorAbstractConfigValue& stack,
                                              const AbstractConfigValuePtr& fallback);
    virtual AbstractConfigValuePtr mergedWithObject(const VectorAbstractConfigValue& stack,
                                                    const AbstractConfigObjectPtr& fallback);
    virtual AbstractConfigValuePtr mergedWithNonObject(const VectorAbstractConfigValue& stack,
                                                       const AbstractConfigValuePtr& fallback);
    virtual AbstractConfigValuePtr mergedWithTheUnmergeable(const UnmergeablePtr& fallback);
    virtual AbstractConfigValuePtr mergedWithObject(const AbstractConfigObjectPtr& fallback);
    virtual AbstractConfigValuePtr mergedWithNonObject(const AbstractConfigValuePtr& fallback);

public:
    virtual AbstractConfigValuePtr withOrigin(const ConfigOriginPtr& origin);

    /// This is only overridden to change the return type
    virtual ConfigMergeablePtr withFallback(const ConfigMergeablePtr& mergeable) override;

protected:
    virtual bool canEqual(const ConfigVariant& other);

public:
    virtual bool equals(const ConfigVariant& other) override;
    virtual uint32_t hashCode() override;
    virtual std::string toString() override;

    static void indent(std::string& s,
                       uint32_t indent,
                       const ConfigRenderOptionsPtr& options);

    virtual void render(std::string& s,
                        uint32_t indent,
                        const boost::optional<std::string>& atKey,
                        const ConfigRenderOptionsPtr& options);
    virtual void render(std::string& s,
                        uint32_t indent,
                        const ConfigRenderOptionsPtr& options);
    virtual std::string render() override;
    virtual std::string render(const ConfigRenderOptionsPtr& options) override;

    /// toString() is a debugging-oriented string but this is defined
    /// to create a string that would parse back to the value in JSON.
    /// It only works for primitive values (that would be a single token)
    /// which are auto-converted to strings when concatenating with
    /// other strings or by the DefaultTransformer.
    virtual std::string transformToString();

private:
    SimpleConfigOriginPtr origin_;
};

}

#endif // ABSTRACT_CONFIG_VALUE_H_
