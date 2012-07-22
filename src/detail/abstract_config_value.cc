/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
/////////////////////////////////////////////////////////////////////////////

#include "configcpp/detail/abstract_config_value.h"
#include "configcpp/detail/abstract_config_object.h"
#include "configcpp/detail/simple_config_origin.h"
#include "configcpp/detail/resolve_context.h"
#include "configcpp/detail/resolve_status.h"
#include "configcpp/detail/config_delayed_merge.h"
#include "configcpp/detail/mergeable_value.h"
#include "configcpp/detail/unmergeable.h"
#include "configcpp/detail/config_impl_util.h"
#include "configcpp/config_render_options.h"
#include "configcpp/config_origin.h"

namespace config {

NotPossibleToResolve::NotPossibleToResolve(const ResolveContextPtr& context) :
    ConfigException("was not possible to resolve"),
    traceString_(context->traceString()) {
}

std::string NotPossibleToResolve::traceString() {
    return traceString_;
}

AbstractConfigValuePtr NoExceptionsModifier::modifyChildMayThrow(const std::string& keyOrNull, const AbstractConfigValuePtr& v) {
    try {
        return modifyChild(keyOrNull, v);
    }
    catch (ConfigException& e) {
        throw;
    }
    catch(std::exception& e) {
        throw ConfigExceptionBugOrBroken("Something Bad happened here:" + std::string(e.what()));
    }
}

AbstractConfigValue::AbstractConfigValue(const ConfigOriginPtr& origin) :
    origin_(std::dynamic_pointer_cast<SimpleConfigOrigin>(origin)) {
}

ConfigOriginPtr AbstractConfigValue::origin() {
    return this->origin_;
}

AbstractConfigValuePtr AbstractConfigValue::resolveSubstitutions(const ResolveContextPtr& context) {
    return shared_from_this();
}

ResolveStatus AbstractConfigValue::resolveStatus() {
    return ResolveStatus::RESOLVED;
}

AbstractConfigValuePtr AbstractConfigValue::relativized(const PathPtr& prefix) {
    return shared_from_this();
}

ConfigValuePtr AbstractConfigValue::toFallbackValue() {
    return shared_from_this();
}

bool AbstractConfigValue::ignoresFallbacks() {
    // if we are not resolved, then somewhere in this value there's
    // a substitution that may need to look at the fallbacks.
    return resolveStatus() == ResolveStatus::RESOLVED;
}

AbstractConfigValuePtr AbstractConfigValue::withFallbacksIgnored() {
    if (ignoresFallbacks()) {
        return shared_from_this();
    }
    else {
        throw ConfigExceptionBugOrBroken("Value class doesn't implement forced fallback-ignoring " + toString());
    }
}

void AbstractConfigValue::requireNotIgnoringFallbacks() {
    if (ignoresFallbacks()) {
        throw ConfigExceptionBugOrBroken("Method should not have been called with ignoresFallbacks=true " + getClassName());
    }
}

AbstractConfigValuePtr AbstractConfigValue::constructDelayedMerge(const ConfigOriginPtr& origin, const VectorAbstractConfigValue& stack) {
    return ConfigDelayedMerge::make_instance(origin, stack);
}

AbstractConfigValuePtr AbstractConfigValue::mergedWithTheUnmergeable(const VectorAbstractConfigValue& stack, const UnmergeablePtr& fallback) {
    requireNotIgnoringFallbacks();

    // if we turn out to be an object, and the fallback also does,
    // then a merge may be required; delay until we resolve.
    VectorAbstractConfigValue newStack(stack);
    VectorAbstractConfigValue unmerged(fallback->unmergedValues());
    newStack.insert(newStack.end(), unmerged.begin(), unmerged.end());
    return constructDelayedMerge(AbstractConfigObject::mergeOrigins(newStack), newStack);
}

AbstractConfigValuePtr AbstractConfigValue::delayMerge(const VectorAbstractConfigValue& stack, const AbstractConfigValuePtr& fallback) {
    // if we turn out to be an object, and the fallback also does,
    // then a merge may be required.
    // if we contain a substitution, resolving it may need to look
    // back to the fallback.
    VectorAbstractConfigValue newStack(stack);
    newStack.push_back(fallback);
    return constructDelayedMerge(AbstractConfigObject::mergeOrigins(newStack), newStack);
}

AbstractConfigValuePtr AbstractConfigValue::mergedWithObject(const VectorAbstractConfigValue& stack, const AbstractConfigObjectPtr& fallback) {
    requireNotIgnoringFallbacks();

    if (instanceof<AbstractConfigObject>(shared_from_this())) {
        throw ConfigExceptionBugOrBroken("Objects must reimplement mergedWithObject");
    }

    return mergedWithNonObject(stack, fallback);
}

AbstractConfigValuePtr AbstractConfigValue::mergedWithNonObject(const VectorAbstractConfigValue& stack, const AbstractConfigValuePtr& fallback) {
    requireNotIgnoringFallbacks();

    if (resolveStatus() == ResolveStatus::RESOLVED) {
        // falling back to a non-object doesn't merge anything, and also
        // prohibits merging any objects that we fall back to later.
        // so we have to switch to ignoresFallbacks mode.
        return withFallbacksIgnored();
    }
    else {
        // if unresolved, we may have to look back to fallbacks as part of
        // the resolution process, so always delay
        return delayMerge(stack, fallback);
    }
}

AbstractConfigValuePtr AbstractConfigValue::mergedWithTheUnmergeable(const UnmergeablePtr& fallback) {
    requireNotIgnoringFallbacks();

    return mergedWithTheUnmergeable({shared_from_this()}, fallback);
}

AbstractConfigValuePtr AbstractConfigValue::mergedWithObject(const AbstractConfigObjectPtr& fallback) {
    requireNotIgnoringFallbacks();

    return mergedWithObject({shared_from_this()}, fallback);
}

AbstractConfigValuePtr AbstractConfigValue::mergedWithNonObject(const AbstractConfigValuePtr& fallback) {
    requireNotIgnoringFallbacks();

    return mergedWithNonObject({shared_from_this()}, fallback);
}

AbstractConfigValuePtr AbstractConfigValue::withOrigin(const ConfigOriginPtr& origin) {
    if (this->origin_ == origin) {
        return shared_from_this();
    }
    else {
        return newCopy(origin);
    }
}

ConfigMergeablePtr AbstractConfigValue::withFallback(const ConfigMergeablePtr& mergeable) {
    if (ignoresFallbacks()) {
        return std::static_pointer_cast<ConfigMergeable>(shared_from_this());
    }
    else {
        auto other = std::dynamic_pointer_cast<MergeableValue>(mergeable)->toFallbackValue();

        if (instanceof<Unmergeable>(other)) {
            return std::static_pointer_cast<ConfigMergeable>(mergedWithTheUnmergeable(std::dynamic_pointer_cast<Unmergeable>(other)));
        }
        else if (instanceof<AbstractConfigObject>(other)) {
            return std::static_pointer_cast<ConfigMergeable>(mergedWithObject(std::dynamic_pointer_cast<AbstractConfigObject>(other)));
        }
        else {
            return std::static_pointer_cast<ConfigMergeable>(mergedWithNonObject(std::dynamic_pointer_cast<AbstractConfigValue>(other)));
        }
    }
}

bool AbstractConfigValue::canEqual(const ConfigVariant& other) {
    return instanceof<ConfigValue>(other);
}

bool AbstractConfigValue::equals(const ConfigVariant& other) {
    // note that "origin" is deliberately NOT part of equality
    if (instanceof<ConfigValue>(other)) {
        return canEqual(other) &&
                (this->valueType() == dynamic_get<ConfigValue>(other)->valueType()) &&
                ConfigImplUtil::equalsHandlingNull(this->unwrapped(), dynamic_get<ConfigValue>(other)->unwrapped());
    }
    else {
        return false;
    }
}

uint32_t AbstractConfigValue::hashCode() {
    // note that "origin" is deliberately NOT part of equality
    return std::hash<ConfigVariant>()(this->unwrapped());
}

std::string AbstractConfigValue::toString() {
    std::string s;
    render(s, 0, nullptr, ConfigRenderOptions::concise());
    return getClassName() + "(" + s + ")";
}

void AbstractConfigValue::indent(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) {
    if (options->getFormatted()) {
        uint32_t remaining = indent;
        while (remaining > 0) {
            s += "    ";
            --remaining;
        }
    }
}

void AbstractConfigValue::render(std::string& s, uint32_t indent, const boost::optional<std::string>& atKey, const ConfigRenderOptionsPtr& options) {
    if (atKey) {
        s += ConfigImplUtil::renderJsonString(*atKey);
        if (options->getFormatted()) {
            s += " : ";
        }
        else {
            s += ":";
        }
    }
    render(s, indent, options);
}

void AbstractConfigValue::render(std::string& s, uint32_t indent, const ConfigRenderOptionsPtr& options) {
    ConfigVariant u = unwrapped();
    s += boost::apply_visitor(VariantString(), u);
}

std::string AbstractConfigValue::render() {
    return render(ConfigRenderOptions::defaults());
}

std::string AbstractConfigValue::render(const ConfigRenderOptionsPtr& options) {
    std::string s;
    render(s, 0, nullptr, options);
    return s;
}

std::string AbstractConfigValue::transformToString() {
    return "";
}

}
